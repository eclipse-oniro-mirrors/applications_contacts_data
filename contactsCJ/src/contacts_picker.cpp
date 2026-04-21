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

#include "contacts_picker_ffi.h"
#include <sstream>
#include <cstring>
#include <thread>
#include <atomic>
#include <mutex>
#include "securec.h"
#include "ability_runtime/cj_ability_context.h"
#include "contacts_control.h"
#include "contacts_permission.h"
#include "contacts_utils.h"
#include "hilog_wrapper_api.h"
#include "native/ffi_remote_data.h"
#include "want.h"
#include "want_params.h"
#include "want_params_wrapper.h"
#include "ui_content.h"
#include "modal_ui_extension_config.h"
#include "context.h"
#include "ability_context.h"
#include "ui_extension_context.h"
#include "napi_common_want.h"
#include "nlohmann/json.hpp"

using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::ContactsFfi;
using namespace OHOS::AAFwk;
using FFI::FFIData;

namespace OHOS {
namespace ContactsFfi {

constexpr int SLEEP_TIME = 100;
constexpr int MAX_SELECTABLE_MIN = 1;
constexpr int MAX_SELECTABLE_MAX = 10000;
constexpr int PICKER_MAX = 100;
constexpr int OFFSET_TWO = 2;
constexpr int PICKER_TIMEOUT_MS = 60000;  // 60秒超时
constexpr int INVALID_SESSION_ID = -1;

const std::string CONTACT_PACKAGE_NAME = "com.ohos.contacts";
const std::string CONTACT_UI_EXT_ABILITY_NAME = "ContactUiExtentionAbility";
const std::string UI_EXT_TYPE = "ability.want.params.uiExtensionType";
const std::string CONTACT_UI_EXT_TYPE = "sys/commonUI";
const std::string UI_EXT_TARGETURL = "targetUrl";
const std::string UI_EXT_ISMULTISELECT = "isContactMultiSelect";

struct PickerCallBack {
    std::atomic<bool> ready{false};
    std::atomic<int32_t> resultCode{0};
    std::atomic<int32_t> dataIndex{-1};
    std::atomic<int32_t> total{-1};
    std::string pickerData[PICKER_MAX] = {};
    std::mutex pickerDataMutex;
};

struct ContactsPickerContext {
    std::shared_ptr<PickerCallBack> pickerCallBack;
    int32_t errCode = SUCCESS;
    int32_t sessionId = INVALID_SESSION_ID;
};

static char* StringToCharPtr(const std::string& str)
{
    size_t len = str.length() + 1;
    char* result = static_cast<char*>(malloc(len));
    if (result != nullptr) {
        if (memcpy_s(result, len, str.c_str(), len) != 0) {
            free(result);
            return nullptr;
        }
    }
    return result;
}

static std::string SplicePickerData(std::string *pickerData, int32_t dataIndex)
{
    std::stringstream splicePickerData;
    splicePickerData << "[";
    for (int32_t i = 0; i < (dataIndex + 1); ++i) {
        std::string currentData;
        if (pickerData[i].length() > OFFSET_TWO) {
            currentData = pickerData[i].substr(1, pickerData[i].length() - OFFSET_TWO);
        } else if (pickerData[i].length() == OFFSET_TWO) {
            currentData = "";  // length=2 时 substr(1,0) 返回空字符串
        } else {
            currentData = pickerData[i];  // length < 2 时保持原样或跳过
        }
        if (i > 0 && !currentData.empty()) {
            splicePickerData << ",";
        }
        splicePickerData << currentData;
    }
    splicePickerData << "]";
    HILOG_INFO("[ContactsPickerFFI] SplicePickerData completed");
    return splicePickerData.str();
}

class ModalUICallbackForPicker {
public:
    explicit ModalUICallbackForPicker(
        Ace::UIContent* uiContent,
        std::shared_ptr<ContactsPickerContext> context)
        : uiContent_(uiContent), pickerContext_(context) {}
    
    void OnRelease(int32_t releaseCode)
    {
        HILOG_INFO("[ContactsPickerFFI] OnRelease, releaseCode: %{public}d", releaseCode);
        if (pickerContext_ != nullptr && pickerContext_->pickerCallBack != nullptr) {
            uiContent_->CloseModalUIExtension(sessionId_);
            pickerContext_->pickerCallBack->ready.store(true);
        }
    }
    
    void OnResultForModal(int32_t resultCode, const OHOS::AAFwk::Want& result)
    {
        HILOG_INFO("[ContactsPickerFFI] OnResultForModal, resultCode: %{public}d", resultCode);
        if (pickerContext_ == nullptr || pickerContext_->pickerCallBack == nullptr) {
            HILOG_ERROR("[ContactsPickerFFI] pickerContext or pickerCallBack is null");
            return;
        }
        
        pickerContext_->pickerCallBack->resultCode.store(resultCode);
    }
    
    void OnReceive(const OHOS::AAFwk::WantParams& request)
    {
        HILOG_INFO("[ContactsPickerFFI] OnReceive");
        if (pickerContext_ == nullptr || pickerContext_->pickerCallBack == nullptr) {
            HILOG_ERROR("[ContactsPickerFFI] pickerContext or pickerCallBack is null");
            return;
        }
        
        int32_t tempIndex = request.GetIntParam("index", -1);
        int32_t currentDataIndex = pickerContext_->pickerCallBack->dataIndex.load();
        pickerContext_->pickerCallBack->dataIndex.store((currentDataIndex < tempIndex) ? tempIndex : currentDataIndex);
        pickerContext_->pickerCallBack->total.store(request.GetIntParam("total", -1));
        
        if (tempIndex <= (PICKER_MAX - 1) && tempIndex >= 0) {
            std::string data = request.GetStringParam("pickerData");
            std::lock_guard<std::mutex> lock(pickerContext_->pickerCallBack->pickerDataMutex);
            pickerContext_->pickerCallBack->pickerData[tempIndex] = data;
        }
        
        HILOG_INFO("[ContactsPickerFFI] total: %{public}d, dataIndex: %{public}d",
            pickerContext_->pickerCallBack->total.load(), pickerContext_->pickerCallBack->dataIndex.load());
    }
    
    void OnError(int32_t code, const std::string& name, const std::string& message)
    {
        HILOG_ERROR("[ContactsPickerFFI] OnError, code: %{public}d, name: %{public}s, message: %{public}s",
            code, name.c_str(), message.c_str());
        if (pickerContext_ != nullptr && pickerContext_->pickerCallBack != nullptr) {
            if (!pickerContext_->pickerCallBack->ready.load()) {
                pickerContext_->errCode = ERROR;
                uiContent_->CloseModalUIExtension(sessionId_);
                pickerContext_->pickerCallBack->ready.store(true);
            }
        }
    }
    
    void OnDestroy()
    {
        HILOG_INFO("[ContactsPickerFFI] OnDestroy");
        if (pickerContext_ != nullptr && pickerContext_->pickerCallBack != nullptr) {
            if (!pickerContext_->pickerCallBack->ready.load()) {
                pickerContext_->pickerCallBack->ready.store(true);
            }
        }
    }
    
    void SetSessionId(int32_t sessionId)
    {
        sessionId_ = sessionId;
    }
    
private:
    int32_t sessionId_ = 0;
    Ace::UIContent* uiContent_;
    std::shared_ptr<ContactsPickerContext> pickerContext_;
};

static Ace::UIContent* ValidatePickerContext(int64_t contextId, std::shared_ptr<ContactsPickerContext> pickerContext)
{
    auto context = FFIData::GetData<CJAbilityContext>(contextId);
    if (context == nullptr) {
        HILOG_ERROR("[ContactsPickerFFI] context is null");
        pickerContext->errCode = ERROR;
        pickerContext->pickerCallBack->ready.store(true);
        return nullptr;
    }
    
    auto abilityContext = context->GetAbilityContext();
    if (abilityContext == nullptr) {
        HILOG_ERROR("[ContactsPickerFFI] abilityContext is null");
        pickerContext->errCode = ERROR;
        pickerContext->pickerCallBack->ready.store(true);
        return nullptr;
    }

    auto uiContent = abilityContext->GetUIContent();
    if (uiContent == nullptr) {
        HILOG_ERROR("[ContactsPickerFFI] UIContent is nullptr");
        pickerContext->errCode = ERROR;
        pickerContext->pickerCallBack->ready.store(true);
        return nullptr;
    }
    return uiContent;
}

static void SetupBasicWantParams(Want& request)
{
    request.SetElementName(CONTACT_PACKAGE_NAME, CONTACT_UI_EXT_ABILITY_NAME);
    request.SetParam(UI_EXT_TYPE, CONTACT_UI_EXT_TYPE);
    request.SetParam(UI_EXT_TARGETURL, std::string("BatchSelectContactsPage"));
    request.SetParam("pageFlag", std::string("page_flag_single_choose"));
    request.SetParam(UI_EXT_ISMULTISELECT, false);
}

static void SetupFilterParams(Want& request, const CContactSelectionOptions* options)
{
    if (!options->hasFilter || options->filterJson == nullptr) {
        return;
    }
    
    std::string filterJsonStr(options->filterJson);
    HILOG_INFO("[ContactsPickerFFI] filterJson length: %{public}zu", filterJsonStr.length());
    
    if (!filterJsonStr.empty() && nlohmann::json::accept(filterJsonStr)) {
        nlohmann::json filterJson = nlohmann::json::parse(filterJsonStr);
        AAFwk::WantParams filterWp;
        from_json(filterJson, filterWp);
        HILOG_INFO("[ContactsPickerFFI] filterWp parsed successfully");
        
        AAFwk::WantParams currentParams = request.GetParams();
        currentParams.SetParam("filter", AAFwk::WantParamWrapper::Box(filterWp));
        request.SetParams(currentParams);
        HILOG_INFO("[ContactsPickerFFI] filter set to Want");
    } else {
        HILOG_ERROR("[ContactsPickerFFI] filterJson is empty or invalid JSON");
    }
    
    if (options->displayType != nullptr) {
        std::string displayTypeStr(options->displayType);
        if (!displayTypeStr.empty()) {
            request.SetParam("displayType", displayTypeStr);
            HILOG_INFO("[ContactsPickerFFI] displayType: %{public}s", displayTypeStr.c_str());
        }
    }
}

static bool SetupMaxSelectableParam(Want& request, const CContactSelectionOptions* options,
    std::shared_ptr<ContactsPickerContext> pickerContext)
{
    if (!options->hasMaxSelectable) {
        return true;
    }
    
    int32_t maxSelectable = options->maxSelectable;
    if (maxSelectable < MAX_SELECTABLE_MIN || maxSelectable > MAX_SELECTABLE_MAX) {
        HILOG_ERROR("[ContactsPickerFFI] maxSelectable out of range: %{public}d", maxSelectable);
        pickerContext->errCode = ERROR;
        pickerContext->pickerCallBack->ready.store(true);
        return false;
    }
    request.SetParam("selectLimit", maxSelectable);
    return true;
}

static void SetupOptionsParams(Want& request, const CContactSelectionOptions* options,
    std::shared_ptr<ContactsPickerContext> pickerContext)
{
    if (options == nullptr) {
        return;
    }
    
    if (options->hasIsMultiSelect) {
        request.SetParam(UI_EXT_ISMULTISELECT, options->isMultiSelect);
        std::string pageFlag = options->isMultiSelect ? "page_flag_multi_choose" : "page_flag_single_choose";
        request.SetParam("pageFlag", pageFlag);
    }
    
    if (!SetupMaxSelectableParam(request, options, pickerContext)) {
        return;
    }
    
    SetupFilterParams(request, options);
}

static void StartUIExtensionAbilityForPicker(
    int64_t contextId,
    const CContactSelectionOptions* options,
    std::shared_ptr<ContactsPickerContext> pickerContext)
{
    HILOG_INFO("[ContactsPickerFFI] StartUIExtensionAbilityForPicker begin");
    
    Ace::UIContent* uiContent = ValidatePickerContext(contextId, pickerContext);
    if (uiContent == nullptr) {
        return;
    }
    
    Want request;
    SetupBasicWantParams(request);
    SetupOptionsParams(request, options, pickerContext);
    if (pickerContext->errCode != SUCCESS) {
        return;
    }
    
    request.SetParam("isContactsPicker", true);
    
    pickerContext->pickerCallBack = std::make_shared<PickerCallBack>();
    
    auto callback = std::make_shared<ModalUICallbackForPicker>(uiContent, pickerContext);
    OHOS::Ace::ModalUIExtensionCallbacks extensionCallbacks = {
        std::bind(&ModalUICallbackForPicker::OnRelease, callback, std::placeholders::_1),
        std::bind(&ModalUICallbackForPicker::OnResultForModal, callback, std::placeholders::_1, std::placeholders::_2),
        std::bind(&ModalUICallbackForPicker::OnReceive, callback, std::placeholders::_1),
        std::bind(&ModalUICallbackForPicker::OnError, callback, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3),
    };
    
    OHOS::Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    
    int32_t sessionId = uiContent->CreateModalUIExtension(request, extensionCallbacks, config);
    if (sessionId == INVALID_SESSION_ID) {
        HILOG_ERROR("[ContactsPickerFFI] CreateModalUIExtension failed, sessionId is invalid");
        pickerContext->errCode = ERROR;
        pickerContext->pickerCallBack->ready.store(true);
        return;
    }
    callback->SetSessionId(sessionId);
    pickerContext->sessionId = sessionId;
    
    HILOG_INFO("[ContactsPickerFFI] StartUIExtensionAbilityForPicker end, sessionId: %{public}d", sessionId);
}

static CPickerResult* CreateEmptyPickerResult(int32_t resultCode)
{
    CPickerResult* result = new CPickerResult();
    result->pickerData = StringToCharPtr("[]");
    result->total = 0;
    result->resultCode = resultCode;
    return result;
}

CPickerResult* CJSelectContacts(int64_t contextId, CContactSelectionOptions* options, int32_t* errCode)
{
    HILOG_INFO("[ContactsPickerFFI] CJSelectContacts begin");
    
    *errCode = SUCCESS;
    
    auto pickerContext = std::make_shared<ContactsPickerContext>();
    pickerContext->pickerCallBack = std::make_shared<PickerCallBack>();
    
    StartUIExtensionAbilityForPicker(contextId, options, pickerContext);
    
    if (pickerContext->errCode != SUCCESS) {
        *errCode = pickerContext->errCode;
        return CreateEmptyPickerResult(-1);
    }
    
    int elapsedMs = 0;
    while (!pickerContext->pickerCallBack->ready.load() && elapsedMs < PICKER_TIMEOUT_MS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
        elapsedMs += SLEEP_TIME;
    }
    
    if (!pickerContext->pickerCallBack->ready.load()) {
        HILOG_ERROR("[ContactsPickerFFI] Picker timeout after %{public}d ms", elapsedMs);
        *errCode = ERROR;
        return CreateEmptyPickerResult(-1);
    }
    
    CPickerResult* result = new CPickerResult();
    result->resultCode = pickerContext->pickerCallBack->resultCode.load();
    result->total = pickerContext->pickerCallBack->total.load();
    
    if (pickerContext->pickerCallBack->resultCode.load() != 0) {
        HILOG_INFO("[ContactsPickerFFI] resultCode: %{public}d", pickerContext->pickerCallBack->resultCode.load());
        result->pickerData = StringToCharPtr("[]");
        result->total = 0;
    } else {
        std::string pickerDataStr;
        {
            std::lock_guard<std::mutex> lock(pickerContext->pickerCallBack->pickerDataMutex);
            pickerDataStr = SplicePickerData(
                pickerContext->pickerCallBack->pickerData,
                pickerContext->pickerCallBack->dataIndex.load());
        }
        result->pickerData = StringToCharPtr(pickerDataStr);
    }
    
    HILOG_INFO("[ContactsPickerFFI] CJSelectContacts end, total: %{public}d", result->total);
    return result;
}

void CJFreePickerResult(CPickerResult* result)
{
    if (result != nullptr) {
        if (result->pickerData != nullptr) {
            free(result->pickerData);
            result->pickerData = nullptr;
        }
        delete result;
    }
}

} // namespace ContactsFfi
} // namespace OHOS
