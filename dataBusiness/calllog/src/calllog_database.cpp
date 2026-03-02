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

#include <filesystem>
#include <fstream>
#include <cstdio>
#include <mutex>
#include <iterator>

#include "board_report_util.h"
#include "calllog_common.h"
#include "common.h"
#include "contacts_database.h"
#include "contacts_type.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "async_task.h"
#include "rdb_store_config.h"
#include "security_label.h"
#include "sql_analyzer.h"
#include "predicates_convert.h"
#include "contacts_common_event.h"
#include "calllog_manager.h"
#include "os_account_manager.h"
#include "raw_data_parser.h"
#include "privacy_contacts_manager.h"

#ifdef ABILITY_CUST_SUPPORT
#include "dlfcn.h"
#endif

#include "rdb_utils.h"
#include "contacts_string_utils.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mutex;
std::mutex g_resourcemutex_;
std::mutex g_mtx;
}
std::shared_ptr<CallLogDataBase> CallLogDataBase::callLogDataBase_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> CallLogDataBase::store_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> CallLogDataBase::storeForC_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> CallLogDataBase::storeForE_ = nullptr;
std::shared_ptr<DataShare::DataShareHelper> CallLogDataBase::dataShareCallLogHelper_ = nullptr;
std::shared_ptr<DataShare::DataShareHelper> CallLogDataBase::dataShareCallLogHelperRefresh_ = nullptr;
static std::string g_databaseName("");

#ifdef ABILITY_CUST_SUPPORT
static const std::string TELEPHONY_CUST_SO_PATH = "libtelephony_cust_api.z.so";
#endif

static const std::string CALLS_DB_NAME = "calls.db";
static const std::string CALLSEL1_DB_NAME = "callsEl1.db";
static const int RETRY_GET_RDBSTORE_COUNT = 5; // 开库失败重试五次
static const int RETRY_GET_RDBSTORE_SLEEP_TIMES = 200; // 200ms
static AsyncTaskQueue *g_asyncTaskQueue;

CallLogDataBase::CallLogDataBase()
{
    // 注册通话记录事件监听
    OHOS::Contacts::CallLogManager::GetInstance();
    if (MoveDbFile()) {
        HILOG_WARN("CallLogDataBase Database file moved successfully, ts = %{public}lld", (long long) time(NULL));
    }
    storeForE_ = CreatSql(CallLogType::E_CallLogType); // el5
    storeForC_ = CreatSql(CallLogType::C_CallLogType); // el1
    store_ = storeForC_;
    if (storeForE_ != nullptr) {
        auto rebuilt = OHOS::NativeRdb::RebuiltType::NONE;
        int rebuiltCode = storeForE_->GetRebuilt(rebuilt);
        if (rebuilt == OHOS::NativeRdb::RebuiltType::REBUILT && rebuiltCode == OHOS::NativeRdb::E_OK) {
            int restoreCode = storeForE_->Restore("calls.db.bak");
            HILOG_ERROR("CallLogDataBase Restore retCode= %{public}d", restoreCode);
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLS_DB_NAME, restoreCode,
                                                      "Restore err");
        }
        UpdateCallLogChangeFile();
    } else {
        store_.reset();
        store_ = storeForC_;
        HILOG_ERROR("Failed to link to the E category. Linking to the C category instead, ts = %{public}lld",
                    (long long) time(NULL));
        if (store_ == nullptr) {
            HILOG_ERROR("Failed to link to the C category. ts = %{public}lld", (long long) time(NULL));
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLSEL1_DB_NAME, -1, "store is nullptr");
            return;
        }
    }
    if (storeForE_ != nullptr) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLS_DB_NAME, 0, "el5 callsdb open success");
    } else {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLSEL1_DB_NAME, 0,
                                                  "el1 callsdb open success");
    }
    g_asyncTaskQueue = AsyncTaskQueue::Instance();
    HILOG_WARN("CallLogDataBase Database end");
}

std::shared_ptr<CallLogDataBase> CallLogDataBase::GetInstance()
{
    if (callLogDataBase_ == nullptr || store_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mutex);
        if (callLogDataBase_ == nullptr || store_ == nullptr) {
            callLogDataBase_.reset(new CallLogDataBase());
            return callLogDataBase_;
        }
    }
    return callLogDataBase_;
}

bool CallLogDataBase::MoveDbFile() // 不同路径下的数据库文件不能同名
{
    std::string CallsDbNamePath = ContactsPath::RDB_EL1_PATH + "/calls.db";
    std::ifstream f(CallsDbNamePath.c_str());
    HILOG_INFO("MoveDbFile CallsDb file is exist = %{public}s, ts = %{public}lld", std::to_string(f.good()).c_str(),
        (long long) time(NULL));
    std::string newDbPath = ContactsPath::RDB_EL1_PATH + "/rdb/";
    
    // GetDefaultDatabasePath 内部rdb会自己拼 rdb 中间目录，并创建好需要的目录
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(ContactsPath::RDB_EL1_PATH, "calls.db",
        getDataBasePathErrCode);
    if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("MoveDbFile GetDefaultDatabasePath err :%{public}d, "
                    "rdb path = %{public}s, ts = %{public}lld",
                    getDataBasePathErrCode, ContactsPath::RDB_EL1_PATH.c_str(), (long long) time(NULL));
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLSEL1_DB_NAME, getDataBasePathErrCode,
                                                  "MoveDbFile GetDefaultDatabasePath err");
        return false;
    }

    int errCode = access(newDbPath.c_str(), W_OK);
    if (errCode != 0) {
        HILOG_ERROR("MoveDbFile newDbPath access fail");
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLSEL1_DB_NAME, errCode,
                                                  "MoveDbFile newDbPath access fail");
        return false;
    }

    std::vector<std::string> fileNames;
    fileNames.push_back("calls.db");
    fileNames.push_back("calls.db-shm");
    fileNames.push_back("calls.db-wal");
    std::vector<std::string> fileEl1Names;
    fileEl1Names.push_back("callsEl1.db");
    fileEl1Names.push_back("callsEl1.db-shm");
    fileEl1Names.push_back("callsEl1.db-wal");
    for (std::size_t i = 0; i < fileNames.size(); ++i) {
        std::string oldDbNamePath = ContactsPath::RDB_EL1_PATH + "/" + fileNames[i];
        std::string oldDbRdbNamePath = ContactsPath::RDB_EL1_PATH + "/rdb/" + fileNames[i];
        std::string newFilePath = newDbPath + fileEl1Names[i];
        if (std::filesystem::exists(newFilePath)) {
            return false;
        }
        std::ifstream f_stream(oldDbNamePath.c_str());
        std::ifstream f_rdb_stream(oldDbRdbNamePath.c_str());
        if (f_stream.good()) {
            std::filesystem::rename(oldDbNamePath, newFilePath);
        } else if (f_rdb_stream.good()) {
            std::filesystem::rename(oldDbRdbNamePath, newFilePath);
        } else {
            HILOG_ERROR("MoveDbFile stream rename fail");
            return false;
        }
    }
    BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLSEL1_DB_NAME, 0, "MoveDbFile success");
    return true;
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
        HILOG_ERROR("CallLogDataBase Commit store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase Commit fail :%{public}d", ret);
    }
    return ret;
}

int CallLogDataBase::RollBack()
{
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase RollBack store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->RollBack();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase RollBack fail :%{public}d", ret);
    }
    return ret;
}

