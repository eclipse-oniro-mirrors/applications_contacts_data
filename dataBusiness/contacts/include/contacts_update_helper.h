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

namespace OHOS {
namespace Contacts {
class ContactsUpdateHelper {
public:
    ContactsUpdateHelper();
    ~ContactsUpdateHelper();
    int UpdateDisplay(std::vector<int> rawContactIdVector, std::vector<std::string> types,
        std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, OHOS::NativeRdb::ValuesBucket linkDataDataValues,
        bool isDelete);
    OHOS::NativeRdb::ValuesBucket GetUpdateDisPlayNameValuesBucket(
        OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete);
    OHOS::NativeRdb::ValuesBucket GetUpdateSearchNameValuesBucket(
        OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete);
    OHOS::NativeRdb::ValuesBucket GetUpdateCompanyValuesBucket(
        OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete);
    int UpdateName(OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete, int rawId, std::string type,
        std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore);
    void UpdateCallLogByPhoneNum(
        std::vector<int> &rawContactIdVector, std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore, bool isDelete);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> QueryDataForCallLog(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore, int contactId);
    void DataToUpdateCallLog(
        bool isDelete, int contactId, std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet);
    int UpdateCallLog(
        std::string &phoneNumber, std::string &name, std::string &quickSearch, bool isDelete, bool isCallLog);
    int UpdateCallLogNameNull(std::string &name, std::string &quickSearch, bool isCallLog);
};
} // namespace Contacts
} // namespace OHOS

#endif // CONTACT_UPDATE_HELPER_H
