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

#ifndef ACCOUNT_EVENT_MANAGER_H
#define ACCOUNT_EVENT_MANAGER_H

#include <string>

namespace OHOS {
namespace Contacts {
class AccountEventManager {
public:
    static std::shared_ptr<AccountEventManager> GetInstance();
    ~AccountEventManager();
    void PublishEvent(std::string data);
    void SubscriberAccountEvent();

private:
    static std::shared_ptr<AccountEventManager> instance_;
    const static std::string EVENT;
    AccountEventManager();
};
} // namespace Contacts
} // namespace OHOS
#endif // ACCOUNT_EVENT_MANAGER_H