int SqliteOpenHelperCallLogCallback::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    HILOG_WARN("CalllogDatabase OnCreate calls db.");
    std::vector<int> judgeSuccess;
    judgeSuccess.push_back(store.ExecuteSql(CREATE_VOICEMAIL));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CALLLOG));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_REPLYING));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_DATABASE_BACKUP_TASK));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_INSERT_BACKUP_TIME));
    judgeSuccess.push_back(store.ExecuteSql(CALL_LOG_PHONE_NUMBER_INDEX));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CALL_SETTINGS));
    judgeSuccess.push_back(store.ExecuteSql(INIT_CALLlOG_CHANGE_TIME));
    judgeSuccess.push_back(store.ExecuteSql(UPDATE_CALLLOG_AVATAR));
    judgeSuccess.push_back(store.ExecuteSql(CALL_LOG_FAIL_ABS_RECORD_ID_INDEX));
    judgeSuccess.push_back(store.ExecuteSql(CALL_LOG_NOTES_ID_INDEX));
    unsigned int size = judgeSuccess.size();
    for (unsigned int i = 0; i < size; i++) {
        int ret = judgeSuccess[i];
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("SqliteOpenHelperCallLogCallback create table index %{public}d error: %{public}d", i, ret);
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_CREATE, CALLS_DB_NAME, ret,
                                                      "create table error");
            return ret;
        }
    }
    BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_CREATE, CALLS_DB_NAME, 0, "OnCreate success");
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperCallLogCallback::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("OnUpgrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);

    // 升级调用前打印数据库所有表，用于排查
    PrintAllTables(store);

    if (oldVersion < DATABASE_NEW_VERSION && newVersion >= DATABASE_NEW_VERSION) {
        store.ExecuteSql("ALTER TABLE database_backup_task ADD COLUMN sync TEXT");
    }
    int result = OHOS::NativeRdb::E_OK;
    result = UpgradeUnderV20(store, oldVersion, newVersion);
    BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_BEFORE, CALLS_DB_NAME, oldVersion);
    if (result != OHOS::NativeRdb::E_OK) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, CALLS_DB_NAME, result,
                                                  "UpgradeUnderV20 fail");
        return result;
    }
    result = UpgradeUnderV30(store, oldVersion, newVersion);
    if (result != OHOS::NativeRdb::E_OK) {
        return result;
    }
    BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, CALLS_DB_NAME, newVersion);
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperCallLogCallback::UpgradeUnderV30(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion < DATABASE_VERSION_21 && newVersion >= DATABASE_VERSION_21) {
        UpgradeToV21(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_23 && newVersion >= DATABASE_VERSION_23) {
        result = UpgradeToV23(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, CALLS_DB_NAME, result,
                                                      "UpgradeToV23 fail");
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_24 && newVersion >= DATABASE_VERSION_24) {
        result = UpgradeToV24(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, CALLS_DB_NAME, result,
                                                      "UpgradeToV24 fail");
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_25 && newVersion >= DATABASE_VERSION_25) {
        result = UpgradeToV25(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, CALLS_DB_NAME, result,
                                                      "UpgradeToV25 fail");
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_26 && newVersion >= DATABASE_VERSION_26) {
        result = UpgradeToV26(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, CALLS_DB_NAME, result,
                                                      "UpgradeToV26 fail");
            return result;
        }
    }
    result = UpgradeV27(store, oldVersion, newVersion);
    if (result != OHOS::NativeRdb::E_OK) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, CALLS_DB_NAME, result,
                                                  "UpgradeV27 fail");
        return result;
    }
    return result;
}

int SqliteOpenHelperCallLogCallback::UpgradeV27(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion < DATABASE_VERSION_27 && newVersion >= DATABASE_VERSION_27) {
        result = UpgradeToV27(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    return result;
}

int SqliteOpenHelperCallLogCallback::UpgradeUnderV20(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion < DATABASE_VERSION_2 && newVersion >= DATABASE_VERSION_2) {
        UpgradeToV2(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_4 && newVersion >= DATABASE_VERSION_4) {
        UpgradeToV4(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_14 && newVersion >= DATABASE_VERSION_14) {
        UpgradeToV14(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_15 && newVersion >= DATABASE_VERSION_15) {
        UpgradeToV15(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_16 && newVersion >= DATABASE_VERSION_16) {
        UpgradeToV16(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_18 && newVersion >= DATABASE_VERSION_18) {
        UpgradeToV18(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_19 && newVersion >= DATABASE_VERSION_19) {
        UpgradeToV19(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_20 && newVersion >= DATABASE_VERSION_20) {
        UpgradeToV20(store, oldVersion, newVersion);
    }
    return OHOS::NativeRdb::E_OK;
}

void SqliteOpenHelperCallLogCallback::UpgradeToV2(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion >= newVersion) {
        return;
    }
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN features INTEGERT DEFAULT 0;");
}

void SqliteOpenHelperCallLogCallback::UpgradeToV4(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion >= newVersion) {
        return;
    }
    store.ExecuteSql(CREATE_CALL_SETTINGS);
    store.ExecuteSql(INIT_CALLlOG_CHANGE_TIME);
}

void SqliteOpenHelperCallLogCallback::UpgradeToV14(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV14 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN mark_count INTEGER DEFAULT 0;");
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN is_cloud_mark INTEGER DEFAULT 0;");
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN hw_is_satellite INTEGER DEFAULT 0;");
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN block_reason INTEGER;");
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN mark_content TEXT;");
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN mark_type TEXT;");
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN is_block INTEGER DEFAULT 0;");
}

void SqliteOpenHelperCallLogCallback::UpgradeToV15(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV15 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN countryiso TEXT;");
}

void SqliteOpenHelperCallLogCallback::UpgradeToV16(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV16 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    store.ExecuteSql("ALTER TABLE calllog ADD COLUMN celia_call_type INTEGER DEFAULT -1;");
}

void SqliteOpenHelperCallLogCallback::UpgradeToV18(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV18 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion >= newVersion) {
        return;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isFormateNumberExists = sqlAnalyzer.CheckColumnExists(store, "calllog", "format_phone_number");
    if (!isFormateNumberExists) {
    result = store.ExecuteSql("ALTER TABLE calllog ADD COLUMN format_phone_number TEXT;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CalllogDataBase UpgradeToV18 alter add column format_phone_number failed, result is %{public}d",
            result);
        return;
    }
    }
    HILOG_INFO("calllog UpgradeToV18 succeed.");
}

void SqliteOpenHelperCallLogCallback::UpgradeToV19(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV19 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    store.ExecuteSql(UPDATE_CALLLOG_AVATAR);
    HILOG_INFO("calllog UpgradeToV19 succeed.");
}

void SqliteOpenHelperCallLogCallback::UpgradeToV20(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV20 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isMarkDetailsExists = sqlAnalyzer.CheckColumnExists(store, "calllog", "mark_details");
    bool isMarkSourceExists = sqlAnalyzer.CheckColumnExists(store, "calllog", "mark_source");
    if (!isMarkDetailsExists) {
        store.ExecuteSql("ALTER TABLE calllog ADD COLUMN mark_details TEXT;");
    }
    if (!isMarkSourceExists) {
        store.ExecuteSql("ALTER TABLE calllog ADD COLUMN mark_source TEXT;");
    }
    HILOG_INFO("calllog UpgradeToV20 succeed.");
}

void SqliteOpenHelperCallLogCallback::UpgradeToV21(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV21 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isDetectDetailsExists = sqlAnalyzer.CheckColumnExists(store, "calllog", "detect_details");
    if (!isDetectDetailsExists) {
        store.ExecuteSql("ALTER TABLE calllog ADD COLUMN detect_details TEXT;");
    }
    HILOG_INFO("calllog UpgradeToV21 succeed.");
}

int SqliteOpenHelperCallLogCallback::UpgradeToV23(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV23 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion >= newVersion) {
        return result;
    }
    result = store.ExecuteSql("DROP TRIGGER if exists UPDATE_CALLLOG_AVATAR");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CalllogDataBase UpgradeToV23 alter DROP UPDATE_CALLLOG_AVATAR failed, result is %{public}d",
            result);
        return result;
    }
    result = store.ExecuteSql(UPDATE_CALLLOG_AVATAR);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CalllogDataBase UpgradeToV23 alter UPDATE_CALLLOG_AVATAR failed, result is %{public}d",
            result);
        return result;
    }
    HILOG_INFO("calllog UpgradeToV23 succeed.");
    return result;
}

int SqliteOpenHelperCallLogCallback::UpgradeToV24(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV24 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion >= newVersion) {
        return result;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isDetectDetailsExists = sqlAnalyzer.CheckColumnExists(store, "calllog", "is_cnap");
    if (!isDetectDetailsExists) {
        result = store.ExecuteSql("ALTER TABLE calllog ADD COLUMN is_cnap INTEGER DEFAULT 0;");
        if (result != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("CalllogDataBase UpgradeToV24 calllog ADD COLUMN is_cnap failed, result is %{public}d",
                result);
            return result;
        }
    }
    HILOG_INFO("calllog UpgradeToV24 succeed.");
    return result;
}

