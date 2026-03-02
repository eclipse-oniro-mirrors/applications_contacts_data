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

#include "contacts_manager.h"
#include <filesystem>
#include <fstream>
#include <mutex>

#include "common.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "async_task.h"
#include "rdb_store_config.h"
#include "security_label.h"

#include "rdb_utils.h"
#include "sql_analyzer.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mutex;
}
std::shared_ptr<OHOS::Contacts::ContactsDataBase> ScreenUnlockedEventSubscriber::contactsDataBase_;
std::shared_ptr<ContactManager> ContactManager::contactManager_ = nullptr;
std::shared_ptr<OHOS::Contacts::ContactConnectAbility> ScreenUnlockedEventSubscriber::connectAbility_ = nullptr;
std::shared_ptr<ContactManager> ContactManager::GetInstance()
{
    if (contactManager_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mutex);
        if (contactManager_ == nullptr) {
            contactManager_.reset(new ContactManager());
            return contactManager_;
        }
    }
    return contactManager_;
}

ContactManager::ContactManager()
{
    Init();
}

ContactManager::~ContactManager()
{
    HILOG_WARN("ContactManager::~ContactManager()");
    if (screenSwitchSubscriber_ != nullptr) {
        bool subRet = CommonEventManager::UnSubscribeCommonEvent(screenSwitchSubscriber_);
        if (!subRet) {
            HILOG_ERROR("UnSubscribe data share ready event failed!");
        }
        screenSwitchSubscriber_ = nullptr;
    }
}

void ContactManager::Init()
{
    OnAddSystemEvent();
}

void ContactManager::OnAddSystemEvent()
{
    HILOG_WARN("ContactManager OnAddSystemEvent");
    MatchingSkills matchingSkills;
    // 监听解锁事件
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    screenSwitchSubscriber_ = std::make_shared<ScreenUnlockedEventSubscriber>(subscriberInfo);
    bool subRet = CommonEventManager::SubscribeCommonEvent(screenSwitchSubscriber_);
    if (!subRet) {
        HILOG_ERROR("Failed to subscribe to the lock screen switchover event!");
    }
}

void ScreenUnlockedEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    OHOS::EventFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    int32_t codeType = data.GetCode();

    if (action == CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED) {
        HILOG_WARN("action is COMMON_EVENT_SCREEN_UNLOCKED");
        contactsDataBase_ = Contacts::ContactsDataBase::GetInstance();
        HILOG_WARN("action = %{public}s, codeType = %{public}d", action.c_str(), codeType);
        contactsDataBase_->DataSyncAfterPrivateAndMainSpaceDataMigration();
        // 进行大头像备份
        connectAbility_ = Contacts::ContactConnectAbility::GetInstance();
        connectAbility_ -> ConnectAbility("", "", "", "", "backUpBigPhotoFormDistribute", "");
    }
}

} // namespace Contacts
} // namespace OHOS