/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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

#ifndef PRIVACY_CONTACTS_MANAGER_H
#define PRIVACY_CONTACTS_MANAGER_H

#include <unordered_set>
#include "datashare_helper.h"
#include "datashare_values_bucket.h"
#include "os_account_info.h"
#include "values_bucket.h"

namespace OHOS {
namespace Contacts {
constexpr int PRIVACY_CONTACTS_MANAGER_RETRY_CREATE_DATASHARE_HELPER_SLEEP_TIME = 100;
constexpr int PRIVACY_CONTACTS_MANAGER_RETRY_CREATE_DATASHARE_HELPER_RETRY_TIMES = 3;

class PrivacyContactsManager {
public:
    static std::shared_ptr<PrivacyContactsManager> GetInstance();
    bool isMigratePrivacyCallLogEnbale();
    int32_t SyncPrivacyContactsBackup();
    bool InsertContactDataToPrivacyBackup(const std::vector<OHOS::NativeRdb::ValuesBucket> &contactDatas);
    bool InsertContactDataToPrivacyBackup(const std::vector<DataShare::DataShareValuesBucket> &contactDatas);
    bool InsertContactDataToPrivacyBackup(const OHOS::NativeRdb::ValuesBucket &contactData);
    bool UpdateContactDataToPrivacyBackup(const OHOS::NativeRdb::ValuesBucket &contactData);
    bool DeleteContactsFromPrivacyBackup(std::vector<int32_t> rawContactIds);
    bool restorePrivacContacts(const std::vector<int32_t> &rawIds);
    bool InsertPrivacContactsByContactDatas(const std::vector<OHOS::NativeRdb::ValuesBucket> &contactDatas);
    bool GetValuesBucketFromContactData(
        const OHOS::NativeRdb::ValuesBucket &contactDataValuesBucket, OHOS::NativeRdb::ValuesBucket &outValuesBucket);

    bool TryToMigratePrivateCallLog(const DataShare::DataShareValuesBucket &value);
    bool QueryCallLogWithPhoneNumber(
        const std::string &phoneNum, std::vector<DataShare::DataShareValuesBucket> &values);
    bool ProcessPrivacyCallLog(std::vector<OHOS::NativeRdb::ValuesBucket> &values,
        std::vector<OHOS::NativeRdb::ValuesBucket> &unprocessedValues);
    bool ProcessPrivacyCallLog(std::vector<DataShare::DataShareValuesBucket> &values,
        std::vector<DataShare::DataShareValuesBucket> &unprocessedValues);
    bool QueryIsPrivacySpacePhoneNumber(const std::string &phoneNum, bool &result);
    bool QueryIsPrivacySpacePhoneNumber(const std::vector<DataShare::DataShareValuesBucket> &phoneNums,
        std::vector<DataShare::DataShareValuesBucket> &privacyNums,
        std::vector<DataShare::DataShareValuesBucket> &normalNums);
    bool QueryIsPrivacySpacePhoneNumber(const std::vector<OHOS::NativeRdb::ValuesBucket> &phoneNums,
        std::vector<DataShare::DataShareValuesBucket> &privacyNums,
        std::vector<OHOS::NativeRdb::ValuesBucket> &normalNums);
    bool QueryPrivacyContactsByRawContactId(
        const std::vector<int32_t> &rawIds, std::vector<DataShare::DataShareValuesBucket> &contactDatas);
    bool UpdatePrivacyTag(std::vector<DataShare::DataShareValuesBucket> &values);
    bool MigratePrivacyCallLog();
    bool QueryCallLogWithPrivacyTag(
        std::vector<DataShare::DataShareValuesBucket> &values, std::vector<std::string> &deleteIds);
    bool InsertPrivacyCallLog(const std::vector<DataShare::DataShareValuesBucket> &values);
    bool DeleteCallLogAfterMigrate(const std::vector<std::string> &deleteIds);
    bool ProcessDataAfterPrivacySpaceDeleted();

    static bool ConvertResultSetToValuesBucket(const std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
        std::vector<DataShare::DataShareValuesBucket> &valuesBuckets);
    static DataShare::DataShareValuesBucket ToDataShareValuesBucket(OHOS::NativeRdb::ValuesBucket valuesBucket);
    static bool IsMainSpace();
    static bool IsPrivacySpace();

private:
    static bool IsSpecialSpace(OHOS::AccountSA::OsAccountType specialType);
    bool InsertPrivacyContactsBackup(std::vector<DataShare::DataShareValuesBucket> &values);
    bool QueryContactsFromPrivacyContactData(std::vector<DataShare::DataShareValuesBucket> &values);
    bool DeleteAllContactsFromPrivacyBackup();
    bool DeleteAllCalllogsWithPrivacyTag();
    bool DeletePrivacyTagOnUnreadCalllog();
    DataShare::DataSharePredicates GetPrivacyCallLogPredicates(const std::string &phoneNum);
    bool UpdatePrivacyTagOfValueBucket(DataShare::DataShareValuesBucket &value);
    bool GetHelperAndUrl(const OHOS::AccountSA::OsAccountType &type, const std::string &datashareUri,
        std::shared_ptr<DataShare::DataShareHelper> &helper, std::string &url);
    int32_t GetSpecialTypeUserId(OHOS::AccountSA::OsAccountType type, int32_t &userId);
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(std::string uri);
    int64_t GetIntFromDataShareValueObject(const DataShare::DataShareValueObject &object);
    bool GetPrivacySpacePhoneNumbers(std::unordered_set<std::string> &allPrivacyNums);
    template <typename Func>
    std::shared_ptr<DataShare::DataShareHelper> RetryCreateDataShareHelper(Func &&func);
    std::mutex migrateContactMutex_;
    std::mutex migrateCallLogMutex_;
    std::mutex processCallLogMutex_;
};
}  // namespace Contacts
}  // namespace OHOS

#endif // PRIVACY_CONTACTS_MANAGER_H