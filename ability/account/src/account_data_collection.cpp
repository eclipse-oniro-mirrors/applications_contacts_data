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

#include "account_data_collection.h"

#include <vector>

#include "contacts_columns.h"

namespace OHOS {
namespace Contacts {
AccountDataCollection::AccountDataCollection(void)
{
}

AccountDataCollection::~AccountDataCollection()
{
}

AccountDataCollection::AccountDataCollection(
    std::string accountName, std::string accountType, std::string dataCollection)
{
    this->cAccountName_ = accountName;
    this->cAccountType_ = accountType;
    this->cDataCollection_ = dataCollection;
}

std::string AccountDataCollection::GetcAccountName()
{
    return this->cAccountName_;
}

std::string AccountDataCollection::GetcAccountType()
{
    return this->cAccountType_;
}

std::string AccountDataCollection::GetcDataCollection()
{
    return this->cDataCollection_;
}

bool AccountDataCollection::IsDefaultAccount()
{
    return AccountData::ACCOUNT_NAME == this->cAccountName_ && AccountData::ACCOUNT_TYPE == this->cAccountType_;
}

bool AccountDataCollection::ContainSysAccounts(std::vector<OHOS::AccountSA::OhosAccountInfo> sysAccounts)
{
    for (auto iter = sysAccounts.begin(); iter != sysAccounts.end(); ++iter) {
        if (iter->name_ == this->cAccountName_ && iter->uid_ == this->cAccountType_) {
            return true;
        }
    }
    return false;
}
} // namespace Contacts
} // namespace OHOS