int SqliteOpenHelperCallLogCallback::UpgradeToV25(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV25 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion >= newVersion) {
        return result;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isDetectDetailsExists = sqlAnalyzer.CheckColumnExists(store, "calllog", "privacy_tag");
    if (!isDetectDetailsExists) {
        result = store.ExecuteSql("ALTER TABLE calllog ADD COLUMN privacy_tag INTEGER DEFAULT -1;");
        if (result != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR(
                "CalllogDataBase UpgradeToV25 calllog ADD COLUMN privacy_tag failed, result is %{public}d", result);
            return result;
        }
    }
    HILOG_INFO("calllog UpgradeToV25 succeed.");
    return result;
}

int SqliteOpenHelperCallLogCallback::UpgradeToV26(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV26 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion >= newVersion) {
        return result;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isDetectDetailsExists = sqlAnalyzer.CheckColumnExists(store, "calllog", "new_calling");
    if (!isDetectDetailsExists) {
        result = store.ExecuteSql("ALTER TABLE calllog ADD COLUMN new_calling INTEGER DEFAULT 0;");
        if (result != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("CalllogDataBase UpgradeToV26 calllog ADD COLUMN is_cnap failed, result is %{public}d",
                result);
            return result;
        }
    }
    HILOG_INFO("calllog UpgradeToV26 succeed.");
    return result;
}

int SqliteOpenHelperCallLogCallback::UpgradeToV27(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV27 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    int result = OHOS::NativeRdb::E_ERROR;
    if (oldVersion >= newVersion) {
        return result;
    }
    if (store.BeginTransaction() != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV27 BeginTransaction failed");
        return OHOS::NativeRdb::E_ERROR;
    }
    SqlAnalyzer sqlAnalyzer;
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "start_time")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN start_time INTEGER DEFAULT 0;")) {
            HILOG_ERROR("alter calllog_start_time failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "abstract_type")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN abstract_type TEXT;")) {
            HILOG_ERROR("alter calllog_abstract_type failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "abstract_profile")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN abstract_profile VARCHAR(256);")) {
            HILOG_ERROR("alter calllog_abstract_profile failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    result = AddColumnAbs(store);
    if(result != OHOS::NativeRdb::E_OK) {
        return OHOS::NativeRdb::E_ERROR; 
    }
    result = AddColumnNotes(store);
    if(result != OHOS::NativeRdb::E_OK) {
        return OHOS::NativeRdb::E_ERROR; 
    }
    if (Commit(store) != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV27 Commit failed");
        RollBack(store);
        return OHOS::NativeRdb::E_ERROR;
    }
    HILOG_INFO("calllog UpgradeToV27 succeed.");
    return OHOS::NativeRdb::E_OK;
}

// 升级到27版本需要添加的字段
int SqliteOpenHelperCallLogCallback::AddColumnAbs(OHOS::NativeRdb::RdbStore &store)
{
    SqlAnalyzer sqlAnalyzer;
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "abs_status")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN abs_status TEXT;")) {
            HILOG_ERROR("alter calllog_abs_status failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "abs_code")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN abs_code TEXT DEFAULT '-1';")) {
            HILOG_ERROR("alter calllog_abs_code failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "fail_abs_record_id")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN fail_abs_record_id INTEGER DEFAULT 0;")) {
            HILOG_ERROR("alter calllog_fail_abs_id failed");
            return OHOS::NativeRdb::E_ERROR;
        }
        ExecuteAndCheck(store, CALL_LOG_FAIL_ABS_RECORD_ID_INDEX);
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "fail_abs_record_status")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN fail_abs_record_status TEXT;")) {
            HILOG_ERROR("alter calllog_fail_abs_record_status failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "fail_abs_record_updater")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN fail_abs_record_updater TEXT;")) {
            HILOG_ERROR("alter calllog_fail_abs_record_updater failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "fail_abs_record_update_time")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN fail_abs_record_update_time TEXT;")) {
            HILOG_ERROR("alter calllog_fail_abs_record_update_time failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "abs_fin_times_code")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN abs_fin_times_code TEXT;")) {
            HILOG_ERROR("alter calllog_abs_fin_times_code failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    return OHOS::NativeRdb::E_OK;
}

// 升级到27版本需要添加的字段
int SqliteOpenHelperCallLogCallback::AddColumnNotes(OHOS::NativeRdb::RdbStore &store)
{
    SqlAnalyzer sqlAnalyzer;
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "notes_id")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN notes_id TEXT;")) {
            HILOG_ERROR("alter calllog_notes_id failed");
            return OHOS::NativeRdb::E_ERROR;
        }
        ExecuteAndCheck(store, CALL_LOG_NOTES_ID_INDEX);
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "notes_status")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN notes_status TEXT;")) {
            HILOG_ERROR("alter calllog_notes_status failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "notes_status_updater")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN notes_status_updater TEXT;")) {
            HILOG_ERROR("alter calllog_notes_status_updater failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    if (!sqlAnalyzer.CheckColumnExists(store, "calllog", "notes_status_update_time")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE calllog ADD COLUMN notes_status_update_time TEXT;")) {
            HILOG_ERROR("alter calllog_notes_status_update_time failed");
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    return OHOS::NativeRdb::E_OK;
}

bool SqliteOpenHelperCallLogCallback::ExecuteAndCheck(OHOS::NativeRdb::RdbStore &store, const std::string &sql)
{
    int result = store.ExecuteSql(sql);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV27 SQL execution failed: %{public}s", sql.c_str());
        RollBack(store);
        return false;
    }
    return true;
}

int SqliteOpenHelperCallLogCallback::Commit(OHOS::NativeRdb::RdbStore &store)
{
    int ret = CallLogHandleRdbStoreRetry([&]() {
        return store.Commit();
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("SqliteOpenHelperCallLogCallback Commit failed :%{public}d", ret);
    }
    return ret;
}
int SqliteOpenHelperCallLogCallback::RollBack(OHOS::NativeRdb::RdbStore &store)
{
    int ret = CallLogHandleRdbStoreRetry([&]() {
        return store.RollBack();
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("SqliteOpenHelperCallLogCallback RollBack failed :%{public}d", ret);
    }
    return ret;
}
int SqliteOpenHelperCallLogCallback::BeginTransaction(OHOS::NativeRdb::RdbStore &store)
{
    int ret = CallLogHandleRdbStoreRetry([&]() {
        return store.BeginTransaction();
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("SqliteOpenHelperCallLogCallback BeginTransaction failed :%{public}d", ret);
    }
    return ret;
}

template <typename Func>
int CallLogDataBase::CallLogHandleRdbStoreRetry(Func &&callback)
{
    int retCode = std::forward<decltype(callback)>(callback)();
    auto itRetryWithSleep = RBD_STORE_RETRY_REQUEST_WITH_SLEEP.find(retCode);
    if (itRetryWithSleep != RBD_STORE_RETRY_REQUEST_WITH_SLEEP.end()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(RBD_STORE_RETRY_REQUEST_WITH_SLEEP_TIMES));
        HILOG_INFO("CallLogHandleRdbStoreRetry ret code is %{public}d", retCode);
        return std::forward<decltype(callback)>(callback)();
    }
    return retCode;
}

template <typename Func>
int SqliteOpenHelperCallLogCallback::CallLogHandleRdbStoreRetry(Func&& callback)
{
    int retCode = std::forward<decltype(callback)>(callback)();
    auto itRetryWithSleep = RBD_STORE_RETRY_REQUEST_WITH_SLEEP.find(retCode);
    if (itRetryWithSleep != RBD_STORE_RETRY_REQUEST_WITH_SLEEP.end()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(RBD_STORE_RETRY_REQUEST_WITH_SLEEP_TIMES));
        HILOG_INFO("SqliteOpenHelperCallLogCallback CallLogHandleRdbStoreRetry ret code is %{public}d", retCode);
        return std::forward<decltype(callback)>(callback)();
    }
    return retCode;
}

int SqliteOpenHelperCallLogCallback::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("OnDowngrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion > newVersion && newVersion == DATABASE_CALL_LOG_OPEN_VERSION) {
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

int SqliteOpenHelperCallLogCallback::OnOpen(OHOS::NativeRdb::RdbStore &rdbStore)
{
    HILOG_INFO("OnOpen start.");
    int ret = OHOS::NativeRdb::E_OK;
    int version = -1;
    ret = rdbStore.GetVersion(version);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase GetVersion error: %{public}d.", ret);
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLS_DB_NAME, ret, "GetVersion error");
        return OHOS::NativeRdb::E_OK;
    }
    HILOG_WARN(
        "OnOpen calls.db version is %{public}d, new version is %{public}d.", version, DATABASE_CALL_LOG_OPEN_VERSION);
    BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLS_DB_NAME, ret);

    return OHOS::NativeRdb::E_OK;
}

void SqliteOpenHelperCallLogCallback::PrintAllTables(OHOS::NativeRdb::RdbStore &rdbStore)
{
    SqlAnalyzer sqlAnalyzer;
    std::vector<std::string> tables = sqlAnalyzer.QueryAllTables(rdbStore);
    if (!tables.empty()) {
        std::string tablesStr = std::accumulate(
            std::next(tables.begin()), tables.end(), tables[0], [](const std::string &a, const std::string &b) {
                return a + ", " + b;
            });
        HILOG_WARN("OnUpgrade all tables: %{public}s.", tablesStr.c_str());
    }
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
    std::string quickSearchKey = "";
    int callDirection = -1;
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase Insert store_ is nullptr");
        BoardReportUtil::InsertCallLogReport(ExecuteResult::FAIL, quickSearchKey, callDirection,
                                             "InsertCallLog store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    QueryContactsByInsertCalls(insertValues);
    PrintCallInfoLog(insertValues);
    int ret = store_->Insert(outRowId, CallsTableName::CALLLOG, insertValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase InsertCallLog ret :%{public}d, outRowId :%{public}lld",
            ret,
            (long long) outRowId);
        std::string faultInfo = "InsertCallLog ret: " + std::to_string(ret) + "row:" +  std::to_string(outRowId);
        BoardReportUtil::InsertCallLogReport(ExecuteResult::FAIL, quickSearchKey, callDirection, faultInfo);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("CallLogDataBase InsertCallLog end, outRowId :%{public}lld, ts = %{public}lld",
        (long long) outRowId,
        (long long) time(NULL));
    if (insertValues.HasColumn(Contacts::CallLogColumns::QUICK_SEARCH_KEY)) {
        OHOS::NativeRdb::ValueObject value;
        insertValues.GetObject(Contacts::CallLogColumns::QUICK_SEARCH_KEY, value);
        value.GetString(quickSearchKey);
    }
    if (insertValues.HasColumn(Contacts::CallLogColumns::CALL_DIRECTION)) {
        OHOS::NativeRdb::ValueObject value;
        insertValues.GetObject(Contacts::CallLogColumns::CALL_DIRECTION, value);
        value.GetInt(callDirection);
    }
    BoardReportUtil::InsertCallLogReport(ExecuteResult::SUCCESS, quickSearchKey, callDirection);
    return outRowId;
}

void CallLogDataBase::PrintCallInfoLog(OHOS::NativeRdb::ValuesBucket insertValues)
{
    std::string name = "";
    if (insertValues.HasColumn(Contacts::CallLogColumns::DISPLAY_NAME)) {
        OHOS::NativeRdb::ValueObject value;
        insertValues.GetObject(Contacts::CallLogColumns::DISPLAY_NAME, value);
        value.GetString(name);
    }
    std::string phoneNumber = "";
    if (insertValues.HasColumn(Contacts::CallLogColumns::PHONE_NUMBER)) {
        OHOS::NativeRdb::ValueObject value;
        insertValues.GetObject(Contacts::CallLogColumns::PHONE_NUMBER, value);
        value.GetString(phoneNumber);
    }
    std::string quickSearchKey = "";
    if (insertValues.HasColumn(Contacts::CallLogColumns::QUICK_SEARCH_KEY)) {
        OHOS::NativeRdb::ValueObject value;
        insertValues.GetObject(Contacts::CallLogColumns::QUICK_SEARCH_KEY, value);
        value.GetString(quickSearchKey);
    }
    int features = -1;
    if (insertValues.HasColumn(Contacts::CallLogColumns::FEATURES)) {
        OHOS::NativeRdb::ValueObject value;
        insertValues.GetObject(Contacts::CallLogColumns::FEATURES, value);
        value.GetInt(features);
    }
    int answerState = -1;
    if (insertValues.HasColumn(Contacts::CallLogColumns::ANSWER_STATE)) {
        OHOS::NativeRdb::ValueObject value;
        insertValues.GetObject(Contacts::CallLogColumns::ANSWER_STATE, value);
        value.GetInt(answerState);
    }
    HILOG_INFO("CallLogDataBase insert info,name size:%{public}lu,number:%{public}s,"
               "contactId:%{public}s,features:%{public}d,answerState:%{public}d",
        (unsigned long) name.size(),
        ContactsStringUtils::MaskPhoneForLog(phoneNumber).c_str(),
        quickSearchKey.c_str(),
        features,
        answerState);
}

/**
 * @brief InsertCallLog operation
 *
 * @param insertValues Conditions for update operation
 *
 * @return InsertCallLog operation results
 */
int64_t CallLogDataBase::BatchInsertCallLog(const std::vector<OHOS::NativeRdb::ValuesBucket> &values)
{
    int64_t outRowId = RDB_EXECUTE_FAIL;
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase BatchInsert store_ is  nullptr");
        BoardReportUtil::InsertCallLogReport(ExecuteResult::FAIL, "", values.size(),
            "BatchInsertCallLog store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->BatchInsert(outRowId, CallsTableName::CALLLOG, values);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase BatchInsertCallLog ret :%{public}d", ret);
        BoardReportUtil::InsertCallLogReport(ExecuteResult::FAIL, "", values.size(), "BatchInsertCallLog is fail");
        return RDB_EXECUTE_FAIL;
    }
    BoardReportUtil::InsertCallLogReport(ExecuteResult::SUCCESS, "", values.size(), "BatchInsertCallLog success");
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
    int changeRow;
    int ret = store_->Update(changeRow, values, predicates);
    int isRead = -1;
    if (values.HasColumn(Contacts::CallLogColumns::IS_READ)) {
        OHOS::NativeRdb::ValueObject value;
        values.GetObject(Contacts::CallLogColumns::IS_READ, value);
        value.GetInt(isRead);
    }
    int answerState = -1;
    if (values.HasColumn(Contacts::CallLogColumns::ANSWER_STATE)) {
        OHOS::NativeRdb::ValueObject value;
        values.GetObject(Contacts::CallLogColumns::ANSWER_STATE, value);
        value.GetInt(answerState);
    }
    HILOG_INFO("CallLogDataBase update info,isRead:%{public}d,answerState:%{public}d,changeRow:%{public}d,"
               "ts = %{public}lld",
        isRead,
        answerState,
        changeRow,
        (long long) time(NULL));
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase UpdateCallLog ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return changeRow;
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
        BoardReportUtil::DeleteCallLogReport(ExecuteResult::FAIL, 0, "DeleteCallLog store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int deleteRow;
    int ret = store_->Delete(deleteRow, predicates);
    HILOG_WARN("CallLogDataBase DeleteCallLog ret :%{public}d, delRawS: :%{public}d", ret, deleteRow);
    if (ret != OHOS::NativeRdb::E_OK) {
        std::string faultInfo = "DeleteCallLog ret: " + std::to_string(ret) + "row:" +  std::to_string(deleteRow);
        BoardReportUtil::DeleteCallLogReport(ExecuteResult::FAIL, 0, faultInfo);
        return RDB_EXECUTE_FAIL;
    }
    BoardReportUtil::DeleteCallLogReport(ExecuteResult::SUCCESS, deleteRow);
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
std::shared_ptr<OHOS::NativeRdb::ResultSet> CallLogDataBase::Query(
    OHOS::NativeRdb::RdbPredicates &predicates, std::vector<std::string> columns)
{
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase Delete store_ is  nullptr");
        return nullptr;
    }
    auto resultSet = store_->Query(predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("CallLogDataBase Delete resultSet is nullptr");
        return nullptr;
    }
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    HILOG_WARN("CalllogDataBase Query rowCount = %{public}d", rowCount);
    return resultSet;
}

/**
 * @brief QueryContacts By InsertCallLog get name and phone to UpdateTopContact
 *
 * @param insertValues Inserted data values
 * @param operateTable
 *
 * @return void
 */
void CallLogDataBase::QueryContactsByInsertCalls(OHOS::NativeRdb::ValuesBucket &insertValues,
    std::string operateTable)
{
    if (!insertValues.HasColumn(CallLogColumns::PHONE_NUMBER)) {
        HILOG_ERROR("QueryContactsByInsertCalls phone_number is required");
        return;
    }
    OHOS::NativeRdb::ValueObject value;
    insertValues.GetObject(CallLogColumns::PHONE_NUMBER, value);
    std::string phoneNumber;
    value.GetString(phoneNumber);
    static std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    if (contactsDataBase == nullptr || contactsDataBase->contactStore_ == nullptr) {
        HILOG_ERROR("ContactsDataBase is nullptr or ContactsDataBase->contactStore_ is nullptr");
        return;
    }
#ifdef ABILITY_CUST_SUPPORT
    // 根据电话号码后七位，匹配联系人，更新通话记录的联系人信息
    unsigned int subPhoneNumberLength = 7;
    if (phoneNumber.length() >= subPhoneNumberLength) {
        // when number length large than 7, use enhanced number match function
        HILOG_INFO("getContactInfo use enhanced query,ts = %{public}lld", (long long) time(NULL));
        GetContactInfoEnhanced(phoneNumber, insertValues, operateTable);
        return;
    }
#endif
    // 根据通话记录的电话查匹配的联系人，根据phoneNumber精确匹配
    std::string sql = QueryContactsByInsertCallsGetSql();
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back(phoneNumber);
    auto resultSet = contactsDataBase->contactStore_->QuerySql(sql, selectionArgs);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryContactsByInsertCalls QuerySqlResult is nullptr");
        return;
    }
    if (resultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
        std::string quickSearchKey;
        std::string name;
        std::string meetimeAvatar;
        resultSet->GetString(INDEX_ZERO, name);
        resultSet->GetString(INDEX_ONE, quickSearchKey);
        // displayName->company->position
        // 如果名称为空，取公司信息
        if (name.empty()) {
            resultSet->GetString(INDEX_TWO, name);
        }
        // 如果名称仍为空，取职位信息
        if (name.empty()) {
            resultSet->GetString(INDEX_THREE, name);
        }
        resultSet->GetString(INDEX_FOUR, meetimeAvatar);
        resultSet->GoToNextRow();
        // 插入的通话记录，设置联系人相关信息
        QueryContactsByCallsInsertValues(insertValues, name, quickSearchKey, meetimeAvatar, operateTable);
    }
    resultSet->Close();
    UpdateTopContact(insertValues);
    UpdateContactedStatus(insertValues);
    HILOG_INFO("QueryContactsByInsertCalls end,ts = %{public}lld", (long long) time(NULL));
}

