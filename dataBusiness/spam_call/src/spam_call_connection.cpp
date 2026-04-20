/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "spam_call_connection.h"

#include "hilog_wrapper.h"
#include "spam_call_proxy.h"
#include "spam_call_adapter.h"

namespace OHOS {
namespace Contacts {

void SpamCallConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    HILOG_INFO("OnAbilityConnectDone, resultCode = %{public}d", resultCode);
    if (remoteObject == nullptr) {
        HILOG_ERROR("Connected service is invalid!");
        return;
    }
    SpamCallProxy service(remoteObject);
    int32_t ret = service.UpdateBlockedMsgCount(phoneNumbers_, blockType_, spamCallAdapter_);
    HILOG_INFO("OnAbilityConnectDone, UpdateBlockedMsgCount ret = %{public}d", ret);

    std::shared_ptr<SpamCallAdapter> spamCallAdapterPtr_ = std::make_shared<SpamCallAdapter>();
    if (spamCallAdapterPtr_ == nullptr) {
        HILOG_ERROR("create SpamCallAdapter object failed!");
        return;
    }
    spamCallAdapterPtr_->DisconnectSpamCallAbility();
}

void SpamCallConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int32_t resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone, resultCode = %{public}d", resultCode);
}
} // namespace Contacts
} // namespace OHOS
