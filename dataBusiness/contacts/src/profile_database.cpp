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

#include "profile_database.h"

#include <cstdio>
#include <mutex>
#include <unistd.h>

#include "common.h"
#include "contacts_account.h"
#include "contacts_columns.h"
#include "contacts_type.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
std::shared_ptr<ProfileDatabase> ProfileDatabase::profileDatabase_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> ProfileDatabase::store_ = nullptr;
static std::string g_databaseName;
namespace {
std::mutex g_mtx;
} // namespace

ProfileDatabase::ProfileDatabase()
{
    int errCode = OHOS::NativeRdb::E_OK;
    g_databaseName = ContactsPath::RDB_PATH + "profile.db";
    OHOS::NativeRdb::RdbStoreConfig config(g_databaseName);
    HILOG_INFO("ProfileDatabase g_databaseName :%{public}s", g_databaseName.c_str());
    SqliteOpenHelperProfileCallback sqliteOpenHelperCallback;
    store_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(config, DATABASE_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    if (errCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ProfileDatabase errCode :%{public}d", errCode);
    } else {
        HILOG_INFO("ProfileDatabase errCode :%{public}d", errCode);
    }
    if (errCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ProfileDatabase rebase open error :%{public}d", errCode);
        return;
    }
    std::shared_ptr<ContactsAccount> contactsAccount = ContactsAccount::GetInstance();
    contactsAccount->PrepopulateCommonAccountTypes(store_);
    ContactsType contactsType;
    contactsType.PrepopulateCommonTypes(store_);
}

std::shared_ptr<ProfileDatabase> ProfileDatabase::GetInstance()
{
    if (profileDatabase_ == nullptr) {
        g_mtx.lock();
        profileDatabase_.reset(new ProfileDatabase());
        g_mtx.unlock();
    }
    return profileDatabase_;
}

void ProfileDatabase::DestroyInstanceAndRestore(std::string restorePath)
{
    g_mtx.lock();
    if (access(restorePath.c_str(), F_OK) != 0) {
        HILOG_ERROR("Restore file %{private}s does not exist", restorePath.c_str());
        g_mtx.unlock();
        return;
    }
    OHOS::NativeRdb::RdbHelper::DeleteRdbStore(g_databaseName);
    OHOS::NativeRdb::RdbHelper::ClearCache();
    profileDatabase_ = nullptr;
    Restore(restorePath);
    g_mtx.unlock();
}

bool ProfileDatabase::Restore(std::string restorePath)
{
    if (rename(restorePath.c_str(), g_databaseName.c_str())) {
        HILOG_ERROR("Restore rename oldFileName = %{private}s to newFileName  %{private}s", restorePath.c_str(),
            g_databaseName.c_str());
        return true;
    }
    return false;
}

int SqliteOpenHelperProfileCallback::OnCreate(OHOS::NativeRdb::RdbStore &store)
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

int SqliteOpenHelperProfileCallback::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("OnUpgrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion < newVersion && newVersion == DATABASE_NEW_VERSION) {
        store.ExecuteSql("ALTER TABLE database_backup_task ADD COLUMN sync TEXT");
    }
    store.SetVersion(newVersion);
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperProfileCallback::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
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
} // namespace Contacts
} // namespace OHOS