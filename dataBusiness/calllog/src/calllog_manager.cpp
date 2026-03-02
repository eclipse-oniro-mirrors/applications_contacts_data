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

#include "calllog_manager.h"
#include <filesystem>
#include <fstream>
#include <mutex>

#include "calllog_common.h"
#include "common.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "async_task.h"
#include "privacy_contacts_manager.h"
#include "rdb_store_config.h"
#include "security_label.h"

#include "rdb_utils.h"
#include "sql_analyzer.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mutex;
}
std::shared_ptr<Contacts::CallLogDataBase> ScreenSwitchEventSubscriber::callLogDataBase_ = nullptr;
std::shared_ptr<CallLogManager> CallLogManager::callLogManager_ = nullptr;
std::shared_ptr<CallLogManager> CallLogManager::GetInstance()
{
    if (callLogManager_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mutex);
        if (callLogManager_ == nullptr) {
            callLogManager_.reset(new CallLogManager());
            return callLogManager_;
        }
    }
    return callLogManager_;
}

CallLogManager::CallLogManager()
{
    Init();
}

CallLogManager::~CallLogManager()
{
    HILOG_WARN("CallLogManager::~CallLogManager()");
    if (screenSwitchSubscriber_ != nullptr) {
        bool subRet = CommonEventManager::UnSubscribeCommonEvent(screenSwitchSubscriber_);
        if (!subRet) {
            HILOG_ERROR("UnSubscribe data share ready event failed!");
        }
        screenSwitchSubscriber_ = nullptr;
    }
    if (accountSubscriber_ != nullptr) {
        ErrCode errCode = AccountSA::OsAccountManager::UnsubscribeOsAccount(accountSubscriber_);
        if (errCode != ERR_OK) {
            HILOG_ERROR("UnSubscribe the os account removed event failed! errCode: %{publid}d", errCode);
        }
        accountSubscriber_ = nullptr;
    }
}

void CallLogManager::Init()
{
    OnAddSystemEvent();
    OnAddOsAccountEvent();
    std::thread thread([]() {
        PrivacyContactsManager::GetInstance()->ProcessDataAfterPrivacySpaceDeleted();
        PrivacyContactsManager::GetInstance()->MigratePrivacyCallLog();
        PrivacyContactsManager::GetInstance()->SyncPrivacyContactsBackup();
    });
    thread.detach();
}

void CallLogManager::OnAddSystemEvent()
{
    HILOG_WARN("CallLogManager OnAddSystemEvent");
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_LOCK_FILE_ACCESS_STATE_CHANGED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SECOND_MOUNTED);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    screenSwitchSubscriber_ = std::make_shared<ScreenSwitchEventSubscriber>(subscriberInfo);
    bool subRet = CommonEventManager::SubscribeCommonEvent(screenSwitchSubscriber_);
    if (!subRet) {
        HILOG_ERROR("Failed to subscribe to the lock screen switchover event!");
    }
}

void ScreenSwitchEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    OHOS::EventFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    int32_t codeType = data.GetCode();

    if (action == CommonEventSupport::COMMON_EVENT_SCREEN_LOCK_FILE_ACCESS_STATE_CHANGED) {
        HILOG_WARN("action is COMMON_EVENT_SCREEN_LOCK_FILE_ACCESS_STATE_CHANGED");
        callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
        HILOG_WARN("action = %{public}s, codeType = %{public}d", action.c_str(), codeType);
        callLogDataBase_->UpdateCallLogChangeFile((CallLogType)codeType, true);
    } else if (action == CommonEventSupport::COMMON_EVENT_SECOND_MOUNTED) {
        HILOG_WARN("action is COMMON_EVENT_SECOND_MOUNTED");
        callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
        callLogDataBase_->UpdateCallLogChangeFile(CallLogType::E_CallLogType, true);
    }
}

void CallLogManager::OnAddOsAccountEvent()
{
    HILOG_WARN("CallLogManager OnAddOsAccountEvent");
    std::set<OHOS::AccountSA::OsAccountState> states = {
        OHOS::AccountSA::OsAccountState::REMOVED, OHOS::AccountSA::OsAccountState::SWITCHED};
    OHOS::AccountSA::OsAccountSubscribeInfo subscribeInfo(states);
    accountSubscriber_ = std::make_shared<AccountSubscriber>(subscribeInfo);
    ErrCode errCode = OHOS::AccountSA::OsAccountManager::SubscribeOsAccount(accountSubscriber_);
    if (errCode != ERR_OK) {
        HILOG_ERROR("Failed to subscribe to the os account removed event! errCode: %{public}d", errCode);
    }
}

void AccountSubscriber::OnStateChanged(const OHOS::AccountSA::OsAccountStateData &data)
{
    if (data.state == OHOS::AccountSA::OsAccountState::REMOVED) {
        HILOG_INFO("user deleted: %{public}d", data.fromId);
        PrivacyContactsManager::GetInstance()->ProcessDataAfterPrivacySpaceDeleted();
    } else if (data.state == OHOS::AccountSA::OsAccountState::SWITCHED) {
        HILOG_INFO("user switched, from %{public}d to %{public}d", data.fromId, data.toId);
        std::thread thread([]() { PrivacyContactsManager::GetInstance()->MigratePrivacyCallLog(); });
        thread.detach();
    }
}

} // namespace Contacts
} // namespace OHOS