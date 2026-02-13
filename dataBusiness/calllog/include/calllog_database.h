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

#ifndef CALLLOG_DATABASE_H
#define CALLLOG_DATABASE_H

#include <pthread.h>
#include <atomic>
#include "datashare_result_set.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"
#include "datashare_helper.h"
#include "dataobs_mgr_client.h"

namespace OHOS {
namespace Contacts {
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;
enum DatabaseStoragePath {
    EL1 = 0,
    EL2,
    EL3,
    EL4,
    EL5
};
class CallLogDataBase {
public:
    static std::shared_ptr<CallLogDataBase> GetInstance();
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
    int64_t InsertCallLog(OHOS::NativeRdb::ValuesBucket insertValues);
    int64_t BatchInsertCallLog(const std::vector<OHOS::NativeRdb::ValuesBucket> &values);
    int UpdateCallLog(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteCallLog(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> Query(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> columns);
    int BeginTransaction();
    int Commit();
    int RollBack();
    void QueryContactsByInsertCalls(OHOS::NativeRdb::ValuesBucket &insertValues,
        std::string operateTable = CallsTableName::CALLLOG);
    void GetContactInfoEnhanced(std::string phoneNumber, OHOS::NativeRdb::ValuesBucket &insertValues,
        std::string operateTable);
    std::string QueryContactsByCallsGetSubPhoneNumberSql(std::string subPhoneNumber);
    void QueryContactsByCallsInsertValues(OHOS::NativeRdb::ValuesBucket &insertValues, std::string name,
        std::string quickSearchKey, std::string meetimeAvatar, std::string operateTable);
    std::string QueryContactsByInsertCallsGetSql();
    void NotifyCallLogChange();
    void UpdateCallLogChangeFile(CallLogType codeType = CallLogType::E_CallLogType, bool isChange = false);
    void RetryCreateStoreForE();

private:
    CallLogDataBase();
    CallLogDataBase(const CallLogDataBase &);
    const CallLogDataBase &operator=(const CallLogDataBase &);
    static std::shared_ptr<CallLogDataBase> callLogDataBase_;
    static std::shared_ptr<DataShare::DataShareHelper> dataShareCallLogHelper_;
    int UpdateTopContact(OHOS::NativeRdb::ValuesBucket &insertValues);
    bool MoveDbFile();
    int UpdateContactedStatus(OHOS::NativeRdb::ValuesBucket &insertValues);
    int GetCallerIndex(std::shared_ptr<DataShare::DataShareResultSet> resultSet, std::string phoneNumber);
    int DeleteEL1CallLog(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryEL1(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> columns);
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> storeForC_;
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> storeForE_;
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> CreatSql(CallLogType codeType);
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> RetryGetRdbStore(
        int errCode, OHOS::NativeRdb::RdbStoreConfig& config, const std::string& callDbName);
    static void CheckIncompleteTable(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);
    static void CheckAndCompleteFields(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);
    void CopySqlFromCToE();
    bool CopyCallLogFromCToE();
    bool CopyCallLog(int querySqlCount);
    bool ResultSetToValuesBucket(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
    bool HandleInsertAndDelete(std::vector<OHOS::NativeRdb::ValuesBucket> &vectorQueryData,
        std::vector<int> &callLogIdList);
    bool BatchDeleteCallLog(std::vector<int> &callLogIdList);
    // The value 0 indicates that data is deleted abnormally. The value 1 indicates that data is deleted normally.
    int errCodeDeleteCallLog_ = 1;
    const int BATCH_MOVE_COUNT = 1000;
    static std::shared_ptr<DataShare::DataShareHelper> dataShareCallLogHelperRefresh_;
    void PrintCallInfoLog(OHOS::NativeRdb::ValuesBucket insertValues);
    template <typename Func>
    int CallLogHandleRdbStoreRetry(Func&& callback);
};

class SqliteOpenHelperCallLogCallback : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    int OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int OnOpen(OHOS::NativeRdb::RdbStore &rdbStore) override;

private:
    void UpgradeToV2(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV4(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV14(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV15(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV16(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV18(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV19(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV20(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeUnderV20(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeUnderV30(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV21(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV23(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV24(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV25(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV26(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeV27(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV27(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int AddColumnAbs(OHOS::NativeRdb::RdbStore &store);
    int AddColumnNotes(OHOS::NativeRdb::RdbStore &store);
    int Commit(OHOS::NativeRdb::RdbStore &store);
    int RollBack(OHOS::NativeRdb::RdbStore &store);
    int BeginTransaction(OHOS::NativeRdb::RdbStore &store);
    template <typename Func>
    int CallLogHandleRdbStoreRetry(Func&& callback);
    bool ExecuteAndCheck(OHOS::NativeRdb::RdbStore &store, const std::string &sql);
    void PrintAllTables(OHOS::NativeRdb::RdbStore &rdbStore);
};
} // namespace Contacts
} // namespace OHOS

#endif // CALLLOG_DATABASE_H