#ifdef ABILITY_CUST_SUPPORT
void CallLogDataBase::GetContactInfoEnhanced(std::string phoneNumber, OHOS::NativeRdb::ValuesBucket &insertValues,
    std::string operateTable)
{
    HILOG_INFO("GetContactInfoEnhanced resultId begin, ts = %{public}lld", (long long) time(NULL));
    unsigned int subPhoneNumberLength = 7;
    std::string subPhoneNumber = phoneNumber.substr(phoneNumber.size() - subPhoneNumberLength, subPhoneNumberLength);
    std::string sql = QueryContactsByCallsGetSubPhoneNumberSql(subPhoneNumber);
    static std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    if (contactsDataBase == nullptr || contactsDataBase->contactStore_ == nullptr) {
        HILOG_ERROR("GetContactInfoEnhanced ContactsDataBase is nullptr or ContactsDataBase->contactStore_ is nullptr");
        return;
    }
    auto resultSet = contactsDataBase->contactStore_->QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("GetContactInfoEnhanced QuerySqlResult is nullptr");
        return;
    }
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    HILOG_INFO("GetContactInfoEnhanced resultSetCount = %{public}d, ts = %{public}lld", rowCount, (long long) time(NULL));
    if (rowCount > 0) {
        auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(resultSet);
        std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult =
            std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
        int resultId = this->GetCallerIndex(sharedPtrResult, phoneNumber);
        if (sharedPtrResult->GoToRow(resultId) == OHOS::NativeRdb::E_OK) {
            HILOG_INFO("GetContactInfoEnhanced resultSet,ts = %{public}lld", (long long) time(NULL));
            std::string quickSearchKey;
            std::string name;
            std::string meetimeAvatar;
            sharedPtrResult->GetString(INDEX_ZERO, name);
            sharedPtrResult->GetString(INDEX_ONE, quickSearchKey);
            // displayName->company->position
            // 如果名称为空，取公司信息
            if (name.empty()) {
                sharedPtrResult->GetString(INDEX_TWO, name);
            }
            // 如果名称仍为空，取职位信息
             if (name.empty()) {
                 sharedPtrResult->GetString(INDEX_THREE, name);
             }
             sharedPtrResult->GetString(INDEX_FOUR, meetimeAvatar);
             sharedPtrResult->Close();
             QueryContactsByCallsInsertValues(insertValues, name, quickSearchKey, meetimeAvatar, operateTable);
        }
    }
    resultSet->Close();
    UpdateTopContact(insertValues);
    UpdateContactedStatus(insertValues);
}

int CallLogDataBase::GetCallerIndex(std::shared_ptr<DataShare::DataShareResultSet> resultSet, std::string phoneNumber)
{
    void *telephonyHandel = dlopen(TELEPHONY_CUST_SO_PATH.c_str(), RTLD_LAZY);
    if (telephonyHandel == nullptr) {
        return -1;
    }
    typedef int (*GetCallerIndex) (std::shared_ptr<DataShare::DataShareResultSet>, std::string);
    GetCallerIndex getCallerIndex = reinterpret_cast<GetCallerIndex>(dlsym(telephonyHandel, "GetCallerNumIndex"));
    if (getCallerIndex == nullptr) {
        dlclose(telephonyHandel);
        return -1;
    }
    int resultId = getCallerIndex(resultSet, phoneNumber);
    dlclose(telephonyHandel);
    return resultId;
}
#endif

std::string CallLogDataBase::QueryContactsByCallsGetSubPhoneNumberSql(std::string subPhoneNumber)
{
    std::string sql = "SELECT display_name, contact_id, company, position, extra3, format_phone_number, detail_info"
        " FROM view_contact_data WHERE primary_contact != 1 AND detail_info LIKE '%";
    sql.append(subPhoneNumber)
        .append("' AND is_deleted = 0 AND type_id = 5 order by raw_contact_id ASC");
    return sql;
}

void CallLogDataBase::QueryContactsByCallsInsertValues(OHOS::NativeRdb::ValuesBucket &insertValues, std::string name,
    std::string quickSearchKey, std::string meetimeAvatar, std::string operateTable)
{
    //如果查出来联系人信息，就替换为数据库的查询结果
    insertValues.Delete(CallLogColumns::DISPLAY_NAME);
    insertValues.PutString(CallLogColumns::DISPLAY_NAME, name);
    insertValues.Delete(CallLogColumns::QUICK_SEARCH_KEY);
    insertValues.PutString(CallLogColumns::QUICK_SEARCH_KEY, quickSearchKey);
    // 对于CALLLog表，变通话记录的话，也要变extra1字段；对于其他的表，不需要
    if (CallsTableName::CALLLOG == operateTable) {
        insertValues.Delete(CallLogColumns::EXTRA1);
        insertValues.PutString(CallLogColumns::EXTRA1, quickSearchKey);
        insertValues.PutString(CallLogColumns::EXTRA4, meetimeAvatar);
    }
}


std::string CallLogDataBase::QueryContactsByInsertCallsGetSql()
{
    static std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    ContactsType contactsType;
    int typeNameId = contactsType.LookupTypeId(contactsDataBase->contactStore_, ContentTypeData::PHONE);
    // 需要查询公司和职位，要从RAW_CONTACT表来查，查RAW_CONTACT时不需要判断类型了
    /**
    SELECT display_name, contact_id, company, position, extra3
        FROM ContactTableName.RAW_CONTACT
        WHERE id = (
            SELECT min(raw_contact_id)
                FROM ViewName.VIEW_CONTACT_DATA
            WHERE detail_info = ?
                AND is_deleted = 0
                AND type_id = ?
            )
        AND is_deleted = 0;
    */
    std::string sql = "SELECT display_name, contact_id, company, position, extra3 FROM ";
    sql.append(ContactTableName::RAW_CONTACT)
        .append(" WHERE id = ")
        .append("(SELECT min(raw_contact_id) FROM ")
        .append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE primary_contact != 1 AND detail_info = ?")
        .append(" AND is_deleted = 0")
        // 子查询中增加电话类型数据过滤，避免比如联系人a的公司信息是111，而111作为号码拨打时，被关联到联系人a
        .append(" AND type_id = ")
        .append(std::to_string(typeNameId))
        .append(") ")
        .append(" AND is_deleted = 0");
    return sql;
}

/**
 * @brief Update the callLog contact frequency and contact time；此更新操作时，插入的通话记录已经关联到了联系人信息
 *
 * @param insertValues Get contact time
 * @param phoneNumber phoneNumber To select callLog
 *
 * @return update frequency and time result code
 */
int CallLogDataBase::UpdateTopContact(OHOS::NativeRdb::ValuesBucket &insertValues)
{
    if (!insertValues.HasColumn(CallLogColumns::BEGIN_TIME)) {
        HILOG_ERROR("CallLogDataBase %{public}s no begin_time,ts = %{public}lld", __func__, (long long) time(NULL));
        return RDB_EXECUTE_FAIL;
    }

    std::vector<OHOS::NativeRdb::ValueObject> bindArgs;
    OHOS::NativeRdb::ValueObject quickSearchKey;
    insertValues.GetObject(CallLogColumns::QUICK_SEARCH_KEY, quickSearchKey);
    std::string quickSearchKeyStr;
    quickSearchKey.GetString(quickSearchKeyStr);
    if (quickSearchKeyStr.empty()) {
        HILOG_INFO("CallLogDataBase %{public}s end, not match contactId.", __func__);
        return RDB_EXECUTE_OK;
    }
    bindArgs.push_back(quickSearchKey);

    OHOS::NativeRdb::ValueObject value;
    insertValues.GetObject(CallLogColumns::BEGIN_TIME, value);
    int contactedTime = 0;
    value.GetInt(contactedTime);
    if (contactedTime == 0) {
        double beginTime = 0;
        value.GetDouble(beginTime);
        HILOG_INFO("insertValues has BEGIN_TIME %{public}f", beginTime);
        if (beginTime != 0) {
            contactedTime = static_cast<int>(std::floor(beginTime));
        }
    }
    std::string sqlBuild = "UPDATE ";
    if (contactedTime == 0) { // 未接通的情况下，time是0，不需要更新lastest_contacted_time
        sqlBuild.append(ContactTableName::RAW_CONTACT)
        .append(" SET contacted_count = (contacted_count + 1) ")
        .append(" WHERE contact_id = ? ");
    } else {
        sqlBuild.append(ContactTableName::RAW_CONTACT)
        .append(" SET lastest_contacted_time = ")
        .append(std::to_string(contactedTime))
        .append(", contacted_count = (contacted_count + 1) ")
        .append(" WHERE contact_id = ? ");
    }

    int ret = ContactsDataBase::GetInstance()->contactStore_->ExecuteSql(sqlBuild, bindArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateTopContact err:%{public}s", sqlBuild.c_str());
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("CallLogDataBase %{public}s end.", __func__);
    return RDB_EXECUTE_OK;
}

int CallLogDataBase::UpdateContactedStatus(OHOS::NativeRdb::ValuesBucket &insertValues)
{
    if (!insertValues.HasColumn(CallLogColumns::BEGIN_TIME)) {
        HILOG_ERROR("%{public}s no begin_time, ts = %{public}lld", __func__, (long long) time(NULL));
        return RDB_EXECUTE_FAIL;
    }
 
    OHOS::NativeRdb::ValueObject phone;
    insertValues.GetObject(CallLogColumns::PHONE_NUMBER, phone);
 
    return ContactsDataBase::GetInstance()->UpdateContactedStautsByPhoneNum(phone);
}

void CallLogDataBase::NotifyCallLogChange()
{
    HILOG_INFO("Calllog NotifyCallLogChange start. AsyncDataShareProxyTask");
    if (dataShareCallLogHelper_ == nullptr) {
        g_mtx.lock();
        if (dataShareCallLogHelper_ == nullptr) {
            DataShare::CreateOptions options;
            options.enabled_ = true;
            dataShareCallLogHelper_ = DataShare::DataShareHelper::Creator(CALL_SETTING_URI, options);
        }
        g_mtx.unlock();
    }
    Uri proxyUri(CALL_SETTING_URI);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", 1);
    DataShare::DataShareValuesBucket valuesBucket;
    valuesBucket.Put("calllog_change_time", time(NULL));
    std::unique_ptr<AsyncItem> task = std::make_unique<AsyncDataShareProxyTask>(dataShareCallLogHelper_, valuesBucket,
        predicates, proxyUri);
    if (g_asyncTaskQueue == nullptr) {
        HILOG_ERROR("NotifyCallLogChange g_asyncTaskQueue is null");
        return;
    }
    g_asyncTaskQueue->Push(task);
    g_asyncTaskQueue->Start();
}

std::shared_ptr<OHOS::NativeRdb::RdbStore> CallLogDataBase::CreatSql(CallLogType codeType)
{
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> storeTemp_ = nullptr;
    std::string callDbName = "";
    if (codeType == CallLogType::E_CallLogType) {
        g_databaseName = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
            ContactsPath::RDB_EL5_PATH, "calls.db", getDataBasePathErrCode);
        callDbName = CALLS_DB_NAME;
    } else {
        g_databaseName = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
            ContactsPath::RDB_EL1_PATH, "callsEl1.db", getDataBasePathErrCode);
        callDbName = CALLSEL1_DB_NAME;
    }
    HILOG_WARN("CallLogDataBase g_databaseName :%{public}s, ts = %{public}lld", g_databaseName.c_str(), (long long) time(NULL));
    if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase GetDefaultDatabasePath err :%{public}d, g_databaseName = %{public}s,",
            getDataBasePathErrCode,
            g_databaseName.c_str());
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, callDbName, getDataBasePathErrCode,
                                                  "GetDefaultDatabasePath err");
        return storeTemp_;
    }
    if (codeType == CallLogType::E_CallLogType) {
        FileManagement::ModuleSecurityLabel::SecurityLabel::SetSecurityLabel(g_databaseName, "s4");
    } else {
        FileManagement::ModuleSecurityLabel::SecurityLabel::SetSecurityLabel(g_databaseName, "s3");
    }
    int errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(g_databaseName);
    config.SetBundleName("com.ohos.contactsdataability");
    if (codeType == CallLogType::E_CallLogType) {
        config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S4);
        config.SetArea(EL5);
        config.SetName("calls.db");
    } else {
        config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S3);
        config.SetArea(EL1);
        config.SetName("callsEl1.db");
    }
    config.SetSearchable(true);
    config.SetAllowRebuild(true);
    SqliteOpenHelperCallLogCallback sqliteOpenHelperCallback;
    storeTemp_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(
        config, DATABASE_CALL_LOG_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    HILOG_WARN("CallLogDataBase GetRdbStore errCode:%{public}d, ts = %{public}lld", errCode, (long long) time(NULL));
    if (errCode != OHOS::NativeRdb::E_OK || storeTemp_ == nullptr) {
        HILOG_ERROR("CallLogDataBase errCode :%{public}d, ts = %{public}lld", errCode, (long long) time(NULL));
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, callDbName, errCode, "CallLogDataBase err");
        storeTemp_ = RetryGetRdbStore(errCode, config, callDbName);
    }
    if (storeTemp_ != nullptr) {
        // 执行表和字段检查任务
        HILOG_WARN("CallLogDataBase execute table and field checks.");
        CheckIncompleteTable(storeTemp_);
        CheckAndCompleteFields(storeTemp_);
    }
    // 数据库开库时，因为有配置文件的存在，所以只有写连接连接了wal文件，如果在等el5秘钥丢弃之后，去读数据，会读取失败
    // 所以也要保证开库时，读写连接都完成wal文件连接
    if (codeType == CallLogType::E_CallLogType && storeTemp_ != nullptr) {
        auto rdbPredicates = OHOS::NativeRdb::RdbPredicates(CallsTableName::CALLLOG);
        std::vector<std::string> columns;
        columns.push_back(CallLogColumns::ID);
        rdbPredicates.EqualTo(CallLogColumns::ID, 1);
        std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet = storeTemp_->Query(rdbPredicates, columns);
        resultSet->Close();
    }
    BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, callDbName, 0, "CreatSql success");
    return storeTemp_;
}

void CallLogDataBase::CheckIncompleteTable(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
{
    SqlAnalyzer sqlAnalyzer;
    int32_t ret = OHOS::NativeRdb::E_ERROR;
    if (rdbStore == nullptr) {
        HILOG_ERROR("CallLogDataBase CheckIncompleteTable rdbStore is nullptr.");
        return;
    }
    std::vector<std::string> tables = sqlAnalyzer.QueryAllTables(*rdbStore);
    for (const auto &tablePair : CALL_LOG_TABLES) {
        if (std::find(tables.begin(), tables.end(), tablePair.first) == tables.end()) {
            HILOG_WARN("CallLogDataBase CheckIncompleteTable not exist table %{public}s, try to create table.",
                       tablePair.first.c_str());
            ret = rdbStore->ExecuteSql(tablePair.second);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("CallLogDataBase CheckIncompleteTable create table %{public}s error: %{public}d.",
                    tablePair.first.c_str(), ret);
            }
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_TABLES_CHECK, CALLS_DB_NAME, ret,
                                                      tablePair.first);
        }
    }
}

void CallLogDataBase::CheckAndCompleteFields(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
{
    SqlAnalyzer sqlAnalyzer;
    int32_t ret = OHOS::NativeRdb::E_ERROR;
    if (rdbStore == nullptr) {
        HILOG_ERROR("CallLogDataBase CheckAndCompleteFields rdbStore is nullptr.");
        return;
    }
    std::vector<std::string> fields = sqlAnalyzer.QueryAllFieldsFromTable(*rdbStore, CallsTableName::CALLLOG);
    // 筛选calllog表中不存在的字段，并自动将对应字段添加到表中
    for (const auto &columnPair : CALL_LOG_ADD_COLUMNS) {
        if (std::find(fields.begin(), fields.end(), columnPair.first) == fields.end()) {
            HILOG_WARN(
                "CallLogDataBase CheckAndCompleteFields not exist field %{public}s in table %{public}s, try to add.",
                columnPair.first.c_str(), CallsTableName::CALLLOG);
            ret = rdbStore->ExecuteSql(columnPair.second);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR(
                    "CallLogDataBase CheckIncompleteTable add field %{public}s in table %{public}s error: %{public}d.",
                    columnPair.first.c_str(), CallsTableName::CALLLOG, ret);
            }
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_COLUMNS_CHECK, CALLS_DB_NAME, ret,
                                                      columnPair.first);
        }
    }
}

std::shared_ptr<OHOS::NativeRdb::RdbStore> CallLogDataBase::RetryGetRdbStore(
    int errCode, OHOS::NativeRdb::RdbStoreConfig& config, const std::string& callDbName)
{
    // 根据特定错误码进行重试
    std::shared_ptr<OHOS::NativeRdb::RdbStore> storeTemp_ = nullptr;
    SqliteOpenHelperCallLogCallback sqliteOpenHelperCallback;
    for (int i = 0; i < RETRY_GET_RDBSTORE_COUNT; i++) { // 重试五次
        if (RBD_STORE_RETRY_REQUEST_WITH_SLEEP.find(errCode) == RBD_STORE_RETRY_REQUEST_WITH_SLEEP.end()) {
            HILOG_WARN("CallLogDataBase Not need retry GetRdbStore errCode:%{public}d", errCode);
            break;
        }
        HILOG_WARN("CallLogDataBase retry GetRdbStore:%{public}d", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_GET_RDBSTORE_SLEEP_TIMES));
        storeTemp_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(
            config, DATABASE_CALL_LOG_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
        if (storeTemp_ != nullptr) {
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN,
                callDbName, errCode, "CallLogDataBase retry success");
            HILOG_WARN("CallLogDataBase retry success GetRdbStore:%{public}d", i);
            break;
        } else {
            HILOG_ERROR("CallLogDataBase GetRdbStore retry fail errCode:%{public}d", errCode);
        }
    }
    if (storeTemp_ == nullptr) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN,
            callDbName, errCode, "CallLogDataBase retry failed");
    }

    return storeTemp_;
}

void CallLogDataBase::UpdateCallLogChangeFile(CallLogType codeType, bool isChange)
{
    if (isChange) {
        if (codeType == CallLogType::E_CallLogType) { // 手机屏幕解锁后链接E类数据库
            RetryCreateStoreForE();
        } else {
            HILOG_WARN("CallLogDataBase UpdateCallLogChangeFile codeType is 0");
        }
    } else {
        CopySqlFromCToE();
    }
}

void CallLogDataBase::RetryCreateStoreForE()
{
    if (storeForE_ == nullptr) {
        storeForE_ = CreatSql(CallLogType::E_CallLogType);
        if (storeForE_ == nullptr) {
            HILOG_ERROR("CallLogDataBase RetryCreateStoreForE CreatSql storeForE_ is nullptr");
            store_ = storeForC_;
            return;
        }
        HILOG_WARN("CallLogDataBase RetryCreateStoreForE CreatSql storeForE_ success");
    }
    CopySqlFromCToE();
    store_ = storeForE_;
}

void CallLogDataBase::CopySqlFromCToE()
{
    std::thread thread([this]() {
        bool copyFileStatus = false;
        copyFileStatus = CopyCallLogFromCToE();
        if (!copyFileStatus) {
            HILOG_ERROR("CallLogDataBase CopySqlFromCToE failed!");
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_OPEN, CALLS_DB_NAME, -1,
                                                      "CopySqlFromCToE failed");
        } else {
            HILOG_WARN("CallLogDataBase CopySqlFromCToE succeed!");
            auto obsMgrClient = DataObsMgrClient::GetInstance();
            Uri uri = Uri(CALL_CHANGER_URI);
            ErrCode ret = obsMgrClient->NotifyChange(uri);
            if (ret != ERR_OK) {
                HILOG_ERROR("obsMgrClient->NotifyChange error return %{public}d", ret);
            } else {
                HILOG_INFO("obsMgrClient->NotifyChange succeed return %{public}d, ts = %{public}lld", ret, (long long) time(NULL));
            }
            HILOG_WARN("CopyCallLogFromCToE RefreshingTelephonyCallLog");
        }
        store_ = storeForE_;
    });
    thread.detach();
}

bool CallLogDataBase::CopyCallLogFromCToE()
{
    bool copyFileStatus = false;
    Contacts::PredicatesConvert predicatesConvert;
    DataShare::DataSharePredicates dataSharePredicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<std::string> columnsTemp;
    std::shared_ptr<OHOS::NativeRdb::ResultSet> result;
    rdbPredicates = predicatesConvert.ConvertPredicates(Contacts::CallsTableName::CALLLOG,
        dataSharePredicates); // 查询所有数据
    result = QueryEL1(rdbPredicates, columnsTemp);
    if (result == nullptr) {
        HILOG_ERROR("resultSet is null!");
        return copyFileStatus;
    }
    int querySqlCount;
    result->GetRowCount(querySqlCount);
    result->Close();
    if (querySqlCount == 0) {
        HILOG_WARN("CallLogDataBase CopyCallLogFromCToE querySqlCount = 0");
        copyFileStatus = true;
    } else {
        BoardReportUtil::MigrationReport(0, 0, querySqlCount, "migration start el1 count");
        copyFileStatus = CopyCallLog(querySqlCount);
    }
    return copyFileStatus;
}

bool CallLogDataBase::CopyCallLog(int querySqlCount)
{
    Contacts::PredicatesConvert predicatesConvert;
    DataShare::DataSharePredicates dataSharePredicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<std::string> columnsTemp;
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    int offsetIndex = 0; // 当执行出错时，执行后面的批次
    int batchCount = (querySqlCount % BATCH_MOVE_COUNT) == 0 ? (querySqlCount / BATCH_MOVE_COUNT) :
     (querySqlCount / BATCH_MOVE_COUNT + 1);
    HILOG_WARN("CallLogDataBase CopyCallLog batchCount = %{public}d", batchCount);
    for (int i = 0; i < batchCount; i++) { // 使用for循环进行批次数据迁移
        rdbPredicates.Limit(BATCH_MOVE_COUNT, offsetIndex * BATCH_MOVE_COUNT); // 每批次查询1000条数据
        rdbPredicates = predicatesConvert.ConvertPredicates(Contacts::CallsTableName::CALLLOG,
            dataSharePredicates);
        resultSet = QueryEL1(rdbPredicates, columnsTemp);
        if (resultSet == nullptr) {
            HILOG_ERROR("CallLogDataBase CopyCallLog result is null, i = %{public}d", i);
            offsetIndex++; // 执行失败，跳过当前批次，执行下一个批次
            continue;
        }
        bool copyFileStatus = ResultSetToValuesBucket(resultSet); // 执行成功时，会删除el1数据
        if (!copyFileStatus) {
            HILOG_ERROR("CallLogDataBase CopyCallLog resultToValuesBucket failed, i = %{public}d", i);
            offsetIndex++; // 执行失败，跳过当前批次，执行下一个批次
        }
        resultSet->Close();
    }
    return true;
}

bool CallLogDataBase::ResultSetToValuesBucket(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
{
    bool copyFileStatus = true;
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    std::vector<OHOS::NativeRdb::ValuesBucket> vectorQueryData;
    std::vector<int> callLogIdList;
    int resultSetNum = resultSet->GoToFirstRow();  // 将查询结果集的位置设置到第一条记录
    OHOS::NativeRdb::ValuesBucket insertValues;
    unsigned int size = columnNames.size();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        OHOS::NativeRdb::ValuesBucket valuesBucketElement;
        unsigned int i = 0;
        for (i = 0; i < size; i++) {
            std::string typeValue = columnNames[i];
            int columnIndex = 0;
            resultSet->GetColumnIndex(typeValue, columnIndex);
            int ret = std::strcmp(typeValue.c_str(), "id");
            OHOS::NativeRdb::ColumnType columnType;
            resultSet->GetColumnType(columnIndex, columnType);
            if (ret == 0) {
                int intValue = 0;
                resultSet->GetInt(columnIndex, intValue);
                callLogIdList.push_back(intValue); // 记录id，批量删除
                HILOG_WARN("CallLogDataBase ResultSetToValuesBucket id = %{public}d", intValue);
                continue;
            }
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
    std::vector<OHOS::NativeRdb::ValuesBucket> unprocessedValues;
    PrivacyContactsManager::GetInstance()->ProcessPrivacyCallLog(vectorQueryData, unprocessedValues);
    copyFileStatus = HandleInsertAndDelete(unprocessedValues, callLogIdList);
    return copyFileStatus;
}

bool CallLogDataBase::HandleInsertAndDelete(std::vector<OHOS::NativeRdb::ValuesBucket> &vectorQueryData,
    std::vector<int> &callLogIdList)
{
    bool copyFileStatus = false;
    int ret = OHOS::NativeRdb::E_ERROR;
    if (storeForE_ == nullptr) {
        HILOG_ERROR("CallLogDataBase HandleInsertAndDelete storeForE_ is null!");
        return copyFileStatus;
    }
    ret = storeForE_->BeginTransaction(); // 开启E库的事物
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase HandleInsertAndDelete BeginTransaction fail: %{public}d", ret);
        // 增加重试机制
        std::this_thread::sleep_for(std::chrono::milliseconds(RBD_STORE_RETRY_REQUEST_WITH_SLEEP_TIMES));
        ret = storeForE_->BeginTransaction();
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("Retry BeginTransaction fail: %{public}d", ret);
            return copyFileStatus;
        }
    }
    int64_t outRowId = RDB_EXECUTE_FAIL;
    ret = storeForE_->BatchInsert(outRowId, CallsTableName::CALLLOG, vectorQueryData); // 数据库批量插入E库
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase HandleInsertAndDelete BatchInsert fail, ret: %{public}d, outRowId: %{public}lld",
            ret, (long long) outRowId);
        storeForE_->RollBack(); // E类批量插入回滚
        BoardReportUtil::MigrationReport(0, ret, outRowId, "migration el5 fail");
        return copyFileStatus;
    }
    BoardReportUtil::MigrationReport(0, ret, outRowId, "migration finish el5 count");
    HILOG_WARN("CallLogDataBase HandleInsertAndDelete BatchInsert outRowId: %{public}lld", (long long) outRowId);
    copyFileStatus = BatchDeleteCallLog(callLogIdList); // 批量删除C库数据
    if (!copyFileStatus) {
        HILOG_ERROR("CallLogDataBase HandleInsertAndDelete BatchDeleteCallLog fail");
        storeForE_->RollBack(); // E类批量插入回滚
    } else {
        HILOG_WARN("CallLogDataBase HandleInsertAndDelete succeess");
        storeForE_->Commit(); // 提交E库批量成功插入事务
    }
    vectorQueryData.clear();
    callLogIdList.clear();
    return copyFileStatus;
}

bool CallLogDataBase::BatchDeleteCallLog(std::vector<int> &callLogIdList)
{
    if (storeForC_ == nullptr) {
        HILOG_ERROR("CallLogDataBase BatchDeleteCallLog storeForC_ is  nullptr");
        BoardReportUtil::DeleteCallLogReport(ExecuteResult::FAIL, 0, "BatchDelete store is nullptr");
        return false;
    }
    int ret = storeForC_->BeginTransaction(); // 开启C库批量删除事物
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase BatchDeleteCallLog BeginTransaction fail, ret: %{public}d", ret);
        std::string failInfo = "BatchDelete beginTransaction fail ret:" + std::to_string(ret);
        BoardReportUtil::DeleteCallLogReport(ExecuteResult::FAIL, 0, failInfo);
        return false;
    }
    for (unsigned long i = 0; i < callLogIdList.size(); i++) {
        DataShare::DataSharePredicates dataSharePredicates;
        Contacts::PredicatesConvert predicatesConvert;
        OHOS::NativeRdb::RdbPredicates rdbPredicates("");
        dataSharePredicates.EqualTo("id", callLogIdList[i]); // 将查询的数据Id作为查询条件
        rdbPredicates = predicatesConvert.ConvertPredicates(Contacts::CallsTableName::CALLLOG,
            dataSharePredicates);
        int deleteRow;
        ret = storeForC_->Delete(deleteRow, rdbPredicates);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("CallLogDataBase BatchDeleteCallLog Delete ret :%{public}d", ret);
            storeForC_->RollBack(); // C库删除失败回滚
            std::string failInfo = "BatchDelete delete fail ret:" + std::to_string(ret);
            BoardReportUtil::DeleteCallLogReport(ExecuteResult::FAIL, 0, failInfo);
            return false;
        }
    }
    HILOG_WARN("CallLogDataBase BatchDeleteCallLog success, delCount: %{public}zu", callLogIdList.size());
    storeForC_->Commit();
    BoardReportUtil::DeleteCallLogReport(ExecuteResult::SUCCESS, callLogIdList.size());
    return true;
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> CallLogDataBase::QueryEL1(
    OHOS::NativeRdb::RdbPredicates &predicates, std::vector<std::string> columns)
{
    if (storeForC_ == nullptr) {
        HILOG_ERROR("CallLogDataBase QueryEL1 storeForC_ is nullptr");
        return nullptr;
    }
    HILOG_WARN("CallLogDataBase QueryEL1");
    return storeForC_->Query(predicates, columns);
}

} // namespace Contacts
} // namespace OHOS