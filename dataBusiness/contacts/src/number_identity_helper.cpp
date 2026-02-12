/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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
#include "number_identity_helper.h"

#include "iservice_registry.h"
#include "hilog_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Contacts {
static const std::string NUMBER_LOCATION = "number_location";
static constexpr const char *NUMBER_IDENTITY_URI = "datashare:///com.ohos.numberlocationability";
std::shared_ptr<NumberIdentityHelper> NumberIdentityHelper::instance_ = nullptr;
std::mutex NumberIdentityHelper::identity_mutex;
std::mutex NumberIdentityHelper::helperMutex_;
std::shared_ptr<NumberIdentityHelper> NumberIdentityHelper::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(identity_mutex);
        if (!instance_) {
            instance_ = std::make_shared<NumberIdentityHelper>();
        }
    }
    return instance_;
}

bool NumberIdentityHelper::Query(std::string &numberLocation, DataShare::DataSharePredicates &predicates)
{
    std::lock_guard<std::mutex> lock(helperMutex_);
    if (helper_ == nullptr) {
        helper_ = CreateDataShareHelper(NUMBER_IDENTITY_URI);
        if (helper_ == nullptr) {
            HILOG_ERROR("helper_ is nullptr");
            return false;
        }
    }
    Uri uri(NUMBER_IDENTITY_URI);
    std::string isExactMatchStr = "true";
    std::vector<std::string> columns;
    columns.push_back(isExactMatchStr);
    auto resultSet = helper_->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("resultSet is nullptr");
        return false;
    }
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOG_ERROR("query success, but rowCount is 0");
        resultSet->Close();
        return true;
    }
    resultSet->GoToFirstRow();
    int columnIndex = 0;
    resultSet->GetColumnIndex(NUMBER_LOCATION, columnIndex);
    resultSet->GetString(columnIndex, numberLocation);
    resultSet->Close();
    HILOG_ERROR("QueryNumberLocation end");
    return true;
}

std::shared_ptr<DataShare::DataShareHelper> NumberIdentityHelper::CreateDataShareHelper(const std::string uri)
{
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HILOG_ERROR("Get system ability mgr failed.");
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("GetSystemAbility Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, uri);
}

NumberIdentityHelper::~NumberIdentityHelper()
{
    HILOG_INFO("NumberIdentityHelper  ~NumberIdentityHelper");
    if (helper_) {
        helper_->Release();
        helper_ = nullptr;
    }
}
}
}