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
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <map>
#include "common_tool_type.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_client.h"

#ifdef CELLULAR_DATA_SUPPORT
#include "cellular_data_client.h"
#endif

#include "account_manager.h"
#include "async_task.h"
#include "blocklist_database.h"
#include "board_report_util.h"
#include "calllog_common.h"
#include "common.h"
#include "contacts.h"
#include "contacts_account.h"
#include "contacts_columns.h"
#include "contacts_common.h"
#include "contacts_datashare_stub_impl.h"
#include "contacts_json_utils.h"
#include "contacts_string_utils.h"
#include "contacts_type.h"
#include "contacts_update_helper.h"
#include "core_service_client.h"
#include "hilog_wrapper.h"
#include "locale_config.h"
#include "match_candidate.h"
#include "merger_contacts.h"
#include "predicates_convert.h"
#include "raw_contacts.h"
#include "rdb_types.h"
#include "rdb_common.h"
#include "rdb_utils.h"
#include "character_transliterate.h"
#include "construction_name.h"
#include "datashare_helper.h"
#include "sql_analyzer.h"
#include "rdb_store_config.h"
#include "spam_call_adapter.h"
#include "security_label.h"
#include "contacts_json_utils.h"
#include "contacts_data_ability.h"
#include "os_account_manager.h"
#include "contacts_manager.h"
#include "system_sound_manager.h"
#include "contacts_string_utils.h"
#include "privacy_contacts_manager.h"
#include "number_identity_helper.h"
#include "phone_number_utils.h"
#include "hi_audit.h"
#include "poster_call_adapter.h"

#ifdef ABILITY_CUST_SUPPORT
#include "dlfcn.h"
#endif

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace Contacts {
std::shared_ptr<ContactsDataBase> ContactsDataBase::contactDataBase_ = nullptr;
std::shared_ptr<CallLogDataBase> ContactsDataBase::callLogDataBase_ = nullptr;
std::shared_ptr<BlocklistDataBase> ContactsDataBase::blocklistDataBase_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> ContactsDataBase::store_ = nullptr;
std::shared_ptr<Media::SystemSoundManager> SystemSoundManager_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> ContactsDataBase::doubleContactDbStore_ = nullptr;

std::shared_ptr<OHOS::NativeRdb::RdbStore> ContactsDataBase::contactStore_ = nullptr;
std::shared_ptr<ContactsObserver> ContactsDataBase::observer_ = nullptr;
std::shared_ptr<OHOS::Contacts::AutoSyncDetailProgressObserver> ContactsDataBase::autoSyncDetailProgressObserver_ =
    nullptr;
std::shared_ptr<ContactConnectAbility> ContactsDataBase::contactsConnectAbility_ = nullptr;
std::shared_ptr<DataShare::DataShareHelper> ContactsDataBase::dataShareHelper_ = nullptr;
volatile bool g_isSyncing = false;
int g_notifyIntervalTime = 5;
// 刷新版本的值
int g_refreshValue = 6;
int g_refreshContactValue = 7;
static int64_t g_lastSyncTime = 0;
volatile int g_retryCount = 0;

static AsyncTaskQueue *g_asyncTaskQueue;
static std::vector<std::string> tablesToSync = {ContactTableName::CLOUD_RAW_CONTACT, ContactTableName::CLOUD_GROUP};
static std::vector<std::string> tablesToSearch = {ContactTableName::CONTACT,
    ContactTableName::RAW_CONTACT,
    ContactTableName::CONTACT,
    ContactTableName::CONTACT_DATA,
    ContactTableName::GROUPS};
static std::string g_databaseName;
static OHOS::DistributedRdb::DistributedConfig distributedConfig;
static VectorWithLock mDirtyRawContacts;
static std::vector<int> mUpdateRawContacts;
static OHOS::DistributedRdb::SubscribeOption option;
static int32_t g_mSyncMode = OHOS::DistributedRdb::DistributedTableType::DISTRIBUTED_CLOUD;

// 资产状态正常
static const int ASSET_NORMAL = 1;

// 每批次uuid云同步数量
static constexpr int CLOUD_SYNC_SIZE = 100;

// 每批次同步等待的最大时间（300s）
static constexpr int SYNC_WAIT_TIME = 5 * 60;

// 有变更，需要上云，但是不需要重新计算 uniqueKey 的联系人，上云结束后设置为 NOT_DIRTY：0
static const int NO_NEED_RECALC = 1;
// 有变更，需要上云，且需要重新计算 uniqueKey 的联系人，计算完 uniqueKey 以后，会设置为 NO_NEED_RECALC
static const int NEED_RECALC = 2;
// 默认黑名单类型 全匹配号码
static const int FULL_MATCH_BLOCK_REASON = 0;
// 开头号码匹配拦截类型黑名单
static const int START_WITH_BLOCK_REASON = 2;
// 通知短信更新全匹配黑名单拦截次数
static constexpr int COMMAND_UPDATE_FULL_MATCH_MSG_COUNT = 2;
// 通知短信更新开头匹配黑名单拦截次数
static constexpr int COMMAND_UPDATE_START_MATCH_MSG_COUNT = 3;
// 黑名单迁移失败重试次数
static constexpr int BLOCKLIST_MOVE_FAILED_RETRY_TIMES = 2;
// 黑名单迁移分页大小
static constexpr int BLOCKLIST_MOVE_PAGE_SIZE = 200;
// 在类外初始化静态成员变量
static const std::regex pattern("\\s");
// 匹配电话号码中的横杠格式化
static const std::regex percent("\\%");
// 刷新号码归属地，失败重试次数
static constexpr int REFRESH_LOCATION_RETRY_NUMBER = 2;
// 云空间满同步触发时间
static constexpr int64_t SYNC_CONTACT_MILLISECOND = 3 * 60 * 60 * 1000;
// 联系人数据库
static const std::string CONTACTS_DB = "contacts.db";

#ifdef ABILITY_CUST_SUPPORT
static const unsigned int ENHANCED_QUERY_LENGTH = 7;
static const std::string TELEPHONY_CUST_SO_PATH = "libtelephony_cust_api.z.so";
#endif

namespace {
std::mutex g_mtx;
std::mutex g_mutexUpdateTimeStamp;
std::mutex g_mutex;
std::mutex g_mutexInit;
}  // namespace

ContactsDataBase::ContactsDataBase()
{
    // 注册联系人事件监听
    OHOS::Contacts::ContactManager::GetInstance();
    // 黑名单数据库开库
    blocklistDataBase_ = BlocklistDataBase::GetInstance();
    if (MoveDbFile()) {  // 如果db文件位于老的不规范目录，则迁移至规范的rdb目录
        HILOG_INFO("ContactsDataBase Database file moved successfully, ts = %{public}lld", (long long) time(NULL));
    }
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    // 调用rdb方法拼接数据库路径
    g_databaseName = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
        ContactsPath::RDB_PATH, "contacts.db", getDataBasePathErrCode);
    HILOG_WARN(
        "ContactsDataBase getDataBasePathErrCode = %{public}d, ts = %{public}lld", getDataBasePathErrCode, (long long) time(NULL));
    if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase GetDefaultDatabasePath err :%{public}d,"
                    "ts = %{public}lld",
            getDataBasePathErrCode,
            (long long) time(NULL));
        return;
    }
    // 老的双升单用户错误设置了 s1 标签，通过文件管理接口修复标签等级才能成功开库
    int SetSecurityLabelCode =
        FileManagement::ModuleSecurityLabel::SecurityLabel::SetSecurityLabel(g_databaseName, "s3");
    HILOG_INFO("ContactsDataBase SetSecurityLabel s3 retCode=%{public}d", SetSecurityLabelCode);
    // 设置非自动同步
    distributedConfig.autoSync = false;
    // 设置头像支持异步下载
    distributedConfig.asyncDownloadAsset = true;
    int errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(g_databaseName);
    config.SetBundleName("com.ohos.contactsdataability");
    config.SetName("contacts.db");
    config.SetArea(1);
    // 数据注入 arkdata 与 融合搜索
    config.SetSearchable(true);
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S3);
    // 数据损坏时，rdb 会自动重建新库
    config.SetAllowRebuild(true);
    // 配置加删除，账号退出时，会设置cloud_raw_data假删除
    // 如果此参数为true，账号退出，会直接真删除cloud_raw_data数据
    config.SetAutoClean(false);
    SqliteOpenHelperContactCallback sqliteOpenHelperCallback;
    HILOG_WARN("ContactsDataBase create getRdbStore start, ts = %{public}lld", (long long) time(NULL));
    contactStore_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(
        config, DATABASE_CONTACTS_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    // 如果开库成功
    if (contactStore_ != nullptr) {
        auto schemaVersion = contactStore_->ExecuteSql("pragma schema_version;");
        HILOG_WARN("ContactsDataBase GetRdbStore errCode:%{public}d,initDB end, contact db version is:%{public}d,"
                   "schemaVersion is %{public}d,ts = %{public}lld",
            errCode,
            DATABASE_CONTACTS_OPEN_VERSION,
            schemaVersion,
            (long long) time(NULL));
        auto rebuilt = OHOS::NativeRdb::RebuiltType::NONE;
        int rebuiltCode = contactStore_->GetRebuilt(rebuilt);
        HILOG_INFO("ContactsDataBase rebuilt retCode= %{public}d,rebuilt= %{public}u", rebuiltCode, rebuilt);
        if (rebuilt == OHOS::NativeRdb::RebuiltType::REBUILT && rebuiltCode == OHOS::NativeRdb::E_OK) {
            // 如果rdb帮我们重建了新库，则调用 rdb 恢复接口，从备份文件恢复数据
            int restoreCode = contactStore_->Restore("contacts.db.bak");
            HILOG_ERROR("ContactsDataBase Restore retCode= %{public}d", restoreCode);
        }
        // 开库表和字段检查
        CheckIncompleteTableForContact(contactStore_);
        CheckAndCompleteFieldsForContact(contactStore_);
        CheckIncompleteViewForContact(contactStore_);
    } else {
        HILOG_ERROR(
            "ContactsDataBase GetRdbStore err, contactStore_ is null, skip rebuilt, ts = %{public}lld", (long long) time(NULL));
    }
    // 开库失败则直接返回
    if (errCode != OHOS::NativeRdb::E_OK || contactStore_ == nullptr) {
        HILOG_ERROR("ContactsDataBase open error :%{public}d, ts = %{public}lld", errCode, (long long) time(NULL));
        return;
    }
    store_ = contactStore_;
    HILOG_WARN("ContactsDataBase start thread, ts = %{public}lld", (long long) time(NULL));
    std::thread thread([this]() {
        HILOG_INFO("ContactsDataBase create SetDistributedTables start, ts = %{public}lld", (long long) time(NULL));
        // 设置哪些表需要进行端云同步，同步配置是啥样
        int setDistributedResultcode = store_->SetDistributedTables(tablesToSync, g_mSyncMode, distributedConfig);
        // 取每次开库的时间，判断云空间满的场景
        g_lastSyncTime = std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
        HILOG_INFO("ContactsDataBase  SetDistributedTables  %{public}d, ts = %{public}lld",
            setDistributedResultcode,
            (long long) time(NULL));
        if (observer_ == nullptr) {
            observer_ = OHOS::Contacts::ContactsObserver::GetInstance();
        }
        option.mode = OHOS::DistributedRdb::SubscribeMode::CLOUD_DETAIL;
        // 监听云表数据变更
        int resultCode = store_->Subscribe(option, observer_);
        if (resultCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("RdbStoreProxy::OnDataChangeEvent: subscribe failed :%{public}d", resultCode);
        }
        if (autoSyncDetailProgressObserver_ == nullptr) {
            autoSyncDetailProgressObserver_ = OHOS::Contacts::AutoSyncDetailProgressObserver::GetInstance();
        }
        // 设置云表同步结果回调
        int status = store_->RegisterAutoSyncCallback(autoSyncDetailProgressObserver_);
        HILOG_WARN(
            "ContactsDataBase RegisterAutoSyncCallback status = %{public}d, ts = %{public}lld", status, (long long) time(NULL));

        // 维测日志，打印本地联系人表 本地云表联系人数量，以及问题联系人数量
        // js 测 checkCursor 里打印，此处不再调用 QueryCloudContactsCount();
        // 以前开库刷新脏数据的函数，现在用于触发 js 测进行 cursor 与其他初始化逻辑
        // 冷启动，开库，调用到checkCursor
        RefreshContacts("coldBoot");
        // uuid置空
        // 去除将uuid置为null的处理
        // 移除 UpdateUUidNull();
        // 设置端云表成功以后，手动触发云空间 与本地云表同步一致
        SyncContacts();
        // 异步填充存量联系人归属地
        NumberLocationRefresh();
    });
    thread.detach();
    HILOG_WARN("ContactsDataBase start thread end, ts = %{public}lld", (long long) time(NULL));
    // 通话记录数据库开库
    callLogDataBase_ = CallLogDataBase::GetInstance();

    // 预置联系人账户 与 详情类型数据
    std::shared_ptr<ContactsAccount> contactsAccount = ContactsAccount::GetInstance();
    contactsAccount->PrepopulateCommonAccountTypes(store_);
    ContactsType contactsType;
    contactsType.PrepopulateCommonTypes(store_);
    // 初始化异步任务队列
    g_asyncTaskQueue = AsyncTaskQueue::Instance();
    MoveBlocklistDataAsyncTask();
}

void ContactsDataBase::CheckIncompleteTableForContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
{
    SqlAnalyzer sqlAnalyzer;
    int32_t ret = OHOS::NativeRdb::E_ERROR;
    if (rdbStore == nullptr) {
        HILOG_ERROR("ContactsDataBase CheckIncompleteTable rdbStore is nullptr.");
        return;
    }
    std::vector<std::string> tables = sqlAnalyzer.QueryAllTables(*rdbStore);
    for (const auto &tablePair : CONTACT_TABLES) {
        if (std::find(tables.begin(), tables.end(), tablePair.first) == tables.end()) {
            HILOG_WARN("ContactsDataBase CheckIncompleteTable not exist table %{public}s, try to create table.",
                       tablePair.first.c_str());
            ret = rdbStore->ExecuteSql(tablePair.second);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("ContactsDataBase CheckIncompleteTable create table %{public}s error: %{public}d.",
                    tablePair.first.c_str(), ret);
            }
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_TABLES_CHECK, CONTACTS_DB, ret,
                                                      tablePair.first);
        }
    }
}

void ContactsDataBase::CheckIncompleteViewForContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
{
    SqlAnalyzer sqlAnalyzer;
    int32_t ret = OHOS::NativeRdb::E_ERROR;
    if (rdbStore == nullptr) {
        HILOG_ERROR("ContactsDataBase CheckIncompleteView rdbStore is nullptr.");
        return;
    }
    std::vector<std::string> views = sqlAnalyzer.QueryAllViews(*rdbStore);
    for (const auto &viewPair : CONTACT_VIEWS) {
        if (std::find(views.begin(), views.end(), viewPair.first) == views.end()) {
            HILOG_WARN("ContactsDataBase CheckIncompleteView not exist view %{public}s, try to create view.",
                       viewPair.first.c_str());
            ret = rdbStore->ExecuteSql(viewPair.second);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("ContactsDataBase CheckIncompleteView create view %{public}s error: %{public}d.",
                    viewPair.first.c_str(), ret);
            }
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_TABLES_CHECK, CONTACTS_DB, ret,
                                                      viewPair.first);
        }
    }
}

void ContactsDataBase::CheckAndCompleteFieldsForContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
{
    SqlAnalyzer sqlAnalyzer;
    int32_t ret = OHOS::NativeRdb::E_ERROR;
    if (rdbStore == nullptr) {
        HILOG_ERROR("ContactsDataBase CheckAndCompleteFields rdbStore is nullptr.");
        return;
    }
    std::vector<std::string> rawFields = sqlAnalyzer.QueryAllFieldsFromTable(*rdbStore, ContactTableName::RAW_CONTACT);
    for (const auto &columnPair : RAW_CONTACT_ADD_COLUMNS) {
        if (std::find(rawFields.begin(), rawFields.end(), columnPair.first) == rawFields.end()) {
            HILOG_WARN(
                "ContactsDataBase CheckAndCompleteFields not exist field %{public}s in table %{public}s, try to add.",
                columnPair.first.c_str(), ContactTableName::RAW_CONTACT);
            ret = rdbStore->ExecuteSql(columnPair.second);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR(
                    "ContactsDataBase CheckIncompleteTable add field %{public}s in table %{public}s error: %{public}d.",
                    columnPair.first.c_str(), ContactTableName::RAW_CONTACT, ret);
            }
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_COLUMNS_CHECK, CONTACTS_DB, ret,
                                                      columnPair.first);
        }
    }

    std::vector<std::string> contactFields = sqlAnalyzer.QueryAllFieldsFromTable(*rdbStore, ContactTableName::CONTACT);
    for (const auto &columnPair : CONTACT_ADD_COLUMNS) {
        if (std::find(contactFields.begin(), contactFields.end(), columnPair.first) == contactFields.end()) {
            HILOG_WARN(
                "ContactsDataBase CheckAndCompleteFields not exist field %{public}s in table %{public}s, try to add.",
                columnPair.first.c_str(), ContactTableName::CONTACT);
            ret = rdbStore->ExecuteSql(columnPair.second);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR(
                    "ContactsDataBase CheckIncompleteTable add field %{public}s in table %{public}s error: %{public}d.",
                    columnPair.first.c_str(), ContactTableName::CONTACT, ret);
            }
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_COLUMNS_CHECK, CONTACTS_DB, ret,
                                                      columnPair.first);
        }
    }

    std::vector<std::string> contactDatafields =
        sqlAnalyzer.QueryAllFieldsFromTable(*rdbStore, ContactTableName::CONTACT_DATA);
    for (const auto &columnPair : CONTACT_DATA_ADD_COLUMNS) {
        if (std::find(contactDatafields.begin(), contactDatafields.end(), columnPair.first) == contactDatafields.end()) {
            HILOG_WARN(
                "ContactsDataBase CheckAndCompleteFields not exist field %{public}s in table %{public}s, try to add.",
                columnPair.first.c_str(), ContactTableName::CONTACT_DATA);
            ret = rdbStore->ExecuteSql(columnPair.second);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR(
                    "ContactsDataBase CheckIncompleteTable add field %{public}s in table %{public}s error: %{public}d.",
                    columnPair.first.c_str(), ContactTableName::CONTACT_DATA, ret);
            }
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_COLUMNS_CHECK, CONTACTS_DB, ret,
                                                      columnPair.first);
        }
    }
}

void ContactsDataBase::MoveBlocklistDataAsyncTask()
{
    std::unique_ptr<AsyncItem> task = std::make_unique<AsyncBlocklistMigrateTask>();
    g_asyncTaskQueue->Push(task);
    g_asyncTaskQueue->Start();
}
// 联系人db内的黑名单数据需要移动到el1下的黑名单db内
bool ContactsDataBase::MoveBlocklistData()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase MoveBlocklistData store_ is nullptr, ts = %{public}lld", (long long) time(NULL));
        return false;
    }
    bool moveStatus = false;
    if (IsMoveBlocklist()) {
        int moveTimes = 0;
        while (moveTimes <= BLOCKLIST_MOVE_FAILED_RETRY_TIMES) { // 迁移失败重试2次
            HILOG_WARN("ContactsDataBase MoveBlocklistData Times is: %{public}d ", moveTimes);
            std::string queryBlocklistCount = "select count(*) from contact_blocklist";
            auto resultSet = store_->QuerySql(queryBlocklistCount);
            if (resultSet == nullptr) {
                HILOG_ERROR("MoveBlocklistData QuerySqlResult is null");
                moveTimes++;
                continue;
            }
            int blocklistCount = 0;
            if (resultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
                resultSet->GetInt(0, blocklistCount);
            } else { // 查询失败，跳出循环重试
                HILOG_WARN("ContactsDataBase MoveBlocklistData queryBlocklistCount faild!");
                moveTimes++;
                continue;
            }
            resultSet->Close();
            if (blocklistCount == 0) { // 查询成功，没有需要迁移的数据，跳出循环
                HILOG_WARN("ContactsDataBase MoveBlocklistData no need migrate data.");
                moveStatus = true;
                break;
            }
            moveStatus = MoveBlocklistByPage(blocklistCount);
            if (moveStatus) { // 迁移成功跳出循环
                break;
            } else { // 迁移失败，迁移次数自增，继续尝试迁移
                moveTimes++;
                continue;
            }
        }
        if (moveStatus) {
            UpdateBlocklistMigrateStatus(BLOCKLIST_MIGRATE_SUCCESS);
        } else {
            UpdateBlocklistMigrateStatus(BLOCKLIST_MIGRATE_FAILED);
            return false;
        }
    }
    return true;
}

bool ContactsDataBase::MoveBlocklistByPage(int blocklistCount)
{
    HILOG_WARN("ContactsDataBase MoveBlocklistData need migrate data size is: %{public}d ", blocklistCount);
    for (int i = 0; i < blocklistCount; i += BLOCKLIST_MOVE_PAGE_SIZE) { // 分页查询并迁移
        std::string queryPageForBlocklist =
            "SELECT * FROM contact_blocklist LIMIT 200 OFFSET " + std::to_string(i);
        std::shared_ptr<OHOS::NativeRdb::ResultSet> pageResultSet;
        pageResultSet = store_->QuerySql(queryPageForBlocklist);
        if (pageResultSet == nullptr) {
            HILOG_ERROR("ContactsDataBase MoveBlocklistData Query pageResultSet is null!");
            return false;
        } else {
            bool insertBlocklistResult;
            int querySqlCount = 0;
            insertBlocklistResult = ResultSetToValuesBucket(pageResultSet, querySqlCount);
            if (!insertBlocklistResult) {
                HILOG_ERROR("ContactsDataBase MoveBlocklistData insertBlocklistResult is faild!");
                return false;
            }
        }
    }
    return true;
}

bool ContactsDataBase::IsMoveBlocklist()
{
    // 先获取setting表的blocklist_migrate_status字段
    std::string queryBlocklistMigrateStatus = "SELECT blocklist_migrate_status FROM settings";
    auto resultSet = store_->QuerySql(queryBlocklistMigrateStatus);
    if (resultSet == nullptr) {
        HILOG_ERROR("IsMoveBlocklist QuerySqlResult is null");
        return false;
    }
    int getRowResult = resultSet->GoToFirstRow();
    std::string blocklist_migrate_status = SettingsColumns::BLOCKLIST_MIGRATE_STATUS;
    int migrateStatusNum = 0;
    if (getRowResult == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        resultSet->GetColumnIndex(blocklist_migrate_status, columnIndex);
        resultSet->GetInt(columnIndex, migrateStatusNum);
    }
    resultSet->Close();
    if (migrateStatusNum == BLOCKLIST_MIGRATE_SUCCESS) {
        return false;
    } else {
        return true;
    }
}

int ContactsDataBase::UpdateBlocklistMigrateStatus(int blocklist_migrate_status)
{
    // 更新blocklist_migrate_status字段
    std::string UpdateBlocklistMigrateStatusSql =
      "UPDATE settings SET blocklist_migrate_status = " + std::to_string(blocklist_migrate_status);
    int ret = store_->ExecuteSql(UpdateBlocklistMigrateStatusSql);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("update blocklist_migrate_status failed, ret:%{public}d", ret);
    }
    return ret;
}

bool ContactsDataBase::ResultSetToValuesBucket(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    int &querySqlCount)
{
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    int resultSetNum = resultSet->GetRowCount(querySqlCount); // 获取当页查询结果数量并打印日志
    HILOG_WARN("ContactsDataBase ResultSetToValuesBucket querySqlCount = %{public}d", querySqlCount);
    if (resultSetNum != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase ResultSetToValuesBucket GetRowCount failed!");
        return false;
    }
    resultSetNum = resultSet->GoToFirstRow();  // 将查询结果集的位置设置到第一条记录
    bool blocklistCopyResult = SqlDataCopy(resultSetNum, resultSet, columnNames);
    HILOG_WARN("ContactsDataBase ResultSetToValuesBucket blocklistCopyResult = %{public}d", blocklistCopyResult);
    resultSet->Close();
    return blocklistCopyResult;
}

bool ContactsDataBase::SqlDataCopy(int resultSetNum,
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
    std::vector<std::string> columnNames)
{
    bool blocklistCopyResult = true;
    OHOS::NativeRdb::ValuesBucket insertValues;
    unsigned int size = columnNames.size();
    std::vector<OHOS::NativeRdb::ValuesBucket> batchInsertData;
    std::vector<std::string> blocklistIds;
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
                std::string idTemp = "";
                resultSet->GetString(columnIndex, idTemp);
                blocklistIds.push_back(idTemp);
                continue;
            }
            if (columnType == OHOS::NativeRdb::ColumnType::TYPE_INTEGER) {
                int64_t intValue = 0;
                resultSet->GetLong(columnIndex, intValue);
                valuesBucketElement.PutLong(typeValue, intValue);
            } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_STRING) {
                std::string stringValue;
                resultSet->GetString(columnIndex, stringValue);
                valuesBucketElement.PutString(typeValue, stringValue);
            }
        }
        batchInsertData.push_back(valuesBucketElement);
        resultSetNum = resultSet->GoToNextRow();
    }
    blocklistCopyResult =  SqlDataChange(batchInsertData, blocklistIds);
    return blocklistCopyResult;
}

bool ContactsDataBase::SqlDataChange(std::vector<OHOS::NativeRdb::ValuesBucket> batchInsertData,
    std::vector<std::string> blocklistIds)
{
    // 开启事务
    blocklistDataBase_->BeginTransaction();
    store_->BeginTransaction();
    // 黑名单插入新表
    int64_t outRowId = OHOS::NativeRdb::E_OK;
    int insertResult = BlocklistDataBase::store_->BatchInsert(
        outRowId, ContactTableName::CONTACT_BLOCKLIST, batchInsertData);
    auto rdbPredicates = OHOS::NativeRdb::RdbPredicates(ContactTableName::CONTACT_BLOCKLIST);
    rdbPredicates.In(ContactBlockListColumns::ID, blocklistIds);
    std::string blocklistIdsStr = "";
    for (const auto& str : blocklistIds) {
        blocklistIdsStr = blocklistIdsStr + str + ",";
    }
    HILOG_WARN("ContactsDataBase moveBlocklist delete old table size:%{public}ld blocklistIds %{public}s",
        (long) blocklistIds.size(), blocklistIdsStr.c_str());
    // 删除旧表黑名单数据
    int deleteResult = DeleteEL2Blacklist(rdbPredicates);
    // 全部成功，则成功，有一个失败则回滚
    if (insertResult == OHOS::NativeRdb::E_OK && deleteResult == OHOS::NativeRdb::E_OK) {
        blocklistDataBase_->Commit();
        store_->Commit();
        return true;
    } else {
        HILOG_WARN("ContactsDataBase moveBlocklist filed! insertResult: %{public}d; deleteResult: %{public}d",
            insertResult, deleteResult);
        blocklistDataBase_->RollBack();
        store_->RollBack();
        return false;
    }
}

int64_t ContactsDataBase::DeleteEL2Blacklist(OHOS::NativeRdb::RdbPredicates &predicates)
{
    int deleteRow;
    int ret = store_->Delete(deleteRow, predicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("BlocklistDatabase DeleteEL1CallLog ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

int ContactsDataBase::QueryContactCountDoubleDb()
{
    int contactCount = 0;
    if (doubleContactDbStore_ == nullptr) {
        // 判断双框架的库是否存在，不存在的话，返回0
        std::ifstream f(ContactsPath::DOUBLE_DB_FILE_PATH.c_str());
        HILOG_WARN("QueryContactCountDoubleDb doubleDb file is exist = %{public}s, , ts = %{public}lld",
            std::to_string(f.good()).c_str(),
            (long long) time(NULL));
        if (!f.good()) {
            HILOG_WARN("QueryContactCountDoubleDb doubleDb file not exist, ts = %{public}lld", (long long) time(NULL));
            return contactCount;
        }
        int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
        std::string doubledatabaseName = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
            ContactsPath::RDB_DOUBLE_DB_PATH, ContactsPath::DOUBLE_DB_NAME, getDataBasePathErrCode);
        HILOG_WARN(
            "QueryContactCountDoubleDb getDataBasePathErrCode = %{public}d, ts = %{public}lld",
            getDataBasePathErrCode, (long long) time(NULL));
        HILOG_WARN(
            "QueryContactCountDoubleDb ContactsPath::RDB_DOUBLE_DB_PATH = %{public}s, RDB_PATH = %{public}s",
                doubledatabaseName.c_str(), ContactsPath::RDB_PATH.c_str());
        if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("QueryContactCountDoubleDb GetDefaultDatabasePath err :%{public}d,"
                        "ts = %{public}lld",
                getDataBasePathErrCode,
                (long long) time(NULL));
            return contactCount;
        }
        EmptyOpenCallback emptyOpenCallback;
        OHOS::NativeRdb::RdbStoreConfig config(doubledatabaseName);
        config.SetBundleName("com.ohos.contactsdataability");
        config.SetName(ContactsPath::DOUBLE_DB_NAME);
        config.SetArea(1);
        config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S1);
        int errCode = OHOS::NativeRdb::E_OK;
        doubleContactDbStore_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(
            config, 1, emptyOpenCallback, errCode);
        // 开库失败则直接返回
        if (errCode != OHOS::NativeRdb::E_OK || doubleContactDbStore_ == nullptr) {
            HILOG_ERROR("QueryContactCountDoubleDb open store error :%{public}d, ts = %{public}lld",
                errCode, (long long) time(NULL));
            return contactCount;
        }
    }

    contactCount = QueryContactCountDoubleDbByStore();
    return contactCount;
}

int ContactsDataBase::QueryContactCountDoubleDbByStore()
{
    int contactCount = 0;
    if (doubleContactDbStore_ == nullptr) {
        return contactCount;
    }
    // 查询account_id信息
    std::string queryAccountIdSql = "select _id from accounts where account_name = 'Phone'";
    auto resultSetAccount = doubleContactDbStore_->QuerySql(queryAccountIdSql);
    if (resultSetAccount == nullptr) {
        HILOG_ERROR("QueryContactCountDoubleDbByStore QuerySqlResult is null");
        return contactCount;
    }
    int accountId = 1;
    int getRowResultAccount = resultSetAccount->GoToFirstRow();
    if (getRowResultAccount == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        resultSetAccount->GetInt(columnIndex, accountId);
    } else {
        HILOG_WARN("QueryContactCountDoubleDb accountId not has value");
    }
    resultSetAccount->Close();

    HILOG_WARN("QueryContactCountDoubleDb accountId %{public}d",
        accountId);

    std::string querySql = "SELECT count( _id ) FROM contacts WHERE _id IN ( SELECT DISTINCT contact_id FROM "
                           "raw_contacts WHERE deleted = 0 AND account_id = " +
                           std::to_string(accountId) + " );";
    auto resultSet = doubleContactDbStore_->QuerySql(querySql);
    if (resultSet == nullptr) {
        HILOG_ERROR("doubleContactDbStore QuerySqlResult is null");
        return contactCount;
    }
    int getRowResult = resultSet->GoToFirstRow();
    if (getRowResult == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        resultSet->GetInt(columnIndex, contactCount);
    } else {
        HILOG_WARN("QueryContactCountDoubleDb select count not has value");
    }
    resultSet->Close();

    HILOG_WARN("QueryContactCountDoubleDb contactCount %{public}d",
        contactCount);
    return contactCount;
}

bool GetBundleNameByUid(int32_t uid, std::string &bundleName)
{
    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smgr == nullptr) {
        HILOG_ERROR("ContactsDataBase GetBundleNameByUid smgr is nullptr");
        return false;
    }
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("ContactsDataBase GetBundleNameByUid remoteObject is nullptr");
        return false;
    }
    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContactsDataBase GetBundleNameByUid bundleMgr is nullptr");
        return false;
    }
    int32_t error = bundleMgr->GetNameForUid(uid, bundleName);
    if (error != ERR_OK) {
        HILOG_WARN("ContactsDataBase GetBundleNameByUid fail,error:%{public}d,uid:%{public}d,bundleName:%{public}s",
            error,
            uid,
            bundleName.c_str());
        return false;
    }
    return true;
}

std::string ContactsDataBase::getCallingBundleName()
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingBundleName;
    if (!GetBundleNameByUid(uid, callingBundleName)) {
        HILOG_ERROR("ContactsDataShareStubImpl Insert getbundleName faild");
    }
    return callingBundleName;
}

// 通过开库操作，对融合搜的搜索表，清理，然后重建索引
int ContactsDataBase::clearAndReCreateTriggerSearchTable()
{
    // 参数1：设置非自动同步
    // 参数3：isRebuild  删表之后设置成 true，表示是重建数据
    OHOS::DistributedRdb::DistributedConfig clearSearchTableDistributedConfig = {false, {}, true};
    store_ = contactStore_;
    HILOG_INFO("ContactsDataBase cleAndReCreateTriggerST create SetDistributedTables start, "
               "ts = %{public}lld",
        (long long) time(NULL));
    // 设置哪些表需要进行端云同步，同步配置是啥样
    int setDistributedResultcode = store_->SetDistributedTables(tablesToSearch,
        OHOS::DistributedRdb::DistributedTableType::DISTRIBUTED_SEARCH,
        clearSearchTableDistributedConfig);
    HILOG_INFO("ContactsDataBase cleAndReCreateTriggerST SetDistributedTables  %{public}d, ts = %{public}lld",
        setDistributedResultcode,
        (long long) time(NULL));
    return setDistributedResultcode;
}

std::string ContactsDataBase::getUriLogPrintByUri(OHOS::Uri uriTemp)
{
    std::string uriStr = uriTemp.ToString();
    std::size_t index = uriStr.rfind("/");
    std::string uriLogPrint = uriStr;
    if (index >= 0 && index < uriStr.length()) {
        uriLogPrint = uriStr.substr(index);
    } else {
        uriLogPrint = uriStr;
    }
    return uriLogPrint;
}

int ContactsDataBase::SetFullSearchSwitch(bool switchStatus)
{
    HILOG_INFO("ContactsDataBase SetFullSearchSwitch start.");
    int result = store_->SetSearchable(switchStatus);
    HILOG_INFO("ContactsDataBase SetFullSearchSwitch end, switchStatus = %{public}d, result = %{public}d",
        switchStatus,
        result);
    return result;
}

int ContactsDataBase::UpdateUUidNull()
{
    // 先获取setting表的refresh_contacts字段
    std::string queryRefreshContacts = "SELECT refresh_contacts FROM settings";
    auto resultSet = store_->QuerySql(queryRefreshContacts);
    if (resultSet == nullptr) {
        HILOG_ERROR("UpdateUUidNull QuerySqlResult is null");
        return RDB_EXECUTE_FAIL;
    }
    int getRowResult = resultSet->GoToFirstRow();
    std::string column_refresh_contacts = SettingsColumns::REFRESH_CONTACTS;
    int refreshContactsNum = 0;
    if (getRowResult == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        resultSet->GetColumnIndex(column_refresh_contacts, columnIndex);
        resultSet->GetInt(columnIndex, refreshContactsNum);
    }
    resultSet->Close();

    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_WARN("UpdateUUidNull BeginTransaction failed, ret:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    // 根据字段值是否小于9判断是否需要更新uuid为null
    // 去除更新uuid为null的处理
    // 移除 if (refreshContactsNum < SettingsColumns::REFRESH_CONTACTS_UUID) {
    // 移除    std::string updateRawUuidNull = "UPDATE raw_contact SET uuid = NULL where uuid not in ("
    // 移除        "select uuid from cloud_raw_contact"
    // 移除        ")";
    // 移除    ret = store_->ExecuteSql(updateRawUuidNull);
    // 移除    if (ret != OHOS::NativeRdb::E_OK) {
    // 移除        HILOG_ERROR("update raw_contact uuid null failed, ret:%{public}d", ret);
    // 移除        RollBack();
    // 移除        return RDB_EXECUTE_FAIL;
    // 移除    }
    // 移除}

    // 更新refresh_contacts字段为9
    std::string updateRefreshContacts = "UPDATE settings SET refresh_contacts = 9";
    ret = store_->ExecuteSql(updateRefreshContacts);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("update refresh_contacts 9 failed, ret:%{public}d", ret);
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    ret = Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateUUidNull failed, ret:%{public}d", ret);
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

void ContactsDataBase::RefreshContacts(std::string paramStr)
{
    HILOG_INFO("RefreshContacts start, ts = %{public}lld", (long long) time(NULL));
    // 调用JS查询cursor变化并进行对应处理
    contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
    contactsConnectAbility_->ConnectAbility("", "", "", "", "checkCursor", paramStr);
}

ContactsDataBase::ContactsDataBase(const ContactsDataBase &)
{}

ContactsDataBase::~ContactsDataBase()
{
    HILOG_WARN("ContactsDataBase  ~ContactsDataBase");
}

bool ContactsDataBase::MoveDbFile()
{
    std::string ContactsDbNamePath = ContactsPath::RDB_PATH + "/contacts.db";
    HILOG_INFO("MoveDbFile ContactsDbNamePath, ts = %{public}lld", (long long) time(NULL));
    char resolvedPath[PATH_MAX];
    if (realpath(ContactsDbNamePath.c_str(), resolvedPath) == nullptr) {
        HILOG_ERROR("ContactsDbNamePath is not valid path");
        return false;
    }
    std::ifstream f(ContactsDbNamePath.c_str());
    HILOG_INFO("MoveDbFile ContactsDb file is exist = %{public}s, ts = %{public}lld",
        std::to_string(f.good()).c_str(),
        (long long) time(NULL));
    if (!f.good()) {
        HILOG_INFO("Database files do not need to be moved, ts = %{public}lld", (long long) time(NULL));
        return false;
    }
    std::string newDbPath = ContactsPath::RDB_PATH + "/rdb/";

    // GetDefaultDatabasePath 内部rdb会自己拼 rdb 中间目录，并创建好需要的目录
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(ContactsPath::RDB_PATH, "contacts.db", getDataBasePathErrCode);
    if (getDataBasePathErrCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("MoveDbFile GetDefaultDatabasePath err :%{public}d, "
                    "ts = %{public}lld",
            getDataBasePathErrCode,
            (long long) time(NULL));
        return false;
    }
    std::vector<std::string> fileNames;
    fileNames.push_back("contacts.db");
    fileNames.push_back("contacts.db-shm");
    fileNames.push_back("contacts.db-wal");
    fileNames.push_back("profile.db");
    fileNames.push_back("profile.db-shm");
    fileNames.push_back("profile.db-wal");
    for (std::vector<std::string>::iterator it = fileNames.begin(); it != fileNames.end(); it++) {
        std::string oldDbNamePath = ContactsPath::RDB_PATH + "/" + *it;
        char realPath[PATH_MAX];
        if (realpath(oldDbNamePath.c_str(), realPath) == nullptr) {
            HILOG_WARN("oldDbNamePath is not valid path");
            continue;
        }
        HILOG_INFO("Database files need to be oldDbNamePath");
        std::ifstream f(oldDbNamePath.c_str());
        if (f.good()) {
            std::string newFilePath = newDbPath + *it;
            HILOG_INFO("MoveDbFile newFilePath");
            std::filesystem::rename(oldDbNamePath, newFilePath);
        }
    }
    return true;
}

std::shared_ptr<ContactsDataBase> ContactsDataBase::GetInstance()
{
    HILOG_INFO("ContactsDataBase GetInstance start.");
    std::lock_guard<std::mutex> guard(g_mutexInit);
    if (contactDataBase_ == nullptr || store_ == nullptr) {
        contactDataBase_ = std::shared_ptr<ContactsDataBase> (new ContactsDataBase());
    }
    return contactDataBase_;
}

int ContactsDataBase::BeginTransaction()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase BeginTransaction store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = HandleRdbStoreRetry([&]() {
        return store_->BeginTransaction();
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase BeginTransaction failed :%{public}d", ret);
    }
    return ret;
}

std::string GetRawIdsToString(std::vector<int> &mDirtyRawContacts)
{
    std::string result;
    int subLength = 2;
    for (std::vector<int>::iterator it = mDirtyRawContacts.begin(); it != mDirtyRawContacts.end(); it++) {
        result.append(std::to_string(*it)).append(", ");
    }
    return result.substr(0, result.length() - subLength);
}

std::string GetRawIdsToString(std::vector<std::string> &mDirtyRawContacts)
{
    std::string result;
    int subLength = 2;
    for (std::vector<std::string>::iterator it = mDirtyRawContacts.begin(); it != mDirtyRawContacts.end(); it++) {
        result.append(*it).append(", ");
    }
    return result.substr(0, result.length() - subLength);
}

std::string GetRawIdsToString(const std::vector<std::string> &mDirtyRawContacts)
{
    std::string result;
    int subLength = 2;
    for (auto it = mDirtyRawContacts.begin(); it != mDirtyRawContacts.end(); it++) {
        result.append(*it).append(", ");
    }
    return result.substr(0, result.length() - subLength);
}

// 根据vector拼接字符串类型的in条件，给每个元素添加' ，拼接到字符串中
std::string GetStrConditionsToString(std::vector<std::string> &valueVector)
{
    std::string result;
    int subLength = 2;
    for (std::vector<std::string>::iterator it = valueVector.begin(); it != valueVector.end(); it++) {
        result.append("'").append(*it).append("'").append(", ");
    }
    return result.substr(0, result.length() - subLength);
}

void ContactsDataBase::UpdateDirtyToOne()
{
    std::vector<int> mDirtyRawContactsCurHandle = mDirtyRawContacts.consume();
    std::string dirtyRawId = GetRawIdsToString(mDirtyRawContactsCurHandle);
    if (dirtyRawId.empty()) {
        return;
    }
    HILOG_INFO(
        "RawContacts UpdateRawContact dirty rawId = %{public}s, ts = %{public}lld", dirtyRawId.c_str(), (long long) time(NULL));
    OHOS::NativeRdb::ValuesBucket upRawContactDirtyValues;
    std::string contactIdKey = RawContactColumns::DIRTY;
    // 本地新建、变更的联系人，在 js 侧重新计算 uniqueKey
    upRawContactDirtyValues.PutInt(contactIdKey, NEED_RECALC);
    std::string upWhereClause;
    upWhereClause.append(RawContactColumns::ID).append(" IN (").append(dirtyRawId).append(")");
    std::vector<std::string> upWhereArgs;
    int changedRows = OHOS::NativeRdb::E_OK;
    int updateDirtyRet = HandleRdbStoreRetry([&]() {
        return store_->Update(changedRows,
            ContactTableName::RAW_CONTACT, upRawContactDirtyValues, upWhereClause, upWhereArgs);
    });
    if (updateDirtyRet == OHOS::NativeRdb::E_OK) {
        HILOG_INFO("RawContacts UpdateRawContact dirty succeed, ts = %{public}lld", (long long) time(NULL));
    } else {
        HILOG_ERROR("UpdateDirtyToOne update dirty to one fail:%{public}d", updateDirtyRet);
    }
}

void ContactsDataBase::UpdateHardDeleteTimeStamp()
{
    OHOS::NativeRdb::ValuesBucket upValues;
    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    HILOG_INFO("UpdateHardDeleteTimeStamp clone_time_stamp:%{public}lld", ms.count());
    upValues.PutDouble("clone_time_stamp", ms.count());
    std::string upWhereClause;
    upWhereClause.append("id=");
    upWhereClause.append("1");
    std::vector<std::string> upWhereArgs;
    int changedRows = OHOS::NativeRdb::E_OK;
    int updateRet =
        store_->Update(changedRows, ContactTableName::SETTINGS, upValues, upWhereClause, upWhereArgs);
    if (updateRet == OHOS::NativeRdb::E_OK) {
        auto obsMgrClient = DataObsMgrClient::GetInstance();
        Uri uri = Uri(SETTING_URI_NOT_PROXY);
        ErrCode ret = obsMgrClient->NotifyChange(uri);
        if (ret != ERR_OK) {
            HILOG_ERROR("obsMgrClient->NotifyChange contactdb setting error return %{public}d", ret);
        } else {
            HILOG_INFO("obsMgrClient->NotifyChange contactdb setting succeed return %{public}d, ts = %{public}lld",
                ret, (long long) time(NULL));
        }
        HILOG_INFO("UpdateHardDeleteTimeStamp succeed, ts = %{public}lld", (long long) time(NULL));
    } else {
        HILOG_ERROR("UpdateHardDeleteTimeStamp fail:%{public}d", updateRet);
    }
}

void ContactsDataBase::UpdateContactTimeStamp()
{
    std::lock_guard<std::mutex> lock(g_mutexUpdateTimeStamp);
    std::string dirtyRawId = GetRawIdsToString(mUpdateRawContacts);
    HILOG_INFO(
        "Contacts UpdateContactTimeStamp update rawId = %{public}s, ts = %{public}lld", dirtyRawId.c_str(), (long long) time(NULL));
    if (dirtyRawId.empty()) {
        return;
    }
    OHOS::NativeRdb::ValuesBucket upRawContactDirtyValues;
    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    HILOG_INFO("UpdateTable contact_last_updated_timestamp:%{public}lld", ms.count());
    upRawContactDirtyValues.PutDouble("contact_last_updated_timestamp", ms.count());

    std::string upWhereClause;
    upWhereClause.append(ContactColumns::NAME_RAW_CONTACT_ID).append(" IN (").append(dirtyRawId).append(")");
    std::vector<std::string> upWhereArgs;
    int changedRows = OHOS::NativeRdb::E_OK;
    int updateDirtyRet =
        store_->Update(changedRows, ContactTableName::CONTACT, upRawContactDirtyValues, upWhereClause, upWhereArgs);
    if (updateDirtyRet == OHOS::NativeRdb::E_OK) {
        HILOG_INFO("Contacts UpdateContactTimeStamp succeed, ts = %{public}lld", (long long) time(NULL));
        mUpdateRawContacts.clear();
    } else {
        HILOG_ERROR("Contacts UpdateContactTimeStamp fail:%{public}d", updateDirtyRet);
    }
}

int ContactsDataBase::Commit()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase Commit store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Commit();
    });
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
    int ret = HandleRdbStoreRetry([&]() {
        return store_->RollBack();
    });
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
    // replace accountId
    ReplaceAccountId(rawContactValues);
    RawContacts rawContacts;
    int64_t outRawContactId = 0;
    // 如果名称为空，设置联系人排序首字母和sort字段
    setAnonymousSortInfo(rawContactValues);
    int rowRet = HandleRdbStoreRetry([&]() {
        return rawContacts.InsertRawContact(store_, outRawContactId, rawContactValues);
    });
    if (rowRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertRawContact insertRawContact failed:%{public}d", rowRet);
        return RDB_EXECUTE_FAIL;
    }
    Contacts contactsContact;
    int64_t contactId = 0;
    int rowContactRet = HandleRdbStoreRetry([&]() {
        return contactsContact.InsertContact(store_, outRawContactId, rawContactValues, contactId);
    });
    if (rowContactRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertRawContact insertContact failed:%{public}d", rowContactRet);
        return RDB_EXECUTE_FAIL;
    }
    // update contactId to rawContacts
    OHOS::NativeRdb::ValuesBucket upRawContactValues;
    upRawContactValues.PutInt(RawContactColumns::CONTACT_ID, contactId);
    std::string upWhereClause;
    upWhereClause.append(ContactPublicColumns::ID).append(" = ?");
    std::vector<std::string> upWhereArgs{std::to_string(outRawContactId)};
    int ret = HandleRdbStoreRetry([&]() {
        return rawContacts.UpdateRawContact(store_, upRawContactValues, upWhereClause, upWhereArgs);
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("insertRawContact Update contactId to rawContacts failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_WARN("insertRawContact succeed,rawId:%{public}lld,contactId:%{public}lld", (long long) outRawContactId, (long long) contactId);
    // 新增成功，异步上报联系人列表数量；通过js上报
    contactsConnectAbility_->ConnectAbility("", "", "", "", "localChangeReport", "insert;" + getCallingBundleName());
    return outRawContactId;
}

int64_t ContactsDataBase::InsertDeleteRawContact(std::string table, OHOS::NativeRdb::ValuesBucket initialValues)
{
    int64_t outRowId = OHOS::NativeRdb::E_OK;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Insert(outRowId, table, initialValues);
    });
    HILOG_INFO("InsertDeleteRawContact result:%{public}d, ts = %{public}lld", ret, (long long) time(NULL));
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    return outRowId;
}

int ContactsDataBase::DeleteCloudContact(int rawContactId)
{
    std::string uuid = GetUuid(rawContactId);
    int rowId = 0;
    std::vector<std::string> whereArgs;
    whereArgs.push_back(uuid);
    std::string whereCase;
    whereCase.append(CloudRawContactColumns::UUID).append(" = ?");
    int retCode = store_->Delete(rowId, ContactTableName::CLOUD_RAW_CONTACT, whereCase, whereArgs);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteCloudContact code:%{public}d", retCode);
        return retCode;
    }
    return retCode;
}

int ContactsDataBase::DeleteCloudGroups(int rawContactId)
{
    std::string uuid = GetUuid(rawContactId);
    int rowId = 0;
    std::vector<std::string> whereArgs;
    whereArgs.push_back(uuid);
    std::string whereCase;
    whereCase.append(CloudGroupsColumns::UUID).append(" = ?");
    int retCode = store_->Delete(rowId, ContactTableName::CLOUD_GROUP, whereCase, whereArgs);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("deleteCloudGroups code:%{public}d", retCode);
        return retCode;
    }
    return retCode;
}

std::string ContactsDataBase::GetUuid(int rawContactId)
{
    std::string uuid = "";
    std::string queryUuid = "SELECT uuid FROM raw_contact WHERE id = " + std::to_string(rawContactId);
    auto resultSet = store_->QuerySql(queryUuid);
    if (resultSet == nullptr) {
        HILOG_ERROR("GetUuid QuerySqlResult is null");
        return uuid;
    }
    int getRowResult = resultSet->GoToFirstRow();
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        std::string columnName = CloudRawContactColumns::UUID;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetString(columnIndex, uuid);
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
    if (uuid.empty()) {
        HILOG_INFO("rawContactId: %{public}d, GetUuid uuid is empty", rawContactId);
    }
    return uuid;
}

int ContactsDataBase::RecycleUpdateCloud(int rawContactId)
{
    std::string uuid = GetUuid(rawContactId);
    std::string whereClause = "";
    std::vector<std::string> whereArgs;
    whereClause.append(CloudRawContactColumns::UUID);
    whereClause.append(" = ? ");
    whereArgs.push_back(uuid);
    OHOS::NativeRdb::ValuesBucket valuesUpCloudRawContacts;
    valuesUpCloudRawContacts.PutInt(CloudRawContactColumns::RECYCLED, NOT_DELETE_MARK);
    int changedRows = OHOS::NativeRdb::E_OK;
    int retUpdateCloud = RDB_EXECUTE_FAIL;
    retUpdateCloud = store_->Update(
        changedRows, ContactTableName::CLOUD_RAW_CONTACT, valuesUpCloudRawContacts, whereClause, whereArgs);
    return retUpdateCloud;
}

// 最近恢复
int ContactsDataBase::RecycleRestore(OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync)
{
    // 查询恢复的rawId
    std::vector<OHOS::NativeRdb::ValuesBucket> queryValuesBucket = QueryDeleteRawContact(rdbPredicates);
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase RecycleRestore store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    unsigned int size = queryValuesBucket.size();
    std::vector<int> restoreRawIds;
    for (unsigned int i = 0; i < size; i++) {
        OHOS::NativeRdb::ValuesBucket valuesElement = queryValuesBucket[i];
        bool hasContactId = valuesElement.HasColumn(DeleteRawContactColumns::RAW_CONTACT_ID);
        if (!hasContactId) {
            continue;
        }
        OHOS::NativeRdb::ValueObject contactIdValue;
        valuesElement.GetObject(DeleteRawContactColumns::RAW_CONTACT_ID, contactIdValue);
        int rawContactId = 0;
        contactIdValue.GetInt(rawContactId);
        restoreRawIds.push_back(rawContactId);
    }
    // 恢复的rawId
    if (restoreRawIds.size() > 0) {
        std::string rawIdStr = GetRawIdsToString(restoreRawIds);
        int deletedRows = OHOS::NativeRdb::E_OK;
        // 删除回收站数据
        int ret = HandleRdbStoreRetry([&]() {
            return store_->Delete(deletedRows, rdbPredicates);
        });
        HILOG_INFO("ContactsDataBase RecycleRestore ret:%{public}d,deletedRows:%{public}d", ret, deletedRows);
        if (PrivacyContactsManager::IsPrivacySpace()) {
            PrivacyContactsManager::GetInstance()->restorePrivacContacts(restoreRawIds);
        }
        // 本地数据恢复，通过js服务处理
        contactsConnectAbility_->ConnectAbility("", "", "", "", "localRestoreContact", rawIdStr);
        return ret;
    } else {
        HILOG_ERROR("ContactsDataBase RecycleRestore get no raw id");
        return RDB_OBJECT_EMPTY;
    }
}

std::vector<OHOS::NativeRdb::ValuesBucket> ContactsDataBase::QueryDeleteRawContact(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    std::vector<std::string> columns;
    columns.push_back(DeleteRawContactColumns::RAW_CONTACT_ID);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet = store_->Query(rdbPredicates, columns);
    return ResultSetToValuesBucket(resultSet);
}

void ContactsDataBase::setAnonymousSortInfo(OHOS::NativeRdb::ValuesBucket &rawContactValues)
{
    if (rawContactValues.HasColumn(RawContactColumns::DISPLAY_NAME)) {
        OHOS::NativeRdb::ValueObject displayNameValue;
        rawContactValues.GetObject(RawContactColumns::DISPLAY_NAME, displayNameValue);
        std::string disPlayName;
        displayNameValue.GetString(disPlayName);
        if (disPlayName.empty()) {
            rawContactValues.PutString(RawContactColumns::SORT_FIRST_LETTER, ANONYMOUS_SORT_FIRST_LETTER);
            rawContactValues.PutString(RawContactColumns::SORT, ANONYMOUS_SORT);
        }
    } else {
        rawContactValues.PutString(RawContactColumns::SORT_FIRST_LETTER, ANONYMOUS_SORT_FIRST_LETTER);
        rawContactValues.PutString(RawContactColumns::SORT, ANONYMOUS_SORT);
    }
}

void ContactsDataBase::GetContactByValue(int &contactValue, OHOS::NativeRdb::ValueObject &value)
{
    if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_NULL) {
        HILOG_ERROR("GetContactByValue value is nullptr, ts = %{public}lld", (long long) time(NULL));
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
        if (CommonToolType::ConvertToInt(tempString, contactValue)) {
            return;
        } else {
            HILOG_ERROR("GetContactByValue ValueObjectType String tempString = %{public}s", tempString.c_str());
        }
    }
    contactValue = 0;
}

std::string GetCountryCode()
{
    std::string countryCode;
#ifdef CELLULAR_DATA_SUPPORT
    int slotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    HILOG_INFO("ContactsDataBase GetCountryCode slotIds is %{public}d, ts = %{public}lld", slotId, (long long) time(NULL));
    sptr<Telephony::NetworkState> networkClient = nullptr;
    DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetNetworkState(slotId, networkClient);
    if (networkClient != nullptr && networkClient->IsRoaming()) {
        HILOG_INFO("ContactsDataBase GetCountryCode networkState is nullptr");
        HILOG_INFO("ContactsDataBase GetCountryCode Roaming is %{public}d", networkClient->IsRoaming());
        return countryCode;
    }
    std::u16string countryCodeForNetwork;
    DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetIsoCountryCodeForNetwork(
        slotId, countryCodeForNetwork);
    countryCode = Str16ToStr8(countryCodeForNetwork);
    if (countryCode.empty()) {
        std::u16string countryCodeForSim;
        DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetISOCountryCodeForSim(
            slotId, countryCodeForSim);
        countryCode = Str16ToStr8(countryCodeForSim);
    }
#endif
    if (countryCode.empty()) {
        countryCode = Global::I18n::LocaleConfig::GetSystemRegion();
    }
    transform(countryCode.begin(), countryCode.end(), countryCode.begin(), toupper);
    HILOG_INFO("ContactsDataBase GetCountryCode region is %{public}s", countryCode.c_str());
    return countryCode;
}

std::string GetE164FormatPhoneNumber(std::string &phoneNumber)
{
    std::string result;
    std::string countryCode = GetCountryCode();
    if (countryCode.empty()) {
        return phoneNumber;
    }
    bool isValidPhoneNumber = PhoneNumberUtils::IsValidPhoneNumber(phoneNumber, countryCode);
    if (isValidPhoneNumber) {
        std::string formatPhoneNumber = PhoneNumberUtils::Format(phoneNumber, countryCode);
        if (!formatPhoneNumber.empty()) {
            result = formatPhoneNumber;
        } else {
            HILOG_INFO("ContactsDataBase GetE164FormatPhoneNumber formatPhoneNumber is empty");
        }
    } else {
        HILOG_INFO("ContactsDataBase GetE164FormatPhoneNumber isValidPhoneNumber false");
    }
    return result;
}

void ContactsDataBase::updateFormatPhoneNumber(int typeId, OHOS::NativeRdb::ValuesBucket &contactDataValues)
{
    if (typeId == ContentTypeData::PHONE_INT_VALUE) {
        std::string number;
        OHOS::NativeRdb::ValueObject valueDetailInfo;
        contactDataValues.GetObject(ContactDataColumns::DETAIL_INFO, valueDetailInfo);
        valueDetailInfo.GetString(number);
        std::string newNumberE164 = GetE164FormatPhoneNumber(number);
        if (!newNumberE164.empty()) {
            contactDataValues.Delete(ContactDataColumns::FORMAT_PHONE_NUMBER);
            contactDataValues.PutString(ContactDataColumns::FORMAT_PHONE_NUMBER, newNumberE164);
        }

        // 针对电话号码字段的入参有空格场景做去空格处理，比如腾讯换机助手导入联系人信息
        std::string phoneNumber = generatePhoneNumber(number);
        contactDataValues.Delete(ContactDataColumns::DETAIL_INFO);
        contactDataValues.PutString(ContactDataColumns::DETAIL_INFO, phoneNumber);
        if (number.length() != phoneNumber.length()) {
            HILOG_WARN("batch insert contact_data transfer phoneNumber. oldPhoneNumber : %{public}s,"
                "newPhoneNumber : %{public}s", ContactsStringUtils::MaskPhoneForLog(number).c_str(),
                ContactsStringUtils::MaskPhoneForLog(phoneNumber).c_str());
        }
    }
}

void ContactsDataBase::FillingNumberLocation(int typeId, OHOS::NativeRdb::ValuesBucket &contactDataValues)
{
    if (typeId == ContentTypeData::PHONE_INT_VALUE) {
        std::string number = "";
        std::string numberLocation = "";
        OHOS::NativeRdb::ValueObject valueDetailInfo;
        contactDataValues.GetObject(ContactDataColumns::DETAIL_INFO, valueDetailInfo);
        valueDetailInfo.GetString(number);
        if (number == "") {
            HILOG_ERROR("accountNumber is null");
            return;
        }
        std::shared_ptr<NumberIdentityHelper> numberIdentityHelper =
            NumberIdentityHelper::GetInstance();
        if (numberIdentityHelper == nullptr) {
            HILOG_ERROR("numberIdentityHelper is nullptr!");
            return;
        }
        DataShare::DataSharePredicates predicates;
        std::vector<std::string> phoneNumber;
        phoneNumber.push_back(number);
        predicates.SetWhereArgs(phoneNumber);
        bool ret = numberIdentityHelper->Query(numberLocation, predicates);
        if (!ret) {
            HILOG_ERROR("Query number location database fail!");
            return;
        }
        if (numberLocation == "") {
            HILOG_ERROR("No location information found!");
            return;
        }
        contactDataValues.Delete(ContactDataColumns::LOCATION);
        contactDataValues.PutString(ContactDataColumns::LOCATION, numberLocation);
    }
}

std::string ContactsDataBase::generatePhoneNumber(std::string fromDetailInfo)
{
    // 电话号码，做去除空白字符信息处理
    std::string transferStr = std::regex_replace(fromDetailInfo, pattern, "");
    return transferStr;
}

// contact_data 新增后处理：更新contact的对应信息；不更新通话记录（在ability处理）
int ContactsDataBase::RecordUpdateContactId(
    int rawContactId, OHOS::NativeRdb::ValuesBucket &contactDataValues, std::string isSync)
{
    int typeId = RDB_EXECUTE_FAIL;
    std::string typeText;
    // 根据typeId，设置typeText类型的名称
    int retCode = GetTypeText(contactDataValues, typeId, rawContactId, typeText);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertContactData getTypeText failed:%{public}d", retCode);
        return retCode;
    }
    if (typeId <= 0) {
        HILOG_ERROR("InsertContactData typeId is required %{public}d", typeId);
        return RDB_EXECUTE_FAIL;
    }
    std::vector<int> rawContactIdVector;
    rawContactIdVector.push_back(rawContactId);
    if (isSync != "true") {
        mDirtyRawContacts.production(rawContactId);
    }
    if ((typeId == ContentTypeData::PHONE_INT_VALUE || typeId == ContentTypeData::NAME_INT_VALUE) &&
        contactDataValues.HasColumn(ContactDataColumns::DETAIL_INFO)) {
        mUpdateRawContacts.push_back(rawContactId);
    }
    int updateDisplayRet = GetUpdateDisplayRet(typeText, rawContactIdVector, contactDataValues);
    if (updateDisplayRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertContactData UpdateDisplay failed:%{public}d", updateDisplayRet);
        return RDB_EXECUTE_FAIL;
    }
    return OHOS::NativeRdb::E_OK;
}

/**
 * @brief Insert data into table contact_data
 *
 * @param table Insert tableName
 * @param contactDataValues Parameters to be passed for insert operation
 *
 * @return The result returned by the insert operation
 */
int64_t ContactsDataBase::InsertContactData(
    std::string table, OHOS::NativeRdb::ValuesBucket contactDataValues, std::string isSync)
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
    HILOG_INFO(
        "ContactsDataBase InsertContactData rawContactId = %{public}d,ts = %{public}lld", rawContactId, (long long) time(NULL));

    if (rawContactId <= 0) {
        HILOG_ERROR("InsertContactData raw_contact_id is required %{public}d", rawContactId);
        return RDB_EXECUTE_FAIL;
    }
    int typeId = RDB_EXECUTE_FAIL;
    std::string typeText;
    int retCode = GetTypeText(contactDataValues, typeId, rawContactId, typeText);
    if (retCode != OHOS::NativeRdb::E_OK || typeId <= 0) {
        HILOG_ERROR("InsertContactData getTypeText code:%{public}d, typeId:%{public}d", retCode, typeId);
        return RDB_EXECUTE_FAIL;
    }
    // delete content_type
    contactDataValues.Delete(ContentTypeColumns::CONTENT_TYPE);
    contactDataValues.PutInt(ContactDataColumns::TYPE_ID, typeId);

    // update format_phone_number
    updateFormatPhoneNumber(typeId, contactDataValues);
    // 查询并填充号码归属地
    FillingNumberLocation(typeId, contactDataValues);
    // 新增contact_data, 记录更新的联系人id
    ContactsDataBase::updateContactIdVector.production(rawContactId);
    int64_t outDataRowId;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Insert(outDataRowId, table, contactDataValues);
    });
    if (ret == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        ret = store_->Restore("contacts.db.bak");
        HILOG_ERROR("InsertContactData Insert Restore retCode= %{public}d", ret);
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertContactData failed:%{public}d,ts = %{public}lld", ret, (long long) time(NULL));
        return RDB_EXECUTE_FAIL;
    }
    if (typeId == ContentTypeData::PHONE_INT_VALUE && PrivacyContactsManager::IsPrivacySpace()) {
        contactDataValues.PutInt(ContactDataColumns::ID, outDataRowId);
        PrivacyContactsManager::GetInstance()->InsertContactDataToPrivacyBackup(contactDataValues);
    }
    int retRecord = RecordUpdateContactId(rawContactId, contactDataValues, isSync);
    if (retRecord != OHOS::NativeRdb::E_OK) {
        return retRecord;
    }
    return outDataRowId;
}

/**
 * @brief Insert data into table privacy_contacts_backup
 *
 * @param table Insert tableName
 * @param contactDataValues Parameters to be passed for insert operation
 *
 * @return The result returned by the insert operation
 */
int64_t ContactsDataBase::InsertPrivacyContactsBackup(
    std::string table, OHOS::NativeRdb::ValuesBucket contactDataValues)
{
    if (store_ == nullptr) {
        HILOG_ERROR("InsertPrivacyContactsBackup store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int64_t outDataRowId;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Insert(outDataRowId, table, contactDataValues);
    });
    if (ret == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        ret = store_->Restore("contacts.db.bak");
        HILOG_ERROR("InsertPrivacyContactsBackup Insert Restore retCode= %{public}d", ret);
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertPrivacyContactsBackup failed:%{public}d,ts = %{public}lld", ret, (long long) time(NULL));
        return RDB_EXECUTE_FAIL;
    }
    return outDataRowId;
}

/**
 * @brief Batch insert datas into table privacy_contacts_backup
 *
 * @param table Insert tableName
 * @param contactDataValues Parameters to be passed for insert operation
 *
 * @return The result returned by the insert operation
 */
int64_t ContactsDataBase::BatchInsertPrivacyContactsBackup(
    std::string table, std::vector<OHOS::NativeRdb::ValuesBucket> contactDataValues)
{
    HILOG_WARN("ContactsDataBase BatchInsertPrivacyContactsBackup start.");
    if (store_ == nullptr) {
        HILOG_ERROR("BatchInsertPrivacyContactsBackup store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int64_t outDataRowNum;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->BatchInsert(outDataRowNum, table, contactDataValues);
    });
    if (ret == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        ret = store_->Restore("contacts.db.bak");
        HILOG_ERROR("BatchInsertPrivacyContactsBackup Insert Restore retCode= %{public}d", ret);
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("BatchInsertPrivacyContactsBackup failed:%{public}d,ts = %{public}lld", ret, (long long) time(NULL));
        return RDB_EXECUTE_FAIL;
    }
    return outDataRowNum;
}

/**
 * @brief BatchInsert data into table contact_data
 *
 * @param table Insert tableName
 * @param contactDataValues Parameters to be passed for insert operation
 *
 * @return The result returned by the BatchInsert operation
 */
int64_t ContactsDataBase::BatchInsert(
    std::string table, const std::vector<DataShare::DataShareValuesBucket> &values, std::string isSyncFromCloud)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase BatchInsertContactData store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    std::vector<OHOS::NativeRdb::ValuesBucket> batchInsertValues;
    unsigned int size = values.size();
    HILOG_INFO("ContactsDataBase BatchInsertContactData start.size is %{public}d", size);
    int ret = this->BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return ret;
    }
    // 内部有记录影响到变更，需要通知的raw_contact_id
    ret = BatchInsertDisposal(batchInsertValues, values, size, isSyncFromCloud);
    if (ret != OHOS::NativeRdb::E_OK) {
        this->RollBack();
        return ret;
    }

    ret = this->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        this->RollBack();
        return ret;
    }
    int64_t outDataRowId;
    ret = HandleRdbStoreRetry([&]() {
        return store_->BatchInsert(outDataRowId, table, batchInsertValues);
    });
    if (ret == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        ret = store_->Restore("contacts.db.bak");
        HILOG_ERROR("BatchInsert Insert Restore retCode= %{public}d", ret);
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CallLogDataBase InsertCallLog ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    SyncBirthToCalAfterBatchInsert(batchInsertValues, values, size);
    return outDataRowId;
}

/**
 * @brief Disposal BatchInsert data into table contact_data
 *
 * @param int ret
 * @param unsigned int size
 * @param batchInsertValues batchInsertValues
 *
 * @return The result returned by the BatchInsert operation
 */
int ContactsDataBase::SyncBirthToCalAfterBatchInsert(std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues,
    const std::vector<DataShare::DataShareValuesBucket> &values, unsigned int size)
{
    int ret = RDB_EXECUTE_OK;
    std::vector<int> totalRawIdVector;
    std::vector<int> rawIdForBirthdayVector;
    for (unsigned int i = 0; i < size; i++) {
        DataShare::DataShareValuesBucket valuesElement = values[i];
        OHOS::NativeRdb::ValuesBucket contactDataValues = RdbDataShareAdapter::RdbUtils::ToValuesBucket(valuesElement);
        int rawContactId = OHOS::NativeRdb::E_OK;
        if (!contactDataValues.HasColumn(ContactDataColumns::RAW_CONTACT_ID)) {
            HILOG_ERROR("BatchInsert ContactData raw_contact_id is required");
            ret = RDB_EXECUTE_FAIL;
            continue;
        }
        if (!contactDataValues.HasColumn(ContactDataColumns::TYPE_ID)) {
            HILOG_ERROR("BatchInsert ContactData type_id is required");
            ret = RDB_EXECUTE_FAIL;
            continue;
        }
        OHOS::NativeRdb::ValueObject value;
        contactDataValues.GetObject(ContactDataColumns::RAW_CONTACT_ID, value);
        GetContactByValue(rawContactId, value);
        if (rawContactId <= 0) {
            HILOG_ERROR("InsertContactData raw_contact_id is required %{public}d", rawContactId);
            ret = RDB_EXECUTE_FAIL;
            continue;
        }
        int typeId;
        OHOS::NativeRdb::ValueObject valueTypeId;
        contactDataValues.GetObject(ContactDataColumns::TYPE_ID, valueTypeId);
        GetContactByValue(typeId, valueTypeId);
        if (typeId == 11) {  // 11代表联系人日程类型
            std::string customData;
            OHOS::NativeRdb::ValueObject valueCustomData;
            contactDataValues.GetObject(ContactDataColumns::CUSTOM_DATA, valueCustomData);
            valueCustomData.GetString(customData);
            // 只有农历生日或者阳历生日才写入日历
            if (customData == "1" || customData == "2") {
                rawIdForBirthdayVector.push_back(rawContactId);
            }
        }
    }
    HILOG_WARN("SyncBirthToCalAfterBatchInsert rawIdForBirthdayVector size is %{public}ld",
        (long) rawIdForBirthdayVector.size());
    if (rawIdForBirthdayVector.size() > 0) {
        std::string rawIdBirthdayStr = GetRawIdsToString(rawIdForBirthdayVector);
        contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
        contactsConnectAbility_->ConnectEventsHandleAbility(CONTACTS_BIRTHDAY_BATCH_INSERT, rawIdBirthdayStr, "");
    }
    return ret;
}

void ContactsDataBase::parseSyncBirthInfo(int &ret, OHOS::NativeRdb::ValuesBucket &contactDataValues,
    std::vector<int> &rawIdForBirthdayVector)
{
    int rawContactId = OHOS::NativeRdb::E_OK;
    if (!contactDataValues.HasColumn(ContactDataColumns::RAW_CONTACT_ID)) {
        HILOG_ERROR("BatchInsert ContactData raw_contact_id is required");
        ret = RDB_EXECUTE_FAIL;
        return;
    }
    if (!contactDataValues.HasColumn(ContactDataColumns::TYPE_ID)) {
        HILOG_ERROR("BatchInsert ContactData type_id is required");
        ret = RDB_EXECUTE_FAIL;
        return;
    }
    OHOS::NativeRdb::ValueObject value;
    contactDataValues.GetObject(ContactDataColumns::RAW_CONTACT_ID, value);
    GetContactByValue(rawContactId, value);
    if (rawContactId <= 0) {
        HILOG_ERROR("InsertContactData raw_contact_id is required %{public}d", rawContactId);
        ret = RDB_EXECUTE_FAIL;
        return;
    }
    int typeId;
    OHOS::NativeRdb::ValueObject valueTypeId;
    contactDataValues.GetObject(ContactDataColumns::TYPE_ID, valueTypeId);
    GetContactByValue(typeId, valueTypeId);
    if (typeId == 11) {  // 11代表联系人日程类型
        std::string customData;
        OHOS::NativeRdb::ValueObject valueCustomData;
        contactDataValues.GetObject(ContactDataColumns::CUSTOM_DATA, valueCustomData);
        valueCustomData.GetString(customData);
        // 只有农历生日或者阳历生日才写入日历
        if (customData == "1" || customData == "2") {
            rawIdForBirthdayVector.push_back(rawContactId);
        }
    }
}

int ContactsDataBase::SyncBirthToCalAfterBatchInsert(std::vector<OHOS::NativeRdb::ValuesBucket> &values)
{
    int ret = RDB_EXECUTE_OK;
    std::vector<int> totalRawIdVector;
    std::vector<int> rawIdForBirthdayVector;
    for (unsigned int i = 0; i < values.size(); i++) {
        parseSyncBirthInfo(ret, values[i], rawIdForBirthdayVector);
    }
    HILOG_WARN("SyncBirthToCalAfterBatchInsert rawIdForBirthdayVector size is %{public}ld",
        (long) rawIdForBirthdayVector.size());
    if (rawIdForBirthdayVector.size() > 0) {
        std::string rawIdBirthdayStr = GetRawIdsToString(rawIdForBirthdayVector);
        contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
        contactsConnectAbility_->ConnectEventsHandleAbility(CONTACTS_BIRTHDAY_BATCH_INSERT, rawIdBirthdayStr, "");
    }
    ret = static_cast<int>(rawIdForBirthdayVector.size());
    return ret;
}

// 解析需要更新通话记录的rawId
int ContactsDataBase::parseNeedUpdateCalllogRawIdBatch(const std::vector<DataShare::DataShareValuesBucket> &values,
    std::vector<int> &rawIdVector)
{
    unsigned int size = values.size();
    int ret = OHOS::NativeRdb::E_OK;
    for (unsigned int i = 0; i < size; i++) {
        ret = parseNeedUpdateCalllogRawId(values[i], rawIdVector);
    }
    return ret;
}

int ContactsDataBase::parseNeedUpdateCalllogRawId(const DataShare::DataShareValuesBucket &dataShareValuesBucket,
    std::vector<int> &rawIdVector)
{
    int ret = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::ValuesBucket contactDataValues =
        RdbDataShareAdapter::RdbUtils::ToValuesBucket(dataShareValuesBucket);
    // 读取rawId
    int rawContactId = OHOS::NativeRdb::E_OK;
    if (!contactDataValues.HasColumn(ContactDataColumns::RAW_CONTACT_ID)) {
        HILOG_ERROR("parase NeedUpdateCalllogRawId raw_contact_id is required");
        ret = RDB_EXECUTE_FAIL;
        return ret;
    }
    OHOS::NativeRdb::ValueObject value;
    contactDataValues.GetObject(ContactDataColumns::RAW_CONTACT_ID, value);
    GetContactByValue(rawContactId, value);
    if (rawContactId <= 0) {
        HILOG_ERROR("parase NeedUpdateCalllogRawId raw_contact_id is required %{public}d", rawContactId);
        ret = RDB_EXECUTE_FAIL;
        return ret;
    }
    // 读取typeId
    int typeId = RDB_EXECUTE_FAIL;
    std::string typeText;
    ret = GetTypeText(contactDataValues, typeId, rawContactId, typeText);
    if (ret != OHOS::NativeRdb::E_OK || typeId <= 0) {
        HILOG_ERROR("parase NeedUpdateCalllogRawId failed:%{public}d, typeId = %{public}d", ret, typeId);
        ret = RDB_EXECUTE_FAIL;
        return ret;
    }
    if (IsAffectCalllog(typeText)) {
        rawIdVector.push_back(rawContactId);
    }
    ret = OHOS::NativeRdb::E_OK;

    return ret;
}

/**
 * @brief Disposal BatchInsert data into table contact_data
 *
 * @param int ret
 * @param unsigned int size
 * @param batchInsertValues batchInsertValues
 *
 * @return The result returned by the BatchInsert operation
 */
int ContactsDataBase::BatchInsertDisposal(std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues,
    const std::vector<DataShare::DataShareValuesBucket> &values, unsigned int size, std::string isSyncFromCloud)
{
    int ret = RDB_EXECUTE_OK;
    int result = RDB_EXECUTE_FAIL;
    std::vector<int> totalRawIdVector;
    for (unsigned int i = 0; i < size; i++) {
        OHOS::NativeRdb::ValuesBucket contactDataValues = RdbDataShareAdapter::RdbUtils::ToValuesBucket(values[i]);
        int rawContactId = OHOS::NativeRdb::E_OK;
        if (!contactDataValues.HasColumn(ContactDataColumns::RAW_CONTACT_ID)) {
            HILOG_ERROR("BatchInsert ContactData raw_contact_id is required");
            ret = RDB_EXECUTE_FAIL;
            continue;
        }
        OHOS::NativeRdb::ValueObject value;
        contactDataValues.GetObject(ContactDataColumns::RAW_CONTACT_ID, value);
        GetContactByValue(rawContactId, value);
        if (rawContactId <= 0) {
            HILOG_ERROR("InsertContactData raw_contact_id is required %{public}d", rawContactId);
            ret = RDB_EXECUTE_FAIL;
            continue;
        }
        // 记录变更的rawcontactid
        ContactsDataBase::updateContactIdVector.production(rawContactId);
        int typeId = RDB_EXECUTE_FAIL;
        std::string typeText;
        ret = GetTypeText(contactDataValues, typeId, rawContactId, typeText);
        if (ret != OHOS::NativeRdb::E_OK || typeId <= 0) {
            HILOG_ERROR("InsertContactData getTypeText failed:%{public}d, typeId = %{public}d", ret, typeId);
            ret = RDB_EXECUTE_FAIL;
            continue;
        }
        updateFormatPhoneNumber(typeId, contactDataValues);
        std::vector<int> rawContactIdVector;
        rawContactIdVector.push_back(rawContactId);
        if (typeId == ContentTypeData::PHONE_INT_VALUE || typeId == ContentTypeData::NAME_INT_VALUE) {
            totalRawIdVector.push_back(rawContactId);
        }
        int updateDisplayRet = GetUpdateDisplayRet(typeText, rawContactIdVector, contactDataValues);
        if (updateDisplayRet != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("InsertContactData UpdateDisplay failed:%{public}d", updateDisplayRet);
            ret = RDB_EXECUTE_FAIL;
            continue;
        }
        BatchInsertPushBack(rawContactId, contactDataValues, batchInsertValues, typeId, isSyncFromCloud);
        result = OHOS::NativeRdb::E_OK;
    }
    if (result == OHOS::NativeRdb::E_OK) {
        ret = result;
    }
    return ret;
}

void ContactsDataBase::BatchInsertPushBack(int rawContactId, OHOS::NativeRdb::ValuesBucket contactDataValues,
    std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues, int typeId, std::string isSyncFromCloud)
{
    if (isSyncFromCloud != "true") {
        mDirtyRawContacts.production(rawContactId);
    }
    if ((typeId == ContentTypeData::PHONE_INT_VALUE || typeId == ContentTypeData::NAME_INT_VALUE) &&
        contactDataValues.HasColumn(ContactDataColumns::DETAIL_INFO)) {
        mUpdateRawContacts.push_back(rawContactId);
    }
    // delete content_type
    contactDataValues.Delete(ContentTypeColumns::CONTENT_TYPE);
    contactDataValues.PutInt(ContactDataColumns::TYPE_ID, typeId);
    batchInsertValues.push_back(contactDataValues);
}

void ContactsDataBase::boardReportBatchInsert(int size)
{
    // 看板上报，回收站彻底删除上报
    std::string paramStr = "batchInsert;";
    std::string callingBundleName = getCallingBundleName();
    paramStr.append(callingBundleName);
    paramStr.append(";");
    paramStr.append(std::to_string(size));
    contactsConnectAbility_->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
}

int64_t ContactsDataBase::BatchInsertRawContactsCore(const std::vector<DataShare::DataShareValuesBucket> &values)
{
    int ret = 0;
    for (unsigned int i = 0; i < values.size(); i++) {
        DataShare::DataShareValuesBucket rawContactValues = values[i];
        OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(rawContactValues);
        // replace accountId
        ReplaceAccountId(value);
        RawContacts rawContacts;
        int64_t outRawContactId = 0;
        // 如果名称为空，设置联系人排序首字母和sort字段
        setAnonymousSortInfo(value);
        // 插入rawcontact
        int rowRet = HandleRdbStoreRetry([&]() {
            return rawContacts.InsertRawContact(store_, outRawContactId, value);
        });
        if (rowRet != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("InsertRawContact insertRawContact failed:%{public}d", rowRet);
            ret = RDB_EXECUTE_FAIL;
            break;
        }
        int64_t contactId = 0;
        // 插入contact
        int rowContactRet = HandleRdbStoreRetry([&]() {
            return Contacts().InsertContact(store_, outRawContactId, value, contactId);
        });
        if (rowContactRet != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("InsertRawContact insertContact failed:%{public}d", rowContactRet);
            ret = RDB_EXECUTE_FAIL;
            break;
        }
        // update contactId to rawContacts
        std::string contactIdKey = RawContactColumns::CONTACT_ID;
        OHOS::NativeRdb::ValuesBucket upRawContactValues;
        upRawContactValues.PutInt(contactIdKey, contactId);
        std::vector<std::string> upWhereArgs;
        upWhereArgs.push_back(std::to_string(outRawContactId));
        std::string upWhereClause;
        upWhereClause.append(ContactPublicColumns::ID).append(" = ?");
        // 更新rawContact的contactId信息
        int ret = HandleRdbStoreRetry([&]() {
            return rawContacts.UpdateRawContact(store_, upRawContactValues, upWhereClause, upWhereArgs);
        });
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("insert raw contact Update contactId to rawContacts failed:%{public}d", rowContactRet);
            ret = RDB_EXECUTE_FAIL;
            break;
        }
    }
    // 看板上报，批量插入时，上报
    boardReportBatchInsert(values.size());
    return ret;
}

int64_t ContactsDataBase::BatchInsertRawContacts(const std::vector<DataShare::DataShareValuesBucket> &values)
{
    HILOG_INFO("BatchInsertRawContacts begin.");
    int ret = this->BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return ret;
    }

    ret = this->BatchInsertRawContactsCore(values);
    if (ret != OHOS::NativeRdb::E_OK) {
        this->RollBack();
        return ret;
    }
    ret = this->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        this->RollBack();
        return ret;
    }
    HILOG_INFO("BatchInsertRawContacts end.");
    return values.size();
}

int64_t ContactsDataBase::BatchInsertBlockList(const std::vector<DataShare::DataShareValuesBucket> &values)
{
    HILOG_INFO("BatchInsertBlockList begin.");
    std::vector<std::string> phoneNumbers;
    std::map<std::string, OHOS::NativeRdb::ValuesBucket> batchInsertMap = ToBlockListValueBucketMap(values);
    std::map<std::string, int64_t> formatPhoneNumberMap = QueryIsInBlockList(batchInsertMap);
    std::vector<OHOS::NativeRdb::ValuesBucket> blockListValues;
    std::vector<OHOS::NativeRdb::ValuesBucket> updateBlockListValues;
    for (auto iter = batchInsertMap.begin(); iter != batchInsertMap.end(); iter++) {
        if (formatPhoneNumberMap.find(iter->first) != formatPhoneNumberMap.end()) {
            iter->second.PutLong(ContactBlockListColumns::ID, formatPhoneNumberMap[iter->first]);
            updateBlockListValues.push_back(iter->second);
        } else {
            std::string phoneNumber;
            OHOS::NativeRdb::ValueObject phoneNumberValue;
            iter->second.GetObject(ContactBlockListColumns::PHONE_NUMBER, phoneNumberValue);
            phoneNumberValue.GetString(phoneNumber);
            if (phoneNumber.empty()) {
                return RDB_EXECUTE_FAIL;
            }
            phoneNumbers.push_back(phoneNumber);
            blockListValues.push_back(iter->second);
        }
    }

    BatchUpdateBlockListOneByOne(updateBlockListValues);

    int64_t rowChangeCount = OHOS::NativeRdb::E_OK;
    auto ret = BlocklistDataBase::store_->BatchInsert(
        rowChangeCount, ContactTableName::CONTACT_BLOCKLIST, blockListValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("BatchInsertBlockList fail:%{public}d", ret);
    }
    NotifyMmsUpdateInterceptionCount(phoneNumbers, COMMAND_UPDATE_FULL_MATCH_MSG_COUNT);
    HILOG_INFO("BatchInsertBlockList end.");
    return rowChangeCount;
}

std::map<std::string, OHOS::NativeRdb::ValuesBucket> ContactsDataBase::ToBlockListValueBucketMap(
    const std::vector<DataShare::DataShareValuesBucket> &values)
{
    std::map<std::string, OHOS::NativeRdb::ValuesBucket> batchInsertMap;
    for (auto &rawValue : values) {
        OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(rawValue);
        std::string phoneNumber;
        if (value.HasColumn(ContactBlockListColumns::PHONE_NUMBER)) {
            OHOS::NativeRdb::ValueObject phoneNumberValue;
            value.GetObject(ContactBlockListColumns::PHONE_NUMBER, phoneNumberValue);
            phoneNumberValue.GetString(phoneNumber);
            if (phoneNumber.empty()) {
                continue;
            }
        }
#ifdef ABILITY_CUST_SUPPORT
        int interceptionCallCount = QueryInterceptionCallCount(phoneNumber);
        value.PutInt(ContactBlockListColumns::INTERCEPTION_CALL_COUNT, interceptionCallCount);
#endif
        std::string formatPhoneNumber = GetE164FormatPhoneNumber(phoneNumber);
        if (formatPhoneNumber.empty()) {
            value.PutString(ContactBlockListColumns::FORMAT_PHONE_NUMBER, phoneNumber);
            if (batchInsertMap.find(phoneNumber) == batchInsertMap.end()) {
                batchInsertMap.insert(std::pair<std::string, OHOS::NativeRdb::ValuesBucket>(phoneNumber, value));
            }
        } else {
            value.PutString(ContactBlockListColumns::FORMAT_PHONE_NUMBER, formatPhoneNumber);
            if (batchInsertMap.find(formatPhoneNumber) == batchInsertMap.end()) {
                batchInsertMap.insert(std::pair<std::string, OHOS::NativeRdb::ValuesBucket>(formatPhoneNumber, value));
            }
        }
    }
    return batchInsertMap;
}

int ContactsDataBase::BatchUpdateBlockListOneByOne(std::vector<OHOS::NativeRdb::ValuesBucket> updateBlockListValues)
{
    HILOG_INFO("BatchUpdateBlockListOneByOne begin.");
    if (updateBlockListValues.size() == 0) {
        HILOG_INFO("BatchUpdateBlockListOneByOne end no need update.");
        return OHOS::NativeRdb::E_OK;
    }
    int successCount = 0;
    std::vector<std::string> phoneNumbers;
    for (auto &blocklistValue : updateBlockListValues) {
        std::string phoneNumber;
        if (blocklistValue.HasColumn(ContactBlockListColumns::PHONE_NUMBER)) {
            OHOS::NativeRdb::ValueObject phoneNumberValue;
            blocklistValue.GetObject(ContactBlockListColumns::PHONE_NUMBER, phoneNumberValue);
            phoneNumberValue.GetString(phoneNumber);
            if (phoneNumber.empty()) {
                HILOG_ERROR("BatchUpdateBlockListOneByOne fail: no phone number");
                continue;
            }
        }
        int64_t id = -1;
        if (blocklistValue.HasColumn(ContactBlockListColumns::ID)) {
            OHOS::NativeRdb::ValueObject idValue;
            blocklistValue.GetObject(ContactBlockListColumns::ID, idValue);
            idValue.GetLong(id);
            if (id < 0) {
                HILOG_ERROR("BatchUpdateBlockListOneByOne fail: invalid id");
                continue;
            }
        }
        std::string whereClause = "";
        std::vector<std::string> whereArgs;
        whereClause.append(ContactBlockListColumns::ID).append(" = ").append(std::to_string(id));
        int rowUpdateCount = OHOS::NativeRdb::E_OK;
        auto updateRet = HandleRdbStoreRetry([&]() {
            return BlocklistDataBase::store_->Update(
                rowUpdateCount, ContactTableName::CONTACT_BLOCKLIST, blocklistValue, whereClause, whereArgs);
        });
        if (updateRet != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("BatchUpdateBlockListOneByOne fail:%{public}d", updateRet);
            continue;
        }
        successCount++;
        phoneNumbers.push_back(phoneNumber);
    }
    NotifyMmsUpdateInterceptionCount(phoneNumbers, COMMAND_UPDATE_FULL_MATCH_MSG_COUNT);
    HILOG_INFO("BatchUpdateBlockListOneByOne end.");
    return successCount;
}

std::map<std::string, int64_t> ContactsDataBase::QueryIsInBlockList(
    const std::map<std::string, OHOS::NativeRdb::ValuesBucket> &values)
{
    HILOG_INFO("QueryIsInBlockList begin.");
    std::map<std::string, int64_t> result;
    if (values.size() == 0) {
        HILOG_INFO("QueryIsInBlockList end no need query.");
        return result;
    }
    auto predicates = OHOS::NativeRdb::RdbPredicates(ContactTableName::CONTACT_BLOCKLIST);
    std::string queryWheres;
    std::string queryArg;
    queryArg.append("(");
    for (auto iter = values.begin(); iter != values.end(); iter++) {
        queryArg.append("'").append(iter->first).append("', ");
    }
    int subLength = 2;
    queryArg = queryArg.substr(0, queryArg.length() - subLength);
    queryArg.append(")");
    queryWheres.append("format_phone_number").append(" in ").append(queryArg);
    // 批量添加黑名单只会与白名单冲突
    queryWheres.append(" and types = 1");
    predicates.SetWhereClause(queryWheres);

    std::vector<std::string> columns;
    columns.push_back(ContactBlockListColumns::FORMAT_PHONE_NUMBER);
    columns.push_back(ContactBlockListColumns::ID);
    auto resultSet = BlocklistDataBase::store_->Query(predicates, columns);
    if (resultSet == nullptr) {
        return result;
    }
    std::string columnName = ContactBlockListColumns::FORMAT_PHONE_NUMBER;
    std::string columnIdName = ContactBlockListColumns::ID;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        std::string stringValue;
        resultSet->GetString(columnIndex, stringValue);
        int columnIdIndex = 0;
        resultSet->GetColumnIndex(columnIdName, columnIdIndex);
        int64_t id;
        resultSet->GetLong(columnIndex, id);
        result.insert(std::pair<std::string, int64_t>(stringValue, id));
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    HILOG_INFO("QueryIsInBlockList end.");
    return result;
}

int ContactsDataBase::GetUpdateDisplayRet(
    std::string &typeText, std::vector<int> &rawContactIdVector, OHOS::NativeRdb::ValuesBucket &contactDataValues)
{
    std::vector<std::string> typeTextVector;
    typeTextVector.push_back(typeText);
    ContactsUpdateHelper contactsUpdateHelper;
    int updateDisplayRet =
        contactsUpdateHelper.UpdateDisplay(rawContactIdVector, typeTextVector, store_, contactDataValues, false);
    return updateDisplayRet;
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
        typeId = contactsType.LookupTypeIdOrInsertTypeValue(store_, typeText);
        if (typeId == RDB_EXECUTE_FAIL) {
            return RDB_EXECUTE_FAIL;
        }
        return RDB_EXECUTE_OK;
    } else if (contactDataValues.HasColumn(ContactDataColumns::TYPE_ID)) {
        OHOS::NativeRdb::ValueObject typeValue;
        contactDataValues.GetObject(ContactDataColumns::TYPE_ID, typeValue);
        GetContactByValue(typeId, typeValue);
        ContactsType contactsType;
        contactsType.GetTypeText(store_, typeId, typeText);
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
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Insert(outGroupRowId, table, initialValues);
    });
    if (ret == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        ret = store_->Restore("contacts.db.bak");
        HILOG_ERROR("ContactsDataBase InsertGroup Restore retCode= %{public}d", ret);
    }
    HILOG_INFO("ContactsDataBase InsertGroup ret:%{public}d, outGroupRowId:%{public}lld, ts = %{public}lld",
        ret,
        (long long) outGroupRowId,
        (long long) time(NULL));
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertGroup failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return outGroupRowId;
}

/**
 * @brief Insert data into the cloud_raw_contact table
 *
 * @param table Insert tableName
 * @param initialValues Parameters to be passed for insert operation
 *
 * @return The result returned by the insert operation
 */
int64_t ContactsDataBase::InsertCloudRawContacts(std::string table, OHOS::NativeRdb::ValuesBucket initialValues)
{
    int64_t outRowId = OHOS::NativeRdb::E_OK;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Insert(outRowId, table, initialValues);
    });
    HILOG_INFO("InsertCloudRawContacts result:%{public}d, ts = %{public}lld", ret, (long long) time(NULL));
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    return outRowId;
}

int64_t ContactsDataBase::InsertPoster(const std::string &table, OHOS::NativeRdb::ValuesBucket initialValues)
{
    int64_t outRowId = OHOS::NativeRdb::E_OK;
    int ret = HandleRdbStoreRetry([&]() { return store_->Insert(outRowId, table, initialValues); });
    HILOG_INFO("InsertPoster result:%{public}d, ts = %{public}lld", ret, (long long) time(NULL));
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    return outRowId;
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
    std::string phoneNumber;
    int types = -1;
    if (initialValues.HasColumn(ContactBlockListColumns::PHONE_NUMBER) &&
        initialValues.HasColumn(ContactBlockListColumns::TYPES)) {
        OHOS::NativeRdb::ValueObject phoneNumberValue;
        initialValues.GetObject(ContactBlockListColumns::PHONE_NUMBER, phoneNumberValue);
        phoneNumberValue.GetString(phoneNumber);
        if (phoneNumber.empty()) {
            HILOG_ERROR("InsertBlockList failed: no phoneNumber");
            return RDB_EXECUTE_FAIL;
        }
        OHOS::NativeRdb::ValueObject typesValue;
        initialValues.GetObject(ContactBlockListColumns::TYPES, typesValue);
        GetContactByValue(types, typesValue);
        if (types < 0) {
            HILOG_ERROR("InsertBlockList failed: invalid type");
            return RDB_EXECUTE_FAIL;
        }
    }
    std::string formatPhoneNumber = GetE164FormatPhoneNumber(phoneNumber);
    if (formatPhoneNumber.empty()) {
        initialValues.PutString(ContactBlockListColumns::FORMAT_PHONE_NUMBER, phoneNumber);
    } else {
        initialValues.PutString(ContactBlockListColumns::FORMAT_PHONE_NUMBER, formatPhoneNumber);
    }
#ifdef ABILITY_CUST_SUPPORT
    if (types == FULL_MATCH_BLOCK_REASON) {
        int interceptionCallCount = QueryInterceptionCallCount(phoneNumber);
        initialValues.PutInt(ContactBlockListColumns::INTERCEPTION_CALL_COUNT, interceptionCallCount);
    } else if (types == START_WITH_BLOCK_REASON) {
        int interceptionCallCount = QueryStartWithTypeInterceptionCallCount(phoneNumber);
        initialValues.PutInt(ContactBlockListColumns::INTERCEPTION_CALL_COUNT, interceptionCallCount);
    }
#endif
    int ret = BlocklistDataBase::store_->Insert(outRowId, table, initialValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("InsertBlockList failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    std::vector<std::string> phoneNumbers;
    phoneNumbers.push_back(phoneNumber);
    if (types == FULL_MATCH_BLOCK_REASON) {
        NotifyMmsUpdateInterceptionCount(phoneNumbers, COMMAND_UPDATE_FULL_MATCH_MSG_COUNT);
    } else if (types == START_WITH_BLOCK_REASON) {
        NotifyMmsUpdateInterceptionCount(phoneNumbers, COMMAND_UPDATE_START_MATCH_MSG_COUNT);
    }
    return outRowId;
}

/**
 * @brief query data in calllog table return the count of intercepted by start with type
 *
 * @param phoneNumber query phoneNumber
 *
 * @return The result returned by the insert operation
 */
int ContactsDataBase::QueryStartWithTypeInterceptionCallCount(std::string phoneNumber)
{
    int interceptionCallCount = 0;
    // 搜索统计通话被拦截（answer_state = 6）,拦截类型为开头号码匹配（block_reason = 6）的次数
    std::string queryInterceptionCallCount =
        "select count(*) as interception_call_count, detect_details from calllog where answer_state = 6 ";
    queryInterceptionCallCount.append("and block_reason = 6 ");
    queryInterceptionCallCount.append("and detect_details = '" + phoneNumber + "' ");
    queryInterceptionCallCount.append("group by detect_details");
    auto resultSet = CallLogDataBase::store_->QuerySql(queryInterceptionCallCount);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryStartWithTypeInterceptionCallCount QuerySqlResult is null");
        return interceptionCallCount;
    }
    int getRowResult = resultSet->GoToFirstRow();
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        std::string columnName = ContactBlockListColumns::INTERCEPTION_CALL_COUNT;
        int columnIndex = 0;
        int currentCount = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetInt(columnIndex, currentCount);
        interceptionCallCount += currentCount;
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
    HILOG_INFO("ContactsDataBase QueryStartWithTypeInterceptionCallCount interceptionCallCount  %{public}d",
        interceptionCallCount);
    return interceptionCallCount;
}

/**
 * @brief query data in calllog table return the count of interception
 *
 * @param phoneNumber query phoneNumber
 *
 * @return The result returned by the insert operation
 */
#ifdef ABILITY_CUST_SUPPORT
int ContactsDataBase::QueryInterceptionCallCount(std::string phoneNumber)
{
    int interceptionCallCount = 0;
    // 搜索统计通话被拦截（answer_state = 6）,拦截类型为全匹配（block_reason = 1）的次数
    std::string queryInterceptionCallCount =
        "select count(*) as interception_call_count, phone_number from calllog where answer_state = 6 ";
    queryInterceptionCallCount.append("and block_reason = 1 ");
    if (phoneNumber.length() >= ENHANCED_QUERY_LENGTH) {
        // 当号码长度大于7时模糊匹配后七位
        queryInterceptionCallCount.append(
            "and phone_number like '%" +
            phoneNumber.substr(phoneNumber.length() - ENHANCED_QUERY_LENGTH, ENHANCED_QUERY_LENGTH) + "' ");
    } else {
        // 小于七位直接匹配
        queryInterceptionCallCount.append("and phone_number = '" + phoneNumber + "' ");
    }
    queryInterceptionCallCount.append("group by phone_number");
    auto resultSet = CallLogDataBase::store_->QuerySql(queryInterceptionCallCount);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryInterceptionCallCount QuerySqlResult is null");
        return interceptionCallCount;
    }
    int getRowResult = resultSet->GoToFirstRow();
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        std::string columnName = ContactBlockListColumns::INTERCEPTION_CALL_COUNT;
        int columnIndex = 0;
        int currentCount = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetInt(columnIndex, currentCount);
        std::string columnPhoneName = ContactBlockListColumns::PHONE_NUMBER;
        int columnPhoneIndex = 0;
        std::string targetPhone;
        resultSet->GetColumnIndex(columnPhoneName, columnPhoneIndex);
        resultSet->GetString(columnPhoneIndex, targetPhone);
        if (this->CompareNumbers(phoneNumber, targetPhone)) {
            // 模糊匹配后需通过TelCustManager的CompareNumbers比较搜索结果中的号码是否与搜索号码完全匹配，仅完全匹配时增加计数
            interceptionCallCount += currentCount;
        }
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
    HILOG_INFO("ContactsDataBase QueryInterceptionCallCount interceptionCallCount  %{public}d", interceptionCallCount);
    return interceptionCallCount;
}

bool ContactsDataBase::CompareNumbers(std::string number1, std::string number2)
{
    // 搜索统计通话被拦截（answer_state = 6）的次数
    // 动态加载telephony组件的so，并获取compareNumbers方法
    void *telephonyHandel = dlopen(TELEPHONY_CUST_SO_PATH.c_str(), RTLD_LAZY);
    if (telephonyHandel == nullptr) {
        return false;
    }

    typedef bool (*CompareNumbers)(std::string, std::string);
    CompareNumbers compareNumbers = reinterpret_cast<CompareNumbers>(dlsym(telephonyHandel, "ComparePhoneNumbers"));
    if (compareNumbers == nullptr) {
        dlclose(telephonyHandel);
        return false;
    }
    bool result = compareNumbers(number1, number2);
    dlclose(telephonyHandel);
    return result;
}
#endif

/**
 * @brief Notify mms update blocklist
 *
 * @param phoneNumber Notify phoneNumber
 *
 */
void ContactsDataBase::NotifyMmsUpdateInterceptionCount(std::vector<std::string> phoneNumbers, int blockType)
{
    std::shared_ptr<SpamCallAdapter> spamCallAdapterPtr_ = std::make_shared<SpamCallAdapter>();
    if (spamCallAdapterPtr_ == nullptr) {
        HILOG_ERROR("create SpamCallAdapter object failed!");
        return;
    }
    spamCallAdapterPtr_->UpdateBlockedMsgCount(phoneNumbers, blockType);
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::DownLoadPosters(const std::string &poster)
{
    std::shared_ptr<PosterCallAdapter> posterCallAdapterPtr_ = std::make_shared<PosterCallAdapter>();
    if (posterCallAdapterPtr_ == nullptr) {
        HILOG_ERROR("create PosterCallAdapter object failed!");
        return nullptr;
    }

    posterCallAdapterPtr_->DownLoadPosters(poster);
    
    return nullptr;
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
    OHOS::NativeRdb::ValuesBucket contactDataValues, OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateContactData store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    // 查询符合更新条件的types信息和calendarEventId信息，返回rawid信息
    std::vector<std::string> types;
    std::string calendarEventIds;
    std::vector<int> rawContactIdVector =
        QueryContactDataRawContactId(rdbPredicates, types, calendarEventIds, "UpdateContactData");
    // 记录更新contactdata信息的rawcontactid，此操作属于更新contact_data，通知使用
    ContactsDataBase::updateContactIdVector.productionMultiple(rawContactIdVector);
    int changedRows = OHOS::NativeRdb::E_OK;
    ret = store_->Update(changedRows, contactDataValues, rdbPredicates);
    if (ret == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        ret = store_->Restore("contacts.db.bak");
        HILOG_ERROR("UpdateContactData Update Restore retCode= %{public}d", ret);
    }
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
    if (isSync != "true") {
        mDirtyRawContacts.productionMultiple(rawContactIdVector);
    }
    std::vector<int> needUpdateCalllogRawIdVector;
    bool isAffectsCalllogFlag = false;
    for (unsigned int i = 0; i < rawContactIdVector.size(); i++) {
        int rawContactId = rawContactIdVector[i];
        std::string type = types[i];
        if ((type == ContentTypeData::PHONE || type == ContentTypeData::NAME) &&
            contactDataValues.HasColumn(ContactDataColumns::DETAIL_INFO)) {
            mUpdateRawContacts.push_back(rawContactId);
            // 畅联更新可能更新手机号的data的畅联标记，但没有更新detail_info，
            // 更新了detail_info才影响通话记录
            needUpdateCalllogRawIdVector.push_back(rawContactId);
            isAffectsCalllogFlag = true;
        }
    }

    // 异步更新通话记录
    MergeUpdateTask(store_, needUpdateCalllogRawIdVector, false, isAffectsCalllogFlag);
    return changedRows;
}

int ContactsDataBase::UpdatePrivacyContactsBackup(
    OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("UpdatePrivacyContactsBackup store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int changedRows = OHOS::NativeRdb::E_OK;
    auto ret = store_->Update(changedRows, values, rdbPredicates);
    if (ret == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        ret = store_->Restore("contacts.db.bak");
        HILOG_ERROR("UpdatePrivacyContactsBackup Update Restore retCode= %{public}d", ret);
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdatePrivacyContactsBackup failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return changedRows;
}

void ContactsDataBase::updateContactDataRawIdInfo(
    std::vector<OHOS::NativeRdb::ValuesBucket> &contactDataValues, int rawId)
{
    for (unsigned int i = 0; i < contactDataValues.size(); i++) {
        contactDataValues[i].Delete("raw_contact_id");
        contactDataValues[i].PutInt("raw_contact_id", rawId);
    }
}

int ContactsDataBase::addContactInfoBatch(const std::vector<DataShare::DataShareValuesBucket> &contactDataViewValues)
{
    int ret = RDB_EXECUTE_FAIL;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase addContactInfoBatch store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    // 获得插入的联系人id，插入的信息头像数量，用于记录日志
    std::set<int> contactIdSet;
    int addPhotoSize = 0;
    ret = getInfoFormValueBucket(contactDataViewValues, contactIdSet, addPhotoSize);
    if (ret == OPERATION_ERROR) {
        HILOG_ERROR("ContactsDataBase addContactInfoBatch getContactIdSetFormValueBucket error");
        return OPERATION_ERROR;
    }
    // 开启事务
    ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }

    // 1. 根据view_contact_data信息 将信息根据联系人id或rawId分组
    // {rawId, 对应的contact_data}，一对多
    std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>> rawIdToContactDataMap;
    // {contactId, 对应的contact}，一对一
    std::map<int, OHOS::NativeRdb::ValuesBucket> contactIdToContactMap;
    // {oldContactId, 对应的old name_raw_contact_id}
    std::map<int, int> oldContactIdToOldNameRawContactIdMap;
    // {contactId, 对应的raw}，一对多
    std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>> contactIdToRawMap;
    
    groupByContactId(contactDataViewValues, rawIdToContactDataMap,
        contactIdToContactMap, oldContactIdToOldNameRawContactIdMap, contactIdToRawMap);

    // 2. 查询最大的id信息，设置给要插入的信息
    // (1)根据contact自增id，给contact 设置id（name_raw_contact_id 后续 通过更新设置），给rawcontact设置contactId外键
    // 记录：<newContactId, oldContactId>
    // (2)遍历contactI的raw，根据raw自增id，给raw_contact 设置rawid，给raw设置contactId外键
    // 记录<oldRawId, curRawId>，关联关系；记录新增的rawId，用于后续更新通话记录
    // (3) 遍历raw的contact_data，给contact_data设置rawcontactId外键；
    // (4)通过<oldContactId, oldNameRawId>，找到curContactId和curNameRawId对应关系
    int maxIdContact = selectMaxIdAutoIncr("contact");
    int maxIdRawContact = selectMaxIdAutoIncr("raw_contact");
    // <oldRawId, newRawId>记录老的rawId和新的rawId的关联关系
    std::map<int, int> oldRawIdToNewRawIdMap;
    // <newContactId, oldContactId>
    std::map<int, int> newContactIdToOldContactIdMap;
    // 新的rawcontactId的集合，用于更新通话记录
    std::vector<int> newRawContactIdVector;
    // （1）遍历contact信息
    std::map<int, OHOS::NativeRdb::ValuesBucket>::iterator iter;
    bool needReassociateBigPhotoFlag = false;
    // 解析出的contactId字符串，如果解析出的联系人数量比插入数量少，打印解析出的id
    std::string parseContactIdStr;
    for (iter = contactIdToContactMap.begin(); iter != contactIdToContactMap.end(); iter++) {
        int oldContactId = iter->first;
        parseContactIdStr.append(std::to_string(oldContactId));
        parseContactIdStr.append(",");
        OHOS::NativeRdb::ValuesBucket &contactValueBucket = iter->second;
        int curContactId = ++maxIdContact;
        // 设置contact的联系人新id
        contactValueBucket.Delete("id");
        contactValueBucket.PutInt("id", curContactId);
        contactValueBucket.Delete("name_raw_contact_id");
        contactValueBucket.Delete("personal_ringtone");
        contactValueBucket.PutString("personal_ringtone", "");
        contactValueBucket.Delete("personal_notification_ringtone");
        contactValueBucket.PutString("personal_notification_ringtone", "");
        contactValueBucket.Delete("ringtone_path");
        contactValueBucket.PutString("ringtone_path", "");
        // 记录新老id对应关系
        newContactIdToOldContactIdMap[curContactId] = oldContactId;

        // (2)遍历contactId对应的所有raw，给raw_contact设置rawid信息和contactId信息
        std::vector<OHOS::NativeRdb::ValuesBucket> &rawVector = contactIdToRawMap[oldContactId];
        for (unsigned int i = 0; i < rawVector.size(); i++) {
            int curRawContactId = ++maxIdRawContact;
            // 记录新的rawId
            newRawContactIdVector.push_back(curRawContactId);
            // 获取之前的id
            int beforeRawId = ContactsDataAbility::getIntValueFromRdbBucket(rawVector[i], "id");
            // 设置新的id信息和contactId信息
            rawVector[i].Delete("id");
            rawVector[i].PutInt("id", curRawContactId);
            ContactsDataBase::updateContactIdVector.production(curRawContactId);
            rawVector[i].Delete("contact_id");
            rawVector[i].PutInt("contact_id", curContactId);
            // 记录<oldRawId, newRawId> 对应关系
            oldRawIdToNewRawIdMap[beforeRawId] = curRawContactId;
            // (3)给data信息设置新的rawcontactid外键
            std::vector<OHOS::NativeRdb::ValuesBucket> &dataVector = rawIdToContactDataMap[beforeRawId];
            updateContactDataRawIdInfo(dataVector, curRawContactId);
            if (getNeedReassociateBigPhotoFlag(dataVector)) {
                needReassociateBigPhotoFlag = true;
            }
        }
    }
    
    HILOG_ERROR("addContactInfoBatch set id info finsh");
    // 3.从分组map拿到要插入的信息
    // 先批量插入contact，然后批量插入raw，然后更新contact的name_raw_contact_id外键
    std::vector<OHOS::NativeRdb::ValuesBucket> contactVector;
    std::vector<OHOS::NativeRdb::ValuesBucket> rawContactVector;
    std::vector<OHOS::NativeRdb::ValuesBucket> contactDataVector;
    // (1)获取待插入 的 contact 信息
    parseMapValueToVector(contactIdToContactMap, contactVector);
    // (2)获取 待插入 的 raw_contact 的valueBucket
    parseMapValueToVector(contactIdToRawMap, rawContactVector);
    // (3) 获取 待插入 的  contact_data 的valueBucket
    parseMapValueToVector(rawIdToContactDataMap, contactDataVector);
    HILOG_WARN("addContactInfoBatch add contactsize: %{public}ld, rawsize: %{public}ld, "
        "datasize: %{public}ld, contactIdSetSize: %{public}ld, photoSize: %{public}d, parseContactId: %{public}s",
        (long) contactVector.size(), (long) rawContactVector.size(), (long) contactDataVector.size(), (long) contactIdSet.size(),
        addPhotoSize, parseContactIdStr.c_str());
    if (contactIdSet.size() != contactVector.size()) {
        std::string addContactIdStr;
        std::set<int>::iterator it;
        for (it = contactIdSet.begin(); it != contactIdSet.end(); it++) {
            addContactIdStr.append(std::to_string(*it));
            addContactIdStr.append(",");
        }
        // 解析错误，解析出来的联系人少了，直接返回
        HILOG_ERROR("contact parse size error, addContactId: %{public}s", addContactIdStr.c_str());
        return RDB_EXECUTE_FAIL;
    }

    // 4.插入数据
    ret = batchInsertContactRelatedInfo(contactVector, rawContactVector, contactDataVector);
    // 插入失败，回滚
    if (ret != OHOS::NativeRdb::E_OK) {
        RollBack();
        HILOG_ERROR("addContactInfoBatch insert data failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }

    // 5.更新contact的nameRawContactId
    HILOG_WARN("addContactInfoBatch insert finsh");
    for (unsigned int i = 0; i < contactVector.size(); i++) {
        // 获取contact的newId
        int contactId = ContactsDataAbility::getIntValueFromRdbBucket(contactVector[i], "id");
        // newId找到oldId
        int oldContactId = newContactIdToOldContactIdMap[contactId];
        // oldId 找到oldNameRawId
        int nameRawContactIdOld = oldContactIdToOldNameRawContactIdMap[oldContactId];
        // oldNameRawId找到newNameRawId
        int nameRawContactIdNew = oldRawIdToNewRawIdMap[nameRawContactIdOld];
        // 注释： HILOG_WARN("meng update contact , contactId: %{public}d, oldContactId: %{public}d, "
        // 注释：    "oldrawid:%{public}d, newrawid: %{public}d",
        // 注释：    contactId, oldContactId, nameRawContactIdOld, nameRawContactIdNew);

        ret = updateContactNameRawId(contactId, nameRawContactIdNew);
        if (ret == RDB_EXECUTE_FAIL) {
            RollBack();
            HILOG_ERROR("addContactInfoBatch update contact nameRawContactId fail, "
                "contactId: %{public}d, ret: %{public}d",
                contactId, ret);
            return RDB_EXECUTE_FAIL;
        }
    }

    HILOG_WARN("addContactInfoBatch update finsh");
    // 提交事务
    if (ret != OHOS::NativeRdb::E_OK) {
        RollBack();
        HILOG_ERROR("addContactInfoBatch update contact NameRawContactId fail :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }

    ret = Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("addContactInfoBatch commit fail :%{public}d, rollback", ret);
        RollBack();
        return RDB_EXECUTE_FAIL;
    }

    // 新增成功，异步上报联系人列表数量；通过js上报
    boardReportBatchInsert(contactVector.size());
    // 同步到主空间privacy_contacts_backup
    if (PrivacyContactsManager::IsPrivacySpace()) {
        PrivacyContactsManager::GetInstance()->InsertPrivacContactsByContactDatas(contactDataVector);
    }
    // 更新通话记录
    MergeUpdateTask(store_, newRawContactIdVector, false);
    // 将raw信息更新为dirty，后续云同步处理
    mDirtyRawContacts.productionMultiple(newRawContactIdVector);

    // 同步日历信息
    int syncSize = SyncBirthToCalAfterBatchInsert(contactDataVector);
    if (needReassociateBigPhotoFlag) {
        // 同步大头像信息
        contactsConnectAbility_->ConnectAbility("", "", "", "", "reassociateBigPhoto", "");
    }
    
    // 更新是否需要同步主空间数据标记
    if (isMainSpace()) {
        ret = updateMainNeedSyncStatus(syncSize, needReassociateBigPhotoFlag);
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("addContactInfoBatch updateMainNeedSyncStatus  error:%{public}d", ret);
    }

    HILOG_WARN("addContactInfoBatch commit finsh");
    return contactVector.size();
}

std::string ContactsDataBase::GetPhoneNumsFromContactData(const std::vector<NativeRdb::ValuesBucket> &contactData)
{
    NativeRdb::ValueObject valueObj;
    std::string phoneNumbers;
    for (const auto &contactData : contactData) {
        contactData.GetObject("type_id", valueObj);
        int64_t typeId = -1;
        valueObj.GetLong(typeId);
        if (typeId == ContentTypeData::PHONE_INT_VALUE) {
            contactData.GetObject("detail_info", valueObj);
            std::string phoneNumber;
            valueObj.GetString(phoneNumber);
            phoneNumbers += phoneNumber + ",";
        }
    }
    return phoneNumbers;
}

void ContactsDataBase::ReportOperationAuditLog(const std::string &auditRequest,
    const std::vector<NativeRdb::ValuesBucket> &rawContacts,
    const std::map<int, std::vector<NativeRdb::ValuesBucket>> &contactDataMap, std::function<int(int)> GetOldContactId)
{
    std::vector<NativeRdb::ValuesBucket> contactInfos;
    for (const auto &contact : rawContacts) {
        NativeRdb::ValuesBucket valueBucket;
        NativeRdb::ValueObject valueObj;
        contact.GetObject("contact_id", valueObj);
        int64_t contactId = -1;
        valueObj.GetLong(contactId);
        valueBucket.PutLong("contact_id", contactId);
        contact.GetObject("display_name", valueObj);
        std::string displayName;
        valueObj.GetString(displayName);
        valueBucket.PutString("display_name", displayName);
        int nameRawContactIdOld = GetOldContactId(contactId);
        auto iter = contactDataMap.find(nameRawContactIdOld);
        std::string phoneNumbers;
        if (iter != contactDataMap.end()) {
            auto contactDatas = iter->second;
            phoneNumbers = GetPhoneNumsFromContactData(contactDatas);
        }
        auto phoneNumbersStr = phoneNumbers.empty() ? phoneNumbers : phoneNumbers.substr(0, phoneNumbers.size() - 1);
        valueBucket.PutString("phoneNumbers", phoneNumbersStr);
        contactInfos.emplace_back(valueBucket);
    }
    size_t contactCount = static_cast<size_t>(GetContactCount());
    HiAudit::GetInstance().WriteLog(auditRequest, OHOS::OperationType::OPERATION_ADD, contactInfos, contactCount);
}

int64_t ContactsDataBase::GetContactCount()
{
    int64_t contactCount = 0;
    if (store_ == nullptr) {
        HILOG_ERROR("%{public}s, store is null", __func__);
        return contactCount;
    }
    const std::string sql = "SELECT count(*) FROM view_contact WHERE is_deleted='0' AND primary_contact<>'1'";
    auto resultSet = store_->QuerySql(sql);
    if (resultSet != nullptr) {
        int getRowResult = resultSet->GoToFirstRow();
        if (getRowResult == OHOS::NativeRdb::E_OK) {
            resultSet->GetLong(0, contactCount);
        }
        resultSet->Close();
    }
    HILOG_INFO("%{public}s, contactCount: %{private}lld", __func__, (long long) contactCount);
    return contactCount;
}

int ContactsDataBase::updateMainNeedSyncStatus(int birthSyncSize, bool needReassociateBigPhotoFlag)
{
    HILOG_WARN("updateMainNeedSyncStatus %{public}d, %{public}s",
        birthSyncSize, needReassociateBigPhotoFlag ? "Y" : "N");
    int syncStatus = MAIN_SPACE_SYNC_DEFAULT;
    syncStatus += (1 << MAIN_SPACE_NEED_SYNC_BIT);
    if (birthSyncSize > 0) {
        syncStatus += (1 << MAIN_SPACE_NEED_SYNC_BIRTH_BIT);
    }
    if (needReassociateBigPhotoFlag) {
        syncStatus += (1 << MAIN_SPACE_NEED_SYNC_BIG_PHOTO_BIT);
    }
    int ret = updateMainNeedSyncStatus(syncStatus);
    return ret;
}

bool ContactsDataBase::getNeedReassociateBigPhotoFlag(std::vector<OHOS::NativeRdb::ValuesBucket> &contactDataValues)
{
    bool needReassociateBigPhotoFlag = false;
    for (unsigned int i = 0; i < contactDataValues.size(); i++) {
        std::string needReassociateBigPhotoFlagStr = ContactsDataAbility::getStringValueFromRdbBucket(
            contactDataValues[i], "needReassociateBigPhotoFlag");
        contactDataValues[i].Delete("needReassociateBigPhotoFlag");
        if (!needReassociateBigPhotoFlagStr.empty() && needReassociateBigPhotoFlagStr == "Y") {
            needReassociateBigPhotoFlag = true;
        }
    }
    return needReassociateBigPhotoFlag;
}

int ContactsDataBase::updateContactNameRawId(int contactId, int nameRawId)
{
    int ret = OHOS::NativeRdb::E_OK;
    std::string whereClause;
    whereClause.append("id");
    whereClause.append(" = ? ");

    std::vector<std::string> whereArgs;
    whereArgs.push_back(std::to_string(contactId));

    OHOS::NativeRdb::ValuesBucket valuesUpContacts;
    valuesUpContacts.PutInt(ContactColumns::NAME_RAW_CONTACT_ID, nameRawId);
    int changedRows = OHOS::NativeRdb::E_OK;
    ret = store_->Update(
        changedRows, ContactTableName::CONTACT, valuesUpContacts, whereClause, whereArgs);
    if (ret == RDB_EXECUTE_FAIL || changedRows == 0) {
        HILOG_ERROR("update contact nameRawContactId fail, "
            "contactId: %{public}d, changedRows:%{public}d, ret: %{public}d",
            contactId, changedRows, ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

int ContactsDataBase::batchInsertContactRelatedInfo(std::vector<OHOS::NativeRdb::ValuesBucket> &contactVector,
    std::vector<OHOS::NativeRdb::ValuesBucket> &rawContactVector,
    std::vector<OHOS::NativeRdb::ValuesBucket> &contactDataVector)
{
    int ret = OHOS::NativeRdb::E_OK;
    // （1）插入contact
    int64_t insertCountCotact = 0;
    ret = store_->BatchInsert(insertCountCotact, "contact", contactVector);
    if (ret != OHOS::NativeRdb::E_OK || insertCountCotact != static_cast<int64_t>(contactVector.size())) {
        HILOG_ERROR("batchInsertContactRelatedInfo contact fail, insertSize:%{public}lld, realSize: %{public}ld",
            (long long) insertCountCotact, (long) contactVector.size());
        return RDB_EXECUTE_FAIL;
    }

    // （2）插入raw_contact
    int64_t insertCountRawcontact = 0;
    ret = store_->BatchInsert(insertCountRawcontact, "raw_contact", rawContactVector);
    if (ret != OHOS::NativeRdb::E_OK || insertCountRawcontact != static_cast<int64_t>(rawContactVector.size())) {
        HILOG_ERROR("batchInsertContactRelatedInfo rawcontact fail, insertSize:%{public}lld, realSize: %{public}ld",
            (long long) insertCountRawcontact, (long) rawContactVector.size());
        return RDB_EXECUTE_FAIL;
    }

    // （3）插入contact_data
    int64_t insertCountData = 0;
    ret = store_->BatchInsert(insertCountData, "contact_data", contactDataVector);
    if (ret != OHOS::NativeRdb::E_OK || insertCountData != static_cast<int64_t>(contactDataVector.size())) {
        HILOG_ERROR("batchInsertContactRelatedInfo contactdata fail, insertSize:%{public}lld, realSize: %{public}ld",
            (long long) insertCountData, (long) contactDataVector.size());
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

void ContactsDataBase::parseMapValueToVector(std::map<int, OHOS::NativeRdb::ValuesBucket> &map_,
    std::vector<OHOS::NativeRdb::ValuesBucket> &vector_)
{
    std::map<int, OHOS::NativeRdb::ValuesBucket>::iterator iter;
    for (iter = map_.begin(); iter != map_.end(); iter++) {
        vector_.push_back(iter->second);
    }
}
void ContactsDataBase::parseMapValueToVector(std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>> &map_,
    std::vector<OHOS::NativeRdb::ValuesBucket> &vector_)
{
    std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>>::iterator iterDataMap;
    for (iterDataMap = map_.begin(); iterDataMap != map_.end(); iterDataMap++) {
        std::vector<OHOS::NativeRdb::ValuesBucket> &subDataVector = iterDataMap->second;
        vector_.insert(vector_.end(), subDataVector.begin(), subDataVector.end());
    }
}

int ContactsDataBase::selectMaxIdAutoIncr(std::string tableName)
{
    int maxId = 0;
    std::string selectSql = "select seq from sqlite_sequence where name = '";
    selectSql.append(tableName);
    selectSql.append("'");
    auto resultSet = store_->QuerySql(selectSql);
    if (resultSet == nullptr) {
        HILOG_ERROR("selectMaxIdAutoIncr QuerySqlResult is null");
        return maxId;
    }
    if (resultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
        resultSet->GetInt(0, maxId);
    }
    resultSet->Close();
    return maxId;
}

void ContactsDataBase::copyStringTypeColValueToTarget(const OHOS::NativeRdb::ValuesBucket &sourceValuesBucket,
    OHOS::NativeRdb::ValuesBucket &targetValuesBucket, std::string colName)
{
    if (!sourceValuesBucket.HasColumn(colName)) {
        return;
    }
    std::string colVal = ContactsDataAbility::getStringValueFromRdbBucket(sourceValuesBucket, colName);
    targetValuesBucket.PutString(colName, colVal);
}

void ContactsDataBase::copyStringTypeColValueToTarget(const OHOS::NativeRdb::ValuesBucket &sourceValuesBucket,
    OHOS::NativeRdb::ValuesBucket &targetValuesBucket, std::string sourceColName, std::string targetColName)
{
    if (!sourceValuesBucket.HasColumn(sourceColName)) {
        return;
    }
    std::string colVal = ContactsDataAbility::getStringValueFromRdbBucket(sourceValuesBucket, sourceColName);
    targetValuesBucket.PutString(targetColName, colVal);
}

void ContactsDataBase::copyIntTypeColValueToTarget(const OHOS::NativeRdb::ValuesBucket &sourceValuesBucket,
    OHOS::NativeRdb::ValuesBucket &targetValuesBucket, std::string colName)
{
    int colVal = ContactsDataAbility::getIntValueFromRdbBucket(sourceValuesBucket, colName);
    if (colVal != OPERATION_ERROR) {
        targetValuesBucket.PutInt(colName, colVal);
    }
}

void ContactsDataBase::copyIntTypeColValueToTarget(const OHOS::NativeRdb::ValuesBucket &sourceValuesBucket,
    OHOS::NativeRdb::ValuesBucket &targetValuesBucket, std::string sourceColName, std::string targetColName)
{
    int colVal = ContactsDataAbility::getIntValueFromRdbBucket(sourceValuesBucket, sourceColName);
    if (colVal != OPERATION_ERROR) {
        targetValuesBucket.PutInt(targetColName, colVal);
    }
}

OHOS::NativeRdb::ValuesBucket ContactsDataBase::parseContactInfo(
    const OHOS::NativeRdb::ValuesBucket &contactDataViewValuesBucket)
{
    OHOS::NativeRdb::ValuesBucket contactValuesBucket;

    // 字符串类型的字段
    std::vector colNameVectorStringType = {ContactColumns::PERSONAL_RINGTONE,
        ContactColumns::COMPANY, ContactColumns::QUICK_SEARCH_KEY,
        // 视图没有： ContactColumns::POSITION,
        ContactColumns::PERSONAL_NOTIFICATION_RINGTONE,
        ContactColumns::FORM_ID, ContactColumns::FOCUS_MODE_LIST};
    for (unsigned int i = 0; i < colNameVectorStringType.size(); i++) {
        copyStringTypeColValueToTarget(contactDataViewValuesBucket, contactValuesBucket,
            colNameVectorStringType[i]);
    }

       // 数字类型的字段
    std::vector colNameVectorIntType = {// 这俩id都没有用？：ContactColumns::PHOTO_ID, ContactColumns::PHOTO_FILE_ID,
        ContactColumns::IS_TRANSFER_VOICEMAIL,
        ContactColumns::HAS_PHONE_NUMBER,
        ContactColumns::HAS_DISPLAY_NAME,
        ContactColumns::HAS_EMAIL,
        ContactColumns::HAS_GROUP,
        ContactColumns::PREFER_AVATAR};
    for (unsigned int i = 0; i < colNameVectorIntType.size(); i++) {
        copyIntTypeColValueToTarget(contactDataViewValuesBucket, contactValuesBucket, colNameVectorIntType[i]);
    }

    copyIntTypeColValueToTarget(contactDataViewValuesBucket, contactValuesBucket, "contact_id", "id");
    copyIntTypeColValueToTarget(contactDataViewValuesBucket, contactValuesBucket, "contact_read_only", "read_only");
    // 迁移联系人，迁移到的空间属于新增，设置时间戳
    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    contactValuesBucket.PutLong("contact_last_updated_timestamp", ms.count());
    return contactValuesBucket;
}

// 解析出raw_contact的valueBucket
OHOS::NativeRdb::ValuesBucket ContactsDataBase::parseRawContactInfo(
    const OHOS::NativeRdb::ValuesBucket &contactDataViewValuesBucket)
{
    OHOS::NativeRdb::ValuesBucket rawContactValuesBucket;

    // 字符串类型的字段
    std::vector colNameVectorStringType = {
        RawContactColumns::PHOTO_FIRST_NAME, RawContactColumns::DISPLAY_NAME, RawContactColumns::SORT,
        RawContactColumns::POSITION, RawContactColumns::SORT_FIRST_LETTER, RawContactColumns::SORT_KEY,
        "extra1", "extra2", "extra3", "extra4", "unique_key", "focus_mode_list"};
        // uuid 不需要迁移，主空间迁出 会删除云端，RawContactColumns::UUID不用了
        // 主空间迁入 需上云，RawContactColumns::UUID 重新生成
    for (unsigned int i = 0; i < colNameVectorStringType.size(); i++) {
        copyStringTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket,
            colNameVectorStringType[i]);
    }

    // 数字类型的字段
    std::vector colNameVectorIntType = {
        // 视图的是 contact表的：RawContactColumns::PHOTO_ID, RawContactColumns::PHOTO_FILE_ID,
        RawContactColumns::IS_DELETED, RawContactColumns::ACCOUNT_ID,
        RawContactColumns::CONTACTED_COUNT, RawContactColumns::LASTEST_CONTACTED_TIME, RawContactColumns::FAVORITE,
        RawContactColumns::FAVORITE_ORDER, RawContactColumns::PHONETIC_NAME_TYPE,
        RawContactColumns::MERGE_MODE, RawContactColumns::IS_NEED_MERGE, RawContactColumns::MERGE_STATUS,
        RawContactColumns::IS_MERGE_TARGET, RawContactColumns::VIBRATION_SETTING, RawContactColumns::SYNC_ID,
        "primary_contact", RawContactColumns::DIRTY, RawContactColumns::AGGREGATION_STATUS};
    for (unsigned int i = 0; i < colNameVectorIntType.size(); i++) {
        copyIntTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket, colNameVectorIntType[i]);
    }

    copyIntTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket, "raw_contact_id", "id");
    copyIntTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket,
        "raw_is_transfer_voicemail", "is_transfer_voicemail");
    copyIntTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket, "raw_version", "version");
    copyIntTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket, "raw_read_only", "read_only");

    copyStringTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket,
        "raw_personal_ringtone", "personal_ringtone");
    copyStringTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket,
        "raw_personal_notification_ringtone", "personal_notification_ringtone");
    copyStringTypeColValueToTarget(contactDataViewValuesBucket,
        rawContactValuesBucket, "raw_phonetic_name", "phonetic_name");
    // 迁移raw的company
    copyStringTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket,
        "raw_company", RawContactColumns::COMPANY);
    copyStringTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket, "raw_syn_1", "syn_1");
    copyStringTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket, "raw_syn_2", "syn_2");
    copyStringTypeColValueToTarget(contactDataViewValuesBucket, rawContactValuesBucket, "raw_syn_3", "syn_3");

    // 智能合并后，存在"primary_contact"为空串的raw
    // ''!=1 为true，可以判断不是我的名片
    // 使用int参数类型迁移,迁移后,这个值变为null, null != 1 为false,会判断为是我的名片
    // 如果这个字段迁移没解析出来值,设置默认值为0
    if (!rawContactValuesBucket.HasColumn("primary_contact")) {
        rawContactValuesBucket.PutInt("primary_contact", 0);
    }

    if (rawContactValuesBucket.HasColumn("lastest_contacted_time")) {
        int lastContactedTimeTarget = ContactsDataAbility::getIntValueFromRdbBucket(rawContactValuesBucket,
            RawContactColumns::LASTEST_CONTACTED_TIME);
        if (lastContactedTimeTarget && lastContactedTimeTarget != OPERATION_ERROR){
            rawContactValuesBucket.Delete("lastest_contacted_time");
            rawContactValuesBucket.PutInt("lastest_contacted_time", static_cast<std::int32_t>(lastContactedTimeTarget));
        }
    }
    return rawContactValuesBucket;
}

// 解析出contact_data的valueBucket
OHOS::NativeRdb::ValuesBucket ContactsDataBase::parseContactDataInfo(
    const OHOS::NativeRdb::ValuesBucket &contactDataViewValuesBucket)
{
    OHOS::NativeRdb::ValuesBucket contactDataValuesBucket;
    // 字符串类型的字段
    std::vector colNameVectorStringType = {ContactDataColumns::DETAIL_INFO, "family_name", "middle_name_phonetic",
        "given_name", "given_name_phonetic", "alias_detail_info", "phonetic_name", "format_phone_number", "position",
        "location", "extend1", "extend2", "extend3", "extend4", "city", "country", "neighborhood", "pobox", "postcode",
        "region", "street", "alpha_name", "other_lan_last_name", "other_lan_first_name", "extend5", "lan_style",
        "custom_data", "extend6", "extend7", "syn_1", "syn_2", "syn_3", "extend8", "extend9", "extend10", "extend11",
        // 生日id不要迁移 "calendar_event_id"
        };
    for (unsigned int i = 0; i < colNameVectorStringType.size(); i++) {
        copyStringTypeColValueToTarget(contactDataViewValuesBucket, contactDataValuesBucket,
            colNameVectorStringType[i]);
    }

    // 数字类型的字段
    std::vector colNameVectorIntType = {ContactDataColumns::TYPE_ID, ContactDataColumns::READ_ONLY,
        ContactDataColumns::VERSION,
        ContactDataColumns::IS_PERFERRED_NUMBER, "is_sync_birthday_to_calendar", "blob_source"};
    for (unsigned int i = 0; i < colNameVectorIntType.size(); i++) {
        copyIntTypeColValueToTarget(contactDataViewValuesBucket, contactDataValuesBucket, colNameVectorIntType[i]);
    }

    // 头像
    std::string blobDataColName = "blob_data";
    if (contactDataViewValuesBucket.HasColumn(blobDataColName)) {
        OHOS::NativeRdb::ValueObject value;
        std::vector<uint8_t> colVal;
        contactDataViewValuesBucket.GetObject(blobDataColName, value);
        value.GetBlob(colVal);
        contactDataValuesBucket.PutBlob(blobDataColName, colVal);
    }

    // 如果是图片类型如果是从A空间（大头像），迁移到B空间，会变成小头像；
    // 然后再从B迁回A，此时B的记录为detailinfo为头像路径，类型为小头像，需要重新使用大头像
    int typeId = ContactsDataAbility::getIntValueFromRdbBucket(contactDataValuesBucket, ContactDataColumns::TYPE_ID);
    int blobSource = ContactsDataAbility::getIntValueFromRdbBucket(contactDataValuesBucket, "blob_source");
    // 大头像:3，大头像无法迁移文件，迁移后只保留小头像，改为小头像:2
    if (typeId == TYPE_ID_PHOTO && blobSource != PHOTO_TYPE_SMALL) {
        contactDataValuesBucket.Delete("blob_source");
        contactDataValuesBucket.PutInt("blob_source", PHOTO_TYPE_SMALL);
        // 在detail_info记录userId，在重新刷新大头像时，根据userId刷新匹配的空间
        std::string detailInfo = ContactsDataAbility::getStringValueFromRdbBucket(
            contactDataValuesBucket, "detail_info");
        contactDataValuesBucket.Delete("detail_info");
        if (isMainSpace()) {
            // 目标为main，源就是private
            detailInfo.append(";").append("privateSpace");
        } else {
            detailInfo.append(";").append("mainSpace");
        }
        contactDataValuesBucket.PutString("detail_info", detailInfo);
    } else if (typeId == TYPE_ID_PHOTO && blobSource == PHOTO_TYPE_SMALL) {
        // 如果迁移的是小头像
        std::string detailInfo = ContactsDataAbility::getStringValueFromRdbBucket(contactDataValuesBucket,
            ContactDataColumns::DETAIL_INFO);
        // detail_info 有值，类型为小头像，说明可能是之前迁移走的数据，再迁回来，需要处理大头像重新关联
        if (!detailInfo.empty()) {
            contactDataValuesBucket.PutString("needReassociateBigPhotoFlag", "Y");
        }
    }

    return contactDataValuesBucket;
}

// 从valueBucket获得信息（联系人id集合，头像信息数量）
int ContactsDataBase::getInfoFormValueBucket(const std::vector<DataShare::DataShareValuesBucket> &values,
    std::set<int> &contactIdSet, int &addPhotoSize)
{
    for (auto &valueDataShare : values) {
        // datashare类型转为rdb的valueBucket
        OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(valueDataShare);
        // 从 contact_data_view 中获取 rawId 和contactId
        int rawId = ContactsDataAbility::getIntValueFromRdbBucket(value,
            ContactDataColumns::RAW_CONTACT_ID);
        int contactId = ContactsDataAbility::getIntValueFromRdbBucket(value,
            RawContactColumns::CONTACT_ID);
        // 有contactdataview 没有 contactId，属于异常数据
        if (contactId == OPERATION_ERROR || rawId == OPERATION_ERROR) {
            HILOG_ERROR("contactDataView not has contactId");
            return OPERATION_ERROR;
        }

        std::string blobDataColName = "blob_data";
        if (value.HasColumn(blobDataColName)) {
            ++addPhotoSize;
        }
        // 记录contactId信息
        contactIdSet.insert(contactId);
    }
    return OHOS::NativeRdb::E_OK;
}

// values，contact_data_view 的所有字段查询集合
// groupByRawIdContactDataMap： 根据rawId分组，每组raw对应的contact_data的map，一对多
// groupByContactIdContactMap 根据contactId分组，每组contactId对应的Contact的map，一对一
// contactIdToNameRawContactIdMap contactId对应的name_raw_contact_id，一对一
// groupByContactIdRawMap 根据contactId分组，每组的raw的map，一对多
void ContactsDataBase::groupByContactId(const std::vector<DataShare::DataShareValuesBucket> &values,
    std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>> &groupByRawIdContactDataMap,
    std::map<int, OHOS::NativeRdb::ValuesBucket> &groupByContactIdContactMap,
    std::map<int, int> &contactIdToNameRawContactIdMap,
    std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>> &groupByContactIdRawMap)
{
    // <contactId，rawIdSet> ，一个contact对应多raw，用于判断是否是，contactId新的raw
    std::map<int, std::set<int>> rawIdSetGroupByContactId;
    for (auto &valueDataShare : values) {
        // datashare类型转为rdb的valueBucket
        OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(valueDataShare);
        // 从 contact_data_view 中获取 rawId 和contactId
        int rawId = ContactsDataAbility::getIntValueFromRdbBucket(value,
            ContactDataColumns::RAW_CONTACT_ID);
        int contactId = ContactsDataAbility::getIntValueFromRdbBucket(value,
            RawContactColumns::CONTACT_ID);
        
        // 每行需添加到<rawId, contactdata> 的map
        // 群组信息，不需要迁移，群组信息，迁移后会丢失
        int typeId = ContactsDataAbility::getIntValueFromRdbBucket(value,
            "type_id");
        if (typeId != TYPE_ID_GROUP && typeId != TYPE_ID_POSTER) {
            groupByRawIdContactDataMap[rawId].push_back(parseContactDataInfo(value));
        }
        // <contactId, contact> 的map处理，每一个contactId对应一个
        // <contactId, name_raw_contact_id> 处理，一对一
        if (groupByContactIdContactMap.find(contactId) == groupByContactIdContactMap.end()) {
            groupByContactIdContactMap[contactId] = parseContactInfo(value);
            contactIdToNameRawContactIdMap[contactId] = ContactsDataAbility::getIntValueFromRdbBucket(value,
                ContactColumns::NAME_RAW_CONTACT_ID);
        }

        // <contactId, rawContactVector> 的map处理，一个contact，多应多个raw，每个rawId对应一个raw
        if (rawIdSetGroupByContactId[contactId].find(rawId) == rawIdSetGroupByContactId[contactId].end()) {
            groupByContactIdRawMap[contactId].push_back(parseRawContactInfo(value));
            rawIdSetGroupByContactId[contactId].insert(rawId);
        }
    }
}

/**
 * @brief Update data in the cloud_raw_contact table
 *
 * @param values Parameters to be passed for update operation
 * @param rdbPredicates Conditions for update operation
 *
 * @return The result returned by the update operation
 */
int ContactsDataBase::UpdateCloudRawContacts(
    OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    HILOG_INFO("UpdateCloudRawContacts start, ts = %{public}lld", (long long) time(NULL));
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateCloudRawContacts store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Update(changedRows, values, rdbPredicates);
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateCloudRawContacts failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return changedRows;
}

/**
 * @brief Methods for Updating Tables.
 *
 * @param values Parameters to be passed for update operation
 * @param rdbPredicates Conditions for update operation
 *
 * @return The result returned by the update operation
 */
int ContactsDataBase::UpdateTable(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    HILOG_INFO("UpdateTable start.");
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateTable store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = HandleRdbStoreRetry([&]() { return store_->Update(changedRows, values, rdbPredicates); });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateTable failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return changedRows;
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
    OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateRawContact store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    std::vector<std::string> columns;
    columns.push_back(ContactPublicColumns::ID);
    auto rawContactResultSet = store_->QueryByStep(rdbPredicates, columns);
    int rawContactResultSetNum = rawContactResultSet->GoToFirstRow();
    std::vector<int> rawContactIdVector;
    if (rawContactResultSetNum == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        rawContactResultSetNum = store_->Restore("contacts.db.bak");
        HILOG_ERROR("UpdateRawContact Update Restore retCode= %{public}d", rawContactResultSetNum);
    }
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
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Update(changedRows, values, rdbPredicates);
    });
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
            HILOG_INFO("Restore contact size:%{public}lu", (unsigned long) rawContactIdVector.size());
            contactsUpdateHelper.UpdateCallLogByPhoneNum(rawContactIdVector, store_, false);
        }
    }

    // 畅连更新头像字段时，异步刷新通话记录
    if (values.HasColumn(RawContactColumns::EXTRA3)) {
        std::unique_ptr<AsyncItem> task = std::make_unique<AsyncMeetimeAvatarAddTask>(rawContactIdVector, store_);
        g_asyncTaskQueue->Push(task);
        g_asyncTaskQueue->Start();
    }

    // Update dirty when the favorite or favoriteorder field is contained.
    if (isSync != "true" &&
        (values.HasColumn(RawContactColumns::FAVORITE) || values.HasColumn(RawContactColumns::FAVORITE_ORDER))) {
        if (rawContactIdVector.size() > 0) {
            int favorite = RDB_EXECUTE_FAIL;
            OHOS::NativeRdb::ValueObject value;
            values.GetObject(RawContactColumns::FAVORITE, value);
            GetContactByValue(favorite, value);
            std::string rawContactIdStr = GetRawIdsToString(rawContactIdVector);
            HILOG_INFO("favorite value:%{public}d,rawContactIdStr:%{public}s", favorite, rawContactIdStr.c_str());
            mDirtyRawContacts.productionMultiple(rawContactIdVector);
        }
    }
    if (isSync != "true") {
        NotifyContactChange();
    }
    return changedRows;
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryContactCloneFlag()
{
    auto predicates = OHOS::NativeRdb::RdbPredicates("settings");
    std::string updateWheres;
    updateWheres.append("id").append(" = ? ");
    std::vector<std::string> updateArgs;
    updateArgs.push_back("1");
    predicates.SetWhereClause(updateWheres);
    predicates.SetWhereArgs(updateArgs);

    std::vector<std::string> columns;
    columns.push_back("contacts_on_card");
    auto resultSet = store_->Query(predicates, columns);
    return resultSet;
}

int ContactsDataBase::UpdateContactedStautsByPhoneNum(const OHOS::NativeRdb::ValueObject &phoneNum)
{
    ContactsType contactsType;
    int typeNameId = contactsType.LookupTypeId(store_, ContentTypeData::PHONE);
    if (typeNameId == RDB_EXECUTE_FAIL) {
        HILOG_ERROR("get type id failed.");
        return RDB_EXECUTE_FAIL;
    }

    std::string sqlQuery = "SELECT min(id) ";
    sqlQuery.append(" FROM ")
        .append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE detail_info = ? ")
        .append(" AND type_id = ")
        .append(std::to_string(typeNameId))
        .append(" AND extend8 IS NULL");
    std::vector<OHOS::NativeRdb::ValueObject> bindArgs;
    bindArgs.push_back(phoneNum);
    auto resultSet = store_->QuerySql(sqlQuery, bindArgs);
    if (resultSet == nullptr) {
        HILOG_ERROR("UpdateContactedStautsByPhoneNum QuerySqlResult is null");
        return RDB_EXECUTE_FAIL;
    }
    int contactDataId = 0;
    if (resultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
        resultSet->GetInt(0, contactDataId);
    }
    resultSet->Close();

    return UpdateContactedStautsById(contactDataId);
}

int ContactsDataBase::UpdateContactedStautsById(int contactDataId)
{
    if (contactDataId <= 0) {
        HILOG_INFO("no need update contact status.");
        return RDB_EXECUTE_OK;
    }

    std::string sqlBuild = "UPDATE ";
    sqlBuild.append(ContactTableName::CONTACT_DATA)
        .append(" SET extend8 = 1 ")
        .append(" WHERE id = ")
        .append(std::to_string(contactDataId));
    int ret = store_->ExecuteSql(sqlBuild);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("update contact status failed.");
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("update contact status end.");

    return RDB_EXECUTE_OK;
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
    HILOG_INFO("ContactsDataBase UpdateBlockList begin");
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateBlockList store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    std::vector<OHOS::NativeRdb::ValuesBucket> updateBlockListValues;
    if (values.HasColumn(ContactBlockListColumns::INTERCEPTION_MSG_COUNT)
        || values.HasColumn(ContactBlockListColumns::INTERCEPTION_CALL_COUNT)) {
        int changedRows = OHOS::NativeRdb::E_OK;
        int ret = HandleRdbStoreRetry([&]() {
            return BlocklistDataBase::store_->Update(changedRows, values, rdbPredicates);
        });
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("UpdateBlockList failed:%{public}d", ret);
            return RDB_EXECUTE_FAIL;
        }
        HILOG_INFO("UpdateBlockList row:%{public}d", changedRows);
        return changedRows;
    } else {
        int ret = CoverToUpdateBlockListValues(values, rdbPredicates, updateBlockListValues);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("UpdateBlockList CoverToUpdateBlockListValues failed:%{public}d", ret);
            return RDB_EXECUTE_FAIL;
        }
    }
    int updateRet = BatchUpdateBlockListOneByOne(updateBlockListValues);
    HILOG_INFO("UpdateBlockList row:%{public}d end", updateRet);
    return updateRet;
}

int ContactsDataBase::CoverToUpdateBlockListValues(
    OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates,
    std::vector<OHOS::NativeRdb::ValuesBucket> &updateBlockListValues)
{
    std::vector<std::string> columns;
    columns.push_back(ContactBlockListColumns::PHONE_NUMBER);
    columns.push_back(ContactBlockListColumns::ID);
    columns.push_back(ContactBlockListColumns::TYPES);
    auto blockListResultSet = BlocklistDataBase::store_->Query(rdbPredicates, columns);
    if (blockListResultSet == nullptr) {
        HILOG_ERROR("ContactsDataBase CoverToUpdateBlockListValues blockListResultSet is nullptr");
        return RDB_EXECUTE_FAIL;
    }
    int blockListResultSetNum = blockListResultSet->GoToFirstRow();
    while (blockListResultSetNum == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        std::string phoneNumber;
        blockListResultSet->GetColumnIndex(ContactBlockListColumns::PHONE_NUMBER, columnIndex);
        blockListResultSet->GetString(columnIndex, phoneNumber);
        if (phoneNumber.empty()) {
            HILOG_ERROR("ContactsDataBase CoverToUpdateBlockListValues fail, query no phone");
            blockListResultSet->Close();
            return RDB_EXECUTE_FAIL;
        }
        int columnTypesIndex = 0;
        int types = -1;
        blockListResultSet->GetColumnIndex(ContactBlockListColumns::TYPES, columnTypesIndex);
        blockListResultSet->GetInt(columnTypesIndex, types);
        if (types < 0) {
            HILOG_ERROR("CoverToUpdateBlockListValues failed: invalid type");
            blockListResultSet->Close();
            return RDB_EXECUTE_FAIL;
        }
        int columnIdIndex = 0;
        int64_t id = -1;
        blockListResultSet->GetColumnIndex(ContactBlockListColumns::ID, columnIdIndex);
        blockListResultSet->GetLong(columnIdIndex, id);
        if (id < 0) {
            HILOG_ERROR("CoverToUpdateBlockListValues failed: invalid id");
            blockListResultSet->Close();
            return RDB_EXECUTE_FAIL;
        }
        OHOS::NativeRdb::ValuesBucket newValue = OHOS::NativeRdb::ValuesBucket(values);
        newValue.PutString(ContactBlockListColumns::PHONE_NUMBER, phoneNumber);
        newValue.PutLong(ContactBlockListColumns::ID, id);
#ifdef ABILITY_CUST_SUPPORT
    if (types == FULL_MATCH_BLOCK_REASON) {
        int interceptionCallCount = QueryInterceptionCallCount(phoneNumber);
        newValue.PutInt(ContactBlockListColumns::INTERCEPTION_CALL_COUNT, interceptionCallCount);
    } else if (types == START_WITH_BLOCK_REASON) {
        int interceptionCallCount = QueryStartWithTypeInterceptionCallCount(phoneNumber);
        newValue.PutInt(ContactBlockListColumns::INTERCEPTION_CALL_COUNT, interceptionCallCount);
    }
#endif
        updateBlockListValues.push_back(newValue);
        blockListResultSetNum = blockListResultSet->GoToNextRow();
    }
    blockListResultSet->Close();
    return OHOS::NativeRdb::E_OK;
}

/**
 * @brief Update data
 *
 * @param values Parameters to be passed for update operation
 * @param rdbPredicates Conditions for update operation
 *
 * @return The result returned by the update operation
 */
int ContactsDataBase::Update(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateGroup store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Update(changedRows, values, rdbPredicates);
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateGroup failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("UpdateGroup row:%{public}d", changedRows);
    return changedRows;
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
    auto whereArgs = rdbPredicates.GetWhereArgs();
    std::string deleteArg = "";
    unsigned int PHONE_POST_NUM = 4;
    unsigned int PHONE_PRE_NUM = 3;
    for (auto it = whereArgs.begin(); it != whereArgs.end(); it++) {
        deleteArg.append(it->size() >= PHONE_PRE_NUM ? it->substr(0, PHONE_PRE_NUM).c_str()
                                             : it->c_str());
        deleteArg.append(":");
        deleteArg.append(it->size() >= PHONE_POST_NUM ? it->substr(it->size() - PHONE_POST_NUM).c_str()
                                             : it->c_str());
        deleteArg.append(":");
        deleteArg.append(std::to_string(it->size()));
        deleteArg.append(",");
        }
    HILOG_WARN("ContactsDataBase DeleteBlockList char:%{public}s", deleteArg.c_str());
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = HandleRdbStoreRetry([&]() {
        return BlocklistDataBase::store_->Delete(changedRows, rdbPredicates);
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteBlockList failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("DeleteBlockList row:%{public}d", changedRows);
    return ret;
}

/**
 * @brief Delete data from cloud_raw_contact table
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::DeleteCloudRawContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteCloudRawContact store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Delete(changedRows, rdbPredicates);
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteCloudRawContact failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("DeleteCloudRawContact row:%{public}d", changedRows);
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

int ContactsDataBase::DeletePoster(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeletePoster store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int deletedRows = OHOS::NativeRdb::E_OK;
    int ret = store_->Delete(deletedRows, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeletePoster  failed:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("DeletePoster row:%{public}d", deletedRows);
    return ret;
}

// 是否影响通话记录
bool ContactsDataBase::IsAffectCalllog(std::string type)
{
    return type == ContentTypeData::PHONE || type == ContentTypeData::NAME;
}

// 添加修改的联系人rawId，至mUpdateRawContacts 集合; 返回是否有修改名称和电话号码信息
bool addMUpdateRawContacts(const std::vector<int> &rawContactIdVector, const std::vector<std::string> &types)
{
    bool modifyPhoneOrNameFlag = false;
    for (unsigned int i = 0; i < rawContactIdVector.size(); i++) {
        int rawContactId = rawContactIdVector[i];
        std::string type = types[i];
        if (type == ContentTypeData::PHONE || type == ContentTypeData::NAME) {
            modifyPhoneOrNameFlag = true;
            mUpdateRawContacts.push_back(rawContactId);
        }
    }
    return modifyPhoneOrNameFlag;
}

/**
 * @brief Delete data from contact_data table
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::DeleteContactData(OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync)
{
    HILOG_INFO("ContactsDataBase DeleteContactData start, ts = %{public}lld", (long long) time(NULL));
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteContactData store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    // 删除的contactdata的type信息
    std::vector<std::string> types;

    // 删除的contacdata的calendar_event_id 信息
    std::string calendarEventIds;

    // 删除contactdata信息的联系人id
    std::vector<int> rawContactIdVector =
        QueryContactDataRawContactId(rdbPredicates, types, calendarEventIds, "DeleteContactData");
    // 记录删除contactdata信息的rawcontactid，此操作属于更新contact_data，通知使用
    ContactsDataBase::updateContactIdVector.productionMultiple(rawContactIdVector);

    // 查询相关联系人的日程信息calendar_event_id
    calendarEventIds = calendarEventIds.substr(0, calendarEventIds.length() - 1);
    contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
    contactsConnectAbility_->ConnectEventsHandleAbility(CONTACTS_BIRTHDAY_DELETE, calendarEventIds, "");

    int deletedRows = OHOS::NativeRdb::E_OK;
    // 删除操作
    int error = store_->Delete(deletedRows, rdbPredicates);
    if (error == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        error = store_->Restore("contacts.db.bak");
        HILOG_ERROR("DeleteContactData Delete Restore retCode= %{public}d", error);
    }
    HILOG_WARN("deleteContactData deletedRows :%{public}d", deletedRows);
    ContactsUpdateHelper contactsUpdateHelper;
    OHOS::NativeRdb::ValuesBucket contactDataValues;
    // 更新公司，职位，姓名等信息至rawcontact表
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
    if (!rawContactIdVector.empty()) {
        HILOG_INFO("ContactsDataBase rawContactIdVector[0] = %{public}d, size = %{public}ld, ts = %{public}lld",
            rawContactIdVector[0],
            (long) rawContactIdVector.size(),
            (long long) time(NULL));
        DeletedRelationAsyncTask(rawContactIdVector[0]);
    }
    if (isSync != "true") {
        mDirtyRawContacts.productionMultiple(rawContactIdVector);
    }
    // 畅连取消能力场景：
    // 存在联系人a，有畅连能力，拨打畅连电话，通话记录关联联系人a；
    // 此时联系人a取消畅连能力，从通话记录进入联系人a详情，会更新畅连能力，会先删除之前的设备信息
    // 此时后续更新通话记录任务会取消通话记录和contactid关联，且后续没有更新了，导致通话记录没关联到联系人
    // 新增判断，如果删除信息没有名称和手机号信息，不会影响通话记录，不进行通话记录更新
    // 是否影响到通话记录
    bool isAffectsCalllog = addMUpdateRawContacts(rawContactIdVector, types);
    // 删除contact_data更新操作，内部有更新通话记录信息
    MergeUpdateTask(store_, rawContactIdVector, true, isAffectsCalllog);
    return ret;
}


int ContactsDataBase::DeletePrivacyContactsBackup(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("DeletePrivacyContactsBackup store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int deletedRows = OHOS::NativeRdb::E_OK;
    // 删除操作
    int error = store_->Delete(deletedRows, rdbPredicates);
    if (error == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        error = store_->Restore("contacts.db.bak");
        HILOG_ERROR("DeletePrivacyContactsBackup Delete Restore retCode= %{public}d", error);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_WARN("DeletePrivacyContactsBackup deletedRows :%{public}d", deletedRows);
    return RDB_EXECUTE_OK;
}

/**
 * @brief Delete data from the contact table
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::DeleteContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteContact store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    // 查询删除联系人的信息,contact信息
    std::vector<std::string> contactIdArr;
    std::vector<OHOS::NativeRdb::ValuesBucket> queryValuesBucket = DeleteContactQuery(rdbPredicates, contactIdArr);

    if (contactIdArr.empty()) {
        HILOG_ERROR("ContactsDataBase DeleteContact contact id array is empty");
        return RDB_EXECUTE_FAIL;
    }
    if (queryValuesBucket.empty()) {
        return DeleteContactDirectly(contactIdArr);
    }

    // 开启事务
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    
    // 更新raw表信息，将raw标记为删除状态
    size_t deleteRawSize = 0;
    int deleteRet = DeleteExecute(queryValuesBucket, isSync, deleteRawSize);
    OHOS::NativeRdb::ValuesBucket contactValues;
    contactValues.PutNull(ContactColumns::FORM_ID);
    int changeRow = OHOS::NativeRdb::E_OK;
    auto contactPredicates = OHOS::NativeRdb::RdbPredicates(ContactTableName::CONTACT);
    contactPredicates.In(ContactColumns::ID, contactIdArr);
    int contactRet = store_->Update(changeRow, contactValues, contactPredicates);
    if (deleteRet == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        deleteRet = store_->Restore("contacts.db.bak");
        HILOG_ERROR("DeleteContact Delete Restore retCode= %{public}d", deleteRet);
    }
    if (deleteRet != OHOS::NativeRdb::E_OK || contactRet != OHOS::NativeRdb::E_OK) {
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    deleteRet = Commit();
    if (deleteRet != OHOS::NativeRdb::E_OK) {
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    // 删除 异步发送通知
    std::string paramStr = "delete;";
    std::string callingBundleName = getCallingBundleName();
    paramStr.append(callingBundleName);
    paramStr.append(";");
    paramStr.append(std::to_string(deleteRawSize));
    contactsConnectAbility_->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
    // 同步插入delete表，前端做了进度条需要改成同步方法
    // 内部有更新通话记录
    DeleteRecordInsert(store_, queryValuesBucket);
    HILOG_INFO("DeleteContact end.");
    NotifyContactChange();
    return deleteRet;
}

int ContactsDataBase::DeleteContactDirectly(std::vector<std::string> &contactIdArr)
{
    auto contactPredicates = OHOS::NativeRdb::RdbPredicates(ContactTableName::CONTACT);
    contactPredicates.In(ContactColumns::ID, contactIdArr);
    int deletedRows = OHOS::NativeRdb::E_OK;
    int delRet = HandleRdbStoreRetry([&]() { return store_->Delete(deletedRows, contactPredicates); });
    HILOG_ERROR("ContactsDataBase DeleteContact queryValuesBucket is empty, contactIdArr:%{public}s,delete contact "
                "delRet:%{public}d,deletedRows:%{public}d",
        GetRawIdsToString(contactIdArr).c_str(),
        delRet,
        deletedRows);
    return delRet;
}

/**
 * Query cloud_raw_contact contact count
 */
void ContactsDataBase::QueryCloudContactsCount()
{
    if (store_ != nullptr) {
        std::string sqlContacts = "select uuid from cloud_raw_contact where recycled != 1 or recycled is null";
        std::string sqlContactsDeleted = "select uuid from cloud_raw_contact where recycled = 1";
        std::string sqlLocalContacts = "select id from raw_contact where is_deleted != 1 or is_deleted is null";
        std::string sqlLocalContactsDeleted = "select id from raw_contact where is_deleted = 1";
        std::string contactIdNullSql = "select id from raw_contact where contact_id is null";
        std::string idNotEqualSql = "select id from raw_contact where contact_id != id and contact_id is not null";
        std::string syn1Sql = "select id from raw_contact where syn_1 != uuid and syn_1 is not null";

        auto resultSetContacts = store_->QuerySql(sqlContacts);
        if (resultSetContacts == nullptr) {
            HILOG_ERROR("resultSetContacts QuerySqlResult is null");
            return;
        }
        int contactsCount;
        resultSetContacts->GetRowCount(contactsCount);
        resultSetContacts->Close();

        auto resultSetContactsDeleted = store_->QuerySql(sqlContactsDeleted);
        if (resultSetContactsDeleted == nullptr) {
            HILOG_ERROR("resultSetContactsDeleted QuerySqlResult is null");
            return;
        }
        int contactsDeleteCount;
        resultSetContactsDeleted->GetRowCount(contactsDeleteCount);
        resultSetContactsDeleted->Close();

        auto resultSetLocalContacts = store_->QuerySql(sqlLocalContacts);
        if (resultSetLocalContacts == nullptr) {
            HILOG_ERROR("resultSetLocalContacts QuerySqlResult is null");
            return;
        }
        int contactsLocalCount;
        resultSetLocalContacts->GetRowCount(contactsLocalCount);
        resultSetLocalContacts->Close();

        auto resultSetLocalContactsDeleted = store_->QuerySql(sqlLocalContactsDeleted);
        if (resultSetLocalContactsDeleted == nullptr) {
            HILOG_ERROR("resultSetLocalContactsDeleted QuerySqlResult is null");
            return;
        }
        int contactsLocalDeleteCount;
        resultSetLocalContactsDeleted->GetRowCount(contactsLocalDeleteCount);
        resultSetLocalContactsDeleted->Close();

        auto contactIdNullSqlRet = store_->QuerySql(contactIdNullSql);
        if (contactIdNullSqlRet == nullptr) {
            HILOG_ERROR("contactIdNullSqlRet QuerySqlResult is null");
            return;
        }
        int contactIdNullCount;
        contactIdNullSqlRet->GetRowCount(contactIdNullCount);
        contactIdNullSqlRet->Close();

        auto idNotEqualRet = store_->QuerySql(idNotEqualSql);
        if (idNotEqualRet == nullptr) {
            HILOG_ERROR("idNotEqualRet QuerySqlResult is null");
            return;
        }
        int idNotEqualCount;
        idNotEqualRet->GetRowCount(idNotEqualCount);
        idNotEqualRet->Close();

        auto syn1Ret = store_->QuerySql(syn1Sql);
        if (syn1Ret == nullptr) {
            HILOG_ERROR("syn1Ret QuerySqlResult is null");
            return;
        }
        int syn1Count;
        syn1Ret->GetRowCount(syn1Count);
        syn1Ret->Close();

        HILOG_INFO("QueryCloudContactsCount cloudNormalContactsCount:%{public}d,cloudDeletedContactsCount:%{public}d,"
                   "localNormalContactsCount:%{public}d,localDeletedContactsCount:%{public}d,"
                   "contactIdNullCount:%{public}d,idNotEqualCount:%{public}d,syn1Count:%{public}d,ts=%{public}lld",
            contactsCount,
            contactsDeleteCount,
            contactsLocalCount,
            contactsLocalDeleteCount,
            contactIdNullCount,
            idNotEqualCount,
            syn1Count,
            (long long) time(NULL));
    } else {
        HILOG_ERROR("QueryCloudContactsCount skip, store is null");
    }
}

int ContactsDataBase::DeleteExecute(
    std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket, std::string isSync, size_t &deleteSize)
{
    unsigned int size = queryValuesBucket.size();
    if (size == 0) {
        return RDB_EXECUTE_FAIL;
    }
    int ret = RDB_EXECUTE_FAIL;
    std::vector<int> rawIdVector;
    std::vector<std::string> uuidVector;
    // 组装rawId数组，构造更新raw表的条件语句
    for (unsigned int i = 0; i < size; i++) {
        OHOS::NativeRdb::ValuesBucket valuesElement = queryValuesBucket[i];
        bool hasId = valuesElement.HasColumn(ContactColumns::ID);
        if (!hasId) {
            continue;
        }
        // 获取rawContact表id
        OHOS::NativeRdb::ValueObject idValue;
        valuesElement.GetObject(ContactPublicColumns::ID, idValue);
        int rawContactId = 0;
        idValue.GetInt(rawContactId);
        rawIdVector.push_back(rawContactId);
        // 删除contact，raw_contact，都会走这里，记录删除的rawcontactid
        ContactsDataBase::deleteContactIdVector.production(rawContactId);
    }

    // 更新raw表，包括is_deleted、dirty字段
    ret = DeleteExecuteUpdate(rawIdVector);
    deleteSize = rawIdVector.size();

    if (PrivacyContactsManager::IsPrivacySpace()) {
        HILOG_WARN("deleteContact rawIdVector size :%{public}zu, ", rawIdVector.size());
        PrivacyContactsManager::GetInstance()->DeleteContactsFromPrivacyBackup(rawIdVector);
    }
    // 删除联系人之后，清除日历中的生日信息
    std::string calendarEventIds;
    QueryCalendarIds(rawIdVector, calendarEventIds);
    if (calendarEventIds.length() > 0) {
        calendarEventIds = calendarEventIds.substr(0, calendarEventIds.length() - 1);
        contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
        contactsConnectAbility_->ConnectEventsHandleAbility(CONTACTS_BIRTHDAY_DELETE, calendarEventIds, "");
    }

    // 如果非云空间删除行为，则触发数据上云
    if (isSync != "true") {
        contactsConnectAbility_->ConnectAbility("", "", "", "", "", "");
    }
    return ret;
}

void ContactsDataBase::queryBigPhotoDetailInfo(std::vector<std::string> &bigPhotoDetailInfoVector,
    std::vector<int> &rawIdVector)
{
    std::string rawIds = GetRawIdsToString(rawIdVector);
    if (rawIds.length() <= 0) {
        return;
    }

    std::string querySql = "";
    querySql.append("select DISTINCT detail_info from contact_data where raw_contact_id in (")
        .append(rawIds)
        .append(")")
        .append(" and type_id = 8");

    auto resultSet = store_->QuerySql(querySql);
    if (resultSet == nullptr) {
        return;
    }
    int resultCount = 0;
    resultSet->GetRowCount(resultCount);

    if (resultCount <= 0) {
        resultSet->Close();
        return;
    }
    // 解析
    int getRowResult = resultSet->GoToFirstRow();
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        std::string columnName = "detail_info";
        std::string detailInfo;
        int columnIndex = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetString(columnIndex, detailInfo);
        if (!detailInfo.empty()) {
            bigPhotoDetailInfoVector.push_back(detailInfo);
        }
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
}

// 查询要删除的联系人的calendar_evnet_id
void ContactsDataBase::QueryCalendarIds(std::vector<int> &rawIdVector, std::string &calendarEventIds)
{
    std::string rawIds = GetRawIdsToString(rawIdVector);
    if (rawIds.length() <= 0) {
        return;
    }
    std::string querySql = "";
    querySql.append("select DISTINCT calendar_event_id from view_contact_data where raw_contact_id in (")
        .append(rawIds)
        .append(")");
    auto resultSet = store_->QuerySql(querySql);
    if (resultSet == nullptr) {
        return;
    }
    int resultCount = 0;
    resultSet->GetRowCount(resultCount);
    if (resultCount <= 0) {
        resultSet->Close();
        return;
    }
    int getRowResult = resultSet->GoToFirstRow();
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        std::string columnName = ContactDataColumns::CALENDAR_EVENT_ID;
        std::string calendarEventId;
        int columnIndex = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetString(columnIndex, calendarEventId);
        if (!calendarEventId.empty()) {
            calendarEventIds.append(calendarEventId).append(",");
        }
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
}

int ContactsDataBase::DeleteExecuteUpdate(std::vector<int> &rawIdVector)
{
    std::string rawIds = GetRawIdsToString(rawIdVector);
    OHOS::NativeRdb::ValuesBucket values;
    values.PutInt(RawContactColumns::IS_DELETED, DELETE_MARK);
    // 本地删除的联系人，不需要重新计算 uniqueKey
    values.PutInt(RawContactColumns::DIRTY, NO_NEED_RECALC);
    int updateRow = OHOS::NativeRdb::E_OK;
    std::string upWhereClause = "";
    upWhereClause.append(ContactPublicColumns::ID).append(" IN (").append(rawIds).append(")");
    std::vector<std::string> upWhereArgs;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->Update(updateRow, ContactTableName::RAW_CONTACT, values, upWhereClause, upWhereArgs);
    });
    HILOG_WARN("set deleted contact dirty,ret:%{public}d,updateRow=%{public}d,rawId=%{public}s",
        ret,
        updateRow,
        rawIds.c_str());
    return ret;
}

void ContactsDataBase::DeletedRelationAsyncTask(int rawContactId)
{
    std::unique_ptr<AsyncItem> task = std::make_unique<AsyncDeleteRelationContactsTask>(rawContactId);
    g_asyncTaskQueue->Push(task);
    g_asyncTaskQueue->Start();
}

std::map<std::string, int> ContactsDataBase::QueryDeletedRelationData(int rawContactId)
{
    std::string querySql = "SELECT detail_info FROM contact_data WHERE "
                           "type_id in (1, 5) and raw_contact_id = " +
                           std::to_string(rawContactId) + ";";
    auto resultSet = store_->QuerySql(querySql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryDeletedRelationData QuerySqlResult is null");
        return {};
    }
    int getRowResult = resultSet->GoToFirstRow();
    std::vector<std::string> detailInfos;
    std::map<std::string, int> newRawContactIds;
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        std::string currentRecord;
        int columnIndex = 0;
        resultSet->GetColumnIndex(ContactDataColumns::DETAIL_INFO, columnIndex);
        resultSet->GetString(columnIndex, currentRecord);
        detailInfos.push_back(currentRecord);
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
    for (std::string singleRecord : detailInfos) {
        querySql = "SELECT name_raw_contact_id, ha.account_id FROM contact c "
                   "JOIN hw_account ha on ha.phone_number = '" +
                   singleRecord + "' OR ha.email = '" +
                   singleRecord +
                   "' "
                   "JOIN raw_contact rc on c.name_raw_contact_id = rc.id AND rc.is_deleted = 0 AND rc.is_deleted != 1 "
                   "WHERE name_raw_contact_id in "
                   "(SELECT raw_contact_id FROM contact_data WHERE detail_info = '" +
                   singleRecord + "') AND name_raw_contact_id != " + std::to_string(rawContactId) +
                   " "
                   "ORDER by contact_last_updated_timestamp DESC LIMIT 1";
        auto resultSet = store_->QuerySql(querySql);
        if (resultSet == nullptr) {
            HILOG_ERROR("QueryDeletedRelationData QuerySqlResult is null");
            continue;
        }
        int getRowResult = resultSet->GoToFirstRow();
        while (getRowResult == OHOS::NativeRdb::E_OK) {
            std::string columnName = ContactColumns::NAME_RAW_CONTACT_ID;
            std::string accountId;
            int newRawContactId = 0;
            int columnIndex = 0;
            resultSet->GetColumnIndex(columnName, columnIndex);
            resultSet->GetInt(columnIndex, newRawContactId);
            columnName = HwAccountColumns::ACCOUNT_ID;
            resultSet->GetColumnIndex(columnName, columnIndex);
            resultSet->GetString(columnIndex, accountId);
            HILOG_INFO("HandleHwAccount newRawContactId = :%{public}d, ts = %{public}lld", newRawContactId, (long long) time(NULL));
            newRawContactIds.insert(std::pair<std::string, int>(accountId, newRawContactId));
            getRowResult = resultSet->GoToNextRow();
        }
        resultSet->Close();
    }
    return newRawContactIds;
}

void ContactsDataBase::HandleHwAccount(int rawContactId)
{
    std::map<std::string, int> processRawContactIds = QueryDeletedRelationData(rawContactId);
    auto iter = processRawContactIds.begin();
    if (processRawContactIds.size() == 0) {
        int rowId = 0;
        std::vector<std::string> whereArgs;
        whereArgs.push_back(std::to_string(rawContactId));
        std::string whereCase;
        whereCase.append(HwAccountColumns::CONTACT_ID).append(" = ?");
        int delAccount = store_->Delete(rowId, ContactTableName::HW_ACCOUNT, whereCase, whereArgs);
        HILOG_INFO("HandleHwAccount delAccount:%{public}d, ts = %{public}lld", delAccount, (long long) time(NULL));
        return;
    }
    while (iter != processRawContactIds.end()) {
        std::string whereClause = "";
        std::vector<std::string> whereArgs;
        whereClause.append(HwAccountColumns::ACCOUNT_ID);
        whereClause.append(" = ? ");
        whereArgs.push_back(iter->first);
        OHOS::NativeRdb::ValuesBucket valuesBucket;
        valuesBucket.PutInt(HwAccountColumns::CONTACT_ID, iter->second);
        int changedRows = OHOS::NativeRdb::E_OK;
        int ret = RDB_EXECUTE_FAIL;
        ret = store_->Update(changedRows, ContactTableName::HW_ACCOUNT, valuesBucket, whereClause, whereArgs);
        HILOG_INFO("HandleHwAccount contact_id %{public}d, accountid %{public}s, size = %{public}ld "
                   "ret:%{public}d, changedRows:%{public}d, ts = %{public}lld",
            iter->second,
            (iter->first).c_str(),
            (long) processRawContactIds.size(),
            ret,
            changedRows,
            (long long) time(NULL));
        iter++;
    }
}

int ContactsDataBase::DeleteExecuteRetUpdateCloud(std::string uuid)
{
    std::string whereClause = "";
    std::vector<std::string> whereArgs;
    whereClause.append(CloudRawContactColumns::UUID);
    whereClause.append(" = ? ");
    whereArgs.push_back(uuid);
    OHOS::NativeRdb::ValuesBucket valuesUpCloudRawContacts;
    valuesUpCloudRawContacts.PutInt(CloudRawContactColumns::RECYCLED, DELETE_MARK);
    int changedRows = OHOS::NativeRdb::E_OK;
    int retUpdateCloud = RDB_EXECUTE_FAIL;
    retUpdateCloud = store_->Update(
        changedRows, ContactTableName::CLOUD_RAW_CONTACT, valuesUpCloudRawContacts, whereClause, whereArgs);
    return retUpdateCloud;
}

void ContactsDataBase::DeleteRecordInsert(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket)
{
    g_mtx.lock();
    unsigned int size = queryValuesBucket.size();
    std::string callingBundleName = DataShare::ContactsDataShareStubImpl::bundleName_;
    std::chrono::milliseconds deleteMills =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::vector<int> rawContactIdVector;
    std::vector<std::string> contactIdVector;
    std::vector<OHOS::NativeRdb::ValuesBucket> batchInsertValues;
    HILOG_INFO("DeleteRecordInsert size is %{public}d,callingBundleName :%{public}s", size, callingBundleName.c_str());
    for (unsigned int i = 0; i < size; i++) {
        OHOS::NativeRdb::ValuesBucket valuesElement = queryValuesBucket[i];
        bool hasId = valuesElement.HasColumn(ContactColumns::ID);
        if (!hasId) {
            continue;
        }
        OHOS::NativeRdb::ValueObject nameValue;
        valuesElement.GetObject(RawContactColumns::DISPLAY_NAME, nameValue);
        std::string disPlayName;
        nameValue.GetString(disPlayName);
        OHOS::NativeRdb::ValueObject idValue;
        valuesElement.GetObject(ContactPublicColumns::ID, idValue);
        int rawContactId = 0;
        idValue.GetInt(rawContactId);
        OHOS::NativeRdb::ValueObject contactIdValue;
        valuesElement.GetObject(RawContactColumns::CONTACT_ID, contactIdValue);
        int contactId = 0;
        contactIdValue.GetInt(contactId);

        DeleteRawContactLocal(
            contactId, rawContactId, "", disPlayName, deleteMills.count(), callingBundleName, batchInsertValues);
        rawContactIdVector.push_back(rawContactId);
        contactIdVector.push_back(std::to_string(contactId));
    }
    int64_t outDataRowId;
    int ret = HandleRdbStoreRetry([&]() {
        return store_->BatchInsert(outDataRowId, ContactTableName::DELETE_RAW_CONTACT, batchInsertValues);
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("deleteRawContact BatchInsert failed:%{public}d", ret);
    }
    
    SplitRawContactAfterDel(rawContactIdVector);
    ContactsUpdateHelper contactsUpdateHelper;
    contactsUpdateHelper.UpdateCallLogWhenBatchDelContact(contactIdVector, store);
    for (unsigned int i = 0; i < size; i++) {
        HandleHwAccount(rawContactIdVector[i]);
    }
    g_mtx.unlock();
    HILOG_INFO("DeleteRecordInsert end.");
}

void ContactsDataBase::SplitRawContactAfterDel(std::vector<int> rawContactIdVector)
{
    if (!rawContactIdVector.empty()) {
        std::string rawIdStr = GetRawIdsToString(rawContactIdVector);
        contactsConnectAbility_->ConnectAbility("", "", "", "", "splitRawContactAfterDel", rawIdStr);
    }
}

// 查询要删除的联系人的raw信息
std::vector<OHOS::NativeRdb::ValuesBucket> ContactsDataBase::DeleteContactQuery(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &contactIdArr)
{
    PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates newRdbPredicates =
        predicatesConvert.CopyPredicates(ViewName::VIEW_CONTACT, rdbPredicates);
    std::vector<std::string> columns;
    // 查询contact表的id和displayname
    columns.push_back(ContactPublicColumns::ID);
    columns.push_back(RawContactColumns::DISPLAY_NAME);
    auto resultSet = store_->Query(newRdbPredicates, columns);
    int resultSetNum = resultSet->GoToFirstRow();
    if (resultSetNum != OHOS::NativeRdb::E_OK) {
        // query size 0
        std::vector<OHOS::NativeRdb::ValuesBucket> vectorQueryData;
        resultSet->Close();
        return vectorQueryData;
    }
    // contact表id作为查询条件
    std::vector<std::string> whereArgs;
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int contactId;
        resultSet->GetInt(0, contactId);
        contactIdArr.push_back(std::to_string(contactId));
        whereArgs.push_back(std::to_string(contactId));
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    unsigned long size = whereArgs.size();
    HILOG_INFO("DeleteContactQuery delete contact data size:%{public}lu", (unsigned long) size);
    OHOS::NativeRdb::RdbPredicates rawContactQueryRdbPredicates(ViewName::VIEW_RAW_CONTACT);
    std::string whereClause;
    for (unsigned long i = 0; i < size; i++) {
        if (i == 0) {
            whereClause.append(" contact_id IN ( ? ");
        } else {
            whereClause.append(", ? ");
        }
        if (i == size - 1) {
            whereClause.append(" ) ");
        }
    }
    // contact表id作为查询条件
    OHOS::NativeRdb::PredicatesUtils::SetWhereClauseAndArgs(&rawContactQueryRdbPredicates, whereClause, whereArgs);
    OHOS::NativeRdb::PredicatesUtils::SetAttributes(&rawContactQueryRdbPredicates,
        rawContactQueryRdbPredicates.IsDistinct(),
        rawContactQueryRdbPredicates.GetIndex(),
        rawContactQueryRdbPredicates.GetGroup(),
        rawContactQueryRdbPredicates.GetOrder(),
        rawContactQueryRdbPredicates.GetLimit(),
        rawContactQueryRdbPredicates.GetOffset());
    // 查询rawcontact视图
    return DeleteRawContactQuery(rawContactQueryRdbPredicates);
}

std::vector<OHOS::NativeRdb::ValuesBucket> ContactsDataBase::DeleteRawContactQuery(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    std::vector<std::string> columns;
    columns.push_back(RawContactColumns::DISPLAY_NAME);
    columns.push_back(ContactColumns::ID);
    columns.push_back(RawContactColumns::CONTACT_ID);
    columns.push_back(RawContactColumns::UUID);
    columns.push_back(RawContactColumns::IS_DELETED);
    columns.push_back(RawContactColumns::AGGREGATION_STATUS);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet = store_->Query(rdbPredicates, columns);
    return ResultSetToValuesBucket(resultSet);
}

void ContactsDataBase::DeleteRawContactLocal(int contactId, int rawContactId, std::string backupData,
    std::string disPlayName, size_t deleteTime, std::string callingBundleName,
    std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues)
{
    OHOS::NativeRdb::ValuesBucket deleteRawContact;
    deleteRawContact.PutInt(DeleteRawContactColumns::RAW_CONTACT_ID, rawContactId);
    deleteRawContact.PutString(DeleteRawContactColumns::BACKUP_DATA, backupData);
    deleteRawContact.PutString(DeleteRawContactColumns::DISPLAY_NAME, disPlayName);
    deleteRawContact.PutInt(DeleteRawContactColumns::CONTACT_ID, contactId);
    deleteRawContact.PutInt(DeleteRawContactColumns::IS_DELETED, 0);
    deleteRawContact.PutDouble(DeleteRawContactColumns::DELETE_TIME, deleteTime);
    deleteRawContact.PutString(DeleteRawContactColumns::DELETE_SOURCE, callingBundleName);
    batchInsertValues.push_back(deleteRawContact);
}

/**
 * @brief Completely delete data from the database
 *
 * @param rdbPredicates Conditions for delete operation
 *
 * @return The result returned by the delete operation
 */
int ContactsDataBase::CompletelyDelete(OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string handleType)
{
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    std::vector<std::string> columns;
    columns.push_back(DeleteRawContactColumns::RAW_CONTACT_ID);
    auto resultSet = store_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        return RDB_EXECUTE_FAIL;
    }
    std::vector<int> rawContactIds;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int RawContactId;
        resultSet->GetInt(0, RawContactId);
        rawContactIds.push_back(RawContactId);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    // 先把回收站需要删除的任务改成1，让页面不显示
    ret = UpdateDeleted(rawContactIds);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("CompletelyDelete UpdateDeleted error:%{public}d", ret);
        RollBack();
        return ret;
    }
    // 删除所有联系人，同步执行，前端做了删除进度条需要同步进行
    DeleteAll(store_, rawContactIds);
    return CompletelyDeleteCommit(ret);
}

// 硬删除联系人，以raw表为条件硬删除
int ContactsDataBase::HardDelete(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteLocalBatch store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }

    std::vector<std::string> columns;
    columns.push_back(RawContactColumns::ID);
    columns.push_back(RawContactColumns::CONTACT_ID);
    auto resultSet = store_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteLocalBatch resultSet is nullptr");
        return RDB_EXECUTE_FAIL;
    }
    // rawId的集合
    std::vector<std::string> rawContactIds;
    std::vector<int> rawContactIdsIntType;
    // 删除的contactId集合，用于返回删除的数量
    std::set<std::string> contactIdSet;
    int resultSetNum = resultSet->GoToFirstRow();
    // 通知删除的联系人rawId
    std::vector<int> notifyDeleteRawIdVector;
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int RawContactId;
        std::string ContactId;
        resultSet->GetInt(0, RawContactId);
        resultSet->GetString(1, ContactId);
        rawContactIds.push_back(std::to_string(RawContactId));
        rawContactIdsIntType.push_back(RawContactId);
        contactIdSet.insert(ContactId);
        notifyDeleteRawIdVector.push_back(RawContactId);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();

    // 需删除的contacdata的calendar_event_id 信息
    std::string calendarEventIds;
    QueryCalendarIds(rawContactIdsIntType, calendarEventIds);
    std::vector<std::string> ringtoneUris;
    QueryPersonalRingtoneList(rawContactIdsIntType, ringtoneUris);
    // 执行删除
    ret = DeleteLocalBatch(rawContactIds);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("HardDelete commit error:%{public}d", ret);
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    if (PrivacyContactsManager::IsPrivacySpace()) {
        PrivacyContactsManager::GetInstance()->DeleteContactsFromPrivacyBackup(rawContactIdsIntType);
    }

    // 更新是否需要同步主空间数据标记
    if (isMainSpace()) {
        // 主空间删除，需要设置同步标记
        if (calendarEventIds.length() > 0) {
            ret = updateMainNeedSyncStatus(MAIN_SPACE_SYNC_FLAG_HAS_BIRTH);
        } else {
            ret = updateMainNeedSyncStatus(MAIN_SPACE_SYNC_FLAG_NOT_HAS_BIRTH);
        }
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("HardDelete updateMainNeedSyncStatus  error:%{public}d", ret);
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    ret = Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("HardDelete commit error:%{public}d", ret);
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    // 有硬删除通知刷新卡片信息
    int notifyFormIdChangeResCode = contactsConnectAbility_->ConnectAbility(
        "", "", "", "", "notifyFormIdChange", "hardDelOperate");
    if (notifyFormIdChangeResCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("HardDelete notifyFormIdChange fail:%{public}d", notifyFormIdChangeResCode);
    }
    // 删除成功了，通知变更的id
    ContactsDataBase::deleteContactIdVector.productionMultiple(notifyDeleteRawIdVector);
    // 更新通话记录
    std::vector<std::string> deleteContactIdVector;
    deleteContactIdVector.assign(contactIdSet.begin(), contactIdSet.end());
    ContactsUpdateHelper contactsUpdateHelper;
    contactsUpdateHelper.UpdateCallLogWhenBatchDelContact(deleteContactIdVector, store_);
    // 更新硬删除时间戳
    UpdateHardDeleteTimeStamp();

    // 看板上报，回收站彻底删除上报
    boardReportHardDelete(rawContactIds.size(), "");

    // 删除日历信息
    if (calendarEventIds.length() > 0) {
        calendarEventIds = calendarEventIds.substr(0, calendarEventIds.length() - 1);
        contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
        contactsConnectAbility_->ConnectEventsHandleAbility(CONTACTS_BIRTHDAY_DELETE, calendarEventIds, "");
    }
    HILOG_INFO("HardDelete, ringtoneUris.size is: %{public}ld", (long) ringtoneUris.size());
    if (ringtoneUris.size() > 0) {
        for (unsigned long i = 0; i < ringtoneUris.size(); i++) {
            DeletePersonalRingtone(ringtoneUris[i]);
        }
    }
    // 触发次云同步
    SyncContacts();
    HILOG_INFO("HardDelete end succeed, size: %{public}ld",
        (long) contactIdSet.size());
    return contactIdSet.size();
}

void ContactsDataBase::ReportOperationAuditLog(const std::vector<std::string> &rawContactIds)
{
    std::string callingName = GetCallingBundleName();
    HILOG_INFO("HardDelete callingBundleName: %{public}s", callingName.c_str());
    if ("com.ohos.privatespacemanager" == callingName) {
        size_t contactCount = static_cast<size_t>(GetContactCount());
        std::vector<NativeRdb::ValuesBucket> contactInfos;
        QueryContactInfosByRawIds(rawContactIds, contactInfos);
        HILOG_INFO("QueryContactInfosByRawIds contactInfos: %{public}zu", contactInfos.size());
        HiAudit::GetInstance().WriteLog(
            "MigrateContactToPrivacy", OHOS::OperationType::OPERATION_DELETE, contactInfos, contactCount);
    }
}

std::string ContactsDataBase::GetCallingBundleName()
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string bundleName = "";
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    BundleMgrClient bundleMgrClient;
    ErrCode res = bundleMgrClient.GetNameForUid(uid, bundleName);
    IPCSkeleton::SetCallingIdentity(identity);
    if (res != ERR_OK) {
        HILOG_ERROR("Failed to get bundle name, ErrCode=%{public}d", static_cast<int32_t>(res));
    }
    if (bundleName.empty()) {
        bundleName.append(std::to_string(uid));
        bundleName.append(std::to_string(IPCSkeleton::GetCallingPid()));
    }
    return bundleName;
}

void ContactsDataBase::QueryContactInfosByRawIds(
    const std::vector<std::string> &rawContactIds, std::vector<NativeRdb::ValuesBucket> &contactInfos)
{
    std::string rawContactStr = GetRawIdsToString(rawContactIds);
    std::string sql = "SELECT "
                        "contact_id, "
                        "GROUP_CONCAT(CASE WHEN type_id = 5 THEN detail_info END) AS phoneNumbers, "
                        "CASE WHEN type_id = 6 THEN detail_info END AS display_name, "
                        "account_id "
                      "FROM view_contact_data WHERE type_id IN(5, 6) AND raw_contact_id IN(" +
                      rawContactStr + ") GROUP BY raw_contact_id";
    auto resultSet = store_->QuerySql(sql);
    if (resultSet != nullptr) {
        int getRowResult = resultSet->GoToFirstRow();
        int columnCount = 0;
        resultSet->GetColumnCount(columnCount);
        while (getRowResult == OHOS::NativeRdb::E_OK) {
            NativeRdb::ValuesBucket contactInfo;
            NativeRdb::ValueObject value;
            for (int i = 0; i < columnCount; i++) {
                resultSet->Get(i, value);
                std::string columnName;
                resultSet->GetColumnName(i, columnName);
                contactInfo.Put(columnName, value);
            }
            contactInfos.emplace_back(contactInfo);
            getRowResult = resultSet->GoToNextRow();
        }
        resultSet->Close();
    }
}

void ContactsDataBase::QueryPersonalRingtoneList(std::vector<int> &rawIdVector, std::vector<std::string> &ringtoneUris)
{
    std::string rawIds = GetRawIdsToString(rawIdVector);
    if (rawIds.length() <= 0) {
        return;
    }
    std::string querySql = "";
    querySql.append("select personal_ringtone from contact where name_raw_contact_id in (").append(rawIds).append(")");
    auto resultSet = store_->QuerySql(querySql);
    if (resultSet == nullptr) {
        return;
    }
    int resultCount = 0;
    resultSet->GetRowCount(resultCount);
    if (resultCount <= 0) {
        resultSet->Close();
        return;
    }
    int getRowResult = resultSet->GoToFirstRow();
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        std::string columnName = ContactColumns::PERSONAL_RINGTONE;
        std::string ringtoneUri;
        int columnIndex = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetString(columnIndex, ringtoneUri);
        if (!ringtoneUri.empty()) {
            ringtoneUris.push_back(ringtoneUri);
        }
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
}

int ContactsDataBase::DataSyncAfterPrivateAndMainSpaceDataMigration()
{
    if (store_ == nullptr) {
        HILOG_ERROR("DataSyncAfterPrivateAndMainSpaceDataMigration store is null");
        return OHOS::NativeRdb::E_ERROR;
    }
    int ret = OHOS::NativeRdb::E_OK;
    int flag = getMainSpaceNeedSyncFlag();
    unsigned int unsignedFlag = static_cast<unsigned int>(flag);
    HILOG_WARN("DataSyncAfterPrivateAndMainSpaceDataMigration, needsyncflag: %{public}d", flag);
    // 默认同步标记0，不需要同步
    if (flag == MAIN_SPACE_SYNC_DEFAULT) {
        HILOG_WARN("DataSyncAfterPrivateAndMainSpaceDataMigration, not needsync, return");
        return ret;
    }
    // 触发云同步
    if ((unsignedFlag >> MAIN_SPACE_NEED_SYNC_BIT) & 1) {
        ret = contactsConnectAbility_->ConnectAbility("", "", "", "", "", "");
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("Connect contact Serviceability cloud sync fail, needsyncflag: %{public}d", flag);
            return ret;
        }
        // 有主空间隐私空间迁移联系人操作，刷新卡片
        int retNotifyFormIdChange = contactsConnectAbility_->ConnectAbility("", "", "", "",
            "notifyFormIdChange", "dataSyncAfterPrivateAndMainSpaceDataMigrationOperate");
        if (retNotifyFormIdChange != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("Connect contact notifyFormIdChange fail!");
        }
        HILOG_WARN("Connect contact Serviceability cloud sync success");
    }
    // 触发重新同步所有生日重新同步
    if ((unsignedFlag >> MAIN_SPACE_NEED_SYNC_BIRTH_BIT) & 1) {
        ret = contactsConnectAbility_->ConnectAbility("", "", "", "", "resetAllCalendar", "");
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("Connect contact Serviceability resetAllCalendar: %{public}d", flag);
            return ret;
        }
        HILOG_WARN("Connect contact Serviceability resetAllCalendar success");
    }

    // 触发重新关联大头像
    if ((unsignedFlag >> MAIN_SPACE_NEED_SYNC_BIG_PHOTO_BIT) & 1) {
        ret = contactsConnectAbility_->ConnectAbility("", "", "", "", "reassociateBigPhoto", "");
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("Connect contact Serviceability reassociateBigPhoto: %{public}d", flag);
            return ret;
        }
        HILOG_WARN("Connect contact Serviceability reassociateBigPhoto success");
    }

    std::string updateSql = "update settings set main_space_need_sync_flag = 0 where id = 1;";
    ret = store_->ExecuteSql(updateSql);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DataSyncAfterPrivateAndMainSpaceDataMigration update flag fail code:%{public}d", ret);
    }
    HILOG_WARN("Connect contact Serviceability update flag to 0 success");
    return ret;
}

bool ContactsDataBase::isMainSpace()
{
    OHOS::AccountSA::OsAccountType type;
    OHOS::AccountSA::OsAccountManager::GetOsAccountTypeFromProcess(type);
    // ADMIN 为主空间
    if (type == OHOS::AccountSA::OsAccountType::ADMIN) {
        HILOG_WARN("IsSpecialSpace, type: %{public}d, isMainSpace true", type);
        return true;
    } else {
        HILOG_WARN("IsSpecialSpace, type: %{public}d, isMainSpace false", type);
        return false;
    }
}

int ContactsDataBase::getMainSpaceNeedSyncFlag()
{
    int flagBefore = MAIN_SPACE_SYNC_DEFAULT;
    if (store_ == nullptr) {
        HILOG_ERROR("getMainSpaceNeedSyncFlag store is null");
        return flagBefore;
    }
    // 主空间操作，需要设置同步标记
    std::string selectNeedSyncFlagSql = "select main_space_need_sync_flag from settings where id = 1;";
    auto resultSet = store_->QuerySql(selectNeedSyncFlagSql);
    if (resultSet == nullptr) {
        HILOG_ERROR("getMainSpaceNeedSyncFlag QuerySqlResult is null");
        return flagBefore;
    }
    int getRowResult = resultSet->GoToFirstRow();
    if (getRowResult == OHOS::NativeRdb::E_OK) {
        resultSet->GetInt(0, flagBefore);
    }
    resultSet->Close();
    return flagBefore;
}

int ContactsDataBase::updateMainNeedSyncStatus(int needSyncFlag)
{
    // 主空间操作，需要设置同步标记
    int flagBefore = getMainSpaceNeedSyncFlag();

    int ret = OHOS::NativeRdb::E_OK;
    int newFlag = static_cast<int>(
        static_cast<unsigned int>(needSyncFlag) | static_cast<unsigned int>(flagBefore));
    HILOG_WARN("updateMainNeedSyncStatus :%{public}d, %{public}d", newFlag, flagBefore);
    if (newFlag != flagBefore) {
        std::string updateSql = "update settings set main_space_need_sync_flag = ";
        updateSql.append(std::to_string(newFlag)).
        append(" where id = 1;");
        ret = store_->ExecuteSql(updateSql);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("updateMainNeedSyncStatus fail code:%{public}d", ret);
            return ret;
        }
    } else {
        HILOG_WARN("updateMainNeedSyncStatus falg not change:%{public}d", newFlag);
    }
    return ret;
}

int ContactsDataBase::DeleteMergeRawContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    std::vector<std::string> columns;
    columns.push_back(ContactColumns::ID);
    columns.push_back(ContactColumns::NAME_RAW_CONTACT_ID);
    auto resultSet = store_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("DeleteMergeRawContact resultSet is null");
        return RDB_EXECUTE_FAIL;
    }
    std::vector<int> contactIds;
    std::vector<int> rawContactIds;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int contactId;
        resultSet->GetInt(0, contactId);
        contactIds.push_back(contactId);

        int RawContactId;
        resultSet->GetInt(1, RawContactId);
        rawContactIds.push_back(RawContactId);
        resultSetNum = resultSet->GoToNextRow();
    }
    if (rawContactIds.size() == 0) {
        HILOG_ERROR("DeleteMergeRawContact get no raw id to deletem return");
        resultSet->Close();
        return OHOS::NativeRdb::E_OK;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        resultSet->Close();
        return RDB_EXECUTE_FAIL;
    }
    ret = DeleteAllMerged(store_, contactIds, rawContactIds);
    resultSet->Close();

    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteMergeRawContact end error:%{public}d", ret);
        ret = RollBack();
        HILOG_ERROR("DeleteMergeRawContact RollBack ret:%{public}d", ret);
        return ret;
    }
    ret = Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteMergeRawContact commit error:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("DeleteMergeRawContact end succeed");
    return ret;
}

void ContactsDataBase::boardReportHardDelete(int delCount, std::string handleType)
{
    // 看板上报，回收站彻底删除上报
    std::string paramStr = "hardDelete;";
    std::string callingBundleName = getCallingBundleName();
    paramStr.append(callingBundleName);
    paramStr.append(";");
    paramStr.append(std::to_string(delCount));
    paramStr.append(";");
    paramStr.append(handleType);
    contactsConnectAbility_->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryDeletedContactsInfo(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::string rawIds)
{
    std::string querySql = "SELECT ";
    querySql.append(ContactTableName::DELETE_RAW_CONTACT)
        .append(".")
        .append(DeleteRawContactColumns::CONTACT_ID)
        .append(", ")
        .append(ContactTableName::DELETE_RAW_CONTACT)
        .append(".")
        .append(DeleteRawContactColumns::RAW_CONTACT_ID)
        .append(", ")
        .append(ContactTableName::RAW_CONTACT)
        .append(".")
        .append(RawContactColumns::IS_DELETED)
        .append(" FROM ")
        .append(ContactTableName::DELETE_RAW_CONTACT)
        .append(" JOIN ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" ON ")
        .append(ContactTableName::DELETE_RAW_CONTACT)
        .append(".")
        .append(DeleteRawContactColumns::RAW_CONTACT_ID)
        .append(" = ")
        .append(ContactTableName::RAW_CONTACT)
        .append(".")
        .append(RawContactColumns::ID)
        .append(" WHERE ")
        .append(ContactTableName::DELETE_RAW_CONTACT)
        .append(".")
        .append(DeleteRawContactColumns::RAW_CONTACT_ID)
        .append(" IN ")
        .append("(")
        .append(rawIds)
        .append(")");
    auto resultSet = store->QuerySql(querySql);
    return resultSet;
}

// 硬删除
int ContactsDataBase::DeleteAll(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> rawContactIds, std::string handleType)
{
    int retCode = RDB_EXECUTE_FAIL;
    int delCount = 0;
    if (rawContactIds.size() > 0) {
        std::vector<int> rawIdVector;
        std::copy(rawContactIds.begin(), rawContactIds.end(), std::back_inserter(rawIdVector));
        // id 转为字符串数组
        std::string dirtyRawId = GetRawIdsToString(rawIdVector);
        HILOG_WARN("DeleteAll dirtyRawId :%{public}s", dirtyRawId.c_str());
        // 查询删除的联系人信息
        auto idSet = QueryDeletedContactsInfo(store, dirtyRawId);
        int resultSetNum = idSet->GoToFirstRow();
        int cloumnIndexIsDelete = 2;
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            int contactId = -1;
            idSet->GetInt(0, contactId);
            int rawContactId = -1;
            idSet->GetInt(1, rawContactId);
            int isDeletedValue = 0;
            idSet->GetInt(cloumnIndexIsDelete, isDeletedValue);
            retCode = DeleteLocal(rawContactId, std::to_string(contactId), isDeletedValue);
            if (retCode != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("DeleteAll error:%{public}d", retCode);
                break;
            }
            resultSetNum = idSet->GoToNextRow();
            delCount++;
        }
        idSet->Close();
        if (retCode == OHOS::NativeRdb::E_OK) {
            SyncContacts();
        } else {
            HILOG_ERROR("DeleteAll error:%{public}d", retCode);
            RollBack();
            return retCode;
        }

        // 看板上报，回收站彻底删除上报
        boardReportHardDelete(delCount, handleType);
        // 设置硬删除时间
        UpdateHardDeleteTimeStamp();
    }
    return retCode;
}

int ContactsDataBase::DeleteAllMerged(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> &contactIds, std::vector<int> &rawContactIds)
{
    int retCode = RDB_EXECUTE_FAIL;

    // 我 std::vector<int> rawIdVector;
    // 我 std::string dirtyRawId = GetRawIdsToString(rawIdVector);
    // 我 HILOG_INFO("DeleteAll dirtyRawId :%{public}s", dirtyRawId.c_str());
    for (unsigned int i = 0; i < rawContactIds.size(); i++) {
        retCode = DeleteLocalMerged(std::to_string(contactIds[i]), std::to_string(rawContactIds[i]));
        if (retCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DeleteAllMerged error:%{public}d", retCode);
            break;
        }
    }
    HILOG_INFO("DeleteAllMerged end:%{public}d", retCode);
    return retCode;
}

int ContactsDataBase::UpdateDeleted(std::vector<int> &rawContactIds)
{
    if (rawContactIds.size() == 0) {
        HILOG_INFO("UpdateDeleted rawContactIds is null");
        return OHOS::NativeRdb::E_OK;
    }
    int updateIsDeletedRet = RDB_EXECUTE_FAIL;
    if (rawContactIds.size() > 0) {
        std::vector<int> rawIdVector;
        std::copy(rawContactIds.begin(), rawContactIds.end(), std::back_inserter(rawIdVector));
        std::string dirtyRawId = GetRawIdsToString(rawIdVector);
        OHOS::NativeRdb::ValuesBucket upIsDeleteValues;
        std::string isDeletedKey = DeleteRawContactColumns::IS_DELETED;
        upIsDeleteValues.PutInt(isDeletedKey, 1);
        std::string upWhereClause;
        upWhereClause.append(DeleteRawContactColumns::RAW_CONTACT_ID).append(" IN (").append(dirtyRawId).append(")");
        std::vector<std::string> upWhereArgs;
        int changedRows = OHOS::NativeRdb::E_OK;
        // 先把回收站需要彻底删除的联系人 is_deleted 改为1，让回收站页面先不展示
        updateIsDeletedRet = HandleRdbStoreRetry([&]() {
            return store_->Update(
                changedRows, ContactTableName::DELETE_RAW_CONTACT, upIsDeleteValues, upWhereClause, upWhereArgs);
        });
        HILOG_WARN("updateDeleted set is_deleted, rawId = %{public}s, ret:%{public}d, changeRow = %{public}d",
            dirtyRawId.c_str(),
            updateIsDeletedRet,
            changedRows);
    }
    return updateIsDeletedRet;
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

std::string ContactsDataBase::QueryPersonalRingtone(std::string contactId)
{
    HILOG_WARN("QueryPersonalRingtone start.");

    // 查询contact表中personal_ringtone和contactId的数据
    std::string queryPersonalRingtone = "SELECT c1.personal_ringtone FROM contact AS c1 JOIN contact AS c2 ON "
                                        "c1.personal_ringtone = c2.personal_ringtone WHERE c2.id = " +
                                        contactId + " AND c1.personal_ringtone IS NOT NULL";
    auto resultSet = store_->QuerySql(queryPersonalRingtone);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryPersonalRingtone QuerySqlResult is null");
        return "";
    }
    int resultSetCount;
    std::string ringtoneUri;
    resultSet->GetRowCount(resultSetCount);
    HILOG_WARN("QueryPersonalRingtone resultSetCount is :%{public}d", resultSetCount);
    if (resultSetCount == 1) { // 等于1则说明没有其他联系人使用该铃声，从铃音库删除该铃声
        int resultSetNum = resultSet->GoToFirstRow();
        HILOG_WARN("QueryPersonalRingtone resultSetNum is :%{public}d", resultSetNum);
        int columnIndex = -1;
        std::string columnName = ContactColumns::PERSONAL_RINGTONE;
        if (resultSetNum == OHOS::NativeRdb::E_OK) {
            resultSet->GetColumnIndex(columnName, columnIndex);
            resultSet->GetString(columnIndex, ringtoneUri);
        }
    }
    resultSet->Close();
    return ringtoneUri;
}
 
int ContactsDataBase::DeletePersonalRingtone(std::string ringtoneUri)
{
    HILOG_WARN("DeletePersonalRingtone start.");
    if (!ringtoneUri.empty()) {
        if (SystemSoundManager_ == nullptr) {
            SystemSoundManager_ = Media::SystemSoundManagerFactory::CreateSystemSoundManager();
        }
        const std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<ContextImpl>();
        if (context == nullptr || SystemSoundManager_ == nullptr) {
            HILOG_WARN("DeletePersonalRingtone context or SystemSoundManager_ is nullptr...");
            return 0;
        }
        int32_t res = SystemSoundManager_->RemoveCustomizedTone(context, ringtoneUri);
        HILOG_WARN("DeletePersonalRingtone end succeed:%{public}d", res);
    }
    return 0;
}

int ContactsDataBase::DeleteLocalBatch(std::vector<std::string> &rawContactVector)
{
    // 删除本地表、云表、回收站表的逻辑
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteLocalBatch store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int retCode = OHOS::NativeRdb::E_OK;
    if (rawContactVector.size() == 0) {
        HILOG_WARN("DeleteLocalBatch rawContactId empty!");
        return retCode;
    }
    std::string dirtyRawIdStr = GetRawIdsToString(rawContactVector);
    HILOG_WARN("ContactsDataBase DeleteLocalBatch deleteId: %{public}s", dirtyRawIdStr.c_str());
    // 更新raw表外键
    std::string updateRawContactSql = "UPDATE raw_contact SET contact_id = NULL WHERE id in (" + dirtyRawIdStr + ")";
    retCode = store_->ExecuteSql(updateRawContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalBatch updateRawContactSql code:%{public}d", retCode);
        return retCode;
    }
    // 更新search表外键
    std::string deleteSearchContactSql = "DELETE FROM search_contact WHERE raw_contact_id in (" + dirtyRawIdStr + ")";
    retCode = store_->ExecuteSql(deleteSearchContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalBatch deleteSearchContactSql code:%{public}d", retCode);
        return retCode;
    }
    // 删除data表
    std::string deleteContactData = "DELETE FROM contact_data WHERE raw_contact_id in (" + dirtyRawIdStr + ")";
    retCode = store_->ExecuteSql(deleteContactData);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalBatch deleteContactData code:%{public}d", retCode);
        return retCode;
    }

    // 删除contact表信息
    std::string deleteContactSql = "DELETE FROM contact WHERE name_raw_contact_id in (" + dirtyRawIdStr + ")";
    retCode = store_->ExecuteSql(deleteContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalBatch deleteContactSql code:%{public}d", retCode);
        return retCode;
    }

    retCode = DeleteCloudBatch(rawContactVector);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalBatch DeleteCloud code:%{public}d", retCode);
        return retCode;
    }
    
    // 删除回收站数据
    std::string deleteDeleteRawContactSql = "DELETE FROM deleted_raw_contact WHERE raw_contact_id in (" +
        dirtyRawIdStr + ")";
    retCode = store_->ExecuteSql(deleteDeleteRawContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalBatch deleted_raw_contact code:%{public}d", retCode);
        return retCode;
    }
    return retCode;
}

int ContactsDataBase::DeleteLocal(int rawContactId, std::string contactId, int isDeleteValue)
{
    // 删除本地表、云表、回收站表的逻辑
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteLocal store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    HILOG_WARN("DeleteLocal rawContactId is :%{public}d, isDeleted is :%{public}d", rawContactId, isDeleteValue);
    std::string rawIdStr = std::to_string(rawContactId);
    std::string ringtone = QueryPersonalRingtone(contactId);
    int retCode = OHOS::NativeRdb::E_OK;
    if (isDeleteValue == 1) {
        std::string updateRawContactSql = "UPDATE raw_contact SET contact_id = NULL WHERE id = " + rawIdStr;
        retCode = store_->ExecuteSql(updateRawContactSql);
        if (retCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DeleteLocal updateRawContactSql code:%{public}d", retCode);
            return retCode;
        }
        std::string deleteSearchContactSql = "DELETE FROM search_contact WHERE raw_contact_id =  " + rawIdStr;
        retCode = store_->ExecuteSql(deleteSearchContactSql);
        if (retCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DeleteLocal deleteSearchContactSql code:%{public}d", retCode);
            return retCode;
        }
        std::string deleteContactData = "DELETE FROM contact_data WHERE raw_contact_id = " + rawIdStr;
        retCode = store_->ExecuteSql(deleteContactData);
        if (retCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DeleteLocal deleteContactData code:%{public}d", retCode);
            return retCode;
        }
        std::string deleteContactSql = "DELETE FROM contact WHERE name_raw_contact_id = " + rawIdStr;
        retCode = store_->ExecuteSql(deleteContactSql);
        if (retCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DeleteLocal deleteContactSql code:%{public}d", retCode);
            return retCode;
        }
        retCode = DeleteCloud(rawContactId, retCode);
        if (retCode != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DeleteLocal DeleteCloud code:%{public}d", retCode);
            return retCode;
        }
    }
    std::string deleteDeleteRawContactSql = "DELETE FROM deleted_raw_contact WHERE raw_contact_id = " + rawIdStr;
    retCode = store_->ExecuteSql(deleteDeleteRawContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal deleted_raw_contact code:%{public}d", retCode);
        return retCode;
    }
    DeletePersonalRingtone(ringtone);
    return retCode;
}

int ContactsDataBase::DeleteLocalMerged(std::string contactId, std::string rawContactId)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase DeleteLocalMerged store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    std::string updateContactSql = "UPDATE contact SET name_raw_contact_id = NULL WHERE id = " + contactId;
    int retCode = store_->ExecuteSql(updateContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalMerged updateContactSql code:%{public}d", retCode);
        return retCode;
    }
    std::string updateRawContactSql = "UPDATE raw_contact SET contact_id = NULL WHERE contact_id = " + contactId;
    retCode = store_->ExecuteSql(updateRawContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalMerged updateRawContactSql code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteSearchContactSql = "DELETE FROM search_contact WHERE contact_id =  " + contactId;
    retCode = store_->ExecuteSql(deleteSearchContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalMerged deleteSearchContactSql code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteContactData = "DELETE FROM contact_data WHERE raw_contact_id = " + rawContactId;
    retCode = store_->ExecuteSql(deleteContactData);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalMerged deleteContactData code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteContactSql = "DELETE FROM contact WHERE id = " + contactId;
    retCode = store_->ExecuteSql(deleteContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalMerged deleteContactSql code:%{public}d", retCode);
        return retCode;
    }
    std::string deleteRawContactSql = "DELETE FROM raw_contact WHERE id =  " + rawContactId;
    retCode = store_->ExecuteSql(deleteRawContactSql);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocalMerged deleteRawContactSql code:%{public}d", retCode);
        return retCode;
    }
    return retCode;
}

int ContactsDataBase::DeleteCloud(int rawContactId, int retCode)
{
    retCode = DeleteCloudContact(rawContactId);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal DeleteCloudContact code:%{public}d", retCode);
        return retCode;
    }
    retCode = DeleteCloudGroups(rawContactId);
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteLocal deleteCloudGroups code:%{public}d", retCode);
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

int ContactsDataBase::DeleteCloudBatch(std::vector<std::string> &rawContactIdVector)
{
    int ret = OHOS::NativeRdb::E_OK;
    // 查询uuid
    std::string dirtyRawIdStr = GetRawIdsToString(rawContactIdVector);
    std::vector<std::string> uuidVector;
    std::string queryUuid = "SELECT uuid FROM raw_contact WHERE id in (" + dirtyRawIdStr + ")";
    auto resultSet = store_->QuerySql(queryUuid);
    if (resultSet == nullptr) {
        HILOG_ERROR("DeleteCloudBatch QuerySqlResult is null");
        return OHOS::NativeRdb::E_ERROR;
    }
    int getRowResult = resultSet->GoToFirstRow();
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        std::string uuid;
        std::string columnName = CloudRawContactColumns::UUID;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetString(columnIndex, uuid);
        if (!uuid.empty()) {
            uuidVector.push_back(uuid);
        }
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
    // 存在uuid，删除云表数据
    if (uuidVector.size() > 0) {
        HILOG_WARN("DeleteCloud uuid size:%{public}ld", (long) uuidVector.size());
        // 根据uuid删除云表数据
        std::string uuidStr = GetStrConditionsToString(uuidVector);
        std::string deleteCloudRawContactSql = "DELETE FROM cloud_raw_contact WHERE uuid in (" + uuidStr + ")";
        ret = store_->ExecuteSql(deleteCloudRawContactSql);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DeleteCloudContactBatch error, deleteCloudRawContactSql execute code:%{public}d", ret);
            return ret;
        }
        // 根据uuid删除云表数据
        std::string deleteGroupSql = "DELETE FROM cloud_groups WHERE uuid in (" + uuidStr + ")";
        ret = store_->ExecuteSql(deleteGroupSql);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("DeleteCloudContactBatch error, deleteGroupSql execute code:%{public}d", ret);
            return ret;
        }
    }

    // 根据rawId删除raw表信息
    std::string deleteRawContactSql = "DELETE FROM raw_contact WHERE id in (" + dirtyRawIdStr + ")";
    ret = store_->ExecuteSql(deleteRawContactSql);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteCloudContactBatch error, deleteRawContactSql execute code:%{public}d", ret);
        return ret;
    }

    return ret;
}

// 查询contact_data的type信息，设置到types；calendarEventId信息，设置到calendarEventIds
std::vector<int> ContactsDataBase::QueryContactDataRawContactId(OHOS::NativeRdb::RdbPredicates &rdbPredicates,
    std::vector<std::string> &types, std::string &calendarEventIds, std::string methodName)
{
    std::vector<std::string> columns;
    columns.push_back(ContactDataColumns::TYPE_ID);
    columns.push_back(ContactDataColumns::RAW_CONTACT_ID);
    columns.push_back(ContactDataColumns::CALENDAR_EVENT_ID);
    auto resultSet = store_->Query(rdbPredicates, columns);
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

        std::string calendarEventIdColumn = ContactDataColumns::CALENDAR_EVENT_ID;
        int columncalendarEventIdIndex = 0;
        resultSet->GetColumnIndex(calendarEventIdColumn, columncalendarEventIdIndex);
        std::string calendarEventId;
        resultSet->GetString(columncalendarEventIdIndex, calendarEventId);
        if (!calendarEventId.empty()) {
            calendarEventIds.append(calendarEventId).append(",");
        }
        OHOS::NativeRdb::ValueObject typeTextObject;
        typeIdVector.push_back(typeId);
        rawContactIdVector.push_back(rawContactId);
        HILOG_WARN("When %{public}s queryContactDataRawContactId typeId is %{public}d, rawContactId is:%{public}d",
            methodName.c_str(),
            typeId,
            rawContactId);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    unsigned int typeIdSize = typeIdVector.size();
    // type信息
    ContactsType contactsType;
    for (unsigned int i = 0; i < typeIdSize; i++) {
        std::string typeText;
        contactsType.GetTypeText(store_, typeIdVector[i], typeText);
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
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::Query(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &columns)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase Query store_ is nullptr, ts = %{public}lld", (long long) time(NULL));
        return nullptr;
    }
    int errCode = OHOS::NativeRdb::E_OK;
    auto resultSet = store_->QueryByStep(rdbPredicates, columns);
    int32_t rowCount = -1;
    errCode = resultSet->GetRowCount(rowCount);
    HILOG_INFO("ContactsDataBase Query retCode= %{public}d, resultCount= %{public}d", errCode, rowCount);
    if (errCode == OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        errCode = HandleRdbStoreRetry([&]() { return store_->Restore("contacts.db.bak"); });
        HILOG_ERROR("ContactsDataBase Query Restore retCode= %{public}d", errCode);
    }
    return resultSet;
}


/**
 * @brief Query the number of contacts by company group
 *
 * @return The result returned by the query operation
 */
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryContactByCompanyGroup()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase QueryContactByCompanyGroup store_ is nullptr");
        return nullptr;
    }
    std::string queryCompanySql =
        "SELECT company, count(distinct(name_raw_contact_id)) AS count FROM view_contact WHERE company is not null "
        "AND company <> '' AND is_deleted = 0 AND primary_contact <> 1 "
        "group by company order by count desc";
    auto resultSet = store_->QuerySql(queryCompanySql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryContactByCompanyGroup error");
    }
    return resultSet;
}

/**
 * @brief Query the number of contacts without company
 *
 * @return The result returned by the query operation
 */
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryCountWithoutCompany()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase QueryCountWithoutCompany store_ is nullptr");
        return nullptr;
    }
    std::string queryNoCompanySql =
        "SELECT count(distinct(name_raw_contact_id)) AS count FROM view_contact WHERE "
        "(company is null OR company = '') AND primary_contact <> 1 AND is_deleted = 0";
    auto resultSet = store_->QuerySql(queryNoCompanySql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryCountWithoutCompany error");
    }
    return resultSet;
}

/**
 * @brief Query the number of contacts by Location group
 *
 * @return The result returned by the query operation
 */
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryContactByLocationGroup()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase QueryContactByLocationGroup store_ is nullptr");
        return nullptr;
    }
    // 需要关注智能合并联系人的处理
    std::string querylocationSql =
    "SELECT CASE WHEN INSTR(location, ' ') > 0  THEN SUBSTR(location, 1, INSTR(location, ' ') - 1) ELSE location "
    "END AS location, COUNT(DISTINCT raw_contact.contact_id) AS count FROM  contact_data LEFT JOIN raw_contact ON "
    "contact_data.raw_contact_id = raw_contact.id WHERE type_id = 5 AND location <> '' AND location IS NOT NULL "
    "AND is_deleted <> 1 AND primary_contact <> 1  GROUP BY CASE WHEN INSTR(location, ' ') > 0 "
    "THEN SUBSTR(location, 1, INSTR(location, ' ') - 1) ELSE location END  ORDER BY  count DESC";
    auto resultSet = store_->QuerySql(querylocationSql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryContactByLocationGroup error");
    }
    return resultSet;
}

/**
 * @brief Query the number of contacts without location
 *
 * @return The result returned by the query operation
 */
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryCountWithoutLocation()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase QueryCountWithoutLocation store_ is nullptr");
        return nullptr;
    }
    std::string queryRawIdSql = "SELECT id FROM raw_contact WHERE is_deleted = 1 or primary_contact = 1;";
    std::string idList = QueryContactIds(queryRawIdSql, RawContactColumns::ID);
    HILOG_ERROR("QueryCountWithoutLocation idList error %{public}s", idList.c_str());
    std::string queryNoLocationSql =
        "SELECT COUNT(*) AS count FROM ( SELECT raw_contact_id FROM contact_data GROUP BY "
        "raw_contact_id HAVING MAX(COALESCE(location, '')) = '') AS filtered WHERE raw_contact_id NOT IN ("+idList+")";
    auto resultSet = store_->QuerySql(queryNoLocationSql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryCountWithoutLocation error");
    }
    return resultSet;
}

/**
 * @brief Query the number of contacts based on the most recent contact time
 *
 * @return The result returned by the query operation
 */
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryContactByRecentTime()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase QueryContactByRecentTime store_ is nullptr");
        return nullptr;
    }
    std::string queryRecentSql = R"(
    SELECT 
        contact_type.type, COALESCE(contact_counts.count, 0) AS count
    FROM (
            SELECT 'CONTACT_WEEKLY' AS type
            UNION ALL
            SELECT 'CONTACT_MONTHLY' AS type
            UNION ALL
            SELECT 'CONTACT_IN_THREE_MONTHS' AS type
            UNION ALL
            SELECT 'CONTACT_OVER_THREE_MONTHS' AS type ) AS contact_type
    LEFT JOIN (
        SELECT 
            COUNT(id) AS count,
            CASE
                WHEN (lastest_contacted_time >= cast(strftime('%s', 'now', '-7 day') as int)) THEN 'CONTACT_WEEKLY'
                WHEN (lastest_contacted_time >= cast(strftime('%s', 'now', '-1 month') as int)) THEN 'CONTACT_MONTHLY'
                WHEN (lastest_contacted_time >= cast(strftime('%s', 'now', '-3 month') as int)) 
                THEN 'CONTACT_IN_THREE_MONTHS' ELSE 'CONTACT_OVER_THREE_MONTHS'
            END AS type
        FROM (
            SELECT 
                id, lastest_contacted_time, contact_id, ROW_NUMBER() OVER (
                    PARTITION BY contact_id ORDER BY lastest_contacted_time DESC
                ) AS rn
            FROM raw_contact WHERE is_deleted <> 1 AND primary_contact <> 1
        ) AS deduplicated
        WHERE rn = 1  -- 仅保留每组 contact_id 中 latest_contacted_time 最大的记录
        GROUP BY type
    ) AS contact_counts
    ON 
        contact_type.type = contact_counts.type
    ORDER BY 
        CASE 
            WHEN contact_type.type = 'CONTACT_WEEKLY' THEN 1
            WHEN contact_type.type = 'CONTACT_MONTHLY' THEN 2
            WHEN contact_type.type = 'CONTACT_IN_THREE_MONTHS' THEN 3
            ELSE 4
        END;)";
    auto resultSet = store_->QuerySql(queryRecentSql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryContactByRecentTime error");
    }
    return resultSet;
}


std::string ContactsDataBase::QueryContactIds(const std::string &querySql, const std::string &columnName)
{
    std::vector<int32_t> ids;
    auto rawSet = store_->QuerySql(querySql);
    if (rawSet) {
        int resultSetNum = rawSet->GoToFirstRow();
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            int columnIndex = 0;
            rawSet->GetColumnIndex(columnName, columnIndex);
            int id = 0;
            rawSet->GetInt(columnIndex, id);
            ids.push_back(id);
            resultSetNum = rawSet->GoToNextRow();
        }
    }
    std::string idList = "";
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i != 0) idList += ", ";
        idList += std::to_string(ids[i]);
    }
    return idList;
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryDetectRepair()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase QueryDetectRepair store_ is nullptr");
	    return nullptr;
    }
    std::string querySql = R"(SELECT delete_source, delete_time, display_name FROM deleted_raw_contact
        WHERE (delete_time >= cast(strftime('%s', 'now', '-15 days') as int));)";
    auto resultSet = store_->QuerySql(querySql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryDetectRepair error");
    }
    return resultSet;
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryLocationContact(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &columns)
{
    // 1、解析参数
    std::shared_ptr<OHOS::NativeRdb::ResultSet> result;
    std::string whereSql = rdbPredicates.GetWhereClause();
    std::vector<std::string> args = rdbPredicates.GetWhereArgs();
    if (args.empty()) {
        HILOG_ERROR("queryArg is empty, cannot construct IN clause");
        return nullptr;
    }
    int limit = rdbPredicates.GetLimit();
    int offset = rdbPredicates.GetOffset();
    if (whereSql.find("LIKE") != std::string::npos) {  // 查询具体归属地的联系人
        std::string queryLocationSql =
            "SELECT DISTINCT raw_contact.contact_id as contact_id FROM contact_data LEFT "
            "JOIN raw_contact ON contact_data.raw_contact_id = raw_contact.id WHERE CASE WHEN INSTR(location, ' ') > 0 "
            "THEN SUBSTR(location, 1, INSTR(location, ' ') - 1) ELSE location END = ? AND type_id = 5;";

        std::vector<std::string> updateArgs;
        updateArgs.push_back(std::regex_replace(args[0], percent, ""));
        if (limit != 0) {
            queryLocationSql = "SELECT DISTINCT raw_contact.contact_id as contact_id FROM contact_data LEFT "
            "JOIN raw_contact ON contact_data.raw_contact_id = raw_contact.id WHERE CASE WHEN INSTR(location, ' ') > 0 "
            "THEN SUBSTR(location, 1, INSTR(location, ' ') - 1) ELSE location END = ? AND type_id = 5 "
            "limit ? offset ?;";
            updateArgs.push_back(std::to_string(limit));
            updateArgs.push_back(std::to_string(offset));
        }

        auto rawSet = store_->QuerySql(queryLocationSql, updateArgs);
 
        std::vector<int32_t> ids;
        if (rawSet) {
            int resultSetNum = rawSet->GoToFirstRow();
            while (resultSetNum == OHOS::NativeRdb::E_OK) {
                int columnIndex = 0;
                rawSet->GetColumnIndex("contact_id", columnIndex);
                int id = 0;
                rawSet->GetInt(columnIndex, id);
                ids.push_back(id);
                resultSetNum = rawSet->GoToNextRow();
            }
        }
        std::string idList = "";
        for (size_t i = 0; i < ids.size(); ++i) {
            if (i != 0) idList += ", ";
            idList += std::to_string(ids[i]);
        }
        if (!idList.empty()) {
            result = QueryViewContact(columns, idList);
        }
    } else { // 查询没有归属地联系人
        std::string queryNoLocationSql ="SELECT contact_id FROM ( SELECT raw_contact.contact_id FROM contact_data "
        "LEFT JOIN raw_contact ON contact_data.raw_contact_id = raw_contact.id GROUP BY "
        "contact_id HAVING MAX(COALESCE(location, '')) = '') AS filtered";
        std::string idList = QueryContactIds(queryNoLocationSql, RawContactColumns::CONTACT_ID);
        if (!idList.empty()) {
            result = QueryViewContact(columns, idList);
        }
    }
    if (result == nullptr) {
        HILOG_ERROR("QueryLocationContact error");
    }
    return result;
}
 
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryViewContact(std::vector<std::string> &columns,
    std::string queryArg)

{
     if (store_ == nullptr) {
        HILOG_ERROR("QueryViewContact store_ is nullptr, ts = %{public}lld", (long long) time(NULL));
        return nullptr;
    }
    auto predicates = OHOS::NativeRdb::RdbPredicates(ViewName::VIEW_CONTACT);
    std::string queryWheres = "is_deleted <> 1 AND primary_contact <> 1 AND id IN (" 
                              + queryArg + ") order by sort, sort_key asc";
    predicates.SetWhereClause(queryWheres);
    auto resultSet = store_->QueryByStep(predicates, columns);

    if (resultSet == nullptr) {
        HILOG_ERROR("QueryViewContact error");
    }
    return resultSet;
}

/**
 * @brief Query uuid not in raw_contact
 *
 * @param rdbPredicates Conditions for query operation
 * @param columns Conditions for query operation
 *
 * @return The result returned by the query operation
 */
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryUuidNotInRawContact()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase QueryUuidNotInRawContact store_ is nullptr");
        return nullptr;
    }
    std::string queryUUIDSql =
        "SELECT uuid from cloud_raw_contact where uuid not in (SELECT uuid from raw_contact WHERE uuid not null)";
    auto resultSet = store_->QuerySql(queryUUIDSql);
    if (resultSet == nullptr) {
        HILOG_INFO("QueryUuidNotInRawContact error");
    }
    return resultSet;
}

/**
 * @brief Query big length name contact
 *
 * @return The result returned by the query operation
 */
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::QueryBigLengthNameContact()
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase QueryBigLengthNameContact store_ is nullptr");
        return nullptr;
    }
    std::string querySql = "SELECT * FROM contact_data WHERE type_id in (3,4,6,7,10) and "
                           "(LENGTH(detail_info) > 7 or LENGTH(position) > 7) GROUP by raw_contact_id";
    auto resultSet = store_->QuerySql(querySql);
    if (resultSet == nullptr) {
        HILOG_INFO("QueryBigLengthNameContact error");
    }
    return resultSet;
}

/**
 * @brief Manually synchronizing contacts.
 */
int ContactsDataBase::SyncContacts(const int32_t syncCode)
{
    HILOG_INFO("Cloud sync contacts start, ts = %{public}lld", (long long) time(NULL));
    if (g_isSyncing) {
        return OPERATION_ERROR;
    }
    if (store_ == nullptr) {
        HILOG_ERROR("Cloud sync contacts store_ is nullptr");
        return OPERATION_ERROR;
    }
    g_isSyncing = true;
    OHOS::DistributedRdb::SyncOption option = {OHOS::DistributedRdb::TIME_FIRST, false};
    auto detailsCallback = [this](OHOS::DistributedRdb::Details &&details) {
        unsigned int size = details.size();
        HILOG_INFO("Cloud sync contacts detailsCallback start details size is :%{public}d, ts is :%{public}lld",
            size, (long long) time(NULL));
        for (auto &detail : details) {
            if (detail.first == "default") {
                HILOG_INFO("cloud sync contact details progress is:%{public}d, ts = %{public}lld",
                    detail.second.progress, (long long) time(NULL));
                if (detail.second.progress == OHOS::DistributedRdb::SYNC_FINISH) {
                    HILOG_WARN("cloud sync contacts finished, detail.second.code = %{public}d, ts = %{public}lld",
                        detail.second.code, (long long) time(NULL));
                    if (detail.second.code == OHOS::DistributedRdb::ProgressCode::SUCCESS) {
                        contactsConnectAbility_->ConnectAbility("", "", "", "", "checkCursorAndUpload", "");
                        ClearRetryCloudSyncCount();
                    } else {
                        RetryCloudSync(details.begin()->second.code);
                        BoardReportUtil::BoardReportCloudState(details.begin()->second.code);
                    }
                }
            }
        }
    };
    int errCode = OHOS::NativeRdb::E_OK;

    // 获取当前系统时间
    const int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
    // 云空间剩余空间小于待同步的资产大小时，将资产待上行的数据进行过滤处理，限制该接口调用次数
    if (syncCode == OHOS::DistributedRdb::ProgressCode::NO_SPACE_FOR_ASSET && CalcSyncTime(currentTime)) {
        std::thread thread([this]() { SyncContactsWithoutSpace(); });
        thread.detach();
    } else {
        errCode = store_->Sync(option, tablesToSync, detailsCallback);
    }

    g_isSyncing = false;
    HILOG_WARN("Cloud sync contacts successfully :%{public}d, ts is :%{public}lld", errCode, (long long) time(NULL));
    if (errCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("Cloud sync contacts error :%{public}d", errCode);
    }
    return errCode;
}

/**
 * calculate the time difference between the current time and the last time.
 */
bool ContactsDataBase::CalcSyncTime(const int64_t currentTime)
{
    if (currentTime - g_lastSyncTime < SYNC_CONTACT_MILLISECOND) {
        // 调用过，但距上次调用时间不超过阈值， 不触发云同步操作，返回false
        HILOG_WARN("Less than 3 hours, Cloud sync contacts failed currentTime:%{public}lld,"
                   "g_lastSyncTime is :%{public}lld",
            (long long) currentTime,
            (long long) g_lastSyncTime);
        return false;
    } else {
        // 距离上次同步超过3h，重新设置同步时间，返回true
        g_lastSyncTime = currentTime;
        HILOG_WARN("Over 3h, data synchronization can continue, reset g_lastSyncTime = %{public}lld.", (long long) g_lastSyncTime);
        return true;
    }
}

/**
 * Reset the number of contact synchronization retries.
 */
void ContactsDataBase::ClearRetryCloudSyncCount()
{
    g_retryCount = 0;
}

void ContactsDataBase::RetryCloudSync(int code)
{
    HILOG_INFO("RetryCloudSync start code is :%{public}d, ts is :%{public}lld", code, (long long) time(NULL));
    int maxRetries = 3;
    int retryDelay = 3;  // in seconds
    switch (code) {
        case OHOS::DistributedRdb::ProgressCode::SUCCESS:
            g_retryCount = 0;
            HILOG_INFO("RetryCloudSync succeed, ts = %{public}lld", (long long) time(NULL));
            break;
        default:
            HILOG_INFO("RetryCloudSync default code is: %{public}d, g_retryCount is: %{public}d, ts is: %{public}lld",
                code,
                g_retryCount,
                (long long) time(NULL));
            if (g_retryCount < maxRetries) {
                std::this_thread::sleep_for(std::chrono::seconds(retryDelay));
                SyncContacts(code);
                g_retryCount++;
            }
            break;
    }
}

std::vector<OHOS::NativeRdb::ValuesBucket> ContactsDataBase::ResultSetToValuesBucket(
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
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
    std::shared_ptr<OHOS::NativeRdb::ResultSet> contactDataResultSet = store_->QuerySql(sql, selectionArgs);
    if (contactDataResultSet == nullptr) {
        HILOG_ERROR("StructureDeleteContactJson QuerySqlResult is null");
        return "";
    }
    std::string backupData = contactsJsonUtils.GetDeleteData(contactDataResultSet);
    contactDataResultSet->Close();
    return backupData;
}

int SqliteOpenHelperContactCallback::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    HILOG_INFO("ContactsDataBase OnCreate contacts db");
    std::vector<int> judgeSuccess;
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_INDEX));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_RAW_CONTACT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_RAW_CONTACT_INDEX));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_RAW_CONTACT_INDEX_UNIQUE_KEY));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_RAW_CONTACT_INDEX_SORT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_DATA));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_INDEX_DATA1));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_INDEX_DATA2));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_BLOCKLIST));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_LOCAL_LANG));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_ACCOUNT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_PHOTO_FILES));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_TYPE));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_GROUPS));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_DELETED_RAW_CONTACT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_SEARCH_CONTACT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_SEARCH_CONTACT_INDEX1));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_SEARCH_CONTACT_INDEX2));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_SEARCH_CONTACT_VIEW));
    judgeSuccess.push_back(store.ExecuteSql(MERGE_INFO));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_VIEW_CONTACT_DATA));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_VIEW_RAW_CONTACT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_VIEW_CONTACT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_VIEW_CONTACT_LOCATION));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_VIEW_GROUPS));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_VIEW_DELETED));
    judgeSuccess.push_back(store.ExecuteSql(UPDATE_RAW_CONTACT_VERSION));
    judgeSuccess.push_back(store.ExecuteSql(INSERT_CONTACT_QUICK_SEARCH));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_DATABASE_BACKUP_TASK));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_INSERT_BACKUP_TIME));
    judgeSuccess.push_back(store.ExecuteSql(UPDATE_CONTACT_BY_INSERT_CONTACT_DATA));
    judgeSuccess.push_back(store.ExecuteSql(UPDATE_CONTACT_BY_DELETE_CONTACT_DATA));
    judgeSuccess.push_back(store.ExecuteSql(UPDATE_CONTACT_BY_UPDATE_CONTACT_DATA));
    judgeSuccess.push_back(store.ExecuteSql(MERGE_INFO_INDEX));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CLOUD_RAW_CONTACT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CLOUD_GROUPS));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_SETTINGS));
    judgeSuccess.push_back(store.ExecuteSql(INIT_CHANGE_TIME));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CONTACT_BLOCKLIST_INDEX_PHONE));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_HW_ACCOUNT));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_CLOUD_CONTACT_BLOCKLIST));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_PRIVACY_CONTACTS_BACKUP));
    judgeSuccess.push_back(store.ExecuteSql(CREATE_POSTER));
    unsigned int size = judgeSuccess.size();
    unsigned int successCount = size;
    for (unsigned int i = 0; i < size; i++) {
        int ret = judgeSuccess[i];
        if (ret != OHOS::NativeRdb::E_OK) {
            successCount--;
            HILOG_ERROR("SqliteOpenHelperCallLogCallback create table index %{public}d error: %{public}d", i, ret);
            BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_CREATE, CONTACTS_DB, ret,
                                                      "create table " + std::to_string(i) + " error");
        }
    }
    if (successCount == size) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_CREATE, CONTACTS_DB, 0, "OnCreate success");
    }
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperContactCallback::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN(
        "ContactsDataBase OnUpgrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion < newVersion && newVersion == DATABASE_NEW_VERSION) {
        store.ExecuteSql("ALTER TABLE database_backup_task ADD COLUMN sync TEXT");
    }
    int result = OHOS::NativeRdb::E_OK;
    std::string dbName = "contacts.db";
    BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_BEFORE, dbName, oldVersion);
    UpgradeUnderV10(store, oldVersion, newVersion);
    UpgradeUnderV20(store, oldVersion, newVersion);
    result = UpgradeUnderV30(store, oldVersion, newVersion);
    if (result != OHOS::NativeRdb::E_OK) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, dbName, result,
                                                  "UpgradeUnderV30 fail");
        return result;
    }
    result = UpgradeUnderV35(store, oldVersion, newVersion);
    if (result != OHOS::NativeRdb::E_OK) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, dbName, result,
                                                  "UpgradeUnderV35 fail");
        return result;
    }
    result = UpgradeUnderV40(store, oldVersion, newVersion);
    if (result != OHOS::NativeRdb::E_OK) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, dbName, result,
                                                  "UpgradeUnderV40 fail");
        return result;
    }
    result = UpgradeUnderV45(store, oldVersion, newVersion);
    if (result != OHOS::NativeRdb::E_OK) {
        BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, dbName, result,
                                                  "UpgradeUnderV45 fail");
        return result;
    }
    BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo::DB_UPGRADE_AFTER, dbName, newVersion);
    HILOG_INFO("ContactsDataBase OnUpgrade result is %{public}d", result);
    return result;
}

void SqliteOpenHelperContactCallback::UpgradeUnderV10(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion < DATABASE_VERSION_2 && newVersion >= DATABASE_VERSION_2) {
        UpgradeToV2(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_3 && newVersion >= DATABASE_VERSION_3) {
        UpgradeToV3(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_4 && newVersion >= DATABASE_VERSION_4) {
        UpgradeToV4(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_5 && newVersion >= DATABASE_VERSION_5) {
        UpgradeToV5(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_6 && newVersion >= DATABASE_VERSION_6) {
        UpgradeToV6(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_7 && newVersion >= DATABASE_VERSION_7) {
        UpgradeToV7(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_8 && newVersion >= DATABASE_VERSION_8) {
        UpgradeToV8(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_9 && newVersion >= DATABASE_VERSION_9) {
        UpgradeToV9(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_10 && newVersion >= DATABASE_VERSION_10) {
        UpgradeToV10(store, oldVersion, newVersion);
    }
}

void SqliteOpenHelperContactCallback::UpgradeUnderV20(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion < DATABASE_VERSION_11 && newVersion >= DATABASE_VERSION_11) {
        UpgradeToV11(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_12 && newVersion >= DATABASE_VERSION_12) {
        UpgradeToV12(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_13 && newVersion >= DATABASE_VERSION_13) {
        UpgradeToV13(store, oldVersion, newVersion);
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
    if (oldVersion < DATABASE_VERSION_17 && newVersion >= DATABASE_VERSION_17) {
        UpgradeToV17(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_18 && newVersion >= DATABASE_VERSION_18) {
        UpgradeToV18(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_19 && newVersion >= DATABASE_VERSION_19) {
        UpgradeToV19(store, oldVersion, newVersion);
    }
}

int SqliteOpenHelperContactCallback::Commit(OHOS::NativeRdb::RdbStore &store)
{
    int ret = HandleRdbStoreRetry([&]() {
        return store.Commit();
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("SqliteOpenHelperContactCallback Commit failed :%{public}d", ret);
    }
    return ret;
}
int SqliteOpenHelperContactCallback::RollBack(OHOS::NativeRdb::RdbStore &store)
{
    int ret = HandleRdbStoreRetry([&]() {
        return store.RollBack();
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("SqliteOpenHelperContactCallback RollBack failed :%{public}d", ret);
    }
    return ret;
}
int SqliteOpenHelperContactCallback::BeginTransaction(OHOS::NativeRdb::RdbStore &store)
{
    int ret = HandleRdbStoreRetry([&]() {
        return store.BeginTransaction();
    });
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("SqliteOpenHelperContactCallback BeginTransaction failed :%{public}d", ret);
    }
    return ret;
}

int SqliteOpenHelperContactCallback::UpgradeUnderV30(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion < DATABASE_VERSION_20 && newVersion >= DATABASE_VERSION_20) {
        UpgradeToV20(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_21 && newVersion >= DATABASE_VERSION_21) {
        UpgradeToV21(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_22 && newVersion >= DATABASE_VERSION_22) {
        UpgradeToV22(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_23 && newVersion >= DATABASE_VERSION_23) {
        UpgradeToV23(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_24 && newVersion >= DATABASE_VERSION_24) {
        UpgradeToV24(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_25 && newVersion >= DATABASE_VERSION_25) {
        UpgradeToV25(store, oldVersion, newVersion);
    }
    if (oldVersion < DATABASE_VERSION_26 && newVersion >= DATABASE_VERSION_26) {
        result = UpgradeToV26(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_27 && newVersion >= DATABASE_VERSION_27) {
        result = UpgradeToV27(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_28 && newVersion >= DATABASE_VERSION_28) {
        result = UpgradeToV28(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_29 && newVersion >= DATABASE_VERSION_29) {
        result = UpgradeToV29(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_30 && newVersion >= DATABASE_VERSION_30) {
        result = UpgradeToV30(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_31 && newVersion >= DATABASE_VERSION_31) {
        result = UpgradeToV31(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeUnderV35(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion < DATABASE_VERSION_32 && newVersion >= DATABASE_VERSION_32) {
        result = UpgradeToV32(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_33 && newVersion >= DATABASE_VERSION_33) {
        result = UpgradeToV33(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_34 && newVersion >= DATABASE_VERSION_34) {
        result = UpgradeToV34(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_35 && newVersion >= DATABASE_VERSION_35) {
        result = UpgradeToV35(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeUnderV40(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion < DATABASE_VERSION_36 && newVersion >= DATABASE_VERSION_36) {
        result = UpgradeToV36(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_37 && newVersion >= DATABASE_VERSION_37) {
        result = UpgradeToV37(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_38 && newVersion >= DATABASE_VERSION_38) {
        result = UpgradeToV38(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_39 && newVersion >= DATABASE_VERSION_39) {
        result = UpgradeToV39(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_40 && newVersion >= DATABASE_VERSION_40) {
        result = UpgradeToV40(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeUnderV45(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    int result = OHOS::NativeRdb::E_OK;
    if (oldVersion < DATABASE_VERSION_41 && newVersion >= DATABASE_VERSION_41) {
        result = UpgradeToV41(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    if (oldVersion < DATABASE_VERSION_42 && newVersion >= DATABASE_VERSION_42) {
        result = UpgradeToV42(store, oldVersion, newVersion);
        if (result != OHOS::NativeRdb::E_OK) {
            return result;
        }
    }
    return result;
}

void SqliteOpenHelperContactCallback::UpgradeToV2(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion >= newVersion) {
        return;
    }
    // raw_contact
    store.ExecuteSql(RAW_CONTACT_ADD_PRIMARY_CONTACT);
    store.ExecuteSql(RAW_CONTACT_ADD_EXTRA1);
    store.ExecuteSql(RAW_CONTACT_ADD_EXTRA2);
    store.ExecuteSql(RAW_CONTACT_ADD_EXTRA3);
    store.ExecuteSql(RAW_CONTACT_ADD_EXTRA4);
    // contact_data
    store.ExecuteSql(CONTACT_DATA_ADD_EXTEND8);
    store.ExecuteSql(CONTACT_DATA_ADD_EXTEND9);
    store.ExecuteSql(CONTACT_DATA_ADD_EXTEND10);
    store.ExecuteSql(CONTACT_DATA_ADD_EXTEND11);
    // drop view
    store.ExecuteSql("DROP VIEW view_contact;");
    store.ExecuteSql("DROP VIEW view_contact_data;");
    store.ExecuteSql("DROP VIEW search_contact_view;");
    store.ExecuteSql("DROP VIEW view_deleted;");
    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT);
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    store.ExecuteSql(CREATE_SEARCH_CONTACT_VIEW);
    store.ExecuteSql(CREATE_VIEW_DELETED);
}

void SqliteOpenHelperContactCallback::UpgradeToV3(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion >= newVersion) {
        return;
    }
    // cloud sync
    store.ExecuteSql(CREATE_CLOUD_RAW_CONTACT);
    store.ExecuteSql(CREATE_CLOUD_GROUPS);

    // raw_contact
    store.ExecuteSql(RAW_CONTACT_ADD_DIRTY);
    store.ExecuteSql(RAW_CONTACT_ADD_UUID);
}

void SqliteOpenHelperContactCallback::UpgradeToV4(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion >= newVersion) {
        return;
    }
    // drop view
    store.ExecuteSql("DROP VIEW view_raw_contact;");

    // create view
    store.ExecuteSql(CREATE_VIEW_RAW_CONTACT);
}

void SqliteOpenHelperContactCallback::UpgradeToV5(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion >= newVersion) {
        return;
    }
    HILOG_INFO("UpgradeToV5 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);

    // create setting table
    store.ExecuteSql(CREATE_SETTINGS);
    store.ExecuteSql(INIT_CHANGE_TIME);
}

void SqliteOpenHelperContactCallback::UpgradeToV6(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion >= newVersion) {
        return;
    }
    HILOG_INFO("UpgradeToV6 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    HILOG_INFO("UpgradeToV6 start!");
    // 1, raw_contact add sort_key
    store.ExecuteSql(RAW_CONTACT_ADD_SORT_KEY);
    // 2, 查询所有记录，更新sortKey，更新无名氏的sort数字和sort首字母
    UpdateSrotInfoByDisplayName(store);
    // 3，view操作，需要加个字段，删除重建
    store.ExecuteSql("drop view if exists view_contact;");
    store.ExecuteSql("drop view if exists view_raw_contact;");
    store.ExecuteSql(CREATE_VIEW_CONTACT);
    store.ExecuteSql(CREATE_VIEW_RAW_CONTACT);
    // 4，索引操作
    store.ExecuteSql(CREATE_RAW_CONTACT_INDEX_SORT);
    HILOG_INFO("UpgradeToV6 end!");
}

void SqliteOpenHelperContactCallback::UpgradeToV7(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    if (oldVersion >= newVersion) {
        return;
    }
    HILOG_INFO("UpgradeToV7 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);

    // update setting table
    store.ExecuteSql("ALTER TABLE settings ADD COLUMN contacts_on_card TEXT DEFAULT ',';");
    store.ExecuteSql("ALTER TABLE settings DROP COLUMN calllog_change_time;");
}

void SqliteOpenHelperContactCallback::UpgradeToV8(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV8 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isColumExists = sqlAnalyzer.CheckColumnExists(store, "contact_data", "format_phone_number");
    if (!isColumExists) {
        store.ExecuteSql(CONTACT_DATA_ADD_FORMAT_PHONE_NUMBER);
    }

    // drop view
    store.ExecuteSql("DROP VIEW view_contact_data;");

    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    UpdateFormatPhoneNumber(store);
}

void SqliteOpenHelperContactCallback::UpgradeToV9(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV9 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isColumExists = sqlAnalyzer.CheckColumnExists(store, "contact_data", "blob_source");
    if (!isColumExists) {
        store.ExecuteSql(CONTACT_DATA_ADD_BLOB_SOURCE);
    }

    // drop view
    store.ExecuteSql("DROP VIEW view_contact_data;");

    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
}

void SqliteOpenHelperContactCallback::UpgradeToV10(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV10 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isFormatPhoneNumberExists = sqlAnalyzer.CheckColumnExists(store, "contact_data", "format_phone_number");
    if (!isFormatPhoneNumberExists) {
        store.ExecuteSql(CONTACT_DATA_ADD_FORMAT_PHONE_NUMBER);
    }

    bool isSortKeyExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "sort_key");
    if (!isSortKeyExists) {
        store.ExecuteSql(RAW_CONTACT_ADD_SORT_KEY);
    }

    // drop view
    store.ExecuteSql("DROP VIEW view_contact_data;");

    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);

    // 3，view操作，需要加个字段，删除重建
    store.ExecuteSql("drop view if exists view_contact;");
    store.ExecuteSql("drop view if exists view_raw_contact;");
    store.ExecuteSql(CREATE_VIEW_CONTACT);
    store.ExecuteSql(CREATE_VIEW_RAW_CONTACT);
    // 4，索引操作
    store.ExecuteSql(CREATE_RAW_CONTACT_INDEX_SORT);
    // 2, 查询所有记录，更新sortKey，更新无名氏的sort数字和sort首字母
    int ret = store.BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DetialInfoTrimSpace BeginTransaction failed");
    }
    UpdateFormatPhoneNumber(store);
    UpdateSrotInfoByDisplayName(store);
    ret = store.Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        store.RollBack();
        HILOG_ERROR("DetialInfoTrimSpace update failed and rollback");
    }
    HILOG_INFO("UpgradeToV10 end");
}

void SqliteOpenHelperContactCallback::UpgradeToV11(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV11 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isSortKeyExists = sqlAnalyzer.CheckColumnExists(store, "contact_data", "blob_source");
    if (!isSortKeyExists) {
        store.ExecuteSql(CONTACT_DATA_ADD_BLOB_SOURCE);
    }

    // drop view
    store.ExecuteSql("DROP VIEW view_contact_data;");

    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
}

void SqliteOpenHelperContactCallback::UpgradeToV12(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV12 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    // update setting table
    SqlAnalyzer sqlAnalyzer;
    bool isColumExists = sqlAnalyzer.CheckColumnExists(store, "contact", "contact_last_updated_timestamp");
    if (!isColumExists) {
        store.ExecuteSql(CONTACT_ADD_CONTACT_LAST_UPDATED_TIMESTAMP);
    }

    OHOS::NativeRdb::ValuesBucket upRawContactDirtyValues;
    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    upRawContactDirtyValues.PutDouble("contact_last_updated_timestamp", ms.count());

    std::string upWhereClause;
    std::vector<std::string> upWhereArgs;
    int changedRows = OHOS::NativeRdb::E_OK;
    int updateDirtyRet =
        store.Update(changedRows, ContactTableName::CONTACT, upRawContactDirtyValues, upWhereClause, upWhereArgs);
    if (updateDirtyRet == OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("Contacts UpgradeToV12 succeed.");
        mUpdateRawContacts.clear();
    } else {
        HILOG_ERROR("Contacts UpgradeToV12 fail:%{public}d", updateDirtyRet);
    }
    // drop view
    store.ExecuteSql("DROP VIEW view_contact_data;");
    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
}

void SqliteOpenHelperContactCallback::UpgradeToV13(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV13 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    // update setting table
    SqlAnalyzer sqlAnalyzer;
    bool isMarkExists = sqlAnalyzer.CheckColumnExists(store, "contact_blocklist", "mark");
    if (!isMarkExists) {
        store.ExecuteSql("ALTER TABLE contact_blocklist ADD COLUMN mark INTEGER NOT NULL DEFAULT 0;");
    }

    bool isCallCountExists = sqlAnalyzer.CheckColumnExists(store, "contact_blocklist", "interception_call_count");
    if (!isCallCountExists) {
        store.ExecuteSql("ALTER TABLE contact_blocklist ADD COLUMN interception_call_count "
                         "INTEGER NOT NULL DEFAULT 0;");
    }

    bool isMsgCountExists = sqlAnalyzer.CheckColumnExists(store, "contact_blocklist", "interception_msg_count");
    if (!isMsgCountExists) {
        store.ExecuteSql("ALTER TABLE contact_blocklist ADD COLUMN interception_msg_count "
                         "INTEGER NOT NULL DEFAULT 0;");
    }

    bool isNameExists = sqlAnalyzer.CheckColumnExists(store, "contact_blocklist", "name");
    if (!isNameExists) {
        store.ExecuteSql("ALTER TABLE contact_blocklist ADD COLUMN name "
                         "TEXT;");
    }

    // create index
    int ret = store.ExecuteSql(CREATE_CONTACT_BLOCKLIST_INDEX_PHONE);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("create block list index error, code:%{public}d", ret);
        int deleteRet = store.ExecuteSql(
            "DELETE FROM contact_blocklist WHERE contact_blocklist.rowid "
            "NOT IN (select MAX(contact_blocklist.rowid) from contact_blocklist group by phone_number, types);");
        HILOG_INFO("delete duplicate data, code:%{public}d", deleteRet);
        if (deleteRet == OHOS::NativeRdb::E_OK) {
            int createIndexRet = store.ExecuteSql(CREATE_CONTACT_BLOCKLIST_INDEX_PHONE);
            HILOG_INFO("create block list index again, code:%{public}d", createIndexRet);
        }
    }

    HILOG_INFO("Contacts UpgradeToV13 succeed.");
}

void SqliteOpenHelperContactCallback::UpgradeToV14(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV14 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    store.ExecuteSql(CREATE_HW_ACCOUNT);
}

void SqliteOpenHelperContactCallback::UpgradeToV15(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV15 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    store.ExecuteSql("DROP TRIGGER IF EXISTS update_contact_by_insert_contact_data;");
    store.ExecuteSql("DROP TRIGGER IF EXISTS update_contact_by_update_contact_data;");
    store.ExecuteSql(UPDATE_CONTACT_BY_INSERT_CONTACT_DATA);
    store.ExecuteSql(UPDATE_CONTACT_BY_UPDATE_CONTACT_DATA);
}

void SqliteOpenHelperContactCallback::UpgradeToV16(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV16 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    // update setting table
    SqlAnalyzer sqlAnalyzer;
    bool isCloneMarkExists = sqlAnalyzer.CheckColumnExists(store, "settings", "clone_time_stamp");
    if (!isCloneMarkExists) {
        store.ExecuteSql("ALTER TABLE settings ADD COLUMN clone_time_stamp INTEGER NOT NULL DEFAULT 0;");
    }
    bool isCalendarEventId = sqlAnalyzer.CheckColumnExists(store, "contact_data", "calendar_event_id");
    if (!isCalendarEventId) {
        store.ExecuteSql(CONTACT_DATA_ADD_CALENDAR_EVENT_ID);
    }
    // drop view
    store.ExecuteSql("DROP VIEW view_contact_data;");
    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    store.ExecuteSql("drop trigger if exists insert_delete_raw_contact;");
    HILOG_INFO("Contacts UpgradeToV16 succeed.");
}

void SqliteOpenHelperContactCallback::UpgradeToV17(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV17 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    // create table
    store.ExecuteSql(CREATE_CLOUD_CONTACT_BLOCKLIST);

    HILOG_INFO("Contacts UpgradeToV17 succeed.");
}

void SqliteOpenHelperContactCallback::UpgradeToV18(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV18 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    SqlAnalyzer sqlAnalyzer;
    bool isFormateNumberExists = sqlAnalyzer.CheckColumnExists(store, "contact_blocklist", "formate_phone_number");
    if (!isFormateNumberExists) {
        store.ExecuteSql("ALTER TABLE contact_blocklist ADD COLUMN format_phone_number TEXT;");
    }

    HILOG_INFO("Contacts UpgradeToV18 succeed.");
}

void SqliteOpenHelperContactCallback::UpgradeToV19(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV19 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    // contact_data的view新增一个字段sort_key
    // drop view
    store.ExecuteSql("DROP VIEW view_contact_data;");
    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    HILOG_INFO("Contacts UpgradeToV19 succeed.");
}

void SqliteOpenHelperContactCallback::UpgradeToV20(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV20 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    // update setting table
    SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "settings", "cloud_raw_contact_cursor");
    if (!isExists) {
        store.ExecuteSql("ALTER TABLE settings ADD COLUMN cloud_raw_contact_cursor INTEGER NOT NULL DEFAULT -1;");
    }
}

void SqliteOpenHelperContactCallback::UpgradeToV21(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV21 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    // update raw_contact table
    SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "unique_key");
    if (!isExists) {
        int addRet = store.ExecuteSql(RAW_CONTACT_ADD_UNIQUE_KEY);
        int idxRet = store.ExecuteSql(CREATE_RAW_CONTACT_INDEX_UNIQUE_KEY);
        HILOG_INFO("CREATE_RAW_CONTACT_INDEX_UNIQUE_KEY addRet %{public}d,idxRet %{public}d", addRet, idxRet);
    }
}

void SqliteOpenHelperContactCallback::UpgradeToV22(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV22 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    // update setting table
    SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "focus_mode_list");
    if (!isExists) {
        store.ExecuteSql(RAW_CONTACT_ADD_FOCUS_MODE_LIST);
    }
    // drop view
    store.ExecuteSql("DROP VIEW view_contact;");
    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT);
}

void SqliteOpenHelperContactCallback::UpgradeToV23(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV23 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    // drop view
    store.ExecuteSql("DROP VIEW view_contact_data;");
    // create view
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    // drop view
    store.ExecuteSql("DROP VIEW view_raw_contact;");
    // create view
    store.ExecuteSql(CREATE_VIEW_RAW_CONTACT);
}

void SqliteOpenHelperContactCallback::UpgradeToV24(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV24 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }

    // update setting table
    SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "form_id");
    if (!isExists) {
        store.ExecuteSql(RAW_CONTACT_ADD_FORM_ID);
    }
    store.ExecuteSql("drop view if exists view_raw_contact;");
    store.ExecuteSql(CREATE_VIEW_RAW_CONTACT);
}

void SqliteOpenHelperContactCallback::UpgradeToV25(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO("UpgradeToV25 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "form_id");
    if (!isExists) {
        store.ExecuteSql(RAW_CONTACT_ADD_FORM_ID);
    }
    isExists = sqlAnalyzer.CheckColumnExists(store, "settings", "cloud_raw_contact_cursor");
    if (!isExists) {
        store.ExecuteSql("ALTER TABLE settings ADD COLUMN cloud_raw_contact_cursor INTEGER NOT NULL DEFAULT -1;");
    }
    isExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "unique_key");
    if (!isExists) {
        int addRet = store.ExecuteSql(RAW_CONTACT_ADD_UNIQUE_KEY);
        int idxRet = store.ExecuteSql(CREATE_RAW_CONTACT_INDEX_UNIQUE_KEY);
        HILOG_INFO("CREATE_RAW_CONTACT_INDEX_UNIQUE_KEY addRet %{public}d,idxRet %{public}d", addRet, idxRet);
    }
    isExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "focus_mode_list");
    if (!isExists) {
        store.ExecuteSql(RAW_CONTACT_ADD_FOCUS_MODE_LIST);
    }
    store.ExecuteSql("drop view if exists view_raw_contact;");
    store.ExecuteSql(CREATE_VIEW_RAW_CONTACT);
    store.ExecuteSql("DROP VIEW view_contact;");
    store.ExecuteSql(CREATE_VIEW_CONTACT);
    store.ExecuteSql("DROP VIEW view_contact_data;");
    store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
}

int SqliteOpenHelperContactCallback::UpgradeToV26(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN(
        "ContactsDataBase UpgradeToV26 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }

    // update contact_data table
    SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "contact_data", "is_sync_birthday_to_calendar");
    int result = OHOS::NativeRdb::E_OK;
    if (!isExists) {
        // 添加生日是否同步到日历标识字段，默认值为0，代表未同步， 1代表已同步
        result = store.ExecuteSql(CONTACT_DATA_ADD_IS_SYNC_BIRTHDAY_TO_CALENDAR);
    }
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR(
            "ContactsDataBase UpgradeToV26 alter is_sync_birthday_to_calendar failed, result is %{public}d", result);
        return result;
    }
    // drop view
    result = store.ExecuteSql("drop view if exists view_contact_data;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV26 drop view_contact_data failed, result is %{public}d", result);
        return result;
    }
    // create view
    result = store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV26 create view_contact_data failed, result is %{public}d", result);
    }
    return result;
}

int SqliteOpenHelperContactCallback::DropAndCreateView(OHOS::NativeRdb::RdbStore &store)
{
    int result = store.ExecuteSql("DROP VIEW if exists view_raw_contact;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV27 drop view_raw_contact failed, result is %{public}d", result);
        return result;
    }
    result = store.ExecuteSql(CREATE_VIEW_RAW_CONTACT);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV27 add view_raw_contact failed, result is %{public}d", result);
        return result;
    }
    result = store.ExecuteSql("DROP VIEW if exists view_contact;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV27 drop view_contact failed, result is %{public}d", result);
        return result;
    }
    result = store.ExecuteSql(CREATE_VIEW_CONTACT);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV27 add view_contact failed, result is %{public}d", result);
        return result;
    }
    result = store.ExecuteSql("DROP VIEW if exists view_contact_data;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV27 drop view_contact_data failed, result is %{public}d", result);
        return result;
    }
    result = store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV27 add view_contact_data failed, result is %{public}d", result);
        return result;
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeToV28AddColumn(OHOS::NativeRdb::RdbStore &store)
{
	SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "contact", "form_id");
    int result = OHOS::NativeRdb::E_OK;
    if (!isExists) {
        result = store.ExecuteSql(CONTACT_ADD_FORM_ID);
    }
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV28 add form_id failed, result is %{public}d", result);
        return result;
    }
    isExists = sqlAnalyzer.CheckColumnExists(store, "contact", "focus_mode_list");
    if (!isExists) {
        result = store.ExecuteSql(CONTACT_ADD_FOCUS_MODE_LIST);
    }
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV28 add focus_mode_list failed, result is %{public}d", result);
        return result;
    }

    isExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "aggregation_status");
    if (!isExists) {
        // 增加aggregation_status字段
        result = store.ExecuteSql(RAW_CONTACT_ADD_AGGREGATION_STATUS);
    }

    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV28 add aggregation_status failed, result is %{public}d", result);
        return result;
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeToV27(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN(
        "ContactsDataBase UpgradeToV27 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }

    std::string querySql = "SELECT count(*) FROM sqlite_master WHERE type = 'table' and name = 'cloud_raw_contact'";
    auto resultSet = store.QuerySql(querySql);
    if (resultSet == nullptr) {
        HILOG_ERROR("UpgradeToV27 QuerySqlResult is null");
        return OHOS::NativeRdb::E_ERROR;
    }
    int resultSetNum = resultSet->GoToFirstRow();
    int result = OHOS::NativeRdb::E_OK;
    if (resultSetNum == OHOS::NativeRdb::E_OK) {
        int value = 0;
        resultSet->GetInt(0, value);
        if (value <= 0) {
            HILOG_ERROR("ContactsDataBase UpgradeToV27 value is %{public}d,try CREATE_CLOUD_RAW_CONTACT", value);
            result = store.ExecuteSql(CREATE_CLOUD_RAW_CONTACT);
        }
    }
    resultSet->Close();
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeToV28(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV28 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
    // 多sql语句场景数据库升级增加事务
    int ret = store.BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV28 BeginTransaction failed");
        return ret;
    }

    int result = OHOS::NativeRdb::E_OK;
    // 封装UpgradeToV28升级需要新增的字段
    result = UpgradeToV28AddColumn(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV28 UpgradeToV28AddColumn failed");
        store.RollBack();
        return result;
    }

    // 封装drop和craete三个view的方法
    result = DropAndCreateView(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV28 DropAndCreateView failed");
        store.RollBack();
        return result;
    }
    
    // 事务提交以及回滚
    ret = store.Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        store.RollBack();
        HILOG_ERROR("UpgradeToV28 add column and drop view failed and rollback");
    }
    return ret;
    // 历史数据迁移放在 js 侧进程冷启动 checkCursor 中处理
}

int SqliteOpenHelperContactCallback::UpgradeToV29(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV29 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
	SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "raw_contact", "focus_mode_list");
    int result = OHOS::NativeRdb::E_OK;
    if (!isExists) {
        // 为了兼容新老版本之间克隆，加回老版本删除的 focus_mode_list 字段
        result = store.ExecuteSql(RAW_CONTACT_ADD_FOCUS_MODE_LIST);
    }
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV29 add focus_mode_list failed, result is %{public}d", result);
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeToV30(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV30 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }

    // 多sql语句场景数据库升级增加事务
    int ret = store.BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV30 BeginTransaction failed");
        return ret;
    }
    
    int result1 = store.ExecuteSql("update raw_contact set dirty = 2 where dirty = 1");
    if (result1 != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV30 update dirty 2, result1 is %{public}d", result1);
        store.RollBack();
        return result1;
    }
    int result2 =
        store.ExecuteSql("update raw_contact set dirty = 3 where dirty != 2");
    if (result2 != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV30 update dirty 3, result2 is %{public}d", result2);
        store.RollBack();
        return result2;
    }

    // 事务提交以及回滚
    ret = store.Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        store.RollBack();
        HILOG_ERROR("UpgradeToV30 update failed and rollback");
    }

    return ret;
}

int SqliteOpenHelperContactCallback::UpgradeToV31(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN(
        "ContactsDataBase UpgradeToV31 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }

    // 多sql语句场景数据库升级增加事务
    int ret = store.BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV31 BeginTransaction failed");
        return ret;
    }
 
    int result = store.ExecuteSql(
        "update raw_contact set contacted_count = 0 where contacted_count = '' or contacted_count is null;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV31 update contacted_count failed, result is %{public}d", result);
    }
 
    result = store.ExecuteSql("update raw_contact set lastest_contacted_time = 0 "
                              "where lastest_contacted_time = '' or lastest_contacted_time is null;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV31 update lastest_contacted_time, result is %{public}d", result);
    }

    // 事务提交以及回滚
    ret = store.Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        store.RollBack();
        HILOG_ERROR("UpgradeToV31 update failed and rollback");
        return ret;
    }

    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperContactCallback::UpgradeToV32(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV32 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }

    // 多sql语句场景数据库升级增加事务
    int ret = store.BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV32 BeginTransaction failed");
        return ret;
    }

    // 老的算法，无姓名联系人 uniqueKey 长度均小于22，新的算法上线后，需要针对无姓名联系人重算 uniqueKey
    int result1 = store.ExecuteSql("update raw_contact set dirty = 2 where dirty = 1 and LENGTH(unique_key) < 22;");
    if (result1 != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV30 update dirty 2, result1 is %{public}d", result1);
        store.RollBack();
        return result1;
    }
    int result2 = store.ExecuteSql("update raw_contact set dirty = 3 where dirty != 2 and LENGTH(unique_key) < 22;");
    if (result2 != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV30 update dirty 3, result2 is %{public}d", result2);
        store.RollBack();
        return result2;
    }

    // 事务提交以及回滚
    ret = store.Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        store.RollBack();
        HILOG_ERROR("UpgradeToV32 update failed and rollback");
    }

    return ret;
}

int SqliteOpenHelperContactCallback::UpgradeToV33(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV33 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }

    // 多sql语句场景数据库升级增加事务
    int storeRet = store.BeginTransaction();
    if (storeRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV33 BeginTransaction failed");
        return storeRet;
    }

    int result = store.ExecuteSql("DROP INDEX IF EXISTS [contact_blocklist_phone_index];");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV33 drop block list index error, result1 is %{public}d", result);
        store.RollBack();
        return result;
    }
    
    int ret = store.ExecuteSql(CREATE_CONTACT_BLOCKLIST_INDEX_PHONE);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("create block list index error, code:%{public}d", ret);
        int deleteRet = store.ExecuteSql("DELETE FROM contact_blocklist WHERE contact_blocklist.rowid "
         "NOT IN (select MAX(contact_blocklist.rowid) from contact_blocklist group by phone_number, types);");
        HILOG_INFO("delete duplicate data, code:%{public}d", deleteRet);
        if (deleteRet == OHOS::NativeRdb::E_OK) {
            int createIndexRet = store.ExecuteSql(CREATE_CONTACT_BLOCKLIST_INDEX_PHONE);
            HILOG_INFO("create block list index again, code:%{public}d", createIndexRet);
        }
    }

    // 事务提交以及回滚
    storeRet = store.Commit();
    if (storeRet != OHOS::NativeRdb::E_OK) {
        store.RollBack();
        HILOG_ERROR("UpgradeToV33 create and drop failed and rollback");
        return storeRet;
    }

    return result;
}

int SqliteOpenHelperContactCallback::UpgradeToV34(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV34 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
    int result = BeginTransaction(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV34 BeginTransaction failed, ret:%{public}d", result);
        return result;
    }
 
    result = store.ExecuteSql("DROP VIEW IF EXISTS view_contact_data;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV34 DROP VIEW error, result1 is %{public}d", result);
        RollBack(store);
        return result;
    }
    
    result = store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV34 create view view_contact_data error, code:%{public}d", result);
        RollBack(store);
        return result;
    }
    SqlAnalyzer sqlAnalyzer;
    // 主空间数据被迁移走到隐私空间，或从隐私空间迁入
    // 主空间的c++测处理，无法拉起js测处理，所以云同步，删除日历，新增日历，删除大头像等无法处理
    // 记录标志位，在联系人应用启动后，做一次全量同步处理
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "settings", "main_space_need_sync_flag");
    if (!isExists) {
        store.ExecuteSql("ALTER TABLE settings ADD COLUMN main_space_need_sync_flag INTEGER NOT NULL DEFAULT 0;");
        if (result != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("UpgradeToV34 add col main_space_need_sync_flag error, code:%{public}d", result);
            RollBack(store);
            return result;
        }
    }

    result = Commit(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV34 Commit failed, ret:%{public}d", result);
        RollBack(store);
        return result;
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeToV35(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV35 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
    SqlAnalyzer sqlAnalyzer;
    int result = OHOS::NativeRdb::E_OK;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "settings", "blocklist_migrate_status");
    if (!isExists) {
        result = store.ExecuteSql(
            "ALTER TABLE settings ADD COLUMN blocklist_migrate_status INTEGER NOT NULL DEFAULT 0;");
    }
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV35 add blocklist_migrate_status failed, result is %{public}d", result);
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeToV36(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV36 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
    int result = BeginTransaction(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV36 BeginTransaction failed, ret:%{public}d", result);
        return result;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "contact", "ringtone_path");
    if (!isExists) {
        result = store.ExecuteSql(CONTACT_ADD_RINGTONE_PATH);
    }
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV36 add ringtone_path failed, result is %{public}d", result);
        RollBack(store);
        return result;
    }
    result = store.ExecuteSql("DROP VIEW IF EXISTS view_contact_data;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV36 DROP view_contact_data error, result is %{public}d", result);
        RollBack(store);
        return result;
    }
    result = store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV36 create view_contact_data error, code:%{public}d", result);
        RollBack(store);
        return result;
    }
    result = Commit(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV36 Commit failed, ret:%{public}d", result);
        RollBack(store);
    }
    return result;
}

int SqliteOpenHelperContactCallback::UpgradeToV37(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV37 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }

    // 为了 5.1 商分增加一次升级，用新的 hash 算法重算 5.0 商分历史联系人的 uniqueKey
    int ret = store.BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV37 BeginTransaction failed");
        return ret;
    }

    int result1 = store.ExecuteSql("update raw_contact set dirty = 2 where dirty = 1");
    if (result1 != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV37 update dirty 2, result1 is %{public}d", result1);
        store.RollBack();
        return result1;
    }
    int result2 = store.ExecuteSql("update raw_contact set dirty = 3 where dirty != 2");
    if (result2 != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV37 update dirty 3, result2 is %{public}d", result2);
        store.RollBack();
        return result2;
    }

    // 事务提交以及回滚
    ret = store.Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        store.RollBack();
        HILOG_ERROR("UpgradeToV37 update failed and rollback");
    }
    return ret;
}

int SqliteOpenHelperContactCallback::UpgradeToV38(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV38 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    // 5.0版本升级到6.0版本，防止重复刷新
    std::string queryRefreshContacts = "SELECT refresh_contacts FROM settings";
    auto resultSet = store.QuerySql(queryRefreshContacts);
    if (resultSet == nullptr) {
        HILOG_ERROR("UpgradeToV38 settins error, ts = %{public}lld", (long long) time(NULL));
        return OHOS::NativeRdb::E_ERROR;
    }
    int getRowResult = resultSet->GoToFirstRow();
    std::string column_refresh_contacts = SettingsColumns::REFRESH_CONTACTS;
    int refreshContacts = 0;
    if (getRowResult != OHOS::NativeRdb::E_OK) {
        resultSet->Close();
        return OHOS::NativeRdb::E_ERROR;
    }
    int columnIndex = 0;
    resultSet->GetColumnIndex(column_refresh_contacts, columnIndex);
    resultSet->GetInt(columnIndex, refreshContacts);
    resultSet->Close();
    HILOG_WARN("UpgradeToV38 settins refreshContacts = %{public}d", refreshContacts);
    if (refreshContacts > 29) {
        return OHOS::NativeRdb::E_OK;
    }

    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
    // 单升单收藏排序按姓名拼音排序对order做一次刷新（保持单升级单后体验一致）
    int ret = store.BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV38 BeginTransaction failed");
        return ret;
    }
    UpdateFavoriteOrder(store);
    // 事务提交以及回滚
    ret = store.Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        store.RollBack();
        HILOG_ERROR("UpgradeToV38 update failed and rollback");
    }
    return ret;
}

int SqliteOpenHelperContactCallback::UpgradeToV39(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV39 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
    int result = store.ExecuteSql(CREATE_PRIVACY_CONTACTS_BACKUP);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsDataBase UpgradeToV39 failed , result is %{public}d", result);
        return result;
    }
    std::thread thread([]() { PrivacyContactsManager::GetInstance()->SyncPrivacyContactsBackup(); });
    thread.detach();
    return result;
}


int SqliteOpenHelperContactCallback::UpgradeToV40(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV40 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
    
    if (BeginTransaction(store) != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV40 BeginTransaction failed");
        return OHOS::NativeRdb::E_ERROR;
    }
    
    SqlAnalyzer sqlAnalyzer;
    if (!sqlAnalyzer.CheckColumnExists(store, "contact_data", "location")) {
        if (!ExecuteAndCheck(store, CONTACT_DATA_ADD_LOCATION)) {
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    
    if (!sqlAnalyzer.CheckColumnExists(store, "settings", "refresh_location")) {
        if (!ExecuteAndCheck(store, "ALTER TABLE settings ADD COLUMN refresh_location INTEGER DEFAULT 0;")) {
            return OHOS::NativeRdb::E_ERROR;
        }
    }
    
    if (!ExecuteAndCheck(store, "DROP VIEW IF EXISTS view_contact_data;") ||
        !ExecuteAndCheck(store, CREATE_VIEW_CONTACT_DATA) ||
        !ExecuteAndCheck(store, "DROP VIEW IF EXISTS view_contact_location;") ||
        !ExecuteAndCheck(store, CREATE_VIEW_CONTACT_LOCATION)) {
        return OHOS::NativeRdb::E_ERROR;
    }

    ExecuteAndCheck(store, CREATE_RAW_INDEX);
    ExecuteAndCheck(store, CREATE_DATA_INDEX);
    ExecuteAndCheck(store, CREATE_LOCATION_INDEX);
    
    if (Commit(store) != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV40 Commit failed");
        RollBack(store);
        return OHOS::NativeRdb::E_ERROR;
    }
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperContactCallback::UpgradeToV41(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV41 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
 
    if (BeginTransaction(store) != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV41 BeginTransaction failed");
        return OHOS::NativeRdb::E_ERROR;
    }
 
    SqlAnalyzer sqlAnalyzer;
    int result = OHOS::NativeRdb::E_OK;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, ContactTableName::SETTINGS, "cloud_sync_delete_flag");
    if (!isExists) {
        result =
            store.ExecuteSql("ALTER TABLE settings ADD COLUMN cloud_sync_delete_flag INTEGER NOT NULL DEFAULT 0;");
    }
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV41 add cloud_sync_delete_flag error, code:%{public}d", result);
        RollBack(store);
        return result;
    }
    result = Commit(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV41 Commit failed, ret:%{public}d", result);
        RollBack(store);
        return result;
    }
    return OHOS::NativeRdb::E_OK;
}

int SqliteOpenHelperContactCallback::UpgradeToV42(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_WARN("UpgradeToV42 oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion >= newVersion) {
        return OHOS::NativeRdb::E_OK;
    }
    int result = BeginTransaction(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV42 BeginTransaction failed, ret:%{public}d", result);
        return result;
    }
    SqlAnalyzer sqlAnalyzer;
    bool isExists = sqlAnalyzer.CheckColumnExists(store, "contact", "prefer_avatar");
    if (!isExists) {
        result = store.ExecuteSql("ALTER TABLE contact ADD COLUMN prefer_avatar INTEGER;");
    }
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV42 and prefer_avatar failed , result is %{public}d", result);
        RollBack(store);
        return result;
    }
    result = store.ExecuteSql(CREATE_POSTER);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV42 create poster failed , result is %{public}d", result);
        RollBack(store);
        return result;
    }
    result = store.ExecuteSql("DROP VIEW IF EXISTS view_contact_data;");
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV42 DROP view_contact_data error, result is %{public}d", result);
        RollBack(store);
        return result;
    }
    result = store.ExecuteSql(CREATE_VIEW_CONTACT_DATA);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV42 create view_contact_data error, code:%{public}d", result);
        RollBack(store);
        return result;
    }
    result = Commit(store);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV42 Commit failed, ret:%{public}d", result);
        RollBack(store);
    }
    return result;
}

bool SqliteOpenHelperContactCallback::ExecuteAndCheck(OHOS::NativeRdb::RdbStore &store, const std::string &sql)
{
    int result = store.ExecuteSql(sql);
    if (result != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpgradeToV40 SQL execution failed: %{public}s", sql.c_str());
        RollBack(store);
        return false;
    }
    return true;
}

void SqliteOpenHelperContactCallback::UpdateFavoriteOrder(OHOS::NativeRdb::RdbStore &store)
{
    std::string sql = "select id from raw_contact where favorite = 1 order by sort,sort_key asc";
    auto resultSet = store.QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("UpdateFavoriteOrder QuerySqlResult is null");
        return;
    }
    int favoriteCount;
    resultSet->GetRowCount(favoriteCount);
    HILOG_INFO("ContactsDataBase UpdateFavoriteOrder favoriteCount = %{public}d", favoriteCount);
    if (favoriteCount == 0) {
        HILOG_INFO("ContactsDataBase UpdateFavoriteOrder no favorite contact need update.");
        resultSet->Close();
        return;
    }
    int result = resultSet->GoToFirstRow();
    int favoriteOrder = 1;
    while (result == OHOS::NativeRdb::E_OK) {
        std::string columnName = RawContactColumns::ID;
        int columnIndex = 0;
        std::string rawContactId;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetString(columnIndex, rawContactId);
        OHOS::NativeRdb::ValuesBucket values;
        values.PutInt(RawContactColumns::FAVORITE_ORDER, favoriteOrder);
        int updateRow = OHOS::NativeRdb::E_OK;
        std::string upWhere = "";
        upWhere.append(RawContactColumns::ID);
        upWhere.append(" = ? ");
        std::vector<std::string> upWhereArgs;
        upWhereArgs.push_back(rawContactId);
        int ret = store.Update(updateRow, ContactTableName::RAW_CONTACT, values, upWhere, upWhereArgs);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("ContactsDataBase UpdateFavoriteOrder error, ts = %{public}lld", (long long) time(NULL));
        }
        result = resultSet->GoToNextRow();
        favoriteOrder++;
    }
    resultSet->Close();
}

/**
 * @brief update format_phone_number by phoneNumber.
 *
 * @param store store for operation database.
 */
void SqliteOpenHelperContactCallback::UpdateFormatPhoneNumber(OHOS::NativeRdb::RdbStore &store)
{
    std::string sql = "select id, detail_info from contact_data where type_id = 5";
    auto resultSet = store.QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("UpdateFormatPhoneNumber QuerySqlResult is null");
        return;
    }
    int displayResult;
    resultSet->GetRowCount(displayResult);
    HILOG_INFO("ContactsDataBase UpdateFormatPhoneNumber sql = %{public}d", displayResult);
    int result = resultSet->GoToFirstRow();
    HILOG_INFO("ContactsDataBase UpdateFormatPhoneNumber result = %{public}d", result);
    std::string systemRegion = Global::I18n::LocaleConfig::GetSystemRegion();
    HILOG_INFO("ContactsDataBase UpdateFormatPhoneNumber systemRegion  %{public}s", systemRegion.c_str());
    while (result == OHOS::NativeRdb::E_OK) {
        std::string columnName = ContactDataColumns::DETAIL_INFO;
        int columnIndex = 0;
        std::string phoneNumber;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetString(columnIndex, phoneNumber);
        bool isValidPhoneNumber = PhoneNumberUtils::IsValidPhoneNumber(phoneNumber, systemRegion);
        if (!isValidPhoneNumber) {
            result = resultSet->GoToNextRow();
            continue;
        }
        std::string formatPhoneNumber = PhoneNumberUtils::Format(phoneNumber, systemRegion);
        if (formatPhoneNumber.empty()) {
            result = resultSet->GoToNextRow();
            continue;
        }
        std::string columnNameId = ContactDataColumns::ID;
        int columnIndexId = 0;
        int id = -1;
        resultSet->GetColumnIndex(columnNameId, columnIndexId);
        resultSet->GetInt(columnIndexId, id);
        OHOS::NativeRdb::ValuesBucket values;
        values.PutString(ContactDataColumns::FORMAT_PHONE_NUMBER, formatPhoneNumber);
        int updateRow = OHOS::NativeRdb::E_OK;
        std::string upWhere = "";
        upWhere.append(ContactDataColumns::ID);
        upWhere.append(" = ? ");
        std::vector<std::string> upWhereArgs;
        upWhereArgs.push_back(std::to_string(id));
        int ret = store.Update(updateRow, ContactTableName::CONTACT_DATA, values, upWhere, upWhereArgs);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("ContactsDataBase UpdateFormatPhoneNumber error, ts = %{public}lld", (long long) time(NULL));
        }
        result = resultSet->GoToNextRow();
    }
    resultSet->Close();
}

/**
 * @brief Fill in records with empty location information，Prevent invalid fill.
 *
 * @param store store for operation database.
 */
void ContactsDataBase::NumberLocationRefresh()
{
    // 先获取setting表的refresh_location字段
    if (store_ == nullptr) {
        HILOG_ERROR("store is null, ts = %{public}lld", (long long) time(NULL));
        return;
    }
    std::string queryRefreshContacts = "SELECT refresh_location, id FROM settings";
    auto resultSet = store_->QuerySql(queryRefreshContacts);
    if (resultSet == nullptr) {
        HILOG_ERROR("query settins error, ts = %{public}lld", (long long) time(NULL));
        return;
    }
    int getRowResult = resultSet->GoToFirstRow();
    std::string column_refresh_location = SettingsColumns::REFRESH_LOCATION;
    std::string column_id = SettingsColumns::ID;
    int refreshLocationNum = 0;
    int idNum = 0;
    if (getRowResult == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        resultSet->GetColumnIndex(column_refresh_location, columnIndex);
        resultSet->GetInt(columnIndex, refreshLocationNum);
        int idIndex = 0;
        resultSet->GetColumnIndex(column_id, idIndex);
        resultSet->GetInt(idIndex, idNum);
    }
    resultSet->Close();
    if (refreshLocationNum == 1) {
        return;
    }

    bool retryStatus = true;
    for (int i = 0; i < REFRESH_LOCATION_RETRY_NUMBER; i++) { //  失败情况重试两次
        retryStatus = FillingHistoryNumberLocation();
        if (retryStatus) {
            break;
        }
    }
    // 更新setting表的refresh_location字段
    if (retryStatus) {
        OHOS::NativeRdb::ValuesBucket values;
        values.PutInt( SettingsColumns::REFRESH_LOCATION, 1);
        int updateRow = OHOS::NativeRdb::E_OK;
        std::string upWhere = "";
        upWhere.append(SettingsColumns::ID);
        upWhere.append(" = ? ");
        std::vector<std::string> upWhereArgs;
        upWhereArgs.push_back(std::to_string(idNum));
        int re = store_->Update(updateRow, ContactTableName::SETTINGS, values, upWhere, upWhereArgs);
        if (re != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("ContactsDataBase NumberLocationRefresh error, ts = %{public}lld", (long long) time(NULL));
        }
    }
}

/**
 * @brief Fill in records with empty location information.
 *
 * @param store store for operation database.
 */
bool ContactsDataBase::FillingHistoryNumberLocation()
{
    bool fillStatu = true; // 是否刷新归属地成功状态
    std::string sql = "select id, detail_info from contact_data where type_id = 5 and location is null";
    auto resultSet = store_->QuerySql(sql);
    if (resultSet == nullptr) {
        return false;
    }
    int displayResult;
    resultSet->GetRowCount(displayResult);

    // 有数据都已经刷新则不需要再刷新了
    if (displayResult == 0 ) {
        std::string typeSql = "select id, detail_info from contact_data where type_id = 5 limit 1";
        auto typeResult = store_->QuerySql(typeSql);
        if (typeResult == nullptr) {
            resultSet->Close();
            return false;
        }

        if (typeResult->GoToFirstRow() != OHOS::NativeRdb::E_OK) {
            resultSet->Close();
            typeResult->Close();
            return false;
        }
        typeResult->Close();
    }

    while (resultSet->GoToNextRow() == OHOS::NativeRdb::E_OK) {
        int columnIndex = 0;
        std::string number;
        resultSet->GetColumnIndex(ContactDataColumns::DETAIL_INFO, columnIndex);
        resultSet->GetString(columnIndex, number);
        std::string numberLocation = "";
        bool ret = QueryNumberLocation(number, numberLocation);
        if (!ret) {
            fillStatu = false;
        } else {
            int columnIndexId = 0;
            int id = -1;
            resultSet->GetColumnIndex( ContactDataColumns::ID, columnIndexId);
            resultSet->GetInt(columnIndexId, id);
            if (!UpdateLocation(id, numberLocation)) {
                fillStatu = false;
            }
        }
    }
    resultSet->Close();
    return fillStatu;
}

bool ContactsDataBase::QueryNumberLocation(const std::string &number, std::string &numberLocation)
{
    std::shared_ptr<NumberIdentityHelper> numberIdentityHelper = NumberIdentityHelper::GetInstance();
    if (numberIdentityHelper == nullptr) {
        HILOG_ERROR("numberIdentityHelper is nullptr!");
        return false;
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> phoneNumbers;
    phoneNumbers.push_back(number);
    predicates.SetWhereArgs(phoneNumbers);
    bool ret = numberIdentityHelper->Query(numberLocation, predicates);
    if (!ret) {
        HILOG_ERROR("Query number location database fail!");
        return false;
    }
    return true;
}

bool ContactsDataBase::UpdateLocation(int id, const std::string &numberLocation)
{
    if (store_ == nullptr) {
         HILOG_ERROR("UpdateLocation store_ is null, ts = %{public}lld", (long long) time(NULL));
        return false;
    }
    int updateRow = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::ValuesBucket values;
    values.PutString(ContactDataColumns::LOCATION, numberLocation);
    std::string upWhere = "";
    upWhere.append(ContactDataColumns::ID);
    upWhere.append(" = ? ");
    std::vector<std::string> upWhereArgs;
    upWhereArgs.push_back(std::to_string(id));
    int re = store_->Update(updateRow, ContactTableName::CONTACT_DATA, values, upWhere, upWhereArgs);
    if (re != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateLocation FillingHistoryNumberLocation error, ts = %{public}lld", (long long) time(NULL));
        return false;
    }
    return true;
}

void SqliteOpenHelperContactCallback::UpdateSrotInfoByDisplayName(OHOS::NativeRdb::RdbStore &store)
{
    std::string sql = "SELECT ";
    sql.append(ContactPublicColumns::ID)
        .append(" , ")
        .append(RawContactColumns::DISPLAY_NAME)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT);

    auto rawIdsSet = store.QuerySql(sql);
    // id集合，displayName集合
    std::vector<int> rawIds;
    std::vector<std::string> displayNameList;
    int resultSetNum = rawIdsSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int id = 0;
        std::string displayName = "";
        rawIdsSet->GetInt(0, id);
        rawIdsSet->GetString(1, displayName);
        rawIds.push_back(id);
        displayNameList.push_back(displayName);
        resultSetNum = rawIdsSet->GoToNextRow();
    }
    rawIdsSet->Close();

    HILOG_INFO("UpgradeToV6 update %{public}s records!", std::to_string(rawIds.size()).c_str());

    CharacterTransliterate characterTransliterate;
    for (unsigned int i = 0; i < rawIds.size(); ++i) {
        int id = rawIds[i];
        std::string displayName = displayNameList[i];
        if (!displayName.empty()) {
            // 排序key，排序首字母，search_contact 信息，等，都需要更新
            // 可能存在老数据 广安银行， 广 是在A下边，需要改成guang
            ConstructionName name;
            name.GetConstructionName(displayName, name);
            // 更新sort信息
            UpdateSortInfo(store, name, id);
        } else {
            // 没有名字，无名氏，标记排序首字母三个点，排序sort 99
            UpdateAnonymousSortInfo(store, id);
        }
    }
}

void SqliteOpenHelperContactCallback::UpdateSortInfo(OHOS::NativeRdb::RdbStore &store, ConstructionName &name, int id)
{
    std::string sortKey = name.sortKey;

    // 更新记录
    if (!name.sortFirstLetter_.empty() || !sortKey.empty()) {
        std::string updateSql = "";
        updateSql.append("update ").append(ContactTableName::RAW_CONTACT).append(" set ");
        if (!sortKey.empty()) {
            updateSql.append(RawContactColumns::SORT_KEY)
                .append(" = ")
                .append("'")
                .append(sortKey)
                .append("'")
                .append(",");
        }
        if (!name.sortFirstLetter_.empty()) {
            updateSql.append(RawContactColumns::SORT_FIRST_LETTER)
                .append(" = ")
                .append("'")
                .append(name.sortFirstLetter_)
                .append("'")
                .append(",");
            updateSql.append(RawContactColumns::SORT)
                .append(" = ")
                .append("'")
                .append(std::to_string(name.sortFirstLetterCode_))
                .append("'")
                .append(",");
        }
        updateSql.erase(updateSql.length() - 1);
        updateSql.append(" where id = ").append(std::to_string(id)).append(" ;");
        store.ExecuteSql(updateSql);
    }
}

void SqliteOpenHelperContactCallback::UpdateAnonymousSortInfo(OHOS::NativeRdb::RdbStore &store, int id)
{
    std::string sql = "";
    sql.append("update ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" set ")
        .append(RawContactColumns::SORT_FIRST_LETTER)
        .append(" = ")
        .append("'")
        .append(ANONYMOUS_SORT_FIRST_LETTER)
        .append("'")
        .append(", ")
        .append(RawContactColumns::SORT)
        .append(" = ")
        .append("'")
        .append(ANONYMOUS_SORT)
        .append("'")
        .append(" where id = ")
        .append(std::to_string(id))
        .append(" ;");
    store.ExecuteSql(sql);
}

int SqliteOpenHelperContactCallback::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    HILOG_INFO(
        "ContactsDataBase OnDowngrade oldVersion is %{public}d , newVersion is %{public}d", oldVersion, newVersion);
    if (oldVersion > newVersion && newVersion == DATABASE_CONTACTS_OPEN_VERSION) {
        store.ExecuteSql(
            "CREATE TABLE IF NOT EXISTS database_backup (id INTEGER PRIMARY KEY AUTOINCREMENT, backup_time "
            "TEXT, backup_path TEXT, remarks TEXT)");
        store.ExecuteSql("INSERT INTO database_backup(id, backup_time, backup_path, remarks) SELECT id, "
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
        HILOG_ERROR("Restore file %{private}s does not exist, ts = %{public}lld", restorePath.c_str(), (long long) time(NULL));
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
std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsDataBase::SelectCandidate()
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

// 默认影响通话记录
void ContactsDataBase::MergeUpdateTask(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store,
    std::vector<int> &rawContactIdVector, bool isDeleted, bool isAffectsCalllog)
{
    // 是否影响到通话记录，如果不影响，不需要处理通话记录；
    // 如删除联系人contact_data，但删除信息里不带电话号码和名称（畅连删除设备信息场景）,不影响通话记录
    // 默认为true，即默认影响
    if (isAffectsCalllog == false) {
        HILOG_INFO("isAffectsCalllog false, MergeUpdateTask return, ts = %{public}lld", (long long) time(NULL));
        return;
    }
    if (rawContactIdVector.size() == 0) {
        HILOG_INFO("rawIdSize is 0, MergeUpdateTask return, ts = %{public}lld", (long long) time(NULL));
        return;
    }

    std::set<int> s(rawContactIdVector.begin(), rawContactIdVector.end());
    std::vector<int> noRepeatRawContactIdVector(s.begin(), s.end());
    HILOG_INFO("MergeUpdateTask start, isDelete = %{public}s, rawIdSize = %{public}ld, "
        "isAffectsCalllog = %{public}s, "
        "ts = %{public}lld",
        std::to_string(isDeleted).c_str(),
        (long) noRepeatRawContactIdVector.size(),
        std::to_string(isAffectsCalllog).c_str(),
        (long long) time(NULL));
    std::unique_ptr<AsyncItem> task =
        std::make_unique<AsyncTask>(store_, noRepeatRawContactIdVector, isDeleted);
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
    auto resultSet = store->QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("MarkMerge QuerySqlResult is null");
        return;
    }
    int mergeResultSetNum = resultSet->GoToFirstRow();
    MatchCandidate matchCandidate;
    while (mergeResultSetNum == OHOS::NativeRdb::E_OK) {
        std::string columnName = MergeInfo::RAW_CONTACT_ID;
        int columnIndex = 0;
        int rawContactId = 0;
        resultSet->GetColumnIndex(columnName, columnIndex);
        resultSet->GetInt(columnIndex, rawContactId);
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
    return typeId;
}

void ContactsDataBase::NotifyContactChange()
{
    HILOG_INFO("Contacts NotifyContactChange start. AsyncDataShareProxyTask");
    if (dataShareHelper_ == nullptr) {
        g_mutex.lock();
        if (dataShareHelper_ == nullptr) {
            DataShare::CreateOptions options;
            options.enabled_ = true;
            dataShareHelper_ = DataShare::DataShareHelper::Creator(SETTING_URI, options);
        }
        g_mutex.unlock();
    }
    Uri proxyUri(SETTING_URI);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", 1);
    DataShare::DataShareValuesBucket valuesBucket;
    valuesBucket.Put("contact_change_time", time(NULL));
    std::unique_ptr<AsyncItem> task =
        std::make_unique<AsyncDataShareProxyTask>(dataShareHelper_, valuesBucket, predicates, proxyUri);
    g_asyncTaskQueue->Push(task);
    g_asyncTaskQueue->Start();
}

int ContactsDataBase::UpdateRawContactSeq(const OHOS::NativeRdb::ValuesBucket &values)
{
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsDataBase UpdateRawContactSeq store_ is nullptr");
        return RDB_EXECUTE_FAIL;
    }
    ContactsJsonUtils contactsJsonUtils;
    int importContactNum = contactsJsonUtils.getIntValueFromRdbBucket(values, "importContactNum");
    if (importContactNum == OPERATION_ERROR || importContactNum <= 0) {
        HILOG_ERROR("getIntValueFromRdbBucket err,importContactNum:%{public}d", importContactNum);
        return RDB_EXECUTE_FAIL;
    }
    int ret = BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateRawContactSeq BeginTransaction failed, ret:%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    auto resultSet = store_->QuerySql("select seq from sqlite_sequence where name='raw_contact';");
    if (resultSet == nullptr) {
        return RDB_EXECUTE_FAIL;
    }
    ret = resultSet->GoToFirstRow();
    if (ret != OHOS::NativeRdb::E_OK) {
        resultSet->Close();
        RollBack();
        HILOG_ERROR("query seq failed, ret:%{public}d", ret);
        // 此时表没数据，可以直接插数据
        return 0;
    }
    int seq = 0;
    resultSet->GetInt(0, seq);
    resultSet->Close();
    std::string placeHoldId = std::to_string(seq + importContactNum);
    ret = store_->ExecuteSql("update sqlite_sequence set seq = " + placeHoldId + " where name = 'raw_contact';");
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("update seq error : %{public}s", placeHoldId.c_str());
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    ret = Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateRawContactSeq Commit failed, ret:%{public}d", ret);
        RollBack();
        return RDB_EXECUTE_FAIL;
    }
    HILOG_WARN("UpdateRawContactSeq succeed seq:%{public}d,importContactNum:%{public}d,placeHoldId:%{public}s",
        seq,
        importContactNum,
        placeHoldId.c_str());
    return seq;
}

void ContactsDataBase::ReplaceAccountId(OHOS::NativeRdb::ValuesBucket &rawContactValues)
{
    OHOS::NativeRdb::ValueObject accountIdValue;
    if (rawContactValues.GetObject(RawContactColumns::ACCOUNT_ID, accountIdValue)) {
        int accountId = -1;
        if (accountIdValue.GetInt(accountId); accountId == -1) {
            HILOG_WARN("The accountId is -1, need to look up from accout.");
            rawContactValues.Put(RawContactColumns::ACCOUNT_ID, AccountManager().GetAccount());
        }
    } else {
        rawContactValues.PutInt(RawContactColumns::ACCOUNT_ID, AccountManager().GetAccount());
    }
}

std::vector<OHOS::NativeRdb::ValueObject> ContactsDataBase::GetUuidsWithAssetsSynced()
{
    OHOS::NativeRdb::AbsRdbPredicates predicate(ContactTableName::CLOUD_RAW_CONTACT);
    predicate.IsNotNull(CloudRawContactColumns::UUID);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        store_->Query(predicate, {CloudRawContactColumns::UUID, CloudRawContactColumns::ATTACHMENTS});
    if (resultSet == nullptr) {
        HILOG_ERROR("GetUuidsWithAssetsSynced QuerySqlResult is null");
        return {};
    }
    std::vector<OHOS::NativeRdb::ValueObject> uuids{};
    int rowRecord = resultSet->GoToFirstRow();
    while (rowRecord == OHOS::NativeRdb::E_OK) {
        std::string cloudId;
        OHOS::NativeRdb::ValueObject::Assets assets;
        int columnIndex = 0;
        resultSet->GetColumnIndex(CloudRawContactColumns::UUID, columnIndex);
        resultSet->GetString(columnIndex, cloudId);
        resultSet->GetColumnIndex(CloudRawContactColumns::ATTACHMENTS, columnIndex);
        resultSet->GetAssets(columnIndex, assets);
        if (assets.size() > 0 && assets[0].status != ASSET_NORMAL) {
            rowRecord = resultSet->GoToNextRow();
            continue;
        }
        uuids.push_back(OHOS::NativeRdb::ValueObject(cloudId));
        rowRecord = resultSet->GoToNextRow();
    }
    resultSet->Close();
    HILOG_INFO("GetUuidsWithAssetsSynced end, uuids size:%{public}ld.", (long) uuids.size());
    return uuids;
}

void ContactsDataBase::SyncContactsWithoutSpace()
{
    HILOG_INFO("SyncContactsWithoutSpace start.");
    std::vector<OHOS::NativeRdb::ValueObject> uuids = GetUuidsWithAssetsSynced();

    // 设置头像支持同步下载（异步资产下载不支持谓词过滤)
    distributedConfig.asyncDownloadAsset = false;
    store_->SetDistributedTables({ContactTableName::CLOUD_RAW_CONTACT}, g_mSyncMode, distributedConfig);
    OHOS::DistributedRdb::SyncOption option = {OHOS::DistributedRdb::TIME_FIRST, false};
    int totalSyncNums = std::ceil(static_cast<double>(uuids.size()) / CLOUD_SYNC_SIZE);

    auto detailsCallback = [this, totalSyncNums](OHOS::DistributedRdb::Details &&details) {
        for (auto &detail : details) {
            if (detail.first == "default") {
                HILOG_INFO("SyncContactsWithoutSpace details progress is:%{public}d, ts = %{public}lld",
                    detail.second.progress, (long long) time(NULL));
                if (detail.second.progress == OHOS::DistributedRdb::SYNC_FINISH) {
                    HILOG_WARN("SyncContactsWithoutSpace finished, detail.second.code = %{public}d, ts = %{public}lld",
                        detail.second.code, (long long) time(NULL));

                    // 通知当前批次已云同步完成
                    {
                        std::lock_guard<std::mutex> lock(syncMutex_);
                        isSyncFinish = true;
                        syncConVar_.notify_all();
                    }
                    if (detail.second.code == OHOS::DistributedRdb::ProgressCode::SUCCESS &&
                        syncNums >= totalSyncNums) {
                        HILOG_INFO("SyncContactsWithoutSpace sync all finish, start to checkCursorAndUpload.");
                        contactsConnectAbility_->ConnectAbility("", "", "", "", "checkCursorAndUpload", "");
                    }
                }
            }
        }
    };

    syncNums = 0;
    // 将没有资产待上行的联系人记录进行分批查询上云
    for (size_t i = 0; i < uuids.size(); i += CLOUD_SYNC_SIZE) {
        syncNums++;
        HILOG_INFO("SyncContactsWithoutSpace batch index: %{public}d.", syncNums);
        size_t end = std::min(i + CLOUD_SYNC_SIZE, uuids.size());
        std::vector<OHOS::NativeRdb::ValueObject> batchUuids(uuids.begin() + i, uuids.begin() + end);
        OHOS::NativeRdb::AbsRdbPredicates cloudPred(ContactTableName::CLOUD_RAW_CONTACT);
        cloudPred.In(CloudRawContactColumns::UUID, batchUuids);
        store_->Sync(option, cloudPred, detailsCallback);

        // 同步等待回调结束，再执行下一批
        WaitForSyncFinish();
    }

    // 同步结束后修改为异步下载资产
    distributedConfig.asyncDownloadAsset = true;
    store_->SetDistributedTables({ContactTableName::CLOUD_RAW_CONTACT}, g_mSyncMode, distributedConfig);
    ClearRetryCloudSyncCount();
    HILOG_INFO("SyncContactsWithoutSpace end.");
}

void ContactsDataBase::WaitForSyncFinish()
{
    std::unique_lock<std::mutex> lock(syncMutex_);
    auto waitStatus =
        syncConVar_.wait_for(lock, std::chrono::seconds(SYNC_WAIT_TIME), [this]() { return isSyncFinish == true; });
    if (!waitStatus) {
        HILOG_WARN("WaitForSyncFinish timeout.");
    }
    isSyncFinish = false;
}

template <typename Func>
int ContactsDataBase::HandleRdbStoreRetry(Func &&callback)
{
    int retCode = std::forward<decltype(callback)>(callback)();
    auto itRetryWithSleep = RBD_STORE_RETRY_REQUEST_WITH_SLEEP.find(retCode);
    if (itRetryWithSleep != RBD_STORE_RETRY_REQUEST_WITH_SLEEP.end()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(RBD_STORE_RETRY_REQUEST_WITH_SLEEP_TIMES));
        HILOG_INFO("HandleRdbStoreRetry ret code is %{public}d", retCode);
        return std::forward<decltype(callback)>(callback)();
    }
    return retCode;
}

template <typename Func>
int SqliteOpenHelperContactCallback::HandleRdbStoreRetry(Func&& callback)
{
    int retCode = std::forward<decltype(callback)>(callback)();
    auto itRetryWithSleep = RBD_STORE_RETRY_REQUEST_WITH_SLEEP.find(retCode);
    if (itRetryWithSleep != RBD_STORE_RETRY_REQUEST_WITH_SLEEP.end()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(RBD_STORE_RETRY_REQUEST_WITH_SLEEP_TIMES));
        HILOG_INFO("SqliteOpenHelperContactCallback HandleRdbStoreRetry ret code is %{public}d", retCode);
        return std::forward<decltype(callback)>(callback)();
    }
    return retCode;
}

// 记录删除的联系人的容器
thread_local VectorThreadLocal ContactsDataBase::deleteContactIdVector;
// 记录更新的联系人的容器
thread_local VectorThreadLocal ContactsDataBase::updateContactIdVector;

bool VectorWithLock::production(int id)
{
    std::unique_lock lk(mut);
    set.emplace(id);
    return true;
}

bool VectorWithLock::productionMultiple(std::vector<int> &v)
{
    std::unique_lock lk(mut);
    for (auto id : v) {
        set.emplace(id);
    }
    return true;
}

std::vector<int> VectorWithLock::consume()
{
    std::unique_lock lk(mut);
    std::vector<int> vectorCopy;
    if (set.empty()) {
        return vectorCopy;
    }
    for (auto id : set) {
        vectorCopy.push_back(id);
    }
    set.clear();
    return vectorCopy;
}

bool VectorThreadLocal::production(int id)
{
    set.emplace(id);
    return true;
}

bool VectorThreadLocal::productionMultiple(std::vector<int> &v)
{
    for (auto id : v) {
        set.emplace(id);
    }
    return true;
}

std::vector<int> VectorThreadLocal::consume()
{
    std::vector<int> vectorCopy;
    if (set.empty()) {
        return vectorCopy;
    }
    for (auto id : set) {
        vectorCopy.push_back(id);
    }
    set.clear();
    return vectorCopy;
}

int EmptyOpenCallback::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    return OHOS::NativeRdb::E_OK;
}

int EmptyOpenCallback::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    return OHOS::NativeRdb::E_OK;
}

}  // namespace Contacts
}  // namespace OHOS