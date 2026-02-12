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

#ifndef CONTACT_DATABASE_H
#define CONTACT_DATABASE_H

#include "auto_sync_detail_progress_observer.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"

#include "calllog_database.h"
#include "profile_database.h"
#include "cloud_contacts_observer.h"
#include "contact_connect_ability.h"
#include "datashare_helper.h"
#include "construction_name.h"
#include <thread>

namespace OHOS {
namespace Contacts {
class BlocklistDataBase;
class VectorWithLock {
private:
    std::mutex mut;
    std::set<int> set;
public:
    bool production(int id);

    bool productionMultiple(std::vector<int> &v);

    std::vector<int> consume();
};
// 使用threadLocal方式使用
class VectorThreadLocal {
private:
    std::set<int> set;
public:
    bool production(int id);

    bool productionMultiple(std::vector<int> &v);

    std::vector<int> consume();
};

class ContactsDataBase {
public:
    static std::shared_ptr<ContactsDataBase> GetInstance();
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> doubleContactDbStore_;
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> contactStore_;
    static std::shared_ptr<OHOS::Contacts::ContactsObserver> observer_;
    static std::shared_ptr<OHOS::Contacts::AutoSyncDetailProgressObserver> autoSyncDetailProgressObserver_;
    static std::string getUriLogPrintByUri(OHOS::Uri uri);
    // 记录删除的联系人的容器
    thread_local static VectorThreadLocal deleteContactIdVector;
    // 记录更新的联系人的容器
    thread_local static VectorThreadLocal updateContactIdVector;
    ~ContactsDataBase();
    std::vector<OHOS::NativeRdb::ValuesBucket> ResultSetToValuesBucket(
        std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
    void GetContactByValue(int &rawContactId, OHOS::NativeRdb::ValueObject &value);
    int64_t InsertRawContact(std::string table, OHOS::NativeRdb::ValuesBucket value);
    int64_t InsertContactData(std::string table, OHOS::NativeRdb::ValuesBucket value, std::string isSync);
    int64_t InsertPrivacyContactsBackup(std::string table, OHOS::NativeRdb::ValuesBucket contactDataValues);
    int64_t BatchInsertPrivacyContactsBackup(
        std::string table, std::vector<OHOS::NativeRdb::ValuesBucket> contactDataValues);
    int64_t BatchInsert(
        std::string table, const std::vector<DataShare::DataShareValuesBucket> &values, std::string isSyncFromCloud);
    int64_t BatchInsertRawContacts(const std::vector<DataShare::DataShareValuesBucket> &values);
    int64_t InsertGroup(std::string table, OHOS::NativeRdb::ValuesBucket value);
    int64_t InsertBlockList(std::string table, OHOS::NativeRdb::ValuesBucket value);
    int64_t InsertCloudRawContacts(std::string table, OHOS::NativeRdb::ValuesBucket value);
    int64_t InsertPoster(const std::string &table, OHOS::NativeRdb::ValuesBucket value);
    int UpdateContactData(
        OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync);
    int UpdatePrivacyContactsBackup(
        OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int UpdateRawContact(
        OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync);
    int UpdateCloudRawContacts(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int Update(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int UpdateTable(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int UpdateBlockList(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteGroup(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteContactData(OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync);
    int DeletePrivacyContactsBackup(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync);
    int DeleteBlockList(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteCloudRawContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeletePoster(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> Query(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &columns);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryContactByCompanyGroup();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryCountWithoutCompany();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryContactByLocationGroup();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryCountWithoutLocation();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryContactByRecentTime();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryDetectRepair();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryLocationContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates, 
        std::vector<std::string> &columns);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryUuidNotInRawContact();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryBigLengthNameContact();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryDeletedContactsInfo(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::string rawIds);
    int CompletelyDelete(OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string handleType);
    int DeleteMergeRawContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int BeginTransaction();
    int Commit();
    int RollBack();
    static void DestroyInstanceAndRestore(std::string restorePath);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> SelectCandidate();
    int Split(DataShare::DataSharePredicates predicates);
    int ContactMerge();
    int ReContactMerge(DataShare::DataSharePredicates predicates);
    int DeleteRecord(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int GetTypeId(std::string typeText);
    void DeleteExecute(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store,
        std::vector<OHOS::NativeRdb::ValuesBucket> queryValuesBucket);
    void InsertMergeData(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> &rawContactIdVector);
    void MarkMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store);
    void MergeUpdateTask(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> &rawContactIdVector,
        bool isDeleted, bool isAffectsCalllog = true);
    void DeleteRecordInsert(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store,
        std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket);
    void DeletedRelationAsyncTask(int rawContactId);
    void HandleHwAccount(int rawContactId);
    void QueryCloudContactsCount();
    bool MoveDbFile();
    int SyncContacts(const int32_t syncCode = OHOS::DistributedRdb::ProgressCode::SUCCESS);
    void RetryCloudSync(int code);
    bool CalcSyncTime(const int64_t currentTime);
    int UpdateUUidNull();
    // checkCursor处理的，更详细的参数
    // paramStr：coldBoot，冷启动调用到checkCursor
    void RefreshContacts(std::string paramStr);
    void NotifyContactChange();
    void UpdateDirtyToOne();
    void UpdateContactTimeStamp();
    int SetFullSearchSwitch(bool switchStatus);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryContactCloneFlag();
    int UpdateContactedStautsByPhoneNum(const OHOS::NativeRdb::ValueObject &phoneNum);
    int UpdateContactedStautsById(int contactDataId);
    void ClearRetryCloudSyncCount();
    int RecycleRestore(OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::string isSync);
    int RecycleUpdateCloud(int rawContactId);
    std::string GetUuid(int rawContactId);
    int DeleteCloudContact(int rawContactId);
    int DeleteCloudGroups(int rawContactId);
    int64_t InsertDeleteRawContact(std::string table, OHOS::NativeRdb::ValuesBucket initialValues);
    int DeleteAll(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> rawContactIds,
        std::string handleType = "");
    int DeleteAllMerged(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> &contactIds,
        std::vector<int> &rawContactIds);
    int UpdateRawContactSeq(const OHOS::NativeRdb::ValuesBucket &values);
	std::string getCallingBundleName();
    int64_t BatchInsertBlockList(const std::vector<DataShare::DataShareValuesBucket> &values);
    int clearAndReCreateTriggerSearchTable();
    bool MoveBlocklistData();
    void MoveBlocklistDataAsyncTask();
    bool IsMoveBlocklist();
    int UpdateBlocklistMigrateStatus(int blocklist_migrate_status);
    int GetTypeText(
        OHOS::NativeRdb::ValuesBucket &contactDataValues, int &typeId, int &rawContactId, std::string &typeText);
    void QueryCalendarIds(std::vector<int> &rawIdVector, std::string &calendarEventIds);
    int QueryContactCountDoubleDbByStore();
    int QueryContactCountDoubleDb();
    // 批量新增联系人
    int addContactInfoBatch(const std::vector<DataShare::DataShareValuesBucket> &contactDataViewValues);
    // 根据contactId分组信息
    void groupByContactId(const std::vector<DataShare::DataShareValuesBucket> &values,
        std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>> &groupByRawIdContactDataMap,
        std::map<int, OHOS::NativeRdb::ValuesBucket> &groupByContactIdContactMap,
        std::map<int, int> &contactIdToNameRawContactIdMap,
        std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>> &groupByContactIdRawMap);
    // 解析出contact的valueBucket
    OHOS::NativeRdb::ValuesBucket parseContactInfo(
        const OHOS::NativeRdb::ValuesBucket &contactDataViewValuesBucket);
    // 解析出raw_contact的valueBucket
    OHOS::NativeRdb::ValuesBucket parseRawContactInfo(
        const OHOS::NativeRdb::ValuesBucket &contactDataViewValuesBucket);
    // 解析出contact_data的valueBucket
    OHOS::NativeRdb::ValuesBucket parseContactDataInfo(
        const OHOS::NativeRdb::ValuesBucket &contactDataViewValuesBucket);
    // 从源valueBucket，赋值列的值到目标valueBucket
    void copyStringTypeColValueToTarget(const OHOS::NativeRdb::ValuesBucket &sourceValuesBucket,
        OHOS::NativeRdb::ValuesBucket &targetValuesBucket, std::string colName);
    void copyStringTypeColValueToTarget(const OHOS::NativeRdb::ValuesBucket &sourceValuesBucket,
        OHOS::NativeRdb::ValuesBucket &targetValuesBucket, std::string sourceColName, std::string targetColName);
    // 从源valueBucket，赋值列的值到目标valueBucket
    void copyIntTypeColValueToTarget(const OHOS::NativeRdb::ValuesBucket &sourceValuesBucket,
        OHOS::NativeRdb::ValuesBucket &targetValuesBucket, std::string colName);
    // 从源valueBucket，赋值列的值到目标valueBucket
    void copyIntTypeColValueToTarget(const OHOS::NativeRdb::ValuesBucket &sourceValuesBucket,
        OHOS::NativeRdb::ValuesBucket &targetValuesBucket, std::string sourceColName, std::string targetColName);

    // 获取表的最大自增id
    int selectMaxIdAutoIncr(std::string tableName);
    // 更新contactData 的rawcontactid信息
    void updateContactDataRawIdInfo(std::vector<OHOS::NativeRdb::ValuesBucket> &contactDataValues, int rawId);
    // 硬删除联系人
    int HardDelete(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    // 批量删除本地联系人
    int DeleteLocalBatch(std::vector<std::string> &rawContactVector);
    // 批量删除云表相关
    int DeleteCloudBatch(std::vector<std::string> &rawContactIdVector);
    // 解析map中的value到vector数组
    void parseMapValueToVector(std::map<int, OHOS::NativeRdb::ValuesBucket> &map_,
        std::vector<OHOS::NativeRdb::ValuesBucket> &vector_);
    // 将 map 的vlaue值，解析到vector数组
    void parseMapValueToVector(std::map<int, std::vector<OHOS::NativeRdb::ValuesBucket>> &map_,
        std::vector<OHOS::NativeRdb::ValuesBucket> &vector_);
    // 批量插入contact相关信息
    int batchInsertContactRelatedInfo(std::vector<OHOS::NativeRdb::ValuesBucket> &contactVector,
        std::vector<OHOS::NativeRdb::ValuesBucket> &rawContactVector,
        std::vector<OHOS::NativeRdb::ValuesBucket> &contactDataVector);
    // 更新contact的namerawId信息
    int updateContactNameRawId(int contactId, int nameRawId);
    // 同步生日信息到日历
    int SyncBirthToCalAfterBatchInsert(std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues);
    // 解析同步的生日信息
    void parseSyncBirthInfo(int &ret, OHOS::NativeRdb::ValuesBucket &contactDataValues,
        std::vector<int> &rawIdForBirthdayVector);
    // 查询大头像信息的集合
    void queryBigPhotoDetailInfo(std::vector<std::string> &bigPhotoDetailInfoVector, std::vector<int> &rawIdVector);
    int DataSyncAfterPrivateAndMainSpaceDataMigration();
    bool isMainSpace();
    int updateMainNeedSyncStatus(int needSyncFlag);
    int getMainSpaceNeedSyncFlag();
    bool getNeedReassociateBigPhotoFlag(std::vector<OHOS::NativeRdb::ValuesBucket> &contactDataValues);
    int updateMainNeedSyncStatus(int birthSyncSize, bool needReassociateBigPhotoFlag);
    int getInfoFormValueBucket(const std::vector<DataShare::DataShareValuesBucket> &values,
        std::set<int> &contactIdSet, int &addPhotoSize);
    void UpdateHardDeleteTimeStamp();
    int parseNeedUpdateCalllogRawIdBatch(
        const std::vector<DataShare::DataShareValuesBucket> &values, std::vector<int> &rawIdVector);
    int parseNeedUpdateCalllogRawId(const DataShare::DataShareValuesBucket &value, std::vector<int> &rawIdVector);

    std::shared_ptr<OHOS::NativeRdb::ResultSet> DownLoadPosters(const std::string &payload);
    void CheckIncompleteTableForContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);
    void CheckIncompleteViewForContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);
    void CheckAndCompleteFieldsForContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);

private:
    ContactsDataBase();
    bool MoveBlocklistByPage(int blocklistCount);
    bool ResultSetToValuesBucket(
        std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        int &querySqlCount);
    bool SqlDataCopy(int resultSetNum,
        std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet,
        std::vector<std::string> columnNames);
    bool SqlDataChange(std::vector<OHOS::NativeRdb::ValuesBucket> batchInsertData,
        std::vector<std::string> blocklistIds);
    int64_t DeleteEL2Blacklist(OHOS::NativeRdb::RdbPredicates &predicates);
    ContactsDataBase(const ContactsDataBase &);
    static std::shared_ptr<ContactsDataBase> contactDataBase_;
    static std::shared_ptr<CallLogDataBase> callLogDataBase_;
    static std::shared_ptr<BlocklistDataBase> blocklistDataBase_;
    static std::shared_ptr<ContactConnectAbility> contactsConnectAbility_;
    static std::shared_ptr<DataShare::DataShareHelper> dataShareHelper_;
    std::vector<int> QueryContactDataRawContactId(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &types, std::string &calendarEventIds,
        std::string methodName);
    std::string StructureDeleteContactJson(
        OHOS::NativeRdb::ValuesBucket rawContactValues, std::string rawContactIdColumn, int rawContactId);
    std::string GetCountryCode();
    std::string QueryPersonalRingtone(std::string contactId);
    int DeleteExecute(std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket,
        std::string isSync, size_t &deleteSize);
    int DeleteExecuteUpdate(std::vector<int> &rawIdVector);
    std::map<std::string, int> QueryDeletedRelationData(int rawContactId);
    void DeleteRawContactLocal(int contactId, int rawContactId, std::string backupData, std::string disPlayName,
        size_t deleteTime, std::string callingBundleName,
        std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues);
    std::vector<OHOS::NativeRdb::ValuesBucket> DeleteContactQuery(OHOS::NativeRdb::RdbPredicates &rdbPredicates,
        std::vector<std::string> &contactIdArr);
    std::vector<OHOS::NativeRdb::ValuesBucket> DeleteRawContactQuery(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteLocal(int rawContactId, std::string contactId, int isDeleteValue);
    int DeletePersonalRingtone(std::string contactId);
    int DeleteLocalMerged(std::string contactId, std::string rawContactId);
    static bool Restore(std::string restorePath);
    int CompletelyDeleteCommit(int retCode);
    int GetUpdateDisplayRet(
        std::string &typeText, std::vector<int> &rawContactIdVector, OHOS::NativeRdb::ValuesBucket &contactDataValues);
    int DeleteExecuteRetUpdateCloud(std::string uuid);
    void setAnonymousSortInfo(OHOS::NativeRdb::ValuesBucket &rawContactValues);
    int64_t BatchInsertRawContactsCore(const std::vector<DataShare::DataShareValuesBucket> &values);
    int BatchInsertDisposal(std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues,
        const std::vector<DataShare::DataShareValuesBucket> &values, unsigned int size, std::string isSyncFromCloud);
    int SyncBirthToCalAfterBatchInsert(std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues,
        const std::vector<DataShare::DataShareValuesBucket> &values, unsigned int size);
    void BatchInsertPushBack(int rawContactId, OHOS::NativeRdb::ValuesBucket contactDataValues,
        std::vector<OHOS::NativeRdb::ValuesBucket> &batchInsertValues, int typeId, std::string isSyncFromCloud);
    int QueryInterceptionCallCount(std::string phoneNumber);
    void NotifyMmsUpdateInterceptionCount(std::vector<std::string> phoneNumbers, int blockType);
    int RecordUpdateContactId(int rawContactId, OHOS::NativeRdb::ValuesBucket &contactDataValues, std::string isSync);
    std::vector<OHOS::NativeRdb::ValuesBucket> QueryDeleteRawContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteCloud(int rawContactId, int retCode);
    int UpdateDeleted(std::vector<int> &rawContactIds);
    bool CompareNumbers(std::string number1, std::string number2);
    void UpdateBlockListCallCounts(std::vector<std::string> phoneNumbers, std::vector<int> interceptionCallCounts);
    void boardReportBatchInsert(int size);
    void boardReportHardDelete(int delCount, std::string handleType);
    void SplitRawContactAfterDel(std::vector<int> rawContactIdVector);
    void QueryPersonalRingtoneList(std::vector<int> &rawIdVector, std::vector<std::string> &ringtoneUris);
    std::map<std::string, int64_t> QueryIsInBlockList(const std::map<std::string,
        OHOS::NativeRdb::ValuesBucket> &values);
    std::set<std::string> GetFormatPhoneNumberSet(const std::vector<DataShare::DataShareValuesBucket> &values);
    int BatchUpdateBlockListOneByOne(std::vector<OHOS::NativeRdb::ValuesBucket> updateBlockListValues);
    std::map<std::string, OHOS::NativeRdb::ValuesBucket> ToBlockListValueBucketMap(
    const std::vector<DataShare::DataShareValuesBucket> &values);
    template <typename Func>
    int HandleRdbStoreRetry(Func&& callback);
    int QueryStartWithTypeInterceptionCallCount(std::string phoneNumber);
    int CoverToUpdateBlockListValues(OHOS::NativeRdb::ValuesBucket values,
        OHOS::NativeRdb::RdbPredicates &rdbPredicates,
        std::vector<OHOS::NativeRdb::ValuesBucket> &updateBlockListValues);
    void updateFormatPhoneNumber(int typeId, OHOS::NativeRdb::ValuesBucket &contactDataValues);
    void FillingNumberLocation(int typeId, OHOS::NativeRdb::ValuesBucket &contactDataValues);
    bool FillingHistoryNumberLocation();
    void NumberLocationRefresh();
    bool UpdateLocation(int id, const std::string &numberLocation);
    bool QueryNumberLocation(const std::string &number, std::string &numberLocation);
    std::string generatePhoneNumber(std::string fromDetailInfo);
    int DeleteContactDirectly(std::vector<std::string> &contactIdArr);
    bool IsAffectCalllog(std::string type);
    std::string QueryContactIds(const std::string &querySql, const std::string &columnName);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryViewContact(std::vector<std::string> &columns,
        std::string queryArg);
    std::string GetPhoneNumsFromContactData(const std::vector<NativeRdb::ValuesBucket> &contactData);
    void ReportOperationAuditLog(const std::string &auditRequest,
        const std::vector<NativeRdb::ValuesBucket> &rawContacts,
        const std::map<int, std::vector<NativeRdb::ValuesBucket>> &contactDataMap,
        std::function<int(int)> GetOldContactId);
    int64_t GetContactCount();
    void ReportOperationAuditLog(const std::vector<std::string> &rawContactIds);
    std::string GetCallingBundleName();
    void QueryContactInfosByRawIds(
        const std::vector<std::string> &rawContactIds, std::vector<NativeRdb::ValuesBucket> &contactInfos);

    /**
     * @brief If the accountId not exists or equal 1, replace it with system account
     *
     * @param rawContactValues raw_contact values
     */
    void ReplaceAccountId(OHOS::NativeRdb::ValuesBucket &rawContactValues);

    /**
     * @brief Get the Uuids With Assets have Synced in cloud
     *
     * @return std::vector<OHOS::NativeRdb::ValueObject> uuids
     */
    std::vector<OHOS::NativeRdb::ValueObject> GetUuidsWithAssetsSynced();

    /**
     * @brief Sync cloud_raw_contact table when no space for asset
     */
    void SyncContactsWithoutSpace();

    /**
     * @brief wait fro sync contacts finish
     */
    void WaitForSyncFinish();

private:
    std::mutex syncMutex_;
    std::condition_variable syncConVar_;
    bool isSyncFinish = false;
    int syncNums = 0;
};

class EmptyOpenCallback : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int OnCreate(OHOS::NativeRdb::RdbStore &store) override;
    int OnUpgrade(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion) override;
};

class SqliteOpenHelperContactCallback : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    int OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;

private:
    void UpgradeToV2(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV3(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV4(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV5(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV6(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV7(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV8(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV9(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV10(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV11(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV12(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV13(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV14(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV15(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV16(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV17(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV18(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV19(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV20(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV21(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV22(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV23(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV24(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeToV25(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV26(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV27(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV28(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int DropAndCreateView(OHOS::NativeRdb::RdbStore &store);
    int UpgradeToV28AddColumn(OHOS::NativeRdb::RdbStore &store);
    int UpgradeToV29(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV30(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV31(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV32(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV33(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV34(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV35(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV36(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV37(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV38(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV39(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV40(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV41(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeToV42(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeUnderV10(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpgradeUnderV20(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeUnderV30(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeUnderV35(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeUnderV40(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    int UpgradeUnderV45(OHOS::NativeRdb::RdbStore &store, int oldVersion, int newVersion);
    void UpdateSrotInfoByDisplayName(OHOS::NativeRdb::RdbStore &store);
    void UpdateAnonymousSortInfo(OHOS::NativeRdb::RdbStore &store, int id);
    void UpdateSortInfo(OHOS::NativeRdb::RdbStore &store, ConstructionName &name, int id);
    void UpdateFormatPhoneNumber(OHOS::NativeRdb::RdbStore &store);
    void UpdateFavoriteOrder(OHOS::NativeRdb::RdbStore &store);
    int Commit(OHOS::NativeRdb::RdbStore &store);
    int RollBack(OHOS::NativeRdb::RdbStore &store);
    int BeginTransaction(OHOS::NativeRdb::RdbStore &store);
    template <typename Func>
    int HandleRdbStoreRetry(Func&& callback);
    bool ExecuteAndCheck(OHOS::NativeRdb::RdbStore &store, const std::string &sql);
};
}  // namespace Contacts
}  // namespace OHOS
#endif  // CONTACT_DATABASE_H
