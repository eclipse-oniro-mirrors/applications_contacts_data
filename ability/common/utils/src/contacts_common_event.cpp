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

#include "contacts_common_event.h"

namespace OHOS {
namespace Contacts {
std::shared_ptr<ContactsCommonEvent> ContactsCommonEvent::subscriber = nullptr;

void ContactsCommonEvent::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    OHOS::AAFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    HILOG_INFO("ContactsCommonEvent::OnReceiveEvent action = %{public}s", action.c_str());
    int msgCode = data.GetCode();
    std::string msgData = data.GetData();
    HILOG_INFO("ContactsCommonEvent::OnReceiveEvent msgData = %{public}s", msgData.c_str());
    HILOG_INFO("ContactsCommonEvent::OnReceiveEvent msgCode = %{public}d", msgCode);
}

bool ContactsCommonEvent::PublishContactEvent(
    const OHOS::AAFwk::Want &want, int eventCode, const std::string &eventData)
{
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(eventCode);
    data.SetData(eventData);
    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool publishResult = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    HILOG_INFO("PublishContactEvent end publishResult = %{public}d", publishResult);
    return publishResult;
}

void ContactsCommonEvent::UnregisterSubscriber(std::shared_ptr<OHOS::EventFwk::CommonEventSubscriber> subscriber)
{
    if (subscriber != nullptr) {
        bool subscribeResult = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber);
        subscriber = nullptr;
        HILOG_INFO("UnregisterSubscriber contacts end###subscribeResult = %{public}d", subscribeResult);
    }
}

void ContactsCommonEvent::RegisterSubscriber()
{
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CONTACT_EVENT);
    matchingSkills.AddEvent(CALL_LOG_EVENT);
    matchingSkills.AddEvent(VOICEMAIL_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber = std::make_shared<ContactsCommonEvent>(subscriberInfo);
    bool subscribeResult = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    HILOG_INFO("RegisterSubscriber contacts end###subscribeResult = %{public}d", subscribeResult);
}

void ContactsCommonEvent::SendContactChange(int actionCode)
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = CONTACT_EVENT_CODE;
    want.SetParam("contactsActionCode", actionCode);
    want.SetAction(CONTACT_EVENT);
    std::string eventData("ContactChange");
    PublishContactEvent(want, eventCode, eventData);
}

void ContactsCommonEvent::SendCallLogChange(int actionCode)
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = CALL_LOG_EVENT_CODE;
    want.SetParam("contactsActionCode", actionCode);
    want.SetAction(CALL_LOG_EVENT);
    std::string eventData("CallLogChange");
    PublishContactEvent(want, eventCode, eventData);
}

void ContactsCommonEvent::SendVoiceMailChange(int actionCode)
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = VOICEMAIL_EVENT_CODE;
    want.SetParam("contactsActionCode", actionCode);
    want.SetAction(VOICEMAIL_EVENT);
    std::string eventData("voicemailChange");
    PublishContactEvent(want, eventCode, eventData);
}
} // namespace Contacts
} // namespace OHOS