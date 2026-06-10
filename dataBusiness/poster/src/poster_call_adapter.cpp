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

#include "poster_call_adapter.h"

#include "extension_manager_client.h"
#include "ipc_skeleton.h"
#include "nlohmann/json.hpp"
#include "hilog_wrapper.h"
#include <securec.h>
#include "poster_call_connection.h"

namespace OHOS {
namespace Contacts {
constexpr int32_t DEFAULT_USER_ID = -1;
sptr<PosterCallConnection> posterConnection_ = nullptr;

PosterCallAdapter::PosterCallAdapter() {}

PosterCallAdapter::~PosterCallAdapter()
{
    HILOG_INFO("~PosterCallAdapter");
}

bool PosterCallAdapter::DownLoadPosters(const std::string& payload)
{
    HILOG_INFO("DownLoadPosters start");
    AAFwk::Want want;
    std::string bundleName = "com.ohos.accountcontactsservice";
    std::string abilityName = "ContactsPosterServiceExtAbility";
    want.SetElementName(bundleName, abilityName);
    if (payload == "disconnect") {
        DisconnectPosterCallAbility();
    } else {
        bool connectResult = ConnectPosterCallAbility(want, payload);
        if (!connectResult) {
            HILOG_ERROR("DownLoadPosters failed!");
            return false;
        }
    }
    return true;
}

bool PosterCallAdapter::ConnectPosterCallAbility(const AAFwk::Want &want, const std::string &payload)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    HILOG_INFO("ConnectPosterCallAbility start");
    posterConnection_ = new (std::nothrow) PosterCallConnection(payload, shared_from_this());
    if (posterConnection_ == nullptr) {
        HILOG_ERROR("posterConnection_ is nullptr");
        return false;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto connectResult = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(want,
        posterConnection_, nullptr, DEFAULT_USER_ID);
    IPCSkeleton::SetCallingIdentity(identity);
    if (connectResult != 0) {
	posterConnection_.clear();
        HILOG_ERROR("ConnectServiceExtensionAbility Failed!");
        return false;
    }
    return true;
}

void PosterCallAdapter::DisconnectPosterCallAbility()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    HILOG_INFO("DisconnectPosterCallAbility start");
    if (posterConnection_ == nullptr) {
        HILOG_ERROR("posterConnection_ is nullptr");
        return;
    }
    auto disconnectResult = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(posterConnection_);
    posterConnection_.clear();
    if (disconnectResult != 0) {
        HILOG_ERROR("DisconnectAbility failed! %d", disconnectResult);
    }
}

} // namespace Contacts
} // namespace OHOS
