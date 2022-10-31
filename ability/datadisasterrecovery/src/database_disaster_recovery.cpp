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

#include "database_disaster_recovery.h"

#include <mutex>

#include "calllog_database.h"
#include "contacts_database.h"
#include "file_utils.h"
#include "profile_database.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mtx;
}

const std::string DataBaseDisasterRecovery::BACKUP_LINK_SYMBOL = "_";
const std::string DataBaseDisasterRecovery::BACKUP_SUFFIX = ".db";
const std::string DataBaseDisasterRecovery::DB_OK = "ok";
std::shared_ptr<DataBaseDisasterRecovery> DataBaseDisasterRecovery::instance_ = nullptr;
std::map<std::string, std::shared_ptr<OHOS::NativeRdb::RdbStore>> DataBaseDisasterRecovery::redbStoreMap;

std::shared_ptr<DataBaseDisasterRecovery> DataBaseDisasterRecovery::GetInstance()
{
    if (instance_ == nullptr) {
        instance_.reset(new DataBaseDisasterRecovery());
    }
    return instance_;
}

DataBaseDisasterRecovery::DataBaseDisasterRecovery()
{
}

DataBaseDisasterRecovery::~DataBaseDisasterRecovery()
{
}

int DataBaseDisasterRecovery::SQLiteCheckDb()
{
    g_mtx.lock();
    int ret = OPERATION_ERROR;
    HILOG_INFO("DataBaseDisasterRecovery SQLliteCheck start.");
    redbStoreMap.clear();
    if (redbStoreMap.empty()) {
        HILOG_ERROR("DataBaseDisasterRecovery redbStoreMap init. nullptr");
        std::shared_ptr<ProfileDatabase> profile = ProfileDatabase::GetInstance();
        std::shared_ptr<ContactsDataBase> contacts = ContactsDataBase::GetInstance();
        redbStoreMap.insert(std::make_pair(PROFILE_DATABASE_NAME, profile->store_));
        redbStoreMap.insert(std::make_pair(CONTACT_DATABASE_NAME, contacts->contactStore_));
    }
    if (redbStoreMap.empty()) {
        HILOG_ERROR("DataBaseDisasterRecovery SQLliteCheck redbStoreMap is empty");
        g_mtx.unlock();
        return RDB_OBJECT_EMPTY;
    }
    for (auto &kv : redbStoreMap) {
        std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ = kv.second;
        ret = SQLiteCheckDb(store_, kv.first);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DataBaseDisasterRecovery SQLiteCheckDb ERROR.");
            g_mtx.unlock();
            return ret;
        }
    }
    g_mtx.unlock();
    HILOG_INFO("DataBaseDisasterRecovery SQLliteCheck end.");
    return RDB_EXECUTE_OK;
}

int DataBaseDisasterRecovery::SQLiteCheckDb(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, std::string dataBaseName)
{
    // default switch
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("DataBaseDisasterRecovery SQLliteCheck %{public}s store_ is nullptr", dataBaseName.c_str());
        return RDB_OBJECT_EMPTY;
    }
    std::string result;
    store_->ExecuteAndGetString(result, "PRAGMA quick_check(0)");
    HILOG_INFO("DataBaseDisasterRecovery check %{public}s result is %{public}s", dataBaseName.c_str(), result.c_str());
    if (result == DB_OK) {
        return RDB_EXECUTE_OK;
    }
    return RDB_EXECUTE_FAIL;
}

int DataBaseDisasterRecovery::BackDatabase()
{
    // Recovery
    HILOG_INFO("entry DataBaseDisasterRecovery");
    FileUtils fileUtils;
    fileUtils.Mkdir(ContactsPath::RDB_BACKUP_PATH);
    redbStoreMap.clear();
    if (redbStoreMap.empty()) {
        HILOG_ERROR("DataBaseDisasterRecovery redbStoreMap init. nullptr");
        std::shared_ptr<ProfileDatabase> profile = ProfileDatabase::GetInstance();
        std::shared_ptr<ContactsDataBase> contacts = ContactsDataBase::GetInstance();
        redbStoreMap.insert(std::make_pair(PROFILE_DATABASE_NAME, profile->store_));
        redbStoreMap.insert(std::make_pair(CONTACT_DATABASE_NAME, contacts->contactStore_));
    }
    if (redbStoreMap.empty()) {
        HILOG_ERROR("DataBaseDisasterRecovery SQLliteCheck redbStoreMap is empty");
        return RDB_OBJECT_EMPTY;
    }
    for (auto &kv : redbStoreMap) {
        std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ = kv.second;
        int version;
        store_->GetVersion(version);
        HILOG_INFO("backup version is %{public}d", version);
        int ret = BackDatabase(kv.first);
        HILOG_INFO("BackDatabase %{public}s status is %{public}d", kv.first.c_str(), ret);
    }
    return RDB_EXECUTE_OK;
}

int DataBaseDisasterRecovery::BackDatabase(std::string dataBaseName)
{
    g_mtx.lock();
    auto iter = redbStoreMap.find(dataBaseName);
    HILOG_INFO("DataBaseDisasterRecovery BackDatabase redbStoreMap size is %{public}zu", redbStoreMap.size());
    if (iter != redbStoreMap.end()) {
        std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ = iter->second;
        if (store_ == nullptr) {
            HILOG_ERROR("DataBaseDisasterRecovery BackDatabase %{public}s store_ is nullptr", dataBaseName.c_str());
            g_mtx.unlock();
            return RDB_OBJECT_EMPTY;
        }
        std::string dbPath = ContactsPath::RDB_BACKUP_PATH + dataBaseName + BACKUP_SUFFIX;
        int64_t outRowId = 0;
        OHOS::NativeRdb::ValuesBucket values;
        values.PutString(DatabaseBackupColumns::BACKUP_PATH, dbPath);
        int ret = store_->Insert(outRowId, ContactTableName::DATABASE_BACKUP_TASK, values);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DataBaseDisasterRecovery Insert failed, status is %{public}d.", ret);
            g_mtx.unlock();
            return RDB_EXECUTE_FAIL;
        }
        OHOS::NativeRdb::RdbHelper::DeleteRdbStore(dbPath);
        ret = store_->Backup(dbPath, std::vector<uint8_t>());
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DataBaseDisasterRecovery Backup failed, status is %{public}d.", ret);
            g_mtx.unlock();
            return RDB_EXECUTE_FAIL;
        }
    }
    g_mtx.unlock();
    return RDB_EXECUTE_OK;
}

std::string DataBaseDisasterRecovery::GetBackUpDatabase(const std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_)
{
    std::string sql = "select backup_path from ";
    sql.append(ContactTableName::DATABASE_BACKUP_TASK).append(" order by backup_time desc limit 0,1");
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> result = store_->QuerySql(sql, std::vector<std::string>());
    std::string currValue;
    if (result->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
        int currValueIndex;
        result->GetColumnIndex(DatabaseBackupColumns::BACKUP_PATH, currValueIndex);
        result->GetString(currValueIndex, currValue);
        result->GoToNextRow();
    }
    result->Close();
    return currValue;
}

int DataBaseDisasterRecovery::RecoveryDatabase(std::string dataBaseName)
{
    if (dataBaseName == PROFILE_DATABASE_NAME) {
        std::string backupPath = ContactsPath::RDB_BACKUP_PATH + dataBaseName + BACKUP_SUFFIX;
        ProfileDatabase::DestroyInstanceAndRestore(backupPath);
    } else if (dataBaseName == CONTACT_DATABASE_NAME) {
        std::string backupPath = ContactsPath::RDB_BACKUP_PATH + dataBaseName + BACKUP_SUFFIX;
        ContactsDataBase::DestroyInstanceAndRestore(backupPath);
    }
    return RDB_EXECUTE_OK;
}
} // namespace Contacts
} // namespace OHOS
