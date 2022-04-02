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

#include "account_event_manager.h"

#include <mutex>

#include "common_event_manager.h"
#include "want.h"

#include "account_change_event.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mtx;
}
const std::string AccountEventManager::EVENT = "com.test.account.change.event";
std::shared_ptr<AccountEventManager> AccountEventManager::instance_ = nullptr;

AccountEventManager::AccountEventManager(void)
{
}

AccountEventManager::~AccountEventManager()
{
}

std::shared_ptr<AccountEventManager> AccountEventManager::GetInstance()
{
    if (instance_ == nullptr) {
        instance_.reset(new AccountEventManager());
    }
    return instance_;
}

void AccountEventManager::PublishEvent(std::string data)
{
    // make a want
    OHOS::AAFwk::Want want;
    want.SetAction(EVENT);
    // make common event
    OHOS::EventFwk::CommonEventData event;
    event.SetWant(want);
    event.SetData(data);

    // lock the mutex
    g_mtx.lock();
    // publish a common event
    OHOS::EventFwk::CommonEventManager::PublishCommonEvent(event);
    g_mtx.unlock();
}

void AccountEventManager::SubscriberAccountEvent()
{
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<AccountChangeEvent> subscriber = std::make_shared<AccountChangeEvent>(subscriberInfo);
    OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
}
} // namespace Contacts
} // namespace OHOS