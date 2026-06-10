/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "telephony_permission.h"

#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace Telephony {
using namespace Security::AccessToken;

std::mutex TelephonyPermission::g_mtx;
// 根据permissionName和tokenId，分组管理计数; 每组两个Counter对象：[0]成功数量，[1]失败数量
std::map<std::string, std::vector<std::shared_ptr<Contacts::Counter>>> TelephonyPermission::countGroupMap;

// 根据key，获得本组计数器(key为taskname，permissionName和tokenId信息)
std::vector<std::shared_ptr<Contacts::Counter>>& TelephonyPermission::getCountGroup(std::string &key)
{
    std::unique_lock<std::mutex> locker(g_mtx);
    if (countGroupMap.find(key) == countGroupMap.end()) {
        std::vector<std::shared_ptr<Contacts::Counter>> v;
        v.push_back(std::make_shared<Contacts::Counter>());
        v.push_back(std::make_shared<Contacts::Counter>());
        countGroupMap[key] = std::move(v);
    }
    return countGroupMap[key];
}

/**
 * @brief Permission check by callingUid.
 * @param permissionName permission name.
 * @return Returns true on success, false on failure.
 */
bool TelephonyPermission::CheckPermission(const std::string &permissionName)
{
    if (permissionName.empty()) {
        HILOG_ERROR("permission check failed, permission name is empty.");
        return false;
    }

    auto callerToken = IPCSkeleton::GetCallingTokenID();
    auto callerPid = IPCSkeleton::GetCallingPid();
    HILOG_INFO("callerPid = %{public}d, permission = %{public}s,ts = %{public}lld", callerPid,
               permissionName.c_str(), (long long) time(NULL));
    auto tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    int result = PermissionState::PERMISSION_DENIED;
    if (tokenType == ATokenTypeEnum::TOKEN_NATIVE) {
        result = PermissionState::PERMISSION_GRANTED;
    } else if (tokenType == ATokenTypeEnum::TOKEN_HAP) {
        result = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    } else {
        HILOG_ERROR("permission check failed");
    }

    if (permissionName == Permission::READ_CALL_LOG
        || permissionName == Permission::READ_CONTACTS || permissionName == Permission::WRITE_CONTACTS
        || permissionName == Permission::OHOS_PERMISSION_MANAGE_VOICEMAIL) {
        if (tokenType == ATokenTypeEnum::TOKEN_HAP) {
            bool status = result == PermissionState::PERMISSION_GRANTED;
            int32_t successCount = status ? 1 : 0;
            int32_t failCount = status ? 0 : 1;
            std::string key;
            key.append("calllog-");
            key.append(std::to_string(callerToken));
            key.append("-");
            key.append(permissionName);
            // 根据key获得数量信息，增加数量
            std::vector<std::shared_ptr<Contacts::Counter>>& countGroup = getCountGroup(key);
            countGroup[0]->incrementCount(successCount);
            countGroup[1]->incrementCount(failCount);
            // 创建延迟任务，延迟执行
            std::shared_ptr<Contacts::AddPermissionUsedRecordTask> addPermissionUsedRecordTask =
                std::make_shared<Contacts::AddPermissionUsedRecordTask>(callerToken, callerPid,
                permissionName, *countGroup[0].get(), *countGroup[1].get());
            Contacts::DelayAsyncTask* delayAsyncTask = Contacts::DelayAsyncTask::GetInstanceDelay1S();
            delayAsyncTask->Start();
            // 添加任务，任务相同的任务，通过key去重；相同token和permissionName，只保留一个任务
            delayAsyncTask->put(key, addPermissionUsedRecordTask);
        }
    }

    if (result != PermissionState::PERMISSION_GRANTED) {
        HILOG_ERROR("permission check failed");
        return false;
    }
    return true;
}
} // namespace Telephony
} // namespace OHOS
