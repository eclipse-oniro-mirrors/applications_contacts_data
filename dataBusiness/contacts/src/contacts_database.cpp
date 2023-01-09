/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "contacts_database.h"

#include <cmath>
#include <cstdio>
#include <mutex>
#include <unistd.h>

#include "account_manager.h"
#include "async_task.h"
#include "common.h"
#include "contacts.h"
#include "contacts_account.h"
#include "contacts_columns.h"
#include "contacts_json_utils.h"
#include "contacts_search.h"
#include "contacts_type.h"
#include "contacts_update_helper.h"
#include "database_disaster_recovery.h"
#include "hilog_wrapper.h"
#include "match_candidate.h"
#include "merger_contacts.h"
#include "predicates_convert.h"
#include "raw_contacts.h"

namespace OHOS {
namespace Contacts {
std::shared_ptr<ContactsDataBase> ContactsDataBase::contactDataBase_ = nullptr;
std::shared_ptr<CallLogDataBase> ContactsDataBase::callLogDataBase_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> ContactsDataBase::store_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> ContactsDataBase::contactStore_ = nullptr;
static AsyncTaskQueue *g_asyncTaskQueue;
static std::string g_databaseName;
namespace {
std::mutex g_mtx;
}

ContactsDataBase::ContactsDataBase()
{
    g_databaseName = ContactsPath::RDB_PATH + "contacts.db";
    HILOG_INFO("ContactsDataBase g_databaseName :%{public}s", g_databaseName.c_str());
    int errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(g_databaseName);
    SqliteOpenHelperContactCallback sqliteOpenHelperCallback;
    contactStore_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, DATABASE_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    if (errCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase :%{public}d", errCode);
        if (REBASE_SETTING == 0) {
            std::shared_ptr<OHOS::Contacts::DataBaseDisasterRecovery> instance =
                OHOS::Contacts::DataBaseDisasterRecovery::GetInstance();
            instance->RecoveryDatabase(CONTACT_DATABASE_NAME);
            contactStore_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(
                config, DATABASE_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
        }
    }
    if (errCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase open error :%{public}d", errCode);
        return;
    }
    store_ = contactStore_;
    std::shared_ptr<ContactsAccount> contactsAccount = ContactsAccount::GetInstance();
    contactsAccount->PrepopulateCommonAccountTypes(store_);
    ContactsType contactsType;
    contactsType.PrepopulateCommonTypes(store_);
    callLogDataBase_ = CallLogDataBase::GetInstance();
    g_asyncTaskQueue = AsyncTaskQueue::Instance();
}

ContactsDataBase::ContactsDataBase(const ContactsDataBase &)
{
}

std::shared_ptr<ContactsDataBase> ContactsDataBase::GetInstance()
{
    if (contactDataBase_ == nullptr) {
        contactDataBase_.reset(new ContactsDataBase());
    }
    return contactDataBase_;
}

int ContactsDataBase::BeginTransaction()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase BeginTransaction store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase BeginTransaction failed :%{public}d", ret);
    }
    return ret;
}

int ContactsDataBase::Commit()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase Commit store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase Commit failed :%{public}d", ret);
    }
    return ret;
}

int ContactsDataBase::RollBack()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase RollBack store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->RollBack();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase RollBack failed :%{public}d", ret);
    }
    return ret;
}

/**
 * @brief Insert contact data into the raw_contact table
 *
 * @param table Raw_contact table
 * @param rawContactValues Contact to be inserted
 *
 * @return The result returned by the insert
 */
int64_t ContactsDataBase::InsertRawContact(std::string table, OHOS::NativeRdb::ValuesBucket rawContactValues)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase InsertRawContact store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    // Get default account
    AccountManager accountManager;
    int accountId = accountManager.GetAccount();
    rawContactValues.PutInt(RawContactColumns::ACCOUNT_ID, accountId);
    RawContacts rawContacts;
    int64_t outRawContactId = 0;
    int rowRet = rawContacts.InsertRawContact(store_, outRawContactId, rawContactValues);
    if (rowRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertRawContact insertRawContact failed:%{public}d", rowRet);
        return RDB_EXECUTE_FAIL;
    }
    Contacts contactsContact;
    int64_t contactId = 0;
    int rowContactRet = contactsContact.InsertContact(store_, outRawContactId, rawContactValues, contactId);
    if (rowContactRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertRawContact insertContact failed:%{public}d", rowContactRet);
        return RDB_EXECUTE_FAIL;
    }
    // update contactId to rawContacts
    OHOS::NativeRdb::ValuesBucket upRawContactValues;
    std::string contactIdKey = RawContactColumns::CONTACT_ID;
    upRawContactValues.PutInt(contactIdKey, contactId);
    std::string upWhereClause;
    upWhereClause.append(ContactPublicColumns::ID).append(" = ?");
    std::vector<std::string> upWhereArgs;
    upWhereArgs.push_back(std::to_string(outRawContactId));
    int ret = rawContacts.UpdateRawContact(store_, upRawContactValues, upWhereClause, upWhereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("insertRawContact Update contactId to rawContacts failed:%{public}d", rowContactRet);
        return RDB_EXECUTE_FAIL;
    }
    // Search insterted contact data
    ContactsSearch contactsSearch;
    int64_t searchContactId = 0;
    int rowSearchContactRet =
        contactsSearch.Insert(store_, contactId, outRawContactId, rawContactValues, searchContactId);
    if (rowSearchContactRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertRawContact insertSearchContact failed:%{public}d", rowSearchContactRet);
        return RDB_EXECUTE_FAIL;
    }
    return outRawContactId;
}

void ContactsDataBase::GetContactByValue(int &contactValue, OHOS::NativeRdb::ValueObject &value)
{
    if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_NULL) {
        HILOG_ERROR("GetContactByValue value is nullptr");
        contactValue = 0;
        return;
    }
    if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_INT) {
        value.GetInt(contactValue);
        return;
    }
    if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_DOUBLE) {
        double temp = 0;
        value.GetDouble(temp);
        contactValue = ceil(temp);
        return;
    }
    if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_STRING) {
        std::string tempString;
        value.GetString(tempString);
        contactValue = std::stoi(tempString);
        return;
    }
    contactValue = 0;
}

/**
 * @brief Insert data into table contact_data
 *
 * @param table Insert tableName
 * @param contactDataValues Parameters to be passed for insert operation
 *
 * @return The result returned by the insert operation
 */
int64_t ContactsDataBase::InsertContactData(std::string table, OHOS::NativeRdb::ValuesBucket contactDataValues)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase InsertContactData store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int rawContactId = OHOS::NativeRdb::E_OK;
    if (!contactDataValues.HasColumn(ContactDataColumns::RAW_CONTACT_ID)) {
        HILOG_ERROR("InsertContactData raw_contact_id is required");
        return RDB_EXECUTE_FAIL;
    }
    OHOS::NativeRdb::ValueObject value;
    contactDataValues.GetObject(ContactDataColumns::RAW_CONTACT_ID, value);
    GetContactByValue(rawContactId, value);
    if (rawContactId <= 0) {
        HILOG_ERROR("InsertContactData raw_contact_id is required %{public}d", rawContactId);
        return RDB_EXECUTE_FAIL;
    }
    int typeId = RDB_EXECUTE_FAIL;
    std::string typeText;
    int retCode = GetTypeText(contactDataValues, typeId, rawContactId, typeText);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertContactData getTypeText failed:%{public}d", retCode);
        return retCode;
    }
    if (typeId <= 0) {
        HILOG_ERROR("InsertContactData typeId is required %{public}d", typeId);
        return RDB_EXECUTE_FAIL;
    }
    // delete content_type
    contactDataValues.Delete(ContentTypeColumns::CONTENT_TYPE);
    contactDataValues.PutInt(ContactDataColumns::TYPE_ID, typeId);
    int64_t outDataRowId;
    int ret = store_->Insert(outDataRowId, table, contactDataValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertContactData failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    std::vector<int> rawContactIdVector;
    rawContactIdVector.push_back(rawContactId);
    std::vector<std::string> typeTextVector;
    typeTextVector.push_back(typeText);
    ContactsUpdateHelper contactsUpdateHelper;
    int updateDisplayRet =
        contactsUpdateHelper.UpdateDisplay(rawContactIdVector, typeTextVector, store_, contactDataValues, false);
    if (updateDisplayRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertContactData UpdateDisplay failed:%{public}d", updateDisplayRet);
        return RDB_EXECUTE_FAIL;
    }
    MergeUpdateTask(store_, rawContactIdVector, false);
    return outDataRowId;
}

int ContactsDataBase::GetTypeText(
    OHOS::NativeRdb::ValuesBucket &contactDataValues, int &typeId, int &rawContactId, std::string &typeText)
{
    // if content_type is added , get type_id by content_type
    if (contactDataValues.HasColumn(ContentTypeColumns::CONTENT_TYPE)) {
        OHOS::NativeRdb::ValueObject typeValue;
        contactDataValues.GetObject(ContentTypeColumns::CONTENT_TYPE, typeValue);
        typeValue.GetString(typeText);
        if (typeText.empty()) {
            HILOG_ERROR("GetTypeText type is required");
            return PARAMETER_EMPTY;
        }
        // get type id
        ContactsType contactsType;
        typeId = contactsType.LookupTypeId(store_, typeText);
        if (typeId == RDB_EXECUTE_FAIL) {
            // type not found. insert the default type
            typeId = contactsType.Insert(store_, typeText, RDB_OBJECT_EMPTY);
        }
        if (typeId == RDB_EXECUTE_FAIL) {
            return RDB_EXECUTE_FAIL;
        }
        return RDB_EXECUTE_OK;
    } else if (contactDataValues.HasColumn(ContactDataColumns::TYPE_ID)) {
        OHOS::NativeRdb::ValueObject typeValue;
        contactDataValues.GetObject(ContactDataColumns::TYPE_ID, typeValue);
        GetContactByValue(typeId, typeValue);
        ContactsType contactsType;
        typeText = contactsType.GetTypeText(store_, typeId);
        return RDB_EXECUTE_OK;
    }
    return RDB_EXECUTE_FAIL;
}

/**
 * @brief Insert data into table groups
 *
 * @param table Insert tableName
 * @param initialValues Parameters to be passed for insert operation
 *
 * @return The result returned by the insert operation
 */
int64_t ContactsDataBase::InsertGroup(std::string table, OHOS::NativeRdb::ValuesBucket initialValues)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase InsertGroup store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    AccountManager accountManager;
    // get default account
    int accountId = accountManager.GetAccount();
    initialValues.PutInt(GroupsColumns::ACCOUNT_ID, accountId);
    int64_t outGroupRowId = OHOS::NativeRdb::E_OK;
    int ret = store_->Insert(outGroupRowId, table, initialValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertGroup failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return outGroupRowId;
}

/**
 * @brief Insert data into the contact_blocklist table
 *
 * @param table Insert tableName
 * @param initialValues Parameters to be passed for insert operation
 *
 * @return The result returned by the insert operation
 */
int64_t ContactsDataBase::InsertBlockList(std::string table, OHOS::NativeRdb::ValuesBucket initialValues)
{
    int64_t outRowId = OHOS::NativeRdb::E_OK;
    int ret = store_->Insert(outRowId, table, initialValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    return outRowId;
}

/**
 * @brief Update data into contact_data table
 *
 * @param contactDataValues Parameters to be passed for update operation
 * @param rdbPredicates Conditions for update operation
 *
 * @return The result returned by the update operation
 */
int ContactsDataBase::UpdateContactData(
    OHOS::NativeRdb::ValuesBucket contactDataValues, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateContactData store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    std::vector<std::string> types;
    std::vector<int> rawContactIdVector = QueryContactDataRawContactId(rdbPredicates, types);
    int changedRows = OHOS::NativeRdb::E_OK;
    ret = store_->Update(changedRows, contactDataValues, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        RollBack();
        HILOG_ERROR("UpdateContactData failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    ContactsUpdateHelper contactsUpdateHelper;
    ret = contactsUpdateHelper.UpdateDisplay(rawContactIdVector, types, store_, contactDataValues, false);
    if (ret != OHOS::NativeRdb::E_OK) {
        RollBack();
        HILOG_ERROR("UpdateContactData UpdateDisplay failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    ret = Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    MergeUpdateTask(store_, rawContactIdVector, false);
    return ret;
}

/**
 * @brief Update data in the raw_contact table
 *
 * @param values Parameters to be passed for update operation
 * @param rdbPredicates Conditions for update operation
 *
 * @return The result returned by the update operation
 */
int ContactsDataBase::UpdateRawContact(
    OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateRawContact store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    std::vector<std::string> columns;
    columns.push_back(ContactPublicColumns::ID);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> rawContactResultSet = store_->Query(rdbPredicates, columns);
    int rawContactResultSetNum = rawContactResultSet->GoToFirstRow();
    std::vector<int> rawContactIdVector;
    while (rawContactResultSetNum == OHOS::NativeRdb::E_OK) {
        std::string columnName = ContactPublicColumns::ID;
        int columnIndex = 0;
        int rawContactId = 0;
        rawContactResultSet->GetColumnIndex(columnName, columnIndex);
        rawContactResultSet->GetInt(columnIndex, rawContactId);
        rawContactIdVector.push_back(rawContactId);
        rawContactResultSetNum = rawContactResultSet->GoToNextRow();
    }
    rawContactResultSet->Close();
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = store_->Update(changedRows, values, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateRawContact failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    // add Restore contact judgment
    int isDelete = RDB_EXECUTE_FAIL;
    if (values.HasColumn(RawContactColumns::IS_DELETED)) {
        OHOS::NativeRdb::ValueObject value;
        values.GetObject(RawContactColumns::IS_DELETED, value);
        GetContactByValue(isDelete, value);
        if (isDelete == 0 && rawContactIdVector.size() > 0) {
            ContactsUpdateHelper contactsUpdateHelper;
            contactsUpdateHelper.UpdateCallLogByPhoneNum(rawContactIdVector, store_, false);
        }
    }
    return ret;
}

/**
 * @brief Update data in the contact_blocklist table
 *
 * @param values Parameters to be passed for update operation
 * @param rdbPredicates Conditions for update operation
 *
 * @return The result returned by the update operation
 */
int ContactsDataBase::UpdateBlockList(
    OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateBlockList store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = store_->Update(changedRows, values, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateBlockList failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("UpdateBlockList row:%{public}d", changedRows);
    return ret;
}

/**
 * @brief Update data in table groups
 *
 * @param values Parameters to be passed for update operation
 * @param rdbPredicates Conditions for update operation
 *
 * @return The result returned by the update operation
 */
int ContactsDataBase::UpdateGroup(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateGroup store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = store_->Update(changedRows, values, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateGroup failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("UpdateGroup row:%{public}d", changedRows);
    return ret;
}

/**
 * @brief Delete data from contact_blocklist table
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::DeleteBlockList(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteBlockList store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = store_->Delete(changedRows, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteBlockList failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("DeleteBlockList row:%{public}d", changedRows);
    return ret;
}

/**
 * @brief Delete data from table groups
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::DeleteGroup(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteGroup store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int deletedRows = OHOS::NativeRdb::E_OK;
    int ret = store_->Delete(deletedRows, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteGroup failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("DeleteGroup row:%{public}d", deletedRows);
    return ret;
}

int ContactsDataBase::DeleteRecord(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteRecord store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int deletedRows = OHOS::NativeRdb::E_OK;
    int ret = store_->Delete(deletedRows, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteRecord raw_contact_deleted failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("DeleteRecord raw_contact_deleted row:%{public}d", deletedRows);
    return ret;
}

/**
 * @brief Delete data from contact_data table
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::DeleteContactData(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteContactData store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    std::vector<std::string> types;
    std::vector<int> rawContactIdVector = QueryContactDataRawContactId(rdbPredicates, types);
    int deletedRows = OHOS::NativeRdb::E_OK;
    store_->Delete(deletedRows, rdbPredicates);
    ContactsUpdateHelper contactsUpdateHelper;
    OHOS::NativeRdb::ValuesBucket contactDataValues;
    int updateDisplayRet =
        contactsUpdateHelper.UpdateDisplay(rawContactIdVector, types, store_, contactDataValues, true);
    if (updateDisplayRet != OHOS::NativeRdb::E_OK) {
        RollBack();
        HILOG_ERROR("deleteContactData UpdateDisplay failed:%{public}d", updateDisplayRet);
        return RDB_EXECUTE_FAIL;
    }
    ret = Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    MergeUpdateTask(store_, rawContactIdVector, true);
    return ret;
}

/**
 * @brief Delete data from the contact table
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::DeleteContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteContact store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    std::vector<OHOS::NativeRdb::ValuesBucket> queryValuesBucket = DeleteContactQuery(rdbPredicates);
    int deleteRet = DeleteExecute(queryValuesBucket);
    if (deleteRet != OHOS::NativeRdb::E_OK) {
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    deleteRet = Commit();
    if (deleteRet != OHOS::NativeRdb::E_OK) {
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    DeletedAsyncTask(store_, queryValuesBucket);
    return deleteRet;
}

void ContactsDataBase::DeletedAsyncTask(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket)
{
    std::unique_ptr<AsyncItem> task = std::make_unique<AsyncDeleteContactsTask>(store_, queryValuesBucket);
    g_asyncTaskQueue->Push(task);
    g_asyncTaskQueue->Start();
}

int ContactsDataBase::DeleteExecute(std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket)
{
    unsigned int size = queryValuesBucket.size();
    if (size == 0) {
        return RDB_EXECUTE_FAIL;
    }
    int ret = RDB_EXECUTE_FAIL;
    for (unsigned int i = 0; i < size; i++) {
        OHOS::NativeRdb::ValuesBucket valuesElement = queryValuesBucket[i];
        bool hasId = valuesElement.HasColumn(ContactColumns::ID);
        if (!hasId) {
            continue;
        }
        OHOS::NativeRdb::ValueObject idValue;
        valuesElement.GetObject(ContactPublicColumns::ID, idValue);
        int rawContactId = 0;
        idValue.GetInt(rawContactId);
        OHOS::NativeRdb::ValuesBucket values;
        values.PutInt(RawContactColumns::IS_DELETED, DELETE_MARK);
        int updateRow = OHOS::NativeRdb::E_OK;
        std::string upWhere = "";
        upWhere.append(ContactPublicColumns::ID);
        upWhere.append(" = ? ");
        std::vector<std::string> upWhereArgs;
        upWhereArgs.push_back(std::to_string(rawContactId));
        ret = store_->Update(updateRow, ContactTableName::RAW_CONTACT, values, upWhere, upWhereArgs);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("deleteRawContact upResultDelete failed:%{public}d", ret);
            return RDB_EXECUTE_FAIL;
        }
    }
    return ret;
}

void ContactsDataBase::DeleteRecordInsert(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket)
{
    g_mtx.lock();
    unsigned int size = queryValuesBucket.size();
    for (unsigned int i = 0; i < size; i++) {
        OHOS::NativeRdb::ValuesBucket valuesElement = queryValuesBucket[i];
        bool hasId = valuesElement.HasColumn(ContactColumns::ID);
        if (!hasId) {
            continue;
        }
        OHOS::NativeRdb::ValueObject idValue;
        valuesElement.GetObject(ContactPublicColumns::ID, idValue);
        int rawContactId = 0;
        idValue.GetInt(rawContactId);
        OHOS::NativeRdb::ValueObject contactIdValue;
        valuesElement.GetObject(RawContactColumns::CONTACT_ID, contactIdValue);
        int contactId = 0;
        contactIdValue.GetInt(contactId);
        OHOS::NativeRdb::ValueObject nameValue;
        valuesElement.GetObject(RawContactColumns::DISPLAY_NAME, nameValue);
        std::string disPlayName;
        nameValue.GetString(disPlayName);
        std::string backupData =
            StructureDeleteContactJson(queryValuesBucket[i], ContactPublicColumns::ID, rawContactId);
        if (backupData.empty()) {
            HILOG_ERROR("deleteRawContact json :%{public}s", backupData.c_str());
        }
        int deleteRet = DeleteRawContactLocal(contactId, rawContactId, backupData, disPlayName);
        if (deleteRet != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("deleteRawContact upResultDelete failed:%{public}d", deleteRet);
        }
        std::vector<int> rawContactIdVector;
        rawContactIdVector.push_back(rawContactId);
        ContactsUpdateHelper contactsUpdateHelper;
        contactsUpdateHelper.UpdateCallLogByPhoneNum(rawContactIdVector, store, true);
    }
    g_mtx.unlock();
}

/**
 * @brief Delete data from the raw_contact table
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::DeleteRawContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteRawContact store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    std::vector<OHOS::NativeRdb::ValuesBucket> queryValuesBucket = DeleteRawContactQuery(rdbPredicates);
    int deleteRet = DeleteExecute(queryValuesBucket);
    if (deleteRet != OHOS::NativeRdb::E_OK) {
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    deleteRet = Commit();
    if (deleteRet != OHOS::NativeRdb::E_OK) {
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    DeletedAsyncTask(store_, queryValuesBucket);
    return deleteRet;
}

std::vector<OHOS::NativeRdb::ValuesBucket> ContactsDataBase::DeleteContactQuery(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates newRdbPredicates =
        predicatesConvert.CopyPredicates(ViewName::VIEW_CONTACT, rdbPredicates);
    std::vector<std::string> columns;
    columns.push_back(ContactPublicColumns::ID);
    columns.push_back(RawContactColumns::DISPLAY_NAME);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store_->Query(newRdbPredicates, columns);
    int resultSetNum = resultSet->GoToFirstRow();
    if (resultSetNum != OHOS::NativeRdb::E_OK) {
        // query size 0
        std::vector<OHOS::NativeRdb::ValuesBucket> vectorQueryData;
        resultSet->GoToNextRow();
        resultSet->Close();
        return vectorQueryData;
    }
    std::vector<std::string> whereArgs;
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int contactId;
        resultSet->GetInt(0, contactId);
        whereArgs.push_back(std::to_string(contactId));
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    unsigned int size = whereArgs.size();
    OHOS::NativeRdb::RdbPredicates rawContactQueryRdbPredicates(ViewName::VIEW_RAW_CONTACT);
    std::string whereClause;
    for (unsigned int i = 0; i < size; i++) {
        whereClause.append(" contact_id = ? ");
        if (i != size - 1) {
            whereClause.append(" OR ");
        }
    }
    OHOS::NativeRdb::PredicatesUtils::SetWhereClauseAndArgs(&rawContactQueryRdbPredicates, whereClause, whereArgs);
    OHOS::NativeRdb::PredicatesUtils::SetAttributes(&rawContactQueryRdbPredicates,
        rawContactQueryRdbPredicates.IsDistinct(), rawContactQueryRdbPredicates.GetIndex(),
        rawContactQueryRdbPredicates.GetGroup(), rawContactQueryRdbPredicates.GetOrder(),
        rawContactQueryRdbPredicates.GetLimit(), rawContactQueryRdbPredicates.GetOffset());
    return DeleteRawContactQuery(rawContactQueryRdbPredicates);
}

std::vector<OHOS::NativeRdb::ValuesBucket> ContactsDataBase::DeleteRawContactQuery(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    std::vector<std::string> columns;
    columns.push_back(RawContactColumns::DISPLAY_NAME);
    columns.push_back(ContactColumns::ID);
    columns.push_back(RawContactColumns::CONTACT_ID);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store_->Query(rdbPredicates, columns);
    return ResultSetToValuesBucket(resultSet);
}

int ContactsDataBase::DeleteRawContactLocal(
    int contactId, int rawContactId, std::string backupData, std::string disPlayName)
{
    OHOS::NativeRdb::ValuesBucket deleteRawContact;
    deleteRawContact.PutInt(DeleteRawContactColumns::RAW_CONTACT_ID, rawContactId);
    deleteRawContact.PutString(DeleteRawContactColumns::BACKUP_DATA, backupData);
    deleteRawContact.PutString(DeleteRawContactColumns::DISPLAY_NAME, disPlayName);
    deleteRawContact.PutInt(DeleteRawContactColumns::CONTACT_ID, contactId);
    int64_t outRowId = OHOS::NativeRdb::E_OK;
    int ret = store_->Insert(outRowId, ContactTableName::DELETE_RAW_CONTACT, deleteRawContact);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("deleteRawContact deleteInsert failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

/**
 * @brief Completely delete data from the database
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::CompletelyDelete(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    std::vector<std::string> columns;
    columns.push_back(DeleteRawContactColumns::CONTACT_ID);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store_->Query(rdbPredicates, columns);
    std::vector<std::string> contactIds;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int contactId;
        resultSet->GetInt(0, contactId);
        contactIds.push_back(std::to_string(contactId));
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    int retCode = RDB_EXECUTE_FAIL;
    unsigned int size = contactIds.size();
    for (unsigned int index = 0; index < size; index++) {
        std::string queryViewContact =
            "SELECT id FROM view_raw_contact WHERE is_deleted = 1 AND contact_id = " + contactIds[index];
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> contactIdSet = store_->QuerySql(queryViewContact);
        resultSetNum = contactIdSet->GoToFirstRow();
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            int value = -1;
            contactIdSet->GetInt(0, value);
            retCode = DeleteLocal(value, contactIds[index]);
            if (retCode != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("CompletelyDelete DeleteLocal error:%{public}d", retCode);
                break;
            }
            resultSetNum = contactIdSet->GoToNextRow();
        }
        contactIdSet->Close();
        if (retCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("CompletelyDelete error:%{public}d", retCode);
            RollBack();
            return retCode;
        }
    }
    return CompletelyDeleteCommit(retCode);
}

int ContactsDataBase::CompletelyDeleteCommit(int retCode)
{
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CompletelyDelete end error:%{public}d", retCode);
        RollBack();
        return retCode;
    }
    int markRet = Commit();
    if (markRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CompletelyDelete end error:%{public}d", markRet);
        return RDB_EXECUTE_FAIL;
    }
    return retCode;
}

int ContactsDataBase::DeleteLocal(int rawContactId, std::string contactId)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteLocal store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    std::string updateContactSql = "UPDATE contact SET name_raw_contact_id = NULL WHERE id = " + contactId;
    int retCode = store_->ExecuteSql(updateContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal updateContactSql code:%{public}d", retCode);
        return retCode;
    }
    std::string updateRawContactSql = "UPDATE raw_contact SET contact_id = NULL WHERE contact_id = " + contactId;
    retCode = store_->ExecuteSql(updateRawContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal updateRawContactSql code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteDeleteRawContactSql = "DELETE FROM deleted_raw_contact WHERE contact_id = " + contactId;
    retCode = store_->ExecuteSql(deleteDeleteRawContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal deleted_raw_contact code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteSearchContactSql = "DELETE FROM search_contact WHERE contact_id =  " + contactId;
    retCode = store_->ExecuteSql(deleteSearchContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal deleteSearchContactSql code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteContactData = "DELETE FROM contact_data WHERE raw_contact_id = " + std::to_string(rawContactId);
    retCode = store_->ExecuteSql(deleteContactData);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal deleteContactData code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteContactSql = "DELETE FROM contact WHERE id = " + contactId;
    retCode = store_->ExecuteSql(deleteContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal deleteContactSql code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteRawContactSql = "DELETE FROM raw_contact WHERE id =  " + std::to_string(rawContactId);
    retCode = store_->ExecuteSql(deleteRawContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal deleteRawContactSql code:%{public}d", retCode);
        return retCode;
    }
    return retCode;
}

std::vector<int> ContactsDataBase::QueryContactDataRawContactId(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &types)
{
    std::vector<std::string> columns;
    columns.push_back(ContactDataColumns::TYPE_ID);
    columns.push_back(ContactDataColumns::RAW_CONTACT_ID);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store_->Query(rdbPredicates, columns);
    std::vector<int> rawContactIdVector;
    std::vector<int> typeIdVector;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        std::string columnName = ContactDataColumns::RAW_CONTACT_ID;
        int columnIndex = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        int rawContactId = 0;
        resultSet->GetInt(columnIndex, rawContactId);
        std::string typeIdKey = ContactDataColumns::TYPE_ID;
        int columnIndexType = 0;
        resultSet->GetColumnIndex(typeIdKey, columnIndexType);
        int typeId = 0;
        resultSet->GetInt(columnIndexType, typeId);
        OHOS::NativeRdb::ValueObject typeTextObject;
        typeIdVector.push_back(typeId);
        rawContactIdVector.push_back(rawContactId);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    unsigned int typeIdSize = typeIdVector.size();
    ContactsType contactsType;
    for (unsigned int i = 0; i < typeIdSize; i++) {
        std::string typeText = contactsType.GetTypeText(store_, typeIdVector[i]);
        types.push_back(typeText);
    }
    return rawContactIdVector;
}

/**
 * @brief Query data according to given conditions
 *
 * @param rdbPredicates Conditions for query operation
 * @param columns Conditions for query operation
 *
 * @return The result returned by the query operation
 */
std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactsDataBase::Query(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &columns)
{
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase Query store_ is nullptr");
        return resultSet;
    }
    int errCode = OHOS::NativeRdb::E_OK;
    resultSet = store_->Query(rdbPredicates, columns);
    if (errCode != OHOS::NativeRdb::E_OK) {
        HILOG_INFO("Query error code is:%{public}d", errCode);
    }
    return resultSet;
}

std::vector<OHOS::NativeRdb::ValuesBucket> ContactsDataBase::ResultSetToValuesBucket(
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet)
{
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    std::vector<OHOS::NativeRdb::ValuesBucket> vectorQueryData;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        OHOS::NativeRdb::ValuesBucket valuesBucketElement;
        unsigned int size = columnNames.size();
        for (unsigned int i = 0; i < size; i++) {
            std::string typeValue = columnNames[i];
            int columnIndex = 0;
            resultSet->GetColumnIndex(typeValue, columnIndex);
            OHOS::NativeRdb::ColumnType columnType;
            resultSet->GetColumnType(columnIndex, columnType);
            if (columnType == OHOS::NativeRdb::ColumnType::TYPE_INTEGER) {
                int intValue = 0;
                resultSet->GetInt(columnIndex, intValue);
                valuesBucketElement.PutInt(typeValue, intValue);
            } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_FLOAT) {
                double doubleValue = 0;
                resultSet->GetDouble(columnIndex, doubleValue);
                valuesBucketElement.PutDouble(typeValue, doubleValue);
            } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_STRING) {
                std::string stringValue;
                resultSet->GetString(columnIndex, stringValue);
                valuesBucketElement.PutString(typeValue, stringValue);
            }
        }
        vectorQueryData.push_back(valuesBucketElement);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return vectorQueryData;
}

std::string ContactsDataBase::StructureDeleteContactJson(
    OHOS::NativeRdb::ValuesBucket rawContactValues, std::string rawContactIdColumn, int rawContactId)
{
    ContactsJsonUtils contactsJsonUtils;
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back(std::to_string(rawContactId));
    std::string queryTabName = ViewName::VIEW_CONTACT_DATA;
    std::vector<std::string> contentColumns;
    contentColumns.push_back(ContentTypeColumns::CONTENT_TYPE);
    contentColumns.push_back(ContactDataColumns::DETAIL_INFO);
    contentColumns.push_back(ContactDataColumns::POSITION);
    contentColumns.push_back(ContactDataColumns::EXTEND1);
    contentColumns.push_back(ContactDataColumns::EXTEND2);
    contentColumns.push_back(ContactDataColumns::EXTEND3);
    contentColumns.push_back(ContactDataColumns::EXTEND4);
    contentColumns.push_back(ContactDataColumns::ALPHA_NAME);
    contentColumns.push_back(ContactDataColumns::OTHER_LAN_LAST_NAME);
    contentColumns.push_back(ContactDataColumns::OTHER_LAN_FIRST_NAME);
    contentColumns.push_back(ContactDataColumns::EXTEND5);
    contentColumns.push_back(ContactDataColumns::LAN_STYLE);
    contentColumns.push_back(ContactDataColumns::CUSTOM_DATA);
    contentColumns.push_back(ContactDataColumns::EXTEND6);
    contentColumns.push_back(ContactDataColumns::EXTEND7);
    contentColumns.push_back(ContactDataColumns::BLOB_DATA);
    std::string queryWhereClause = DeleteRawContactColumns::RAW_CONTACT_ID;
    queryWhereClause.append(" = ? ");
    std::string sql = "SELECT ";
    unsigned int size = contentColumns.size();
    for (unsigned int i = 0; i < size; i++) {
        sql.append(contentColumns[i]);
        if (i != size - 1) {
            sql.append(", ");
        }
    }
    sql.append(" FROM ").append(queryTabName).append(" WHERE ").append(queryWhereClause);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> contactDataResultSet = store_->QuerySql(sql, selectionArgs);
    std::string backupData = contactsJsonUtils.GetDeleteData(contactDataResultSet);
    contactDataResultSet->Close();
    return backupData;
}

int SqliteOpenHelperContactCallback::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    store.ExecuteSql(CREATE_CONTACT);
    store.ExecuteSql(CREATE_CONTACT_INDEX);
    store.ExecuteSql(CREATE_RAW_CONTACT);
    store.ExecuteSql(CREATE_RAW_CONTACT_INDEX);
    store.ExecuteSql(CREATE_CONTACT_DATA);
    store.ExecuteSql(CREATE_CONTACT_INDEX_DATA1);
    store.ExecuteSql(CREATE_CONTACT_INDEX_DATA2);
    store.ExecuteSql(CREATE_CONTACT_BLOCKLIST);
    store.ExecuteSql(CREATE_LOCAL_LANG);
    store.ExecuteSql(CREATE_ACCOUNT);
    store.ExecuteSql(CREATE_PHOTO_FILES);
    store.ExecuteSql(CREATE_CONTACT_TYPE);
    store.ExecuteSql(CREATE_GROUPS);
    store.ExecuteSql(CREATE_DELETED_RAW_CONTACT);
    store.ExecuteSql(CREATE_SEARCH_CONTACT);
    store.ExecuteSql(CREATE_SEARCH_CONTACT_INDEX1);
    store.ExecuteSql(CREATE_SEARCH_CONTACT_INDEX2);
    store.ExecuteSql(CREATE_SEARCH_CONTACT_VIEW);
    store.ExecuteSql(MERGE_INFO);
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    store.ExecuteSql(CREATE_VIEW_RAW_CONTACT);
    store.ExecuteSql(CREATE_VIEW_CONTACT);
    store.ExecuteSql(CREATE_VIEW_GROUPS);
    store.ExecuteSql(CREATE_VIEW_DELETED);
    store.ExecuteSql(INSERT_DELETE_RAW_CONTACT);
    store.ExecuteSql(UPDATE_RAW_CONTACT_VERSION);
    store.ExecuteSql(UPDATE_CONTACT_DATA_VERSION);
    store.ExecuteSql(INSERT_CONTACT_QUICK_SEARCH);
    store.ExecuteSql(CREATE_DATABASE_BACKUP_TASK);
    store.ExecuteSql(CREATE_INSERT_BACKUP_TIME);
    store.ExecuteSql(UPDATE_CONTACT_BY_INSERT_CONTACT_DATA);
    store.ExecuteSql(UPDATE_CONTACT_BY_DELETE_CONTACT_DATA);
    store.ExecuteSql(UPDATE_CONTACT_BY_UPDATE_CONTACT_DATA);
    store.ExecuteSql(MERGE_INFO_INDEX);
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperContactCallback::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("OnUpgrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion < newVersion && newVersion == DATABASE_NEW_VERSION) {
        store.ExecuteSql("ALTER TABLE database_backup_task ADD COLUMN sync TEXT");
    }
    store.SetVersion(newVersion);
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperContactCallback::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("OnDowngrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion > newVersion && newVersion == DATABASE_OPEN_VERSION) {
        store.ExecuteSql(
            "CREATE TABLE IF NOT EXISTS database_backup (id INTEGER PRIMARY KEY AUTOINCREMENT, backup_time "
            "TEXT, backup_path TEXT, remarks TEXT)");
        store.ExecuteSql(
            "INSERT INTO database_backup(id, backup_time, backup_path, remarks) SELECT id, "
            "backup_time, backup_path, remarks FROM database_backup_task");
        store.ExecuteSql("DROP table database_backup_task");
        store.ExecuteSql("ALTER table database_backup RENAME TO database_backup_task");
        store.ExecuteSql(CREATE_INSERT_BACKUP_TIME);
    }
    int ret = store.SetVersion(newVersion);
    return ret;
}

void ContactsDataBase::DestroyInstanceAndRestore(std::string restorePath)
{
    g_mtx.lock();
    if (access(restorePath.c_str(), F_OK) != 0) {
        HILOG_ERROR("Restore file %{private}s does not exist", restorePath.c_str());
        g_mtx.unlock();
        return;
    }
    OHOS::NativeRdb::RdbHelper::DeleteRdbStore(g_databaseName);
    OHOS::NativeRdb::RdbHelper::ClearCache();
    contactDataBase_ = nullptr;
    Restore(restorePath);
    g_mtx.unlock();
}

bool ContactsDataBase::Restore(std::string restorePath)
{
    HILOG_INFO("ContactsDataBase Restore start ");
    if (rename(restorePath.c_str(), g_databaseName.c_str()) == 0) {
        HILOG_INFO("ContactsDataBase Restore rename ok ");
        return true;
    }
    return false;
}

/**
 * @brief Select candidates
 *
 * @return The result returned by the selectCandidate operation
 */
std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactsDataBase::SelectCandidate()
{
    MarkMerge(store_);
    MergerContacts mergerContacts;
    return mergerContacts.SelectCandidate(store_);
}

/**
 * @brief Perform a split operation
 *
 * @param predicates Conditions for split operation
 *
 * @return The result returned by the split operation
 */
int ContactsDataBase::Split(DataShare::DataSharePredicates predicates)
{
    std::vector<std::string> whereArgs = predicates.GetWhereArgs();
    if (whereArgs.size() > 1) {
        HILOG_ERROR("Invalid parameter passed");
        return RDB_EXECUTE_FAIL;
    }
    MatchCandidate matchCandidate;
    int code = RDB_EXECUTE_FAIL;
    for (auto value : whereArgs) {
        code = matchCandidate.Split(store_, atoi(value.c_str()));
    }
    if (code != RDB_EXECUTE_OK) {
        HILOG_INFO("Split code %{public}d", code);
    }
    return code;
}

/**
 * @brief Perform an autoMerge operation
 *
 * @return The result returned by the autoMerge operation
 */
int ContactsDataBase::ContactMerge()
{
    MarkMerge(store_);
    int code = RDB_EXECUTE_FAIL;
    MergerContacts mergerContacts;
    if (store_ != nullptr) {
        code = mergerContacts.ContactMerge(store_);
        if (code != RDB_EXECUTE_OK) {
            HILOG_ERROR("ContactMerge ERROR!");
        }
    }
    return code;
}

/**
 * @brief Perform a manualMerge operation
 *
 * @param predicates Conditions for manualMerge operation
 *
 * @return The result returned by the manualMerge operation
 */
int ContactsDataBase::ReContactMerge(DataShare::DataSharePredicates predicates)
{
    MarkMerge(store_);
    int code = RDB_EXECUTE_FAIL;
    MergerContacts mergerContacts;
    if (store_ != nullptr) {
        code = mergerContacts.ReContactMerge(store_, predicates);
        if (code != RDB_EXECUTE_OK) {
            HILOG_ERROR("ReContactMerge ERROR!");
        }
    }
    return code;
}

void ContactsDataBase::InsertMergeData(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> &rawContactIdVector)
{
    unsigned int size = rawContactIdVector.size();
    for (unsigned int i = 0; i < size; i++) {
        OHOS::NativeRdb::ValuesBucket mergeInfoValues;
        mergeInfoValues.PutInt(MergeInfo::RAW_CONTACT_ID, rawContactIdVector[i]);
        int64_t mergeInfoRowId = 0;
        int mergeInfoRet = store->Insert(mergeInfoRowId, ContactTableName::MERGE_INFO, mergeInfoValues);
        if (mergeInfoRet != RDB_EXECUTE_OK) {
            HILOG_ERROR("mergeInfo insert error : %{public}d ", mergeInfoRet);
        }
    }
}

void ContactsDataBase::MergeUpdateTask(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> &rawContactIdVector, bool isDeleted)
{
    std::unique_ptr<AsyncItem> task = std::make_unique<AsyncTask>(store_, rawContactIdVector, isDeleted);
    g_asyncTaskQueue->Push(task);
    g_asyncTaskQueue->Start();
}

void ContactsDataBase::MarkMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store)
{
    std::string sql = "SELECT ";
    sql.append(MergeInfo::RAW_CONTACT_ID)
        .append(" FROM ")
        .append(ContactTableName::MERGE_INFO)
        .append(" GROUP BY ")
        .append(MergeInfo::RAW_CONTACT_ID);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sql);
    int mergeResultSetNum = resultSet->GoToFirstRow();
    MatchCandidate matchCandidate;
    while (mergeResultSetNum == OHOS::NativeRdb::E_OK) {
        std::string columnName = MergeInfo::RAW_CONTACT_ID;
        int columnIndex = 0;
        int rawContactId = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetInt(columnIndex, rawContactId);
        int error = matchCandidate.FindMatchContact(store, rawContactId);
        if (error != RDB_EXECUTE_OK) {
            HILOG_ERROR("Find error is : %{public}d ", error);
        }
        std::string deleteMergeInfo = "DELETE FROM ";
        deleteMergeInfo.append(ContactTableName::MERGE_INFO)
            .append(" WHERE ")
            .append(MergeInfo::RAW_CONTACT_ID)
            .append(" = ")
            .append(std::to_string(rawContactId));
        int ret = store->ExecuteSql(deleteMergeInfo);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("deleteMergeInfo error");
        }
        mergeResultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
}

int ContactsDataBase::GetTypeId(std::string typeText)
{
    ContactsType contactsType;
    int typeId = contactsType.LookupTypeId(store_, typeText);
    HILOG_INFO("ContactsDataBase GetTypeId %{public}d", typeId);
    return typeId;
}
} // namespace Contacts
} // namespace OHOS