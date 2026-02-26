/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "contacts_napi_utils.h"
#include "hilog_wrapper_api.h"
#include "ability.h"
#include "ability_context.h"
#include "context.h"
#include "ui_content.h"
#include "ui_extension_context.h"
#include "want.h"

namespace OHOS {
namespace ContactsApi {
static constexpr const char *JS_ERROR_INVALID_INPUT_PARAMETER_STRING =
    "parameter error. Mandatory parameters are left unspecified.";
static constexpr const char *JS_ERROR_VERIFICATION_FAILED_PARAMETER_STRING =
    "parameter error. The type of id must be number.";
static constexpr const char *JS_ERROR_INVALID_PARAMETER_STRING =
    "parameter error. Invalid parameter value.";
static constexpr const char *JS_ERROR_PERMISSION_DENIED_STRING = "Permission denied";
napi_value ContactsNapiUtils::ToInt32Value(napi_env env, int32_t value)
{
    napi_value staticValue = nullptr;
    napi_create_int32(env, value, &staticValue);
    return staticValue;
}

napi_value ContactsNapiUtils::CreateClassConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    return thisArg;
}

bool ContactsNapiUtils::MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

bool ContactsNapiUtils::MatchParameters(
    napi_env env, const napi_value parameters[], std::initializer_list<napi_valuetype> valueTypes)
{
    if (parameters == nullptr) {
        return false;
    }
    int i = 0;
    for (auto beg = valueTypes.begin(); beg != valueTypes.end(); ++beg) {
        if (!MatchValueType(env, parameters[i], *beg)) {
            return false;
        }
        ++i;
    }
    return true;
}

napi_value ContactsNapiUtils::CreateError(napi_env env, int32_t err)
{
    napi_value businessError = nullptr;
    napi_value errorCode = nullptr;
    napi_value errorMessage = nullptr;
    if (err == PERMISSION_ERROR) {
        napi_create_string_utf8(env, JS_ERROR_PERMISSION_DENIED_STRING, NAPI_AUTO_LENGTH, &errorMessage);
    } else if (err == PARAMETER_ERROR) {
        napi_create_string_utf8(env, JS_ERROR_INVALID_INPUT_PARAMETER_STRING, NAPI_AUTO_LENGTH, &errorMessage);
    } else if (err == INVALID_PARAMETER) {
        napi_create_string_utf8(env, JS_ERROR_INVALID_PARAMETER_STRING, NAPI_AUTO_LENGTH, &errorMessage);
    }
    napi_create_int32(env, err, &errorCode);
    napi_create_error(env, nullptr, errorMessage, &businessError);
    napi_set_named_property(env, businessError, "code", errorCode);
    HILOG_INFO("CreateError end");
    return businessError;
}

napi_value ContactsNapiUtils::CreateErrorByVerification(napi_env env, int32_t err)
{
    napi_value businessError = nullptr;
    napi_value errorCode = nullptr;
    napi_value errorMessage = nullptr;
    if (err == PERMISSION_ERROR) {
        napi_create_string_utf8(env, JS_ERROR_PERMISSION_DENIED_STRING, NAPI_AUTO_LENGTH, &errorMessage);
    } else if (err == PARAMETER_ERROR) {
        napi_create_string_utf8(env, JS_ERROR_VERIFICATION_FAILED_PARAMETER_STRING, NAPI_AUTO_LENGTH, &errorMessage);
    } else if (err == INVALID_PARAMETER) {
        napi_create_string_utf8(env, JS_ERROR_INVALID_PARAMETER_STRING, NAPI_AUTO_LENGTH, &errorMessage);
    }
    napi_create_int32(env, err, &errorCode);
    napi_create_error(env, nullptr, errorMessage, &businessError);
    napi_set_named_property(env, businessError, "code", errorCode);
    HILOG_INFO("CreateErrorByVerification end");
    return businessError;
}

OHOS::Ace::UIContent* ContactsNapiUtils::GetUIContent(
    std::unique_ptr<OHOS::ContactsApi::BaseContext>& asyncContext)
{
    OHOS::Ace::UIContent* uiContent = nullptr;
    if (asyncContext->context != nullptr) {
        HILOG_INFO("get uiContext by ability context");
        uiContent = asyncContext->context->GetUIContent();
    } else if (asyncContext->uiExtensionContext != nullptr) {
        HILOG_INFO("get uiContext by uiExtensionContext");
        uiContent = asyncContext->uiExtensionContext->GetUIContent();
    } else {
        HILOG_ERROR("get uiContext failed.");
    }
    HILOG_INFO("ContactsNapiUtils::GetUIContent END");
    return uiContent;
}

napi_status ContactsNapiUtils::GetParamCallback(napi_env env,
    std::unique_ptr<OHOS::ContactsApi::BaseContext> &context, size_t argc, napi_value argv[])
{
    /* Parse the last argument into callbackref if any */
    bool isCallback = false;
    CHECK_STATUS_RET(HasCallback(env, argc, argv, isCallback), "Failed to check callback");
    if (isCallback) {
        CHECK_STATUS_RET(GetParamFunction(env, argv[argc - 1], context->callbackRef),
            "Failed to get callback");
    }
    return napi_ok;
}

void ContactsNapiUtils::InvokeJSAsyncMethod(napi_env env, napi_deferred deferred, napi_ref callbackRef,
    napi_async_work work, const JSAsyncContextOutput &asyncContext)
{
    napi_value retVal;
    napi_value callback = nullptr;

    /* Deferred is used when JS Callback method expects a promise value */
    if (deferred) {
        if (asyncContext.status) {
            napi_resolve_deferred(env, deferred, asyncContext.data);
        } else {
            napi_reject_deferred(env, deferred, asyncContext.error);
        }
    } else {
        napi_value result[ARGS_TWO];
        result[PARAM0] = asyncContext.error;
        result[PARAM1] = asyncContext.data;
        napi_get_reference_value(env, callbackRef, &callback);
        napi_call_function(env, nullptr, callback, ARGS_TWO, result, &retVal);
        napi_delete_reference(env, callbackRef);
    }
    napi_delete_async_work(env, work);
}

napi_value ContactsNapiUtils::NapiCreateAsyncWork(napi_env env,
    std::unique_ptr<ContactsApi::BaseContext> &asyncContext, const std::string &resourceName,
    void (*execute)(napi_env, void *), void (*complete)(napi_env, napi_status, void *))
{
    napi_value result = nullptr;
    napi_value resource = nullptr;
    NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
    NAPI_CREATE_RESOURCE_NAME(env, resource, resourceName.c_str());

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resource, execute, complete,
        static_cast<void *>(asyncContext.get()), &asyncContext->work));
    NAPI_CALL(env, napi_queue_async_work(env, asyncContext->work));
    asyncContext.release();

    return result;
}

napi_status ContactsNapiUtils::HasCallback(napi_env env, const size_t argc, const napi_value argv[],
    bool &isCallback)
{
    isCallback = false;
    if (argc < ARGS_ONE) {
        return napi_ok;
    }
    napi_valuetype valueType = napi_undefined;
    napi_status statusRet = napi_typeof(env, argv[argc - 1], &valueType);
    if (statusRet != napi_ok) {
        return statusRet;
    }
    isCallback = (valueType == napi_function);
    return napi_ok;
}

napi_status ContactsNapiUtils::GetParamFunction(napi_env env, napi_value arg, napi_ref &callbackRef)
{
    napi_valuetype valueType = napi_undefined;
    napi_status statusRet = napi_typeof(env, arg, &valueType);
    if (statusRet != napi_ok) {
        return statusRet;
    }
    CHECK_COND_RET(valueType == napi_function, napi_function_expected, "Type is not as expected function");
    statusRet = napi_create_reference(env, arg, NAPI_INIT_REF_COUNT, &callbackRef);
    if (statusRet != napi_ok) {
        return statusRet;
    }
    return napi_ok;
}

void ContactsNapiUtils::ThrowError(napi_env env, int32_t err, const std::string &errMsg)
{
    std::string message = errMsg;
    if (message.empty()) {
        message = "operation not support";
    }

    HILOG_ERROR("ThrowError errCode:%{public}d errMsg:%{public}s", err, message.c_str());
    NAPI_CALL_RETURN_VOID(env, napi_throw_error(env, std::to_string(err).c_str(), message.c_str()));
}

void ContactsNapiUtils::ThrowError(napi_env env, int32_t err,
    const char *funcName, int32_t line, const std::string &errMsg)
{
    std::string message = errMsg;
    if (message.empty()) {
        message = "operation not support";
    }
    HILOG_ERROR("{%{public}s:%d} ThrowError errCode:%{public}d errMsg:%{public}s", funcName, line,
        err, message.c_str());
    NAPI_CALL_RETURN_VOID(env, napi_throw_error(env, std::to_string(err).c_str(), message.c_str()));
}

ModalUICallback::ModalUICallback(Ace::UIContent* uiContent,
    std::unique_ptr<OHOS::ContactsApi::BaseContext>& baseContext)
{
    this->uiContent = uiContent;
    this->baseContext = baseContext.get();
    this->pickerCallBack_ = baseContext->pickerCallBack.get();
}

void ModalUICallback::SetSessionId(int32_t sessionId)
{
    this->sessionId_=sessionId;
}

void ModalUICallback::OnRelease(int32_t releaseCode)
{
    HILOG_INFO("OnRelease enter. release code is %{public}d", releaseCode);
    this->uiContent->CloseModalUIExtension(this->sessionId_);
    this->pickerCallBack_->ready = true;
}

void ModalUICallback::OnError(int32_t code, const std::string& name, const std::string& message)
{
    HILOG_ERROR("OnError enter. errorCode=%{public}d, name=%{public}s, message=%{public}s",
        code, name.c_str(), message.c_str());
    if (!this->pickerCallBack_->ready) {
        this->uiContent->CloseModalUIExtension(this->sessionId_);
        this->pickerCallBack_->ready = true;
    }
}

void ModalUICallback::OnResultForModal(int32_t resultCode, const OHOS::AAFwk::Want &result)
{
    HILOG_INFO("OnResultForModal enter. resultCode is %{public}d", resultCode);
    this->pickerCallBack_->resultCode = resultCode;
}

void ModalUICallback::OnReceive(const OHOS::AAFwk::WantParams &request)
{
    HILOG_INFO("OnReceive enter.");
    bool isFromSaveContactPicker = request.GetParam("isFromSaveContactPicker");
    if (isFromSaveContactPicker) {
        int32_t contactIndex = request.GetIntParam("contactIndex", -1);
        this->pickerCallBack_->dataIndex = contactIndex;
        this->uiContent->CloseModalUIExtension(this->sessionId_);
        this->pickerCallBack_->ready = true;
        HILOG_WARN("%{public}s isFromSaveContactPicker  contactIndex : %{public}d ",
            __func__, contactIndex);
    } else {
        int32_t tempIndex = request.GetIntParam("index", -1);
        this->pickerCallBack_->dataIndex =
            (this->pickerCallBack_->dataIndex < tempIndex ? tempIndex : this->pickerCallBack_->dataIndex);
        this->pickerCallBack_->total = request.GetIntParam("total", -1);
        if (tempIndex <= (PICKER_MAX - 1) && tempIndex >= 0) {
            this->pickerCallBack_->pickerData[tempIndex] = request.GetStringParam("pickerData");
        }
        HILOG_WARN("%{public}s total : %{public}d index : %{public}d "
            "tempIndex : %{public}d pickerData size %{public}zu",
            __func__, this->pickerCallBack_->total, this->pickerCallBack_->dataIndex, tempIndex,
            sizeof(this->pickerCallBack_->pickerData));
    }
}

void ModalUICallback::OnDestroy()
{
    HILOG_INFO("OnDestroy enter.");
}

} // namespace ContactsApi
} // namespace OHOS
