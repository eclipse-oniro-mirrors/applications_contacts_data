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

#include "contacts_data_ability.h"

#include <mutex>

#include "ability_loader.h"
#include "common.h"
#include "contacts_columns.h"
#include "contacts_common_event.h"
#include "contacts_datashare_stub_impl.h"
#include "datashare_ext_ability_context.h"
#include "datashare_predicates.h"
#include "database_disaster_recovery.h"
#include "file_utils.h"
#include "hilog_wrapper.h"
#include "profile_database.h"
#include "rdb_predicates.h"
#include "rdb_utils.h"
#include "sql_analyzer.h"
#include "uri_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
std::mutex g_mutex;
}
std::shared_ptr<Contacts::ContactsDataBase> ContactsDataAbility::contactDataBase_ = nullptr;
std::shared_ptr<Contacts::ProfileDatabase> ContactsDataAbility::profileDataBase_ = nullptr;
std::map<std::string, int> ContactsDataAbility::uriValueMap_ = {
    {"/com.ohos.contactsdataability/contacts/contact", Contacts::CONTACTS_CONTACT},
    {"/com.ohos.contactsdataability/contacts/deleted_raw_contact", Contacts::CONTACTS_DELETE},
    {"/com.ohos.contactsdataability/contacts/deleted_raw_contact_record", Contacts::CONTACTS_DELETE_RECORD},
    {"/com.ohos.contactsdataability/contacts/raw_contact", Contacts::CONTACTS_RAW_CONTACT},
    {"/com.ohos.contactsdataability/contacts/account", Contacts::ACCOUNT},
    {"/com.ohos.contactsdataability/contacts/raw_contact/query_merge_list", Contacts::QUERY_MERGE_LIST},
    {"/com.ohos.contactsdataability/contacts/raw_contact/split_contact", Contacts::SPLIT_CONTACT},
    {"/com.ohos.contactsdataability/contacts/raw_contact/manual_merge", Contacts::MANUAL_MERGE},
    {"/com.ohos.contactsdataability/contacts/raw_contact/auto_merge", Contacts::AUTO_MERGE},
    {"/com.ohos.contactsdataability/contacts/contact_data", Contacts::CONTACTS_CONTACT_DATA},
    {"/com.ohos.contactsdataability/contacts/contact_type", Contacts::CONTACT_TYPE},
    {"/com.ohos.contactsdataability/contacts/groups", Contacts::CONTACTS_GROUPS},
    {"/com.ohos.contactsdataability/contacts/contact_blocklist", Contacts::CONTACTS_BLOCKLIST},
    {"/com.ohos.contactsdataability/contacts/photo_files", Contacts::CONTACTS_PHOTO_FILES},
    {"/com.ohos.contactsdataability/contacts/search_contact", Contacts::CONTACTS_SEARCH_CONTACT},
    {"/com.ohos.contactsdataability/contacts/backup", Contacts::CONTACT_BACKUP},
    {"/com.ohos.contactsdataability/profile/backup", Contacts::PROFILE_BACKUP},
    {"/com.ohos.contactsdataability/contacts/recover", Contacts::CONTACT_RECOVER},
    {"/com.ohos.contactsdataability/profile/recover", Contacts::PROFILE_RECOVER},
    {"/com.ohos.contactsdataability/profile/contact", Contacts::PROFILE_CONTACT},
    {"/com.ohos.contactsdataability/profile/raw_contact", Contacts::PROFILE_RAW_CONTACT},
    {"/com.ohos.contactsdataability/profile/contact_data", Contacts::PROFILE_CONTACT_DATA},
    {"/com.ohos.contactsdataability/profile/groups", Contacts::PROFILE_GROUPS},
    {"/com.ohos.contactsdataability/profile/contact_blocklist", Contacts::PROFILE_BLOCKLIST},
    {"/com.ohos.contactsdataability/profile/photo_files", Contacts::PROFILE_PHOTO_FILES},
    {"/com.ohos.contactsdataability/profile/search_contact", Contacts::PROFILE_SEARCH_CONTACT},
    {"/com.ohos.contactsdataability/profile/deleted_raw_contact_record", Contacts::PROFILE_DELETE_RECORD},
    {"/com.ohos.contactsdataability/profile/deleted_raw_contact", Contacts::PROFILE_DELETE},
    {"/com.ohos.contactsdataability/profile/contact_type", Contacts::PROFILE_TYPE}
};

ContactsDataAbility* ContactsDataAbility::Create()
{
    return new ContactsDataAbility();
}

ContactsDataAbility::ContactsDataAbility() : DataShareExtAbility()
{
}

ContactsDataAbility::~ContactsDataAbility()
{
}

static DataShare::DataShareExtAbility *ContactsDataShareCreator(const std::unique_ptr<Runtime> &runtime)
{
    HILOG_INFO("ContactsDataCreator::%{public}s", __func__);
    return ContactsDataAbility::Create();
}

__attribute__((constructor)) void RegisterDataShareCreator()
{
    HILOG_INFO("ContactsDataCreator::%{public}s", __func__);
    DataShare::DataShareExtAbility::SetCreator(ContactsDataShareCreator);
}

sptr<IRemoteObject> ContactsDataAbility::OnConnect(const AAFwk::Want &want)
{
    HILOG_INFO("ContactsDataAbility %{public}s begin.", __func__);
    Extension::OnConnect(want);
    sptr<DataShare::ContactsDataShareStubImpl> remoteObject = new (std::nothrow) DataShare::ContactsDataShareStubImpl(
        std::static_pointer_cast<ContactsDataAbility>(shared_from_this()));
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    HILOG_INFO("ContactsDataAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void ContactsDataAbility::OnStart(const Want &want)
{
    HILOG_INFO("ContactsDataAbility %{public}s begin.", __func__);
    Extension::OnStart(want);
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context != nullptr) {
        std::string basePath = context->GetDatabaseDir();
        Contacts::ContactsPath::RDB_PATH = basePath + "/";
        Contacts::ContactsPath::RDB_BACKUP_PATH = basePath + "/backup/";
    }
}

/**
 * @brief Check whether BeginTransaction of ContactsDataAbility is empty
 *
 * @param code the return number of BeginTransaction
 * @param mutex transmission parameter : lock
 *
 * @return True if BeginTransaction is empty; flase otherwise
 */
bool ContactsDataAbility::IsBeginTransactionOK(int code, std::mutex &mutex)
{
    mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsBeginTransactionOK failed");
        mutex.unlock();
        return false;
    }
    return true;
}

/**
 * @brief Check if ContactsDataAbility Commit is empty
 *
 * @param code the return number of Commit
 * @param mutex transmission parameter : lock
 *
 * @return True if ContactsDataAbility Commit is empty; flase otherwise
 */
bool ContactsDataAbility::IsCommitOK(int code, std::mutex &mutex)
{
    mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsCommitOK failed");
        mutex.unlock();
        return false;
    }
    return true;
}

/**
 * @brief Insert ContactsDataAbility into the database
 *
 * @param uri URI of the data table tobe inserted
 * @param value Inserted data value of the database
 *
 * @return Insert database results code
 */
int ContactsDataAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    HILOG_INFO("ContactsDataAbility Insert start");
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("ContactsDataAbility CheckValuesBucket error");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    int resultId = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int code = UriParseAndSwitch(uriTemp);
    int ret = contactDataBase_->BeginTransaction();
    if (!IsBeginTransactionOK(ret, g_mutex)) {
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    resultId = InsertExecute(code, valuesBucket);
    HILOG_INFO("Insert id = %{public}d", resultId);
    if (resultId == Contacts::OPERATION_ERROR) {
        contactDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::OPERATION_ERROR;
    }
    ret = contactDataBase_->Commit();
    if (!IsCommitOK(ret, g_mutex)) {
        contactDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
    HILOG_INFO("ContactsDataAbility Insert end");
    return resultId;
}

int ContactsDataAbility::InsertExecute(int &code, const OHOS::NativeRdb::ValuesBucket &value)
{
    int rowId = Contacts::RDB_EXECUTE_FAIL;
    switch (code) {
        case Contacts::CONTACTS_RAW_CONTACT:
        case Contacts::PROFILE_RAW_CONTACT:
            // contact Basic Information
            rowId = contactDataBase_->InsertRawContact(Contacts::ContactTableName::RAW_CONTACT, value);
            break;
        case Contacts::CONTACTS_CONTACT_DATA:
        case Contacts::PROFILE_CONTACT_DATA:
            // contact Information add
            rowId = contactDataBase_->InsertContactData(Contacts::ContactTableName::CONTACT_DATA, value);
            break;
        case Contacts::CONTACTS_GROUPS:
        case Contacts::PROFILE_GROUPS:
            // insert group
            rowId = contactDataBase_->InsertGroup(Contacts::ContactTableName::GROUPS, value);
            break;
        case Contacts::CONTACTS_BLOCKLIST:
        case Contacts::PROFILE_BLOCKLIST:
            // add blocklist
            rowId = contactDataBase_->InsertBlockList(Contacts::ContactTableName::CONTACT_BLOCKLIST, value);
            break;
        default:
            rowId = Contacts::OPERATION_ERROR;
            HILOG_INFO("ContactsDataAbility ====>no match uri action");
            break;
    }
    return rowId;
}

/**
 * @brief ContactsDataAbility BatchInsert database
 *
 * @param uri Determine the data table name based on the URI
 * @param value Inserted data value of the database
 *
 * @return BatchInsert database results code
 */
int ContactsDataAbility::BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    unsigned int size = values.size();
    if (size <= 0) {
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    OHOS::Uri uriTemp = uri;
    int code = UriParseAndSwitch(uriTemp);
    int ret = contactDataBase_->BeginTransaction();
    if (!IsBeginTransactionOK(ret, g_mutex)) {
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    int count = 0;
    for (unsigned int i = 0; i < size; i++) {
        ++count;
        DataShare::DataShareValuesBucket rawContactValues = values[i];
        OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(rawContactValues);
        int rowRet = InsertExecute(code, value);
        if (rowRet == Contacts::OPERATION_ERROR) {
            contactDataBase_->RollBack();
            g_mutex.unlock();
            return rowRet;
        }
        if (count % Contacts::BATCH_INSERT_COUNT == 0) {
            int markRet = contactDataBase_->Commit();
            int beginRet = contactDataBase_->BeginTransaction();
            if (!IsCommitOK(markRet, g_mutex) || !IsBeginTransactionOK(beginRet, g_mutex)) {
                contactDataBase_->RollBack();
                g_mutex.unlock();
                return Contacts::RDB_EXECUTE_FAIL;
            }
        }
    }
    int markRet = contactDataBase_->Commit();
    if (!IsCommitOK(markRet, g_mutex)) {
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
    g_mutex.unlock();
    return Contacts::OPERATION_OK;
}

/**
 * @brief Update ContactsDataAbility in the database
 *
 * @param uri URI of the data table to be inserted
 * @param predicates Conditions for updating data value
 *
 * @return Update database results code
 */
int ContactsDataAbility::Update(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, const DataShare::DataShareValuesBucket &value)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("ContactsDataAbility CheckValuesBucket error");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    int retCode = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int code = UriParseAndSwitch(uriTemp);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    UpdateExecute(retCode, code, valuesBucket, dataSharePredicates);
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_UPDATE, uri);
    return retCode;
}

void ContactsDataAbility::UpdateExecute(int &retCode, int code, const OHOS::NativeRdb::ValuesBucket &value,
    DataShare::DataSharePredicates &dataSharePredicates)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (code) {
        case Contacts::CONTACTS_CONTACT:
        case Contacts::PROFILE_CONTACT:
            break;
        case Contacts::CONTACTS_RAW_CONTACT:
        case Contacts::PROFILE_RAW_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->UpdateRawContact(value, rdbPredicates);
            break;
        case Contacts::CONTACTS_CONTACT_DATA:
        case Contacts::PROFILE_CONTACT_DATA:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT_DATA, dataSharePredicates);
            retCode = contactDataBase_->UpdateContactData(value, rdbPredicates);
            break;
        case Contacts::CONTACTS_GROUPS:
        case Contacts::PROFILE_GROUPS:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::GROUPS, dataSharePredicates);
            retCode = contactDataBase_->UpdateGroup(value, rdbPredicates);
            break;
        case Contacts::CONTACTS_BLOCKLIST:
        case Contacts::PROFILE_BLOCKLIST:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::CONTACT_BLOCKLIST, dataSharePredicates);
            retCode = contactDataBase_->UpdateBlockList(value, rdbPredicates);
            break;
        default:
            SwitchUpdate(retCode, code, value, dataSharePredicates);
            break;
    }
}

void ContactsDataAbility::SwitchUpdate(int &retCode, int &code, const OHOS::NativeRdb::ValuesBucket &value,
    DataShare::DataSharePredicates &dataSharePredicates)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (code) {
        case Contacts::SPLIT_CONTACT:
            retCode = contactDataBase_->Split(dataSharePredicates);
            break;
        case Contacts::MANUAL_MERGE:
            retCode = contactDataBase_->ReContactMerge(dataSharePredicates);
            break;
        case Contacts::AUTO_MERGE:
            retCode = contactDataBase_->ContactMerge();
            break;
        case Contacts::CONTACT_BACKUP:
        case Contacts::PROFILE_BACKUP:
            retCode = BackUp();
            break;
        case Contacts::CONTACT_RECOVER:
        case Contacts::PROFILE_RECOVER:
            retCode = Recover(code);
            break;
        default:
            retCode = Contacts::RDB_EXECUTE_FAIL;
            HILOG_ERROR("ContactsDataAbility ====>no match uri action");
            break;
    }
}

/**
 * @brief Delete ContactsDataAbility from the database
 *
 * @param uri URI for the data table storing ContactsDataAbility
 * @param predicates Conditions for deleting data values
 *
 * @return Delete database results code
 */
int ContactsDataAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    HILOG_INFO("ContactsDataAbility ====>Delete");
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    int retCode = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int code = UriParseAndSwitch(uriTemp);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    DeleteExecute(retCode, code, dataSharePredicates);
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_DELETE, uri);
    return retCode;
}

void ContactsDataAbility::DeleteExecute(
    int &retCode, int code, DataShare::DataSharePredicates &dataSharePredicates)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (code) {
        case Contacts::CONTACTS_CONTACT:
        case Contacts::PROFILE_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT, dataSharePredicates);
            retCode = contactDataBase_->DeleteContact(rdbPredicates);
            break;
        case Contacts::CONTACTS_RAW_CONTACT:
        case Contacts::PROFILE_RAW_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->DeleteRawContact(rdbPredicates);
            break;
        case Contacts::CONTACTS_CONTACT_DATA:
        case Contacts::PROFILE_CONTACT_DATA:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT_DATA, dataSharePredicates);
            retCode = contactDataBase_->DeleteContactData(rdbPredicates);
            break;
        case Contacts::CONTACTS_GROUPS:
        case Contacts::PROFILE_GROUPS:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::GROUPS, dataSharePredicates);
            retCode = contactDataBase_->DeleteGroup(rdbPredicates);
            break;
        case Contacts::CONTACTS_BLOCKLIST:
        case Contacts::PROFILE_BLOCKLIST:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::CONTACT_BLOCKLIST, dataSharePredicates);
            retCode = contactDataBase_->DeleteBlockList(rdbPredicates);
            break;
        case Contacts::CONTACTS_DELETE:
        case Contacts::PROFILE_DELETE:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_DELETED, dataSharePredicates);
            retCode = contactDataBase_->CompletelyDelete(rdbPredicates);
            break;
        case Contacts::CONTACTS_DELETE_RECORD:
        case Contacts::PROFILE_DELETE_RECORD:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::DELETE_RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->DeleteRecord(rdbPredicates);
            break;
        default:
            retCode = Contacts::OPERATION_ERROR;
            HILOG_ERROR("ContactsDataAbility ====>No matching URI action");
            break;
    }
}

/**
 * @brief Query contact data in the database
 *
 * @param uri URI of the data table that stores the contact data
 * @param columns Columns where the contact data is located
 * @param predicates Condition for querying data values
 *
 * @return Database query result
 */
std::shared_ptr<DataShare::DataShareResultSet> ContactsDataAbility::Query(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns)
{
    HILOG_INFO("ContactsDataAbility ====>Query start");
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> result;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParseAndSwitch(uriTemp);
    std::vector<std::string> columnsTemp = columns;
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    bool isUriMatch = QueryExecute(result, dataSharePredicates, columnsTemp, parseCode);
    if (!isUriMatch) {
        g_mutex.unlock();
        return nullptr;
    }
    auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(result);
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult =
        std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
    g_mutex.unlock();
    HILOG_INFO("ContactsDataAbility ====>Query end");
    return sharedPtrResult;
}

bool ContactsDataAbility::QueryExecute(std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &result,
    DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp,
    int &parseCode)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    bool isUriMatch = true;
    switch (parseCode) {
        case Contacts::CONTACTS_CONTACT:
        case Contacts::PROFILE_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_CONTACT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_RAW_CONTACT:
        case Contacts::PROFILE_RAW_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_RAW_CONTACT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_CONTACT_DATA:
        case Contacts::PROFILE_CONTACT_DATA:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_CONTACT_DATA, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_GROUPS:
        case Contacts::PROFILE_GROUPS:
            rdbPredicates = predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_GROUPS, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        default:
            isUriMatch = QueryExecuteSwitchSplit(result, dataSharePredicates, columnsTemp, parseCode);
            break;
    }
    return isUriMatch;
}

bool ContactsDataAbility::QueryExecuteSwitchSplit(std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &result,
    DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp,
    int &parseCode)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    bool isUriMatch = true;
    switch (parseCode) {
        case Contacts::CONTACTS_BLOCKLIST:
        case Contacts::PROFILE_BLOCKLIST:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::CONTACT_BLOCKLIST, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_DELETE:
        case Contacts::PROFILE_DELETE:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_DELETED, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_SEARCH_CONTACT:
        case Contacts::PROFILE_SEARCH_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::SEARCH_CONTACT_VIEW, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::QUERY_MERGE_LIST:
            result = contactDataBase_->SelectCandidate();
            break;
        case Contacts::CONTACT_TYPE:
        case Contacts::PROFILE_TYPE:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT_TYPE, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::ACCOUNT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::ACCOUNT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        default:
            isUriMatch = false;
            HILOG_ERROR("ContactsDataAbility ====>no matching uri action");
            break;
    }
    return isUriMatch;
}

int ContactsDataAbility::UriParseAndSwitch(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    int parseCode = uriUtils.UriParse(uri, uriValueMap_);
    if (parseCode != Contacts::OPERATION_ERROR) {
        SwitchProfile(uri);
    }
    return parseCode;
}

void ContactsDataAbility::SwitchProfile(Uri &uri)
{
    std::vector<std::string> pathVector;
    uri.GetPathSegments(pathVector);
    if (pathVector.size() > 1 && pathVector[1].find("profile") == std::string::npos) {
        contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
        contactDataBase_->store_ = contactDataBase_->contactStore_;
    } else {
        profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
        contactDataBase_->store_ = profileDataBase_->store_;
    }
}

int ContactsDataAbility::BackUp()
{
    std::shared_ptr<OHOS::Contacts::DataBaseDisasterRecovery> instance =
        OHOS::Contacts::DataBaseDisasterRecovery::GetInstance();
    int retCode = instance->SQLiteCheckDb();
    if (retCode == 0) {
        retCode = instance->BackDatabase();
    }
    return retCode;
}

int ContactsDataAbility::Recover(int &code)
{
    std::string name = Contacts::PROFILE_DATABASE_NAME;
    if (code == Contacts::CONTACT_RECOVER) {
        name = Contacts::CONTACT_DATABASE_NAME;
    }
    std::shared_ptr<OHOS::Contacts::DataBaseDisasterRecovery> instance =
        OHOS::Contacts::DataBaseDisasterRecovery::GetInstance();
    int retCode = instance->RecoveryDatabase(name);
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    return retCode;
}

void ContactsDataAbility::DataBaseNotifyChange(int code, Uri uri)
{
    Contacts::ContactsCommonEvent::SendContactChange(code);
}
} // namespace AbilityRuntime
} // namespace OHOS