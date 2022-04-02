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
#include "account_manager.h"

#include "contacts_account.h"
#include "contacts_columns.h"
#include "contacts_database.h"

namespace OHOS {
namespace Contacts {
AccountManager::AccountManager(void)
{
}

AccountManager::~AccountManager()
{
}

int AccountManager::GetAccount()
{
    // query accountId
    std::shared_ptr<ContactsAccount> contactsAccount = ContactsAccount::GetInstance();
    int accountId = contactsAccount->LookupAccountTypeId(
        ContactsDataBase::store_, AccountData::ACCOUNT_NAME, AccountData::ACCOUNT_TYPE);
    return accountId;
}
} // namespace Contacts
} // namespace OHOS