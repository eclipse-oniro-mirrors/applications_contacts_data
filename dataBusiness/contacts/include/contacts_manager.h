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
#ifndef CONTACT_MANAGER_H
#define CONTACT_MANAGER_H

#include "common_event_subscriber.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"
#include "common.h"
#include "common_event_support.h"
#include "common_event_manager.h"
#include "contacts_database.h"
#include "contact_connect_ability.h"

namespace OHOS {
namespace Contacts {
using namespace OHOS::EventFwk;

class ScreenUnlockedEventSubscriber : public CommonEventSubscriber {
public:
    explicit ScreenUnlockedEventSubscriber(const CommonEventSubscribeInfo &info) : CommonEventSubscriber(info) {}
    ~ScreenUnlockedEventSubscriber() = default;
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
private:
    static std::shared_ptr<OHOS::Contacts::ContactsDataBase> contactsDataBase_;
    static std::shared_ptr<OHOS::Contacts::ContactConnectAbility> connectAbility_;
};

class ContactManager {
public:
    static std::shared_ptr<ContactManager> GetInstance();
    ContactManager();
    ContactManager(const ContactManager &) = delete;
    ContactManager &operator=(const ContactManager &) = delete;
    ~ContactManager();
    
private:
    void OnAddSystemEvent();
    void Init();
    std::shared_ptr<ScreenUnlockedEventSubscriber> screenSwitchSubscriber_ = nullptr;
    static std::shared_ptr<ContactManager> contactManager_;
};
}
}

#endif // CONTACT_MANAGER_H