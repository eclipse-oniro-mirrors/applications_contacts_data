/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef CALLLOG_MANAGER_H
#define CALLLOG_MANAGER_H

#include "common_event_subscriber.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"
#include "common.h"
#include "calllog_database.h"
#include "common_event_support.h"
#include "common_event_manager.h"
#include "os_account_manager.h"

namespace OHOS {
namespace Contacts {
using namespace OHOS::EventFwk;
using CommonEventSubscribeInfo = OHOS::EventFwk::CommonEventSubscribeInfo;
using CommonEventSubscriber = OHOS::EventFwk::CommonEventSubscriber;

class ScreenSwitchEventSubscriber : public CommonEventSubscriber {
public:
    explicit ScreenSwitchEventSubscriber(const CommonEventSubscribeInfo &info) : CommonEventSubscriber(info) {}
    ~ScreenSwitchEventSubscriber() = default;
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
private:
    static std::shared_ptr<OHOS::Contacts::CallLogDataBase> callLogDataBase_;
};

class AccountSubscriber final : public OHOS::AccountSA::OsAccountSubscriber {
public:
    AccountSubscriber(const OHOS::AccountSA::OsAccountSubscribeInfo &info) : OsAccountSubscriber(info){};

    void OnStateChanged(const OHOS::AccountSA::OsAccountStateData &data) override;
};

class CallLogManager {
public:
    static std::shared_ptr<CallLogManager> GetInstance();
    CallLogManager();
    CallLogManager(const CallLogManager &) = delete;
    CallLogManager &operator=(const CallLogManager &) = delete;
    ~CallLogManager();
    
private:
    void OnAddSystemEvent();
    void OnAddOsAccountEvent();
    void Init();
    std::shared_ptr<ScreenSwitchEventSubscriber> screenSwitchSubscriber_ = nullptr;
    std::shared_ptr<AccountSubscriber> accountSubscriber_ = nullptr;
    static std::shared_ptr<CallLogManager> callLogManager_;
};
}
}

#endif // CALLLOG_MANAGER_H