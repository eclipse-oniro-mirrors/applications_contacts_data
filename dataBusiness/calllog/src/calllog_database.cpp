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

#include "calllog_database.h"

#include "common.h"
#include "contacts_database.h"
#include "contacts_type.h"

namespace OHOS {
namespace Contacts {
std::shared_ptr<CallLogDataBase> CallLogDataBase::callLogDataBase_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> CallLogDataBase::store_ = nullptr;
static std::string g_databaseName;

CallLogDataBase::CallLogDataBase()
{
    g_databaseName = ContactsPath::RDB_PATH + "calls.db";
    HILOG_INFO("CallLogDataBase g_databaseName :%{public}s", g_databaseName.c_str());
    int errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(g_databaseName);
    SqliteOpenHelperCallLogCallback sqliteOpenHelperCallback;
    store_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(config, DATABASE_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    if (errCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase errCode :%{public}d", errCode);
    } else {
        HILOG_INFO("CallLogDataBase errCode :%{public}d", errCode);
    }
}

std::shared_ptr<CallLogDataBase> CallLogDataBase::GetInstance()
{
    if (callLogDataBase_ == nullptr) {
        callLogDataBase_.reset(new CallLogDataBase());
        return callLogDataBase_;
    }
    return callLogDataBase_;
}

int CallLogDataBase::BeginTransaction()
{
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase BeginTransaction store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase BeginTransaction fail :%{public}d", ret);
    }
    return ret;
}

int CallLogDataBase::Commit()
{
    if (store_ == nullptr) {
        HILOG_ERROR(" CallLogDataBase Commit store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR(" CallLogDataBase Commit fail :%{public}d", ret);
    }
    return ret;
}

int CallLogDataBase::RollBack()
{
    if (store_ == nullptr) {
        HILOG_ERROR(" CallLogDataBase RollBack store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->RollBack();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR(" CallLogDataBase RollBack fail :%{public}d", ret);
    }
    return ret;
}

int SqliteOpenHelperCallLogCallback::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    std::vector<int> judgeSuccess;
    judgeSuccess.push_back(store.ExecuteSql(CREATE_VOICEMAIL));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CALLLOG));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_REPLYING));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_DATABASE_BACKUP_TASK));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_INSERT_BACKUP_TIME));
    judgeSuccess.push_back(store.ExecuteSql(CALL_LOG_PHONE_NUMBER_INDEX));
    unsigned int size = judgeSuccess.size();
    for (unsigned int i = 0; i < size; i++) {
        if (judgeSuccess[i] != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("SqliteOpenHelperCallLogCallback create table error : %{public}d", judgeSuccess[i]);
        }
    }
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperCallLogCallback::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("OnUpgrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion < newVersion && newVersion == DATABASE_NEW_VERSION) {
        store.ExecuteSql("ALTER TABLE database_backup_task ADD COLUMN sync TEXT");
    }
    store.SetVersion(newVersion);
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperCallLogCallback::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
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

/**
 * @brief InsertCallLog operation
 *
 * @param insertValues Conditions for update operation
 *
 * @return InsertCallLog operation results
 */
int64_t CallLogDataBase::InsertCallLog(OHOS::NativeRdb::ValuesBucket insertValues)
{
    int64_t outRowId = RDB_EXECUTE_FAIL;
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase Insert store_ is  nullptr");
        return RDB_OBJECT_EMPTY;
    }
    QueryContactsByInsertCalls(insertValues);
    int ret = store_->Insert(outRowId, CallsTableName::CALLLOG, insertValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase InsertCallLog ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return outRowId;
}

/**
 * @brief UpdateCallLog operation
 *
 * @param values Conditions for update operation
 * @param predicates Conditions for update operation
 *
 * @return UpdateCallLog operation results
 */
