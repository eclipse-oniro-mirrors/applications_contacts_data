/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef CONTACT_NAPI_UTILS_H
#define CONTACT_NAPI_UTILS_H

#include "napi/native_node_api.h"

#include "contacts_napi_common.h"
#include "ui_extension_context.h"
#include "hilog_wrapper_api.h"

#define CHECK_STATUS_RET(cond, message)                             \
    do {                                                            \
        napi_status __ret = (cond);                                 \
        if (__ret != napi_ok) {                                     \
            HILOG_ERROR(message);                                   \
            return __ret;                                           \
        }                                                           \
    } while (0)

#define CHECK_COND_RET(cond, ret, message, ...)                     \
    do {                                                            \
        if (!(cond)) {                                              \
            HILOG_ERROR(message, ##__VA_ARGS__);                    \
            return ret;                                             \
        }                                                           \
    } while (0)

#define CHECK_ARGS_BASE(env, cond, err, retVal)                     \
    do {                                                            \
        if ((cond) != napi_ok) {                                    \
            ContactsNapiUtils::ThrowError(env, err, __FUNCTION__, __LINE__); \
            return retVal;                                          \
        }                                                           \
    } while (0)

#define GET_AND_THROW_LAST_ERROR(env)                                                                   \
    do {                                                                                                \
        const napi_extended_error_info* errorInfo = nullptr;                                            \
        napi_get_last_error_info((env), &errorInfo);                                                    \
        bool isPending = false;                                                                         \
        napi_is_exception_pending((env), &isPending);                                                   \
        if (!isPending && errorInfo != nullptr) {                                                       \
            const char* errorMessage =                                                                  \
                errorInfo->error_message != nullptr ? errorInfo->error_message : "empty error message"; \
            napi_throw_error((env), nullptr, errorMessage);                                             \
        }                                                                                               \
    } while (0)

#define NAPI_CALL_BASE(env, theCall, retVal) \
    do {                                     \
        if ((theCall) != napi_ok) {          \
            GET_AND_THROW_LAST_ERROR((env)); \
            return retVal;                   \
        }                                    \
    } while (0)

#define NAPI_CALL(env, theCall) NAPI_CALL_BASE(env, theCall, nullptr)

#define CHECK_ARGS(env, cond, err) CHECK_ARGS_BASE(env, cond, err, nullptr)

#define CHECK_ARGS_RET_VOID(env, cond, err) CHECK_ARGS_BASE(env, cond, err, NAPI_RETVAL_NOTHING)

#define NAPI_CALL_RETURN_VOID(env, theCall) NAPI_CALL_BASE(env, theCall, NAPI_RETVAL_NOTHING)

#define NAPI_CREATE_PROMISE(env, callbackRef, deferred, result)     \
    do {                                                            \
        if ((callbackRef) == nullptr) {                             \
            napi_create_promise(env, &(deferred), &(result));       \
        }                                                           \
    } while (0)

#define NAPI_CREATE_RESOURCE_NAME(env, resource, resourceName)             \
    do {                                                                            \
        napi_create_string_utf8(env, resourceName, NAPI_AUTO_LENGTH, &(resource));  \
    } while (0)

#define CHECK_NULL_PTR_RETURN_VOID(ptr, message)   \
    do {                                           \
        if ((ptr) == nullptr) {                    \
            HILOG_INFO(message);          \
            return;                                \
        }                                          \
    } while (0)

#define CHECK_STATUS_RET(cond, message)                             \
    do {                                                            \
        napi_status __ret = (cond);                                 \
        if (__ret != napi_ok) {                                     \
            HILOG_ERROR(message);                                   \
            return __ret;                                           \
        }                                                           \
    } while (0)

/* Constants for array index */
const int32_t PARAM0 = 0;
const int32_t PARAM1 = 1;

constexpr uint32_t NAPI_INIT_REF_COUNT = 1;

/* error code */
constexpr int32_t JS_ERR_PARAMETER_INVALID = 101;        // input parameter invalid
constexpr int32_t JS_INNER_FAIL            = 201;

/* Picker Optional Number of People */
constexpr int32_t PICKER_MAX = 100; // picker多选上限10000人，每次传输100人
constexpr int32_t OFFSET_TWO = 2;

namespace OHOS {
namespace ContactsApi {
#define EXPORT __attribute__ ((visibility ("default")))

/**
 * contact NAPI utility class.
 */
constexpr const char* CONTACT_UI_EXT_ABILITY_NAME = "ContactUiExtentionAbility";
constexpr const char* CONTACT_PACKAGE_NAME = "com.ohos.contacts";
constexpr const char* UI_EXT_TYPE = "ability.want.params.uiExtensionType";
constexpr const char* CONTACT_UI_EXT_TYPE = "sys/commonUI";
constexpr const char* UI_EXT_TARGETURL = "targetUrl";
constexpr const char* UI_EXT_ISMULTISELECT = "isContactMultiSelect";

struct JSAsyncContextOutput {
    napi_value error;
    napi_value data;
    bool status;
};

struct PickerCallBack {
    bool ready = false;
    int32_t resultCode;
    int32_t dataIndex = -1;
    int32_t total = -1;
    std::string pickerData[PICKER_MAX] = {};
};

struct BaseContext {
    napi_async_work work;
    napi_ref callbackRef;
    napi_env env = nullptr;
    napi_deferred deferred = nullptr;
    OHOS::ErrCode errCode = OHOS::ERR_OK;
    std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> context = nullptr;
    std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> uiExtensionContext = nullptr;
    std::string errorMessage;
    std::shared_ptr<OHOS::ContactsApi::PickerCallBack> pickerCallBack = nullptr;

    ~BaseContext()
    {
        HILOG_INFO("release enter");
        if (this->callbackRef != nullptr) {
            napi_delete_reference(this->env, this->callbackRef);
            this->callbackRef = nullptr;
            HILOG_INFO("release callbackRef");
        }

        if (this->deferred != nullptr) {
            this->deferred = nullptr;
            HILOG_INFO("release deferred");
        }
        HILOG_INFO("release exit");
    }
};

class ContactsNapiUtils {
public:
    static napi_value ToInt32Value(napi_env env, int32_t value);
    static napi_value CreateClassConstructor(napi_env env, napi_callback_info info);
    static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType);
    static bool MatchParameters(
        napi_env env, const napi_value parameters[], std::initializer_list<napi_valuetype> valueTypes);
    static napi_value CreateError(napi_env, int32_t errorCode);
    static napi_value CreateErrorByVerification(napi_env, int32_t errorCode);
    static napi_status HasCallback(
        napi_env env, const size_t argc, const napi_value argv[], bool &isCallback);
    static napi_status GetParamFunction(napi_env env, napi_value arg, napi_ref &callbackRef);
    static OHOS::Ace::UIContent* GetUIContent(std::unique_ptr<OHOS::ContactsApi::BaseContext>& asyncContext);
    static void InvokeJSAsyncMethod(napi_env env, napi_deferred deferred, napi_ref callbackRef,
        napi_async_work work, const JSAsyncContextOutput &asyncContext);
    static napi_value NapiCreateAsyncWork(napi_env env,
        std::unique_ptr<ContactsApi::BaseContext> &asyncContext, const std::string &resourceName,
        void (*execute)(napi_env, void *), void (*complete)(napi_env, napi_status, void *));
    EXPORT static void ThrowError(napi_env env, int32_t err, const std::string &errMsg = "");
    EXPORT static void ThrowError(napi_env env, int32_t err, const char *func, int32_t line,
        const std::string &errMsg = "");
    static napi_status GetParamCallback(napi_env env,
        std::unique_ptr<OHOS::ContactsApi::BaseContext> &context, size_t argc, napi_value argv[]);
};

class ModalUICallback {
public:
    explicit ModalUICallback(Ace::UIContent* uiContent, std::unique_ptr<OHOS::ContactsApi::BaseContext>& baseContext);
    void OnRelease(int32_t releaseCode);
    void OnResultForModal(int32_t resultCode, const OHOS::AAFwk::Want& result);
    void OnReceive(const OHOS::AAFwk::WantParams& request);
    void OnError(int32_t code, const std::string& name, const std::string& message);
    void OnDestroy();
    void SetSessionId(int32_t sessionId);
private:
    int32_t sessionId_ = 0;
    OHOS::ContactsApi::BaseContext* baseContext;
    OHOS::ContactsApi::PickerCallBack* pickerCallBack_;
    Ace::UIContent* uiContent;
};

} // namespace ContactsApi
} // namespace OHOS

#endif