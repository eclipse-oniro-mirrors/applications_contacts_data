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

#include "spam_call_adapter.h"

#include "extension_manager_client.h"
#include "ipc_skeleton.h"
#include "nlohmann/json.hpp"
#include "hilog_wrapper.h"
#include <securec.h>
#include "spam_call_connection.h"

namespace OHOS {
namespace Contacts {
constexpr int32_t DEFAULT_USER_ID = -1;
sptr<SpamCallConnection> connection_ = nullptr;

SpamCallAdapter::SpamCallAdapter() {}

SpamCallAdapter::~SpamCallAdapter()
{
    HILOG_INFO("~SpamCallAdapter");
}

bool SpamCallAdapter::UpdateBlockedMsgCount(const std::vector<std::string> &phoneNumbers, int blockType)
{
    HILOG_INFO("UpdateBlockedMsgCount start");
    AAFwk::Want want;
    std::string bundleName = "com.ohos.mms";
    std::string abilityName = "BlockedContactServiceExtAbility";
    want.SetElementName(bundleName, abilityName);
    bool connectResult = ConnectSpamCallAbility(want, phoneNumbers, blockType);
    if (!connectResult) {
        HILOG_ERROR("UpdateBlockedMsgCount failed!");
        return false;
    }
    return true;
}

bool SpamCallAdapter::ConnectSpamCallAbility(const AAFwk::Want &want, const std::vector<std::string> &phoneNumbers,
    int blockType)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    HILOG_INFO("ConnectSpamCallAbility start");
    connection_ = new (std::nothrow) SpamCallConnection(phoneNumbers, blockType, shared_from_this());
    if (connection_ == nullptr) {
        HILOG_ERROR("connection_ is nullptr");
        return false;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto connectResult = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(want,
        connection_, nullptr, DEFAULT_USER_ID);
    IPCSkeleton::SetCallingIdentity(identity);
    if (connectResult != 0) {
        HILOG_ERROR("ConnectServiceExtensionAbility Failed!");
        return false;
    }
    return true;
}

void SpamCallAdapter::DisconnectSpamCallAbility()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    HILOG_INFO("DisconnectSpamCallAbility start");
    if (connection_ == nullptr) {
        HILOG_ERROR("connection_ is nullptr");
        return;
    }
    auto disconnectResult = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(connection_);
    connection_.clear();
    if (disconnectResult != 0) {
        HILOG_ERROR("DisconnectAbility failed! %d", disconnectResult);
    }
}

} // namespace Contacts
} // namespace OHOS
