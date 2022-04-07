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

#ifndef CONTACT_ACCOUNT_H
#define CONTACT_ACCOUNT_H

#include "rdb_errno.h"
#include "rdb_store.h"
#include "result_set.h"

#include "account_data_collection.h"

namespace OHOS {
namespace Contacts {
class ContactsAccount {
public:
    static std::shared_ptr<ContactsAccount> GetInstance();
    ~ContactsAccount();
    int64_t Insert(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, std::string accountName, std::string accountType);
    int LookupAccountTypeId(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, std::string accountName, std::string accountType);
    void PrepopulateCommonAccountTypes(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore);
    std::vector<AccountDataCollection> GetAccountFromLocal(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore);
    int GetNotExistAccount(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, AccountDataCollection collection);
    int DeleteDataByRawId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int needDeleteRawContactId);
    int DeleteGroupsByAccountId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int accountId);
    int DeleteAccountByAccountId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int accountId);
    int StopForegin(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore);
    int OpenForegin(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore);

private:
    ContactsAccount();
    static std::shared_ptr<ContactsAccount> instance_;
};
} // namespace Contacts
} // namespace OHOS

#endif // CONTACT_ACCOUNT_H
