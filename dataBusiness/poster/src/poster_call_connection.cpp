/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "poster_call_connection.h"

#include "hilog_wrapper.h"
#include "poster_call_proxy.h"
#include "poster_call_adapter.h"

namespace OHOS {
namespace Contacts {

void PosterCallConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    HILOG_INFO("OnAbilityConnectDone, resultCode = %{public}d", resultCode);
    if (remoteObject == nullptr) {
        HILOG_ERROR("Connected service is invalid!");
        return;
    }
    PosterCallProxy service(remoteObject);
    int32_t ret = service.DownLoadPosters(payload_, posterCallAdapter_);
    HILOG_INFO("OnAbilityConnectDone, DownLoadPosters ret = %{public}d", ret);

    std::shared_ptr<PosterCallAdapter> posterCallAdapterPtr_ = std::make_shared<PosterCallAdapter>();
    if (posterCallAdapterPtr_ == nullptr) {
        HILOG_ERROR("create PosterCallAdapter object failed!");
        return;
    }
    posterCallAdapterPtr_->DisconnectPosterCallAbility();
}

void PosterCallConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int32_t resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone, resultCode = %{public}d", resultCode);
}
} // namespace Contacts
} // namespace OHOS
