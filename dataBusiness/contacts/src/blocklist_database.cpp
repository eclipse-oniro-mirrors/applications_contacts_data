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
#include "blocklist_database.h"

#include <filesystem>
#include <fstream>

#include "async_task.h"
#include "common.h"
#include "contacts_common.h"
#include "contacts_database.h"
#include "contacts_type.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "rdb_store_config.h"
#include "security_label.h"
#include "sql_analyzer.h"
#include "predicates_convert.h"

#include "rdb_utils.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mutex;
}
std::shared_ptr<BlocklistDataBase> BlocklistDataBase::blocklistDataBase_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> BlocklistDataBase::store_ = nullptr;
static std::string g_databaseName("");

BlocklistDataBase::BlocklistDataBase()
{
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    //调用rdb方法拼接数据库路径
    g_databaseName = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
        ContactsPath::RDB_EL1_PATH, "blocklist.db", getDataBasePathErrCode);
    HILOG_WARN(
        "BlocklistDataBase getDataBasePathErrCode = %{public}d, ts = %{public}lld", getDataBasePathErrCode, (long long) time(NULL));
    if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("BlocklistDataBase GetDefaultDatabasePath err :%{public}d,"
                    "ts = %{public}lld",
            getDataBasePathErrCode,
            (long long) time(NULL));
        return;
    }
// LCOV_EXCL_START
    int SetSecurityLabelCode =
            FileManagement::ModuleSecurityLabel::SecurityLabel::SetSecurityLabel(g_databaseName, "s1");
    HILOG_INFO("BlocklistDataBase SetSecurityLabel s1 retCode=%{public}d", SetSecurityLabelCode);
    int errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(g_databaseName);
    config.SetBundleName("com.ohos.contactsdataability");
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S1);
    config.SetArea(0);
    config.SetName("blocklist.db");
    config.SetAllowRebuild(true);
    SqliteOpenHelperBlocklistCallback sqliteOpenHelperCallback;
    store_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(
        config, DATABASE_BLOCKLIST_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    HILOG_WARN("BlocklistDataBase GetRdbStore errCode:%{public}d, ts = %{public}lld", errCode, (long long) time(NULL));
// LCOV_EXCL_STOP
}

std::shared_ptr<BlocklistDataBase> BlocklistDataBase::GetInstance()
{
    if (blocklistDataBase_ == nullptr || store_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mutex);
        if (blocklistDataBase_ == nullptr || store_ == nullptr) {
            blocklistDataBase_.reset(new BlocklistDataBase());
            return blocklistDataBase_;
        }
    }
    return blocklistDataBase_;
}

int BlocklistDataBase::BeginTransaction()
{
    if (store_ == nullptr) {
        HILOG_ERROR("BlocklistDataBase BeginTransaction store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("BlocklistDataBase BeginTransaction fail :%{public}d", ret);
    }
    return ret;
}

int BlocklistDataBase::Commit()
{
    if (store_ == nullptr) {
        HILOG_ERROR("BlocklistDataBase Commit store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("BlocklistDataBase Commit fail :%{public}d", ret);
    }
    return ret;
}

int BlocklistDataBase::RollBack()
{
    if (store_ == nullptr) {
        HILOG_ERROR("BlocklistDataBase RollBack store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->RollBack();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("BlocklistDataBase RollBack fail :%{public}d", ret);
    }
    return ret;
}

// LCOV_EXCL_START
/**
 * @brief Query data according to given conditions
 *
 * @param rdbPredicates Conditions for query operation
 * @param columns Conditions for query operation
 *
 * @return The result returned by the query operation
 */
std::shared_ptr<OHOS::NativeRdb::ResultSet> BlocklistDataBase::Query(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &columns)
{
    if (store_ == nullptr) {
        HILOG_ERROR("BlocklistDataBase Query store_ is nullptr, ts = %{public}lld", (long long) time(NULL));
        return nullptr;
    }
    int errCode = OHOS::NativeRdb::E_OK;
    auto resultSet = store_->QueryByStep(rdbPredicates, columns);
    int rowCount = 0;
    errCode = resultSet->GetRowCount(rowCount);
        if (rowCount > 0) {
        int resultSetNum = resultSet->GoToFirstRow();
        std::string phones = "";
        std::string formatPhones = "";
        unsigned int PHONE_POST_NUM = 4;
        unsigned int PHONE_PRE_NUM = 3;
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            std::string phoneColumn = ContactBlockListColumns::PHONE_NUMBER;
            int phoneIndex = 0;
            resultSet->GetColumnIndex(phoneColumn, phoneIndex);
            std::string phone;
            resultSet->GetString(phoneIndex, phone);
            if (!phone.empty()) {
                phones.append(phone.size() >= PHONE_PRE_NUM ? phone.substr(0, PHONE_PRE_NUM).c_str()
                    : phone.c_str()).append(":").append(phone.size() >= PHONE_POST_NUM ?
                    phone.substr(phone.size() - PHONE_POST_NUM).c_str() : phone.c_str()).append(":")
                    .append(std::to_string(phone.size())).append(",");
            }

            std::string formatPhoneColumn = ContactBlockListColumns::FORMAT_PHONE_NUMBER;
            int formatPhoneIndex = 0;
            resultSet->GetColumnIndex(formatPhoneColumn, formatPhoneIndex);
            std::string formatPhone;
            resultSet->GetString(formatPhoneIndex, formatPhone);
            if (!formatPhone.empty()) {
                formatPhones.append(formatPhone.size() >= PHONE_PRE_NUM ? formatPhone.substr(0, PHONE_PRE_NUM).c_str()
                    : formatPhone.c_str()).append(":").append(formatPhone.size() >= PHONE_POST_NUM ?
                    formatPhone.substr(formatPhone.size() - PHONE_POST_NUM).c_str() : formatPhone.c_str())
                    .append(":").append(std::to_string(formatPhone.size())).append(",");
            }
            resultSetNum = resultSet->GoToNextRow();
        }
        HILOG_WARN("When QueryBlockList phones is %{public}s; formatPhone is:%{public}s", phones.c_str(),
            formatPhones.c_str());
    }
    HILOG_INFO("BlocklistDataBase Query GetRowCount retCode= %{public}d, rowCount= %{public}d", errCode, rowCount);
    if (errCode != OHOS::NativeRdb::E_OK) {
        HILOG_INFO("Query error code is:%{public}d", errCode);
    }
    return resultSet;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
int SqliteOpenHelperBlocklistCallback::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    HILOG_INFO("BlocklistDataBase OnCreate blocklist db");
    std::vector<int> judgeSuccess;
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_BLOCKLIST));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_BLOCKLIST_INDEX_PHONE));
    unsigned int size = judgeSuccess.size();
    for (unsigned int i = 0; i < size; i++) {
        if (judgeSuccess[i] != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("SqliteOpenHelperBlocklistCallback create table error : %{public}d", judgeSuccess[i]);
        }
    }
    return OHOS::NativeRdb::E_OK;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
int SqliteOpenHelperBlocklistCallback::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("OnUpgrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    store.SetVersion(newVersion);
    return OHOS::NativeRdb::E_OK;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
int SqliteOpenHelperBlocklistCallback::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("OnDowngrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    int ret = store.SetVersion(newVersion);
    return ret;
}
// LCOV_EXCL_STOP
}  // namespace Contacts
}  // namespace OHOS
