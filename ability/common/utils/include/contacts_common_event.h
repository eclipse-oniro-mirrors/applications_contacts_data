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

#ifndef CONTACT_COMMON_EVENT_H
#define CONTACT_COMMON_EVENT_H

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
constexpr const char *CONTACT_EVENT = "com.ohos.contactsdataability";
constexpr const char *CALL_LOG_EVENT = "com.ohos.calllogability";
constexpr const char *VOICEMAIL_EVENT = "com.ohos.voicemailability";
constexpr int CONTACT_EVENT_CODE = 10000;
constexpr int CALL_LOG_EVENT_CODE = 20000;
constexpr int VOICEMAIL_EVENT_CODE = 30000;

// action
constexpr int CONTACT_INSERT = 0;
constexpr int CONTACT_UPDATE = 1;
constexpr int CONTACT_DELETE = 2;

class ContactsCommonEvent : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit ContactsCommonEvent(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo)
    {
    }
    ~ContactsCommonEvent() = default;
    static std::shared_ptr<ContactsCommonEvent> subscriber;
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data);
    static bool PublishContactEvent(const OHOS::AAFwk::Want &want, int eventCode, const std::string &eventData);
    static void UnregisterSubscriber(std::shared_ptr<OHOS::EventFwk::CommonEventSubscriber> subscriber);
    static void RegisterSubscriber();
    static void SendContactChange(int actionCode);
    static void SendCallLogChange(int actionCode);
    static void SendVoiceMailChange(int actionCode);
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_COMMON_EVENT_H
