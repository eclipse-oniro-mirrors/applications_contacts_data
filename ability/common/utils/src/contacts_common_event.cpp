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

bool ContactsCommonEvent::PublishContactEvent(const OHOS::AAFwk::Want &want, int eventCode,
    const std::string &eventData, const std::string &permission, bool order = true)
{
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(eventCode);
    data.SetData(eventData);
    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(order);
    std::vector<std::string> permissions;
    permissions.emplace_back(permission);
    publishInfo.SetSubscriberPermissions(permissions);
    bool publishResult = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    HILOG_INFO("PublishContactEvent end publishResult = %{public}d, eventCode = %{public}d, ts = %{public}lld",
        publishResult, eventCode, (long long) time(NULL));
    return publishResult;
}

void ContactsCommonEvent::SendContactChange(int actionCode)
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = CONTACT_EVENT_CODE;
    want.SetParam("contactsActionCode", actionCode);
    want.SetAction(CONTACT_EVENT);
    std::string eventData("ContactChange");
    std::string permission("ohos.permission.READ_CONTACTS");
    PublishContactEvent(want, eventCode, eventData, permission);
}

void ContactsCommonEvent::SendPosterEvent(int eventCode, const std::string& eventData)
{
    OHOS::AAFwk::Want want;
    want.SetAction(POSTER_EVENT);
    std::string permission("ohos.permission.WRITE_CONTACTS");
    PublishContactEvent(want, eventCode, eventData, permission, false);
}

void ContactsCommonEvent::SendCallLogChange(int actionCode)
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = CALL_LOG_EVENT_CODE;
    want.SetParam("contactsActionCode", actionCode);
    want.SetAction(CALL_LOG_EVENT);
    std::string eventData("CallLogChange");
    std::string permission("ohos.permission.READ_CALL_LOG");
    PublishContactEvent(want, eventCode, eventData, permission);
}

void ContactsCommonEvent::SendVoiceMailChange(int actionCode)
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = VOICEMAIL_EVENT_CODE;
    want.SetParam("contactsActionCode", actionCode);
    want.SetAction(VOICEMAIL_EVENT);
    std::string eventData("voicemailChange");
    std::string permission("ohos.permission.MANAGE_VOICEMAIL");
    PublishContactEvent(want, eventCode, eventData, permission);
}
} // namespace Contacts
} // namespace OHOS
