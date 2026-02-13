/*
 * Copyright (C) 2023-2023 Huawei Device Co., Ltd.
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

#include <thread>

#include "ability_manager_client.h"
#include "want.h"
#include "string_wrapper.h"
#include "int_wrapper.h"
#include "hilog_wrapper.h"
#include "contact_connect_ability.h"
#include "os_account_manager.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mutex;
}
std::condition_variable ContactConnectAbility::cv_;
std::shared_ptr<ContactConnectAbility> ContactConnectAbility::contactsConnectAbility_ = nullptr;
const int32_t RESULT_OK = 0;

ContactConnectAbility::~ContactConnectAbility()
{}

ContactConnectAbility::ContactConnectAbility()
{}

std::shared_ptr<ContactConnectAbility> ContactConnectAbility::GetInstance()
{
    if (contactsConnectAbility_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mutex);
        if (contactsConnectAbility_ == nullptr) {
            contactsConnectAbility_.reset(new ContactConnectAbility());
        }
    }
    return contactsConnectAbility_;
}

int ContactConnectAbility::ConnectAbility(std::string uuidToInsert, std::string uuidToUpdate, std::string uuidToDelete,
    std::string compareCloudAndRawContact, std::string refreshContact, std::string mergeContactArgs)
{
    int32_t userId = -1;
    OHOS::AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(userId);
    HILOG_INFO("Connect contact ability, userId: = %{public}d, ts = %{public}lld", userId, (long long) time(NULL));
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.contactsdataability", "com.ohos.contactsdataability.ServiceAbility");
    want.SetElement(element);
    AAFwk::WantParams wantParams;

    wantParams.SetParam("insert", AAFwk::String::Box(uuidToInsert));
    wantParams.SetParam("update", AAFwk::String::Box(uuidToUpdate));
    wantParams.SetParam("delete", AAFwk::String::Box(uuidToDelete));
    wantParams.SetParam("compareCloudAndRawContact", AAFwk::String::Box(compareCloudAndRawContact));
    wantParams.SetParam("refreshContact", AAFwk::String::Box(refreshContact));
    wantParams.SetParam("mergeContactArgs", AAFwk::String::Box(mergeContactArgs));
    want.SetParams(wantParams);
    auto result = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, nullptr, userId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (result != RESULT_OK) {
        HILOG_ERROR("Connect contact Serviceability failed.result:%{public}d, %{public}s, %{public}s",
            result, refreshContact.c_str(), mergeContactArgs.c_str());
    }
    return result;
}

void ContactConnectAbility::ConnectBackupAbility()
{
    HILOG_INFO("Connect contact backup ability, ts = %{public}lld", (long long) time(NULL));
    AAFwk::Want want;
    AppExecFwk::ElementName element(
        "", "com.ohos.contactsdataability", "com.ohos.contactsdataability.BackupServiceAbility");
    want.SetElement(element);
    int32_t userId = -1;
    OHOS::AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(userId);
    auto result = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, nullptr, userId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (result != RESULT_OK) {
        HILOG_ERROR("Connect contact Serviceability failed.");
    }
}

void ContactConnectAbility::ConnectRelationChainAbility(std::string operation, std::string jsonData)
{
    HILOG_INFO("Connect relation chain ability, ts = %{public}lld", (long long) time(NULL));
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.contactsdataability",
        "com.ohos.contactsservice");
    want.SetElement(element);

    AAFwk::WantParams wantParams;
    wantParams.SetParam("operation", AAFwk::String::Box(operation));
    wantParams.SetParam("data", AAFwk::String::Box(jsonData));
    want.SetParams(wantParams);
    int32_t userId = -1;
    OHOS::AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(userId);
    auto result = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, nullptr, userId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (result != RESULT_OK) {
        HILOG_ERROR("Connect relation chain ability failed.");
    }
}

/*
 * @param operationType 操作类型，1插入，2更新，3删除
*/
void ContactConnectAbility::ConnectEventsHandleAbility(int operationType, std::string values,
    std::string syncBirthdays)
{
    int32_t userId = -1;
    OHOS::AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(userId);
    HILOG_INFO("Connect Events handle ability, userId: = %{public}d, ts = %{public}lld", userId, (long long) time(NULL));
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.contactsdataability",
        "com.ohos.contactsdataability.HandleContactEventsServiceAbility");
    want.SetElement(element);

    AAFwk::WantParams wantParams;
    wantParams.SetParam("operationType", AAFwk::Integer::Box(operationType));
    wantParams.SetParam("values", AAFwk::String::Box(values));
    wantParams.SetParam("syncBirthdays", AAFwk::String::Box(syncBirthdays));
    want.SetParams(wantParams);
    // 连接js服务
    int result = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, nullptr, userId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (result != RESULT_OK) {
        HILOG_ERROR("Connect birEventsthday handle ability failed.");
    }
}

void ContactConnectAbility::DisconnectAbility()
{
    AAFwk::Want want;
    int32_t userId = -1;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto result =
        AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(want, nullptr, userId, extensionType);
    if (result != RESULT_OK) {
        HILOG_ERROR("Stop Serviceability failed.");
    }
}

}  // namespace Contacts
}  // namespace OHOS