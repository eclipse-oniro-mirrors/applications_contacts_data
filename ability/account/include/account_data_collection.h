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

#ifndef ACCOUNT_DATA_COLLECTION_H
#define ACCOUNT_DATA_COLLECTION_H

#include <string>
#include <ctime>

#include "account_info.h"

namespace OHOS {
namespace Contacts {
class AccountDataCollection {
public:
    AccountDataCollection();
    ~AccountDataCollection();
    AccountDataCollection(std::string accountName, std::string accountType, std::string dataColletion);
    // Judge whether local account
    bool IsDefaultAccount();
    // Judge whether the system account
    bool ContainSysAccounts(std::vector<OHOS::AccountSA::OhosAccountInfo> sysAccounts);
    std::string GetcAccountName();
    std::string GetcAccountType();
    std::string GetcDataCollection();

private:
    std::string cAccountName_;
    std::string cAccountType_;
    std::string cDataCollection_;
};
} // namespace Contacts
} // namespace OHOS

#endif // ACCOUNT_DATA_COLLECTION_H
