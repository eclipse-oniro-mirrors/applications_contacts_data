/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "kit_bundle_mgr_helper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "hilog_wrapper.h"
#include "bundle_mgr_interface.h"
#include "bundle_info.h"

namespace OHOS {
namespace ContactsApi {
namespace ContactBundleMgrHelper {

sptr<AppExecFwk::IBundleMgr> GetBundleMgr()
{
    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smgr == nullptr) {
        HILOG_ERROR("ContactBundleMgrHelper GetBundleNameForSelf smgr is nullptr");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("ContactBundleMgrHelper GetBundleNameForSelf remoteObject is nullptr");
        return nullptr;
    }
    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    return bundleMgr;
}

bool GetBundleNameForSelf(std::string &bundleName)
{
    sptr<AppExecFwk::IBundleMgr> bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContactBundleMgrHelper GetBundleNameForSelf bundleMgr is nullptr");
        return false;
    }
    AppExecFwk::BundleInfo bundleInfo;
    int32_t error = bundleMgr->GetBundleInfoForSelf(AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    if (error != ERR_OK) {
        HILOG_ERROR("ContactBundleMgrHelper GetBundleNameForSelf fail, error:%{public}d", error);
        return false;
    }
    bundleName = bundleInfo.name;
    HILOG_INFO("ContactBundleMgrHelper GetBundleNameForSelf bundleName=%{public}s", bundleName.c_str());
    return true;
}

}
} // namespace ContactsApi
} // namespace OHOS