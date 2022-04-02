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

#include "account_change_event.h"

#include "account_sync.h"
#include "common.h"
#include "contacts_string_utils.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
AccountChangeEvent::AccountChangeEvent(const OHOS::EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
{
}

AccountChangeEvent::~AccountChangeEvent()
{
}

void AccountChangeEvent::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    std::shared_ptr<AccountSync> accountSync = AccountSync::GetInstance();
    std::vector<OHOS::AccountSA::OhosAccountInfo> sysAccounts;
    std::vector<std::string> accountObjVec = ContactsStringUtils::SplitStr(data.GetData(), SPLIT_SYMBOL_VERTICAL);
    for (size_t i = 0; i < accountObjVec.size(); i++) {
        std::vector<std::string> accountObj = ContactsStringUtils::SplitStr(accountObjVec[i], SPLIT_SYMBOL_COMMA);
        OHOS::AccountSA::OhosAccountInfo var;
        var.name_ = accountObj[0];
        var.uid_ = accountObj[1];
        sysAccounts.push_back(var);
    }
    accountSync->SyncUpdateAccount(sysAccounts, true);
    HILOG_INFO("AccountChangeEvent OnReceiveEvent update profile successful");
    accountSync->SyncUpdateAccount(sysAccounts, false);
    HILOG_INFO("AccountChangeEvent OnReceiveEvent update local successful");
}
} // namespace Contacts
} // namespace OHOS