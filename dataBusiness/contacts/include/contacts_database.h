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

#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"

#include "calllog_database.h"
#include "profile_database.h"

namespace OHOS {
namespace Contacts {
class ContactsDataBase {
public:
    static std::shared_ptr<ContactsDataBase> GetInstance();
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> contactStore_;

    std::vector<OHOS::NativeRdb::ValuesBucket> ResultSetToValuesBucket(
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet);
    void GetContactByValue(int &rawContactId, OHOS::NativeRdb::ValueObject &value);
    int64_t InsertRawContact(std::string table, OHOS::NativeRdb::ValuesBucket value);
    int64_t InsertContactData(std::string table, OHOS::NativeRdb::ValuesBucket value);
    int64_t InsertGroup(std::string table, OHOS::NativeRdb::ValuesBucket value);
    int64_t InsertBlockList(std::string table, OHOS::NativeRdb::ValuesBucket value);
    int UpdateContactData(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int UpdateRawContact(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int UpdateGroup(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int UpdateBlockList(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteGroup(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteContactData(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteRawContact(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteBlockList(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> Query(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &columns);
    int CompletelyDelete(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int BeginTransaction();
    int Commit();
    int RollBack();
    static void DestroyInstanceAndRestore(std::string restorePath);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> SelectCandidate();
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
        std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> &rawContactIdVector, bool isDeleted);
    void DeleteRecordInsert(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store,
        std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket);
    void DeletedAsyncTask(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store,
        std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket);

private:
    ContactsDataBase();
    ContactsDataBase(const ContactsDataBase &);
    static std::shared_ptr<ContactsDataBase> contactDataBase_;
    static std::shared_ptr<CallLogDataBase> callLogDataBase_;

    std::vector<int> QueryContactDataRawContactId(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &types);
    std::string StructureDeleteContactJson(
        OHOS::NativeRdb::ValuesBucket rawContactValues, std::string rawContactIdColumn, int rawContactId);
    int DeleteExecute(std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket);
    int DeleteRawContactLocal(int contactId, int rawContactId, std::string backupData, std::string disPlayName);
    std::vector<OHOS::NativeRdb::ValuesBucket> DeleteContactQuery(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    std::vector<OHOS::NativeRdb::ValuesBucket> DeleteRawContactQuery(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteLocal(int rawContactId, std::string contactId);
    static bool Restore(std::string restorePath);
    int GetTypeText(
        OHOS::NativeRdb::ValuesBucket &contactDataValues, int &typeId, int &rawContactId, std::string &typeText);
    int CompletelyDeleteCommit(int retCode);
};

class SqliteOpenHelperContactCallback : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    int OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_DATABASE_H