int CallLogDataBase::UpdateCallLog(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &predicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase Update store_ is  nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (values.HasColumn(CallLogColumns::PHONE_NUMBER)) {
        QueryContactsByInsertCalls(values);
    }
    int changeRow;
    int ret = store_->Update(changeRow, values, predicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase UpdateCallLog ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

/**
 * @brief DeleteCallLog operation
 *
 * @param predicates Conditions for delete operation
 *
 * @return DeleteCallLog operation results
 */
int CallLogDataBase::DeleteCallLog(OHOS::NativeRdb::RdbPredicates &predicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase Delete store_ is  nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int deleteRow;
    int ret = store_->Delete(deleteRow, predicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase DeleteCallLog ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

/**
 * @brief QueryContacts operation
 *
 * @param predicates Conditions for query operation
 * @param columns Conditions for query operation
 *
 * @return Query database results
 */
std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> CallLogDataBase::Query(
    OHOS::NativeRdb::RdbPredicates &predicates, std::vector<std::string> columns)
{
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase Delete store_ is  nullptr");
        return nullptr;
    }
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> result = store_->Query(predicates, columns);
    return result;
}

/**
 * @brief QueryContacts By InsertCallLog get name and phone to UpdateTopContact
 *
 * @param insertValues Inserted data values
 *
 * @return void
 */
void CallLogDataBase::QueryContactsByInsertCalls(OHOS::NativeRdb::ValuesBucket &insertValues)
{
    if (!insertValues.HasColumn(CallLogColumns::PHONE_NUMBER)) {
        HILOG_ERROR("QueryContactsByInsertCalls phone_number is required");
        return;
    }
    OHOS::NativeRdb::ValueObject value;
    insertValues.GetObject(CallLogColumns::PHONE_NUMBER, value);
    std::string phoneNumber;
    value.GetString(phoneNumber);
    ContactsType contactsType;
    static std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int typeNameId = contactsType.LookupTypeId(contactsDataBase->contactStore_, ContentTypeData::PHONE);
    std::string sql = "SELECT display_name, contact_id FROM ";
    sql.append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE raw_contact_id = ")
        .append("(SELECT min(raw_contact_id) FROM ")
        .append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE detail_info = ?")
        .append(" AND is_deleted = 0")
        .append(") AND type_id = ")
        .append(std::to_string(typeNameId))
        .append(" AND is_deleted = 0");
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back(phoneNumber);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        contactsDataBase->contactStore_->QuerySql(sql, selectionArgs);

    if (resultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
        std::string quickSearchKey;
        std::string name;
        resultSet->GetString(0, name);
        resultSet->GetString(1, quickSearchKey);
        resultSet->GoToNextRow();
        insertValues.Delete(CallLogColumns::DISPLAY_NAME);
        insertValues.PutString(CallLogColumns::DISPLAY_NAME, name);
        insertValues.Delete(CallLogColumns::QUICK_SEARCH_KEY);
        insertValues.PutString(CallLogColumns::QUICK_SEARCH_KEY, quickSearchKey);
    }
    resultSet->Close();
    int ret = UpdateTopContact(insertValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("QueryContactsByInsertCalls UpdateTopContact is error");
    }
}

/**
 * @brief Update the callLog contact frequency and contact time
 *
 * @param insertValues Get contact time
 * @param phoneNumber phoneNumber To select callLog
 *
 * @return update frequency and time result code
 */
int CallLogDataBase::UpdateTopContact(OHOS::NativeRdb::ValuesBucket &insertValues)
{
    if (!insertValues.HasColumn(CallLogColumns::BEGIN_TIME)) {
        return RDB_EXECUTE_FAIL;
    }
    OHOS::NativeRdb::ValueObject value;
    insertValues.GetObject(CallLogColumns::BEGIN_TIME, value);
    int contactedTime;
    value.GetInt(contactedTime);
    std::string sqlBuild = "UPDATE ";
    sqlBuild.append(ContactTableName::RAW_CONTACT)
        .append(" SET lastest_contacted_time = ")
        .append(std::to_string(contactedTime))
        .append(", contacted_count = (contacted_count + 1) ")
        .append(" WHERE contact_id = ")
        .append("(SELECT DISTINCT min(contact_id) FROM ")
        .append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE detail_info = ? ")
        .append(")");
    std::vector<OHOS::NativeRdb::ValueObject> bindArgs;
    OHOS::NativeRdb::ValueObject phone;
    insertValues.GetObject(CallLogColumns::PHONE_NUMBER, phone);
    bindArgs.push_back(phone);
    int ret = ContactsDataBase::GetInstance()->contactStore_->ExecuteSql(sqlBuild, bindArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("store ExecuteSql error");
        return RDB_EXECUTE_FAIL;
    }
    return RDB_EXECUTE_OK;
}
} // namespace Contacts
} // namespace OHOS