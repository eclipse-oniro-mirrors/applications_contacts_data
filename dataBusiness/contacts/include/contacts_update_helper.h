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

#ifndef CONTACT_UPDATE_HELPER_H
#define CONTACT_UPDATE_HELPER_H

#include "datashare_predicates.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_store.h"
#include "calllog_database.h"

namespace OHOS {
namespace Contacts {
class ContactsUpdateHelper {
public:
    ContactsUpdateHelper();
    ~ContactsUpdateHelper();
    int UpdateDisplay(std::vector<int> rawContactIdVector, std::vector<std::string> types,
        std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, OHOS::NativeRdb::ValuesBucket linkDataDataValues,
        bool isDelete);
    int UpdateDisplayResult(std::vector<int> rawContactIdVector, std::vector<std::string> types,
        std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, OHOS::NativeRdb::ValuesBucket contactDataDataValues,
        bool isDelete, unsigned int count);
    OHOS::NativeRdb::ValuesBucket GetUpdateDisPlayNameValuesBucket(
        OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete);
    OHOS::NativeRdb::ValuesBucket GetUpdateSearchNameValuesBucket(
        OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete);
    OHOS::NativeRdb::ValuesBucket GetUpdateCompanyValuesBucket(
        OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete);
    int UpdateName(OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete, int rawId, std::string type,
        std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore);
    int UpdateNameResult(OHOS::NativeRdb::ValuesBucket rawContactValues, int rawContactId, std::string type,
        std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore);
    void UpdateCallLogByPhoneNum(
        std::vector<int> &rawContactIdVector, std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore, bool isDelete);
    void UpdateCallLogWhenBatchDelContact(
        std::vector<std::string> &contactIdVector, std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);
    void UpdateCallLogPhoto(std::vector<int> &rawContactIdVector, std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);
    std::string getExtra3Print(std::string extra3);
    void UpdateCallLogByPhoneNumNotifyChange(int totalUpdateRow);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> QueryDataForCallLog(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore, int contactId);
    int DataToUpdateCallLog(
        bool isDelete, int contactId, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet);
    int UpdateCallLog(
        std::vector<std::string> &phoneNumberArr, std::vector<std::string> &formatPhoneNumberArr,
        std::string &name, std::string &quickSearch, std::string &extra3, bool isDelete);
    int UpdateCallLogWhenContactChange(
        std::vector<std::string> &phoneNumberArr, std::vector<std::string> &formatPhoneNumberArr,
        std::string &name, std::string &quickSearch, std::string &extra3);
    int UpdateCallLogWhenDeleteContact(
        std::vector<std::string> &phoneNumberArr, std::vector<std::string> &formatPhoneNumberArr,
        std::string &quickSearch);
    int UpdateCallLogNameNull(std::string &name, std::string &quickSearch);
    std::vector<std::string> QueryDeletedPhoneData(std::vector<std::string> &contactIdArr,
        std::set<std::string> &detailInfoSet, std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_);
    std::map<std::string, std::pair<std::string, int>> CheckPhoneInOtherContact(std::vector<std::string> &contactIdArr,
        std::vector<std::string> &detailInfos, std::set<std::string> &detailInfoSet,
        std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore);
    int UpdateCallLogToOtherContact(std::map<std::string, std::pair<std::string, int>> &phoneToNewContactId,
        std::shared_ptr<CallLogDataBase> &callLogDataBase);

private:
    int UpdateCallLogAddPhoneNumber(std::vector<std::string> &phoneNumberArr, std::string &name,
        std::string &quickSearch, std::string &extra3, bool isFormatNumber);
    int UpdateCallLogDelPhoneNumber(std::vector<std::string> &formatPhoneNumberArr,
        std::vector<std::string> &phoneNumberArr, std::string &name, std::string &quickSearch);
};
} // namespace Contacts
} // namespace OHOS

#endif // CONTACT_UPDATE_HELPER_H
