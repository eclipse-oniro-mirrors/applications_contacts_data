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

#include "contacts_sync_api.h"

#include <mutex>
#include <chrono>
#include <unistd.h>

#include "contacts_api.h"
#include "contacts_napi_common.h"
#include "contacts_napi_utils.h"
#include "hilog_wrapper_api.h"
#include "result_convert.h"
#include "sync_json_helper.h"
#include "../../ability/common/include/contacts_columns.h"
#include "ability.h"
#include "ability_context.h"
#include "context.h"
#include "ui_content.h"
#include "ui_extension_context.h"
#include "napi_common_want.h"
#include "modal_ui_extension_config.h"
#include "window.h"
#include "want.h"
#include "kit_bundle_mgr_helper.h"
#include "app_mgr_client.h"
#ifdef CONTACT_API_METRICS_ENABLE
#include "contacts_metric_utils.h"
#include "histogram_plugin_macros.h"
#endif

namespace OHOS {
namespace ContactsApi {

/**
 * @brief Validate SyncContacts parameters
 */
static bool ValidateSyncContactsParams(napi_env env, size_t argc, napi_value *argv)
{
    napi_value errorCode = ContactsNapiUtils::CreateError(env, INVALID_PARAMETER);
    switch (argc) {
        case ARGS_FOUR:
            if (!ContactsNapiUtils::MatchParameters(env, argv,
                {
                    napi_object,
                    napi_number,
                    napi_object,
                    napi_object
                })) {
                HILOG_ERROR("ValidateSyncContactsParams argc=4 invalid");
                napi_throw(env, errorCode);
                return false;
            }
            break;
        case ARGS_FIVE:
            if (!ContactsNapiUtils::MatchParameters(env, argv,
                {
                    napi_object,
                    napi_number,
                    napi_object,
                    napi_object,
                    napi_function
                })) {
                HILOG_ERROR("ValidateSyncContactsParams argc=5 invalid");
                napi_throw(env, errorCode);
                return false;
            }
            break;
        default:
            HILOG_ERROR("ValidateSyncContactsParams invalid argc=%{public}zu", argc);
            napi_throw(env, errorCode);
            return false;
    }
    return true;
}

/**
 * @brief Parse sync mode and progress parameters from JS object
 */
static void ParseSyncContactsParams(napi_env env, ExecuteHelper *executeHelper,
    size_t argc, napi_value *argv, bool isStageMode)
{
    if (isStageMode) {
        executeHelper->abilityContext = argv[0];
    }
    ContactsBuild contactsBuild;
    int modeIndex = isStageMode ? 1 : 0;
    if (argc > static_cast<size_t>(modeIndex) && argv[modeIndex] != nullptr) {
        executeHelper->syncMode = contactsBuild.GetInt(env, argv[modeIndex]);
    }
    int progressIndex = modeIndex + 1;
    if (argc > static_cast<size_t>(progressIndex) && argv[progressIndex] != nullptr) {
        napi_value progressObj = argv[progressIndex];
        napi_value propValue = nullptr;
        if (napi_get_named_property(env, progressObj, "syncId", &propValue) == napi_ok && propValue != nullptr) {
            executeHelper->syncId = contactsBuild.GetInt(env, propValue);
        }
        propValue = nullptr;
        if (napi_get_named_property(env, progressObj, "currentBatch", &propValue) == napi_ok && propValue != nullptr) {
            executeHelper->currentBatch = contactsBuild.GetInt(env, propValue);
        }
        propValue = nullptr;
        if (napi_get_named_property(env, progressObj, "totalBatches", &propValue) == napi_ok && propValue != nullptr) {
            executeHelper->totalBatches = contactsBuild.GetInt(env, propValue);
        }
    }
}

/**
 * @brief Sync call parameters from NAPI callback
 */
struct SyncCallParams {
    size_t argc;
    napi_value *argv;
    bool isStageMode;
};

/**
 * @brief Detect sync call type (callback or promise) from last argument
 */
static void DetectSyncCallType(napi_env env, ExecuteHelper *executeHelper, napi_value lastArg)
{
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, lastArg, &valuetype);
    executeHelper->sync = (valuetype == napi_function) ? NAPI_CALL_TYPE_CALLBACK : NAPI_CALL_TYPE_PROMISE;
}

/**
 * @brief Copy argv parameters to ExecuteHelper, adjusting for stage/FA mode
 */
static void CopySyncParamsToHelper(ExecuteHelper *executeHelper, const SyncCallParams &params)
{
    executeHelper->argc = params.argc;
    if (params.isStageMode) {
        for (int i = 1; i < MAX_PARAMS; i++) {
            executeHelper->argv[i - 1] = params.argv[i];
        }
        executeHelper->argc -= 1;
    } else {
        for (int i = 0; i < MAX_PARAMS; i++) {
            executeHelper->argv[i] = params.argv[i];
        }
    }
}

/**
 * @brief Initialize context for first batch sync dialog
 */
static void InitFirstBatchSyncContext(napi_env env, napi_callback_info info,
    ExecuteHelper *executeHelper, const SyncCallParams &params)
{
    GetDataShareHelper(env, info, executeHelper);
    CopySyncParamsToHelper(executeHelper, params);
    DetectSyncCallType(env, executeHelper, params.argv[params.argc - 1]);
    executeHelper->isFirstSync = true;
    SetChildActionCodeAndConvertParams(env, executeHelper);
}

/**
 * @brief Prepare first batch sync with confirmation dialog
 */
static napi_value PrepareFirstBatchSync(napi_env env, napi_callback_info info,
    ExecuteHelper *executeHelper, const SyncCallParams &params)
{
    InitFirstBatchSyncContext(env, info, executeHelper, params);
    napi_deferred deferred;
    napi_value result = nullptr;
    napi_create_promise(env, &deferred, &result);
    TriggerSyncConfirmDialog(env, executeHelper, deferred);
    return result;
}

/**
 * @brief Check if sync confirmation record exists for current syncId
 */
static bool CheckIsExistSyncConfirmation(ExecuteHelper *executeHelper)
{
    if (executeHelper->dataShareHelper == nullptr) {
        return false;
    }
    DataShare::DataSharePredicates checkPredicates;
    checkPredicates.EqualTo("bundle_name", executeHelper->callerBundleName);
    checkPredicates.EqualTo("sync_id", executeHelper->syncId);
    std::vector<std::string> checkColumns;
    Uri checkUri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
    auto checkResult = executeHelper->dataShareHelper->Query(checkUri, checkPredicates, checkColumns);
    if (checkResult == nullptr) {
        return false;
    }
    int rowCount = 0;
    checkResult->GetRowCount(rowCount);
    HILOG_INFO("CheckIsExistSyncConfirmation rowCount=%{public}d", rowCount);
    return rowCount != 0;
}

/**
 * @brief Parsed sync confirmation record from database
 */
struct SyncConfirmationRecord {
    std::string completedBatchesStr;
    int insertStatus = 0;
    int userConfirmResult = 0;
};

/**
 * @brief Query and parse sync confirmation record from database
 */
static bool QuerySyncConfirmationRecord(ExecuteHelper *executeHelper, SyncConfirmationRecord &record)
{
    if (executeHelper->dataShareHelper == nullptr) {
        return false;
    }
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("bundle_name", executeHelper->callerBundleName);
    predicates.EqualTo("sync_id", executeHelper->syncId);
    std::vector<std::string> columns;
    columns.push_back("completed_batches");
    columns.push_back("insert_status");
    columns.push_back("user_confirm_result");
    Uri uri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
    auto result = executeHelper->dataShareHelper->Query(uri, predicates, columns);
    if (result == nullptr) {
        HILOG_ERROR("QuerySyncConfirmationRecord query failed, syncId=%{public}d", executeHelper->syncId);
        return false;
    }
    int rowCount = 0;
    result->GetRowCount(rowCount);
    if (rowCount == 0) {
        result->Close();
        HILOG_ERROR("QuerySyncConfirmationRecord no record, syncId=%{public}d", executeHelper->syncId);
        return false;
    }
    result->GoToFirstRow();
    int idx = 0;
    if (result->GetColumnIndex("completed_batches", idx) == 0) {
        result->GetString(idx, record.completedBatchesStr);
    }
    if (result->GetColumnIndex("insert_status", idx) == 0) {
        result->GetInt(idx, record.insertStatus);
    }
    if (result->GetColumnIndex("user_confirm_result", idx) == 0) {
        result->GetInt(idx, record.userConfirmResult);
    }
    result->Close();
    return true;
}

/**
 * @brief Check if sync has been confirmed and current batch is valid
 * Checks: insert_status (previous batch failed?) and completed_batches (current batch already done?)
 */
static bool CheckSyncConfirmation(ExecuteHelper *executeHelper)
{
    SyncConfirmationRecord record;
    if (!QuerySyncConfirmationRecord(executeHelper, record)) {
        return false;
    }
    HILOG_INFO("CheckSyncConfirmation syncId=%{public}d, batch=%{public}d, "
        "completed_batches=%{public}s, insert_status=%{public}d, userConfirmResult=%{public}d",
        executeHelper->syncId, executeHelper->currentBatch, record.completedBatchesStr.c_str(),
        record.insertStatus, record.userConfirmResult);
    if (record.insertStatus == BATCH_INSERT_FAILED) {
        HILOG_ERROR("CheckSyncConfirmation previous batch failed, syncId=%{public}d", executeHelper->syncId);
        return false;
    }
    bool confirmed = record.userConfirmResult == CONFIRM_RESULT_YES;
    if (!record.completedBatchesStr.empty()) {
        std::vector<int> completed = ParseCompletedBatches(record.completedBatchesStr);
        for (int batch : completed) {
            if (batch == executeHelper->currentBatch) {
                HILOG_INFO("CheckSyncConfirmation batch %{public}d already completed, skipping",
                    executeHelper->currentBatch);
                return confirmed;
            }
        }
    }
    return confirmed;
}

/**
 * @brief Reject sync with error and clean up helper
 */
static napi_value RejectSyncWithError(napi_env env, ExecuteHelper *executeHelper, int errorCode)
{
    executeHelper->resultData = errorCode;
    napi_value error;
    HandleSyncContactsErrorCode(env, executeHelper, error);
    delete executeHelper;
    napi_throw(env, error);
    return error;
}

napi_value SyncContacts(napi_env env, napi_callback_info info)
{
#ifdef CONTACT_API_METRICS_ENABLE
    auto beginTime = std::chrono::system_clock::now();
    HISTOGRAM_BOOLEAN("ContactsKit.SyncContacts.ApiCall", 1);
#endif
    HILOG_INFO("SyncContacts start");
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    napi_value result = nullptr;

    if (isStageMode && !ValidateSyncContactsParams(env, argc, argv)) {
        return result;
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    executeHelper->actionCode = SYNC_CONTACTS;
    if (executeHelper == nullptr) {
        napi_create_int64(env, ERROR, &result);
        return result;
    }

    ParseSyncContactsParams(env, executeHelper, argc, argv, isStageMode);
    HILOG_INFO("SyncContacts mode=%{public}d, syncId=%{public}d, batch=%{public}d/%{public}d",
        executeHelper->syncMode, executeHelper->syncId, executeHelper->currentBatch, executeHelper->totalBatches);
    if (!IsCurAppForeground()) {
        return RejectSyncWithError(env, executeHelper, ERROR_BACKGROUND_CALL);
    }
    ContactBundleMgrHelper::GetBundleNameForSelf(executeHelper->callerBundleName);
    GetDataShareHelper(env, info, executeHelper);
    if (!CheckIsExistSyncConfirmation(executeHelper)) {
#ifdef CONTACT_API_METRICS_ENABLE
        executeHelper->beginTime = beginTime;
#endif
        SyncCallParams params = { argc, argv, isStageMode };
        return PrepareFirstBatchSync(env, info, executeHelper, params);
    }
    if (!CheckSyncConfirmation(executeHelper)) {
        afterExecuteSyncContacts(env, executeHelper, false);
        HILOG_ERROR("SyncContacts syncId=%{public}d not confirmed", executeHelper->syncId);
        return RejectSyncWithError(env, executeHelper, ERROR_USER_CANCEL);
    }
#ifdef CONTACT_API_METRICS_ENABLE
    executeHelper->beginTime = beginTime;
#endif
    return Scheduling(env, info, executeHelper, SYNC_CONTACTS);
}

bool IsCurAppForeground()
{
    HILOG_INFO("IsCurAppForeground start");
    OHOS::AppExecFwk::AppMgrClient appMgrClient;
    std::vector<OHOS::AppExecFwk::RunningProcessInfo> runningProcessInfos;
    int32_t ret = appMgrClient.GetAllRunningProcesses(runningProcessInfos);
    if (ret != 0) {
        HILOG_ERROR("IsCurAppForeground GetAllRunningProcesses failed, ret=%{public}d", ret);
        return false;
    }

    int32_t currentPid = getpid();
    HILOG_INFO("IsCurAppForeground currentPid=%{public}d, processCount=%{public}zu",
        currentPid, runningProcessInfos.size());

    for (const auto &processInfo : runningProcessInfos) {
        HILOG_INFO("IsCurAppForeground process pid=%{public}d, bundle=%{public}s, state=%{public}d",
            processInfo.pid_, processInfo.bundleNames.empty() ? "none" : processInfo.bundleNames[0].c_str(),
            static_cast<int>(processInfo.state_));

        if (processInfo.pid_ == currentPid) {
            if (processInfo.state_ == OHOS::AppExecFwk::AppProcessState::APP_STATE_FOREGROUND ||
                processInfo.state_ == OHOS::AppExecFwk::AppProcessState::APP_STATE_FOCUS) {
                HILOG_INFO("IsCurAppForeground: YES");
                return true;
            } else {
                HILOG_INFO("IsCurAppForeground: NO, state=%{public}d",
                    static_cast<int>(processInfo.state_));
                return false;
            }
        }
    }
    HILOG_WARN("IsCurAppForeground: currentPid not found");
    return false;
}

/**
 * @brief Execute callback for sync dialog - runs in work thread, waits for dialog result
 */
static void SyncDialogExecute(napi_env env, void *data)
{
    auto *syncCtx = static_cast<SyncDialogContext *>(data);
    if (syncCtx == nullptr || syncCtx->callback == nullptr) {
        HILOG_ERROR("SyncDialogExecute syncCtx or callback is nullptr");
        return;
    }

    HILOG_INFO("SyncDialogExecute wait for dialog result");

    std::unique_lock<std::mutex> lock(syncCtx->callback->mutex);
    syncCtx->callback->cv.wait(lock, [syncCtx]() {
        return syncCtx->callback->ready;
    });

    HILOG_INFO("SyncDialogExecute dialog closed, confirmResult=%{public}d", syncCtx->callback->confirmResult);
}

/**
 * @brief Handle user confirmed sync dialog
 */
static void HandleSyncDialogConfirmed(napi_env env, SyncDialogContext *syncCtx)
{
    syncCtx->helper->confirmResult = CONFIRM_RESULT_YES;
    syncCtx->helper->deferred = syncCtx->callback->deferred;
    if (syncCtx->helper->actionCode != SYNC_CONTACTS) {
        HILOG_WARN("HandleSyncDialogConfirmed: fixing actionCode from %{public}d to SYNC_CONTACTS",
            syncCtx->helper->actionCode);
        syncCtx->helper->actionCode = SYNC_CONTACTS;
    }
    HILOG_INFO("HandleSyncDialogConfirmed creating async work");
    CreateAsyncWork(env, syncCtx->helper);
}

/**
 * @brief Handle user cancelled sync dialog
 */
static void HandleSyncDialogCancelled(napi_env env, SyncDialogContext *syncCtx)
{
    HILOG_INFO("HandleSyncDialogCancelled");
    afterExecuteSyncContacts(env, syncCtx->helper, false);
    napi_value error = ContactsNapiUtils::CreateError(env, ERROR_USER_CANCEL);
    napi_reject_deferred(env, syncCtx->callback->deferred, error);
    // Do NOT delete syncCtx->helper here. The async work created by TriggerSyncConfirmDialog
    // holds a pointer to executeHelper, and ExecuteDone will delete it when the async work
    // completes. Deleting here would cause a double-free / use-after-free.
}


/**
 * @brief Complete callback for sync dialog - runs in JS main thread
 */
static void SyncDialogComplete(napi_env env, napi_status status, void *data)
{
    auto *syncCtx = static_cast<SyncDialogContext *>(data);
    HILOG_INFO("SyncDialogComplete confirmResult=%{public}d", syncCtx->callback->confirmResult);

    if (syncCtx->callback->confirmResult == CONFIRM_RESULT_YES) {
        HandleSyncDialogConfirmed(env, syncCtx);
        return;
    }
    HandleSyncDialogCancelled(env, syncCtx);
    delete syncCtx->callback;
    delete syncCtx;
}

/**
 * @brief Build Want for sync confirmation dialog
 */
static OHOS::AAFwk::Want BuildSyncConfirmWant(ExecuteHelper *executeHelper)
{
    OHOS::AAFwk::Want want;
    want.SetElementName("com.ohos.contacts", "SyncContactsExtAbility");
    want.SetParam("syncMode", executeHelper->syncMode);
    want.SetParam("syncId", executeHelper->syncId);
    want.SetParam("bundleName", executeHelper->callerBundleName);
    want.SetParam("ability.want.params.uiExtensionType", std::string("sys/commonUI"));
    want.SetParam("targetUrl", std::string("SyncContactsDialog"));
    HILOG_INFO("BuildSyncConfirmWant bundleName=%{public}s, syncId=%{public}d, syncMode=%{public}d",
        executeHelper->callerBundleName.c_str(), executeHelper->syncId, executeHelper->syncMode);
    return want;
}

/**
 * @brief Get UIContent from ability context for dialog
 */
static OHOS::Ace::UIContent *GetSyncDialogUIContent(napi_env env, ExecuteHelper *executeHelper,
    napi_deferred deferred)
{
    auto asyncContext = std::make_unique<OHOS::ContactsApi::BaseContext>();
    asyncContext->env = env;
    napi_value contextValue = executeHelper->abilityContext;
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, contextValue, isStageMode);
    if (isStageMode) {
        auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextValue);
        if (context != nullptr) {
            asyncContext->context =
                OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::AbilityContext>(context);
            if (asyncContext->context == nullptr) {
                HILOG_INFO("GetSyncDialogUIContent: AbilityContext is nullptr, try UIExtensionContext");
                asyncContext->uiExtensionContext =
                    OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::UIExtensionContext>(context);
            }
        }
    }
    if (asyncContext->context == nullptr && asyncContext->uiExtensionContext == nullptr) {
        HILOG_ERROR("GetSyncDialogUIContent: context is nullptr");
        napi_value error;
        executeHelper->resultData = CONTACT_GENERAL_ERROR;
        HandleSyncContactsErrorCode(env, executeHelper, error);
        napi_reject_deferred(env, deferred, error);
        return nullptr;
    }
    auto uiContent = ContactsNapiUtils::GetUIContent(asyncContext);
    if (uiContent == nullptr) {
        HILOG_ERROR("GetSyncDialogUIContent UIContent is nullptr");
        napi_value error;
        executeHelper->resultData = CONTACT_GENERAL_ERROR;
        HandleSyncContactsErrorCode(env, executeHelper, error);
        napi_reject_deferred(env, deferred, error);
        return nullptr;
    }
    return uiContent;
}

/**
 * @brief Reject deferred promise with sync dialog error
 */
static void RejectSyncDialogError(napi_env env, ExecuteHelper *executeHelper, napi_deferred deferred)
{
    napi_value error;
    executeHelper->resultData = CONTACT_GENERAL_ERROR;
    HandleSyncContactsErrorCode(env, executeHelper, error);
    napi_reject_deferred(env, deferred, error);
}

/**
 * @brief Create modal UI extension for sync confirmation dialog
 */
static int32_t CreateSyncModalUI(OHOS::Ace::UIContent *uiContent, const OHOS::AAFwk::Want &want,
    const std::shared_ptr<SyncModalCallback> &callback)
{
    OHOS::Ace::ModalUIExtensionCallbacks extensionCallbacks = {
        std::bind(&SyncModalCallback::OnRelease, callback, std::placeholders::_1),
        std::bind(&SyncModalCallback::OnResultForModal, callback,
            std::placeholders::_1, std::placeholders::_2),
        std::bind(&SyncModalCallback::OnReceive, callback, std::placeholders::_1),
        std::bind(&SyncModalCallback::OnError, callback,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
    };
    OHOS::Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, extensionCallbacks, config);
    callback->SetSessionId(sessionId);
    return sessionId;
}

/**
 * @brief Trigger sync confirmation dialog via CreateModalUIExtension
 */
void TriggerSyncConfirmDialog(napi_env env, ExecuteHelper *executeHelper, napi_deferred deferred)
{
    HILOG_INFO("TriggerSyncConfirmDialog start");
    OHOS::AAFwk::Want want = BuildSyncConfirmWant(executeHelper);

    auto *uiContent = GetSyncDialogUIContent(env, executeHelper, deferred);
    if (uiContent == nullptr) {
        return;
    }

    auto *syncDialogCallback = new (std::nothrow) SyncDialogCallback();
    if (syncDialogCallback == nullptr) {
        HILOG_ERROR("TriggerSyncConfirmDialog failed to allocate SyncDialogCallback");
        RejectSyncDialogError(env, executeHelper, deferred);
        return;
    }
    syncDialogCallback->env = env;
    syncDialogCallback->deferred = deferred;
    auto syncModalCallback = std::make_shared<SyncModalCallback>(uiContent, syncDialogCallback);

    int32_t sessionId = CreateSyncModalUI(uiContent, want, syncModalCallback);
    HILOG_INFO("TriggerSyncConfirmDialog sessionId=%{public}d", sessionId);

    auto *syncCtx = new (std::nothrow) SyncDialogContext();
    if (syncCtx == nullptr) {
        HILOG_ERROR("TriggerSyncConfirmDialog failed to allocate SyncDialogContext");
        delete syncDialogCallback;
        RejectSyncDialogError(env, executeHelper, deferred);
        return;
    }
    syncCtx->callback = syncDialogCallback;
    syncCtx->helper = executeHelper;

    napi_value workName = nullptr;
    napi_create_string_latin1(env, "SyncContactsDialog", NAPI_AUTO_LENGTH, &workName);

    napi_async_work work = nullptr;
    napi_create_async_work(env, nullptr, workName, SyncDialogExecute, SyncDialogComplete,
        reinterpret_cast<void *>(syncCtx), &work);

    executeHelper->work = work;
    napi_queue_async_work(env, work);
    HILOG_INFO("TriggerSyncConfirmDialog async work queued");
}

napi_value QueryContactSyncInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("fengyang QueryContactSyncInfo start");
#ifdef CONTACT_API_METRICS_ENABLE
    auto beginTime = std::chrono::system_clock::now();
    HISTOGRAM_BOOLEAN("ContactsKit.QueryContactSyncInfo.ApiCall", 1);
#endif
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, INVALID_PARAMETER);
        switch (argc) {
            case ARGS_ONE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object })) {
                    HILOG_ERROR("QueryContactSyncInfo argc is 1 and param is invalid");
                    napi_throw(env, errorCode);
                    return errorCode;
                }
                break;
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_function })) {
                    HILOG_ERROR("QueryContactSyncInfo argc is 2 and param is invalid");
                    napi_throw(env, errorCode);
                    return errorCode;
                }
                break;
            default:
                HILOG_ERROR("QueryContactSyncInfo Invalid number of parameters");
                napi_throw(env, errorCode);
                return errorCode;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
#ifdef CONTACT_API_METRICS_ENABLE
        executeHelper->beginTime = beginTime;
#endif
        result = Scheduling(env, info, executeHelper, QUERY_CONTACT_SYNC_INFO);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

} // namespace ContactsApi
} // namespace OHOS