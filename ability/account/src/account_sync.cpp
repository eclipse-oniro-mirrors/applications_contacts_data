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

#include "account_sync.h"

#include <mutex>

#include "common.h"
#include "contacts.h"
#include "contacts_account.h"
#include "contacts_columns.h"
#include "contacts_database.h"
#include "contacts_string_utils.h"
#include "hilog_wrapper.h"
#include "raw_contacts.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mtx;
}
std::shared_ptr<ContactsDataBase> AccountSync::contactsDataBase_ = nullptr;
std::shared_ptr<ProfileDatabase> AccountSync::profileDataBase_ = nullptr;
std::shared_ptr<AccountSync> AccountSync::instance_ = nullptr;
std::shared_ptr<ContactsAccount> g_contactsAccount = nullptr;

std::shared_ptr<AccountSync> AccountSync::GetInstance()
{
    if (instance_ == nullptr) {
        instance_.reset(new AccountSync());
    }
    return instance_;
}

AccountSync::AccountSync()
{
    contactsDataBase_ = ContactsDataBase::GetInstance();
    g_contactsAccount = ContactsAccount::GetInstance();
    profileDataBase_ = ProfileDatabase::GetInstance();
}

AccountSync::~AccountSync()
{
}

void AccountSync::getShouldUpdateAndAddAccounts(std::vector<OHOS::AccountSA::OhosAccountInfo> systemAccounts,
    std::vector<AccountDataCollection> localAccounts,
    std::vector<OHOS::AccountSA::OhosAccountInfo> &shouldUpdateAccounts,
    std::vector<OHOS::AccountSA::OhosAccountInfo> &shouldAddAccounts)
{
    for (size_t i = 0; i < systemAccounts.size(); i++) {
        for (size_t j = 0; j < localAccounts.size(); j++) {
            if (systemAccounts[i].uid_ == localAccounts[j].GetcAccountType()) {
                break;
            }
            if (j == localAccounts.size() - 1) {
                shouldAddAccounts.push_back(systemAccounts[i]);
            }
        }
    }
}

void AccountSync::SyncUpdateAccount(std::vector<OHOS::AccountSA::OhosAccountInfo> sysAccounts, bool isProfile)
{
    g_mtx.lock();
    if (sysAccounts.empty()) {
        HILOG_ERROR("AccountSync::SyncUpdateAccount sysAccounts is null");
        g_mtx.unlock();
        return;
    }
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store = nullptr;
    if (isProfile) {
        store = profileDataBase_->store_;
        HILOG_INFO("AccountSync::SyncUpdateAccount this is profileDataBase_ operation");
    } else {
        store = contactsDataBase_->contactStore_;
        HILOG_INFO("AccountSync::SyncUpdateAccount this is contactsDataBase_ operation");
    }
    if (store == nullptr) {
        HILOG_ERROR("SyncUpdateAccount isProfile values is %{public}d this database store ponit is null", isProfile);
        g_mtx.unlock();
        return;
    }
    std::vector<AccountDataCollection> accounts = g_contactsAccount->GetAccountFromLocal(store);
    std::vector<AccountDataCollection> notInSysAccounts;
    for (size_t i = 0; i < accounts.size(); i++) {
        auto iter = accounts[i];
        // if account is not default and not in sysAccounts then this should remove
        if (!iter.IsDefaultAccount() && !iter.ContainSysAccounts(sysAccounts)) {
            notInSysAccounts.push_back(iter);
        }
    }
    HILOG_INFO("SyncUpdateAccount getNotExistAccount notInSysAccounts size is :%{public}zu", notInSysAccounts.size());
    std::vector<OHOS::AccountSA::OhosAccountInfo> shouldUpdateAccounts;
    std::vector<OHOS::AccountSA::OhosAccountInfo> shouldAddAccounts;
    getShouldUpdateAndAddAccounts(sysAccounts, accounts, shouldUpdateAccounts, shouldAddAccounts);
    HILOG_INFO("SyncUpdateAccount:%{public}zu, :%{public}zu", shouldUpdateAccounts.size(), shouldAddAccounts.size());
    store->BeginTransaction();
    if (!notInSysAccounts.empty()) {
        g_contactsAccount->StopForegin(store);
        store->Commit();
        for (size_t i = 0; i < notInSysAccounts.size(); i++) {
            int accountId = g_contactsAccount->GetNotExistAccount(store, notInSysAccounts[i]);
            HILOG_INFO("SyncUpdateAccount getNotExistAccount value is :%{public}d", accountId);
            ClearData(store, accountId);
        }
    }
    for (size_t i = 0; i < shouldAddAccounts.size(); i++) {
        g_contactsAccount->Insert(store, shouldAddAccounts[i].name_, shouldAddAccounts[i].uid_);
        store->Commit();
    }
    g_contactsAccount->OpenForegin(store);
    store->Commit();
    g_mtx.unlock();
}

int AccountSync::ClearData(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int accountId)
{
    RawContacts contactsRawContact;
    Contacts contactsContact;
    if (accountId > ID_EMPTY) {
        int needDeleteRawContactId = contactsRawContact.GetDeleteRawContactIdByAccountId(store, accountId);
        HILOG_INFO("GetDeleteRawContactIdByAccountId successfully, needDeleteRawContactId is %{public}d",
            needDeleteRawContactId);
        int needDeleteContactId = contactsRawContact.GetDeleteContactIdByAccountId(store, accountId);
        HILOG_INFO("GetDeleteContactIdByAccountId successfully needDeleteContactId is %{public}d", needDeleteContactId);
        g_contactsAccount->DeleteDataByRawId(store, needDeleteRawContactId);
        int ret = store->Commit();
        HILOG_INFO("DeleteDataByRawId successfully commit ret %{public}d", ret);
        contactsRawContact.DeleteRawcontactByRawId(store, needDeleteRawContactId);
        ret = store->Commit();
        HILOG_INFO("DeleteRawcontactByRawId successfully commit ret %{public}d", ret);
        contactsContact.DeleteContactById(store, needDeleteContactId);
        ret = store->Commit();
        HILOG_INFO("DeletecontactsById successfully commit ret %{public}d", ret);
        g_contactsAccount->DeleteGroupsByAccountId(store, accountId);
        ret = store->Commit();
        HILOG_INFO("DeleteGroupsByAccountId successfully commit ret %{public}d", ret);
        g_contactsAccount->DeleteAccountByAccountId(store, accountId);
        ret = store->Commit();
        HILOG_INFO("DeleteAccountByAccountId successfully commit ret %{public}d", ret);
    }
    return RDB_EXECUTE_OK;
}
} // namespace Contacts
} // namespace OHOS
