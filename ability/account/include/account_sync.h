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

#ifndef ACCOUNT_SYNC_H
#define ACCOUNT_SYNC_H

#include <vector>
#include <ctime>

#include "account_info.h"

#include "account_data_collection.h"
#include "contacts_database.h"
#include "profile_database.h"

namespace OHOS {
namespace Contacts {
class AccountSync {
public:
    ~AccountSync();
    static std::shared_ptr<AccountSync> GetInstance();
    static std::shared_ptr<ContactsDataBase> contactsDataBase_;
    static std::shared_ptr<ProfileDatabase> profileDataBase_;
    // Update account synchronously
    void SyncUpdateAccount(std::vector<OHOS::AccountSA::OhosAccountInfo> sysAccounts, bool isProfile);

private:
    AccountSync();
    static std::shared_ptr<AccountSync> instance_;
    int ClearData(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int accountId);
    void getShouldUpdateAndAddAccounts(std::vector<OHOS::AccountSA::OhosAccountInfo> systemAccounts,
        std::vector<AccountDataCollection> localAccounts,
        std::vector<OHOS::AccountSA::OhosAccountInfo> &shouldUpdateAccounts,
        std::vector<OHOS::AccountSA::OhosAccountInfo> &shouldAddAccounts);
};
} // namespace Contacts
} // namespace OHOS

#endif // ACCOUNT_SYNC_H
