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

#include "contacts_sync_operations.h"

#include "contacts_api.h"
#include "contacts_napi_common.h"
#include "contacts_napi_utils.h"
#include "hilog_wrapper_api.h"
#include "result_convert.h"
#include "sync_json_helper.h"
#include "datashare_helper.h"
#include "../../ability/common/include/contacts_columns.h"
#include "kit_bundle_mgr_helper.h"

namespace OHOS {
namespace ContactsApi {

void querySyncInfoRecord(ExecuteHelper *executeHelper, std::string &completedBatchesStr)
{
    if (executeHelper->dataShareHelper == nullptr) {
        return;
    }
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo("bundle_name", executeHelper->callerBundleName);
    queryPredicates.EqualTo("sync_id", executeHelper->syncId);
    std::vector<std::string> columns;
    columns.push_back("completed_batches");
    columns.push_back("total_batches");
    Uri queryUri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
    auto queryResult = executeHelper->dataShareHelper->Query(queryUri, queryPredicates, columns);
    if (queryResult == nullptr) {
        HILOG_WARN("CheckDeleteSyncInfoRecord query failed, syncId=%{public}d", executeHelper->syncId);
        return;
    }
    int rowCount = 0;
    queryResult->GetRowCount(rowCount);
    if (rowCount == 0) {
        queryResult->Close();
        HILOG_WARN("CheckDeleteSyncInfoRecord no record found, syncId=%{public}d", executeHelper->syncId);
        return;
    }
    queryResult->GoToFirstRow();
    completedBatchesStr.clear();
    int completedIdx = 0;
    if (queryResult->GetColumnIndex("completed_batches", completedIdx) == 0) {
        queryResult->GetString(completedIdx, completedBatchesStr);
    }
    int totalBatches = executeHelper->totalBatches;
    int totalBatchesIdx = 0;
    if (queryResult->GetColumnIndex("total_batches", totalBatchesIdx) == 0) {
        queryResult->GetInt(totalBatchesIdx, totalBatches);
    }
    queryResult->Close();
    HILOG_INFO("CheckDeleteSyncInfoRecord syncId=%{public}d, completed_batches=%{public}s, total_batches=%{public}d",
        executeHelper->syncId, completedBatchesStr.c_str(), totalBatches);
}

/**
 * @brief Delete sync_info record when all batches have completed
 * Only deletes if all batches (1 through totalBatches) are in completed_batches
 */
void CheckDeleteSyncInfoRecord(ExecuteHelper *executeHelper)
{
    if (executeHelper == nullptr || executeHelper->dataShareHelper == nullptr) {
        return;
    }
    DataShare::DataSharePredicates deletePredicates;
    deletePredicates.EqualTo("bundle_name", executeHelper->callerBundleName);
    deletePredicates.EqualTo("sync_id", executeHelper->syncId);
    Uri deleteUri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
    auto deleteResult = executeHelper->dataShareHelper->Delete(deleteUri, deletePredicates);
    HILOG_INFO("CheckDeleteSyncInfoRecord syncId=%{public}d, deleted=%{public}d",
        executeHelper->syncId, deleteResult);
}

/**
 * @brief Insert sync_info record (for first batch)
 */
void InsertSyncInfoRecord(ExecuteHelper *executeHelper, bool confirmResult)
{
    if (executeHelper == nullptr || executeHelper->dataShareHelper == nullptr) {
        return;
    }
    int userSelectInt = confirmResult ? CONFIRM_RESULT_YES : CONFIRM_RESULT_NO;
    int64_t nowTime = static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    DataShare::DataShareValuesBucket insertValues;
    insertValues.Put("bundle_name", executeHelper->callerBundleName);
    insertValues.Put("sync_mode", executeHelper->syncMode);
    insertValues.Put("sync_id", executeHelper->syncId);
    insertValues.Put("first_sync_time", nowTime);
    insertValues.Put("last_sync_time", nowTime);
    insertValues.Put("current_batch", executeHelper->currentBatch);
    insertValues.Put("total_batches", executeHelper->totalBatches);
    insertValues.Put("completed_batches", MakeInitialCompletedBatches(executeHelper->currentBatch));
    insertValues.Put("user_confirm_result", userSelectInt);
    int insertStatus = (executeHelper->resultData == DataShare::ExecErrorCode::EXEC_SUCCESS)
        ? BATCH_INSERT_SUCCESS : BATCH_INSERT_FAILED;
    insertValues.Put("insert_status", insertStatus);
    Uri insertUri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
    auto insertResult = executeHelper->dataShareHelper->Insert(insertUri, insertValues);
    HILOG_INFO("InsertSyncInfoRecord syncId=%{public}d, status=%{public}d, userSelectInt=%{public}d"
        "inserted=%{public}d", executeHelper->syncId, insertStatus, userSelectInt, insertResult);
}

/**
 * @brief Verify update result by re-querying and check for mismatch
 */
static void VerifyUpdateResult(ExecuteHelper *executeHelper, const std::string &newCompletedBatchesStr)
{
    Uri queryUri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("bundle_name", executeHelper->callerBundleName);
    predicates.EqualTo("sync_id", executeHelper->syncId);
    std::vector<std::string> columns;
    columns.push_back("completed_batches");
    auto result = executeHelper->dataShareHelper->Query(queryUri, predicates, columns);
    if (result == nullptr) {
        return;
    }
    int rowCount = 0;
    result->GetRowCount(rowCount);
    if (rowCount <= 0) {
        result->Close();
        return;
    }
    result->GoToFirstRow();
    std::string verifiedBatches;
    int idx = 0;
    if (result->GetColumnIndex("completed_batches", idx) == 0) {
        result->GetString(idx, verifiedBatches);
    }
    result->Close();
    HILOG_INFO("UpdateSyncInfoRecord verified=%{public}s", verifiedBatches.c_str());
    if (verifiedBatches != newCompletedBatchesStr) {
        HILOG_WARN("UpdateSyncInfoRecord verification mismatch, expected=%{public}s, got=%{public}s",
            newCompletedBatchesStr.c_str(), verifiedBatches.c_str());
        Uri delUri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
        executeHelper->dataShareHelper->Delete(delUri, predicates);
        InsertSyncInfoRecord(executeHelper, true);
    }
}

/**
 * @brief Update sync_info record (for subsequent batches)
 */
static void UpdateSyncInfoRecord(ExecuteHelper *executeHelper, const std::string &newCompletedBatchesStr)
{
    if (executeHelper->dataShareHelper == nullptr) {
        return;
    }
    int64_t nowTime = static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    int insertStatus = (executeHelper->resultData == DataShare::ExecErrorCode::EXEC_SUCCESS)
        ? BATCH_INSERT_SUCCESS : BATCH_INSERT_FAILED;

    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("bundle_name", executeHelper->callerBundleName);
    predicates.EqualTo("sync_id", executeHelper->syncId);
    DataShare::DataShareValuesBucket values;
    values.Put("last_sync_time", nowTime);
    values.Put("current_batch", executeHelper->currentBatch);
    values.Put("completed_batches", newCompletedBatchesStr);
    values.Put("insert_status", insertStatus);
    Uri updateUri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
    auto updateResult = executeHelper->dataShareHelper->Update(updateUri, predicates, values);
    HILOG_INFO("UpdateSyncInfoRecord syncId=%{public}d, updated=%{public}d", executeHelper->syncId, updateResult);

    if (updateResult <= 0) {
        HILOG_WARN("UpdateSyncInfoRecord Update returned %{public}d, falling back to Insert", updateResult);
        InsertSyncInfoRecord(executeHelper, true);
        return;
    }
    VerifyUpdateResult(executeHelper, newCompletedBatchesStr);
}

void LocalExecuteSyncContacts(napi_env env, ExecuteHelper *executeHelper)
{
    if (executeHelper->syncMode != MODE_INCREMENTAL && executeHelper->syncMode != MODE_CLOUD_BASED) {
        HILOG_ERROR("LocalExecuteSyncContacts invalid syncMode:%{public}d", executeHelper->syncMode);
        executeHelper->resultData = VERIFICATION_PARAMETER_ERROR;
        return;
    }
    HILOG_INFO("LocalExecuteSyncContacts syncMode:%{public}d, batch:%{public}d/%{public}d, "
        "statements:%{public}zu",
        executeHelper->syncMode, executeHelper->currentBatch, executeHelper->totalBatches,
        executeHelper->operationStatements.size());

    if (executeHelper->operationStatements.empty()) {
        HILOG_INFO("LocalExecuteSyncContacts no operations");
        executeHelper->resultData = DataShare::ExecErrorCode::EXEC_SUCCESS;
    } else {
        ProcessInsertContactsWithLimit(env, executeHelper);
    }

    afterExecuteSyncContacts(env, executeHelper, true);

    HILOG_INFO("LocalExecuteSyncContacts end, resultData:%{public}d", executeHelper->resultData);
}


void HandleSyncContactsResult(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    std::string syncModeStr = executeHelper->syncMode == MODE_INCREMENTAL ? "MODE_INCREMENTAL" : "MODE_CLOUD_BASED";
    std::string resultStr = executeHelper->resultData == 0 ? "SUCCESS" : "FAILED";
    HILOG_INFO("HandleSyncContactsResult syncMode:%{public}s, result:%{public}s",
        syncModeStr.c_str(), resultStr.c_str());
    HandleAddContactsResult(env, executeHelper, result);
}

void LocalExecuteQueryContactSyncInfo(napi_env env, ExecuteHelper *executeHelper)
{
    if (executeHelper->dataShareHelper == nullptr) {
        return;
    }
    HILOG_INFO("LocalExecuteQueryContactSyncInfo start");
    ContactsControl contactsControl;
    std::string bundleName;
    ContactBundleMgrHelper::GetBundleNameForSelf(bundleName);

    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("bundle_name", bundleName);
    std::vector<std::string> columns;
    Uri queryUri("datashare:///com.ohos.contactsdataability/contacts/kit_contacts_sync_info");
    auto resultSet = executeHelper->dataShareHelper->Query(queryUri, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("LocalExecuteQueryContactSyncInfo query failed, resultSet is null");
        executeHelper->resultData = -1;
    } else {
        executeHelper->resultSet = resultSet;
        executeHelper->resultData = 0;
        HILOG_INFO("LocalExecuteQueryContactSyncInfo query success, bundleName=%{public}s", bundleName.c_str());
    }
}

/**
 * @brief Set a string property from ResultSet column to JS object
 */
static void SetSyncStringProperty(napi_env env, napi_value &obj, const char *key,
    const std::shared_ptr<DataShare::DataShareResultSet> &resultSet, int colIndex)
{
    std::string value;
    if (colIndex >= 0) {
        resultSet->GetString(colIndex, value);
    }
    napi_value jsValue;
    napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &jsValue);
    napi_set_named_property(env, obj, key, jsValue);
}

/**
 * @brief Set an int property from ResultSet column to JS object
 */
static void SetSyncIntProperty(napi_env env, napi_value &obj, const char *key,
    const std::shared_ptr<DataShare::DataShareResultSet> &resultSet, int colIndex)
{
    int value = 0;
    if (colIndex >= 0) {
        resultSet->GetInt(colIndex, value);
    }
    napi_value jsValue;
    napi_create_int32(env, value, &jsValue);
    napi_set_named_property(env, obj, key, jsValue);
}

/**
 * @brief Set an int64 property from ResultSet column to JS object
 */
static void SetSyncInt64Property(napi_env env, napi_value &obj, const char *key,
    const std::shared_ptr<DataShare::DataShareResultSet> &resultSet, int colIndex)
{
    int64_t value = 0;
    if (colIndex >= 0) {
        resultSet->GetLong(colIndex, value);
    }
    napi_value jsValue;
    napi_create_int64(env, value, &jsValue);
    napi_set_named_property(env, obj, key, jsValue);
}

/**
 * @brief Set completedBatches array property from JSON string to JS array
 */
static void SetSyncCompletedBatchesProperty(napi_env env, napi_value &obj,
    const std::shared_ptr<DataShare::DataShareResultSet> &resultSet, int colIndex)
{
    std::string completedBatchesStr;
    if (colIndex >= 0) {
        resultSet->GetString(colIndex, completedBatchesStr);
    }
    std::vector<int> batches = ParseCompletedBatches(completedBatchesStr);
    napi_value jsArray;
    napi_create_array(env, &jsArray);
    for (size_t i = 0; i < batches.size(); i++) {
        napi_value batchValue;
        napi_create_int32(env, batches[i], &batchValue);
        napi_set_element(env, jsArray, i, batchValue);
    }
    napi_set_named_property(env, obj, "completedBatches", jsArray);
}

/**
 * @brief Column indices for ContactSyncInfo result set
 */
struct SyncInfoColumnIndices {
    int colBundleName = -1;
    int colMode = -1;
    int colSyncId = -1;
    int colFirstSyncTime = -1;
    int colLastSyncTime = -1;
    int colUserConfirmResult = -1;
    int colCurrentBatch = -1;
    int colCompletedBatches = -1;
    int colTotalBatches = -1;
};

/**
 * @brief Get column indices from result set for ContactSyncInfo fields
 */
static SyncInfoColumnIndices GetSyncInfoColumnIndices(
    const std::shared_ptr<DataShare::DataShareResultSet> &resultSet)
{
    SyncInfoColumnIndices idx;
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::BUNDLE_NAME, idx.colBundleName);
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::SYNC_MODE, idx.colMode);
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::SYNC_ID, idx.colSyncId);
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::FIRST_SYNC_TIME, idx.colFirstSyncTime);
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::LAST_SYNC_TIME, idx.colLastSyncTime);
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::USER_CONFIRM_RESULT,
        idx.colUserConfirmResult);
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::CURRENT_BATCH, idx.colCurrentBatch);
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::COMPLETED_BATCHES,
        idx.colCompletedBatches);
    resultSet->GetColumnIndex(::OHOS::Contacts::KitContactsSyncInfoColumns::TOTAL_BATCHES, idx.colTotalBatches);
    return idx;
}

/**
 * @brief Build a single ContactSyncInfo JS object from a ResultSet row
 */
static void BuildContactSyncInfoFromRow(napi_env env, ExecuteHelper *executeHelper,
    napi_value &syncInfoObj, const SyncInfoColumnIndices &idx)
{
    SetSyncStringProperty(env, syncInfoObj, "bundleName",
        executeHelper->resultSet, idx.colBundleName);
    SetSyncIntProperty(env, syncInfoObj, "mode",
        executeHelper->resultSet, idx.colMode);
    SetSyncIntProperty(env, syncInfoObj, "syncId",
        executeHelper->resultSet, idx.colSyncId);
    SetSyncInt64Property(env, syncInfoObj, "firstSyncTime",
        executeHelper->resultSet, idx.colFirstSyncTime);
    SetSyncInt64Property(env, syncInfoObj, "lastSyncTime",
        executeHelper->resultSet, idx.colLastSyncTime);
    SetSyncIntProperty(env, syncInfoObj, "userConfirmResult",
        executeHelper->resultSet, idx.colUserConfirmResult);
    SetSyncIntProperty(env, syncInfoObj, "currentBatch",
        executeHelper->resultSet, idx.colCurrentBatch);
    SetSyncIntProperty(env, syncInfoObj, "totalBatches",
        executeHelper->resultSet, idx.colTotalBatches);
    SetSyncCompletedBatchesProperty(env, syncInfoObj,
        executeHelper->resultSet, idx.colCompletedBatches);
}

void HandleQueryContactSyncInfoResult(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    napi_create_array(env, &result);
    if (executeHelper->resultSet == nullptr) {
        HILOG_INFO("HandleQueryContactSyncInfoResult no result, return empty array");
        return;
    }
    if (executeHelper->resultSet->GoToFirstRow() != OHOS::NativeRdb::E_OK) {
        HILOG_INFO("HandleQueryContactSyncInfoResult GoToFirstRow failed, return empty array");
        return;
    }

    SyncInfoColumnIndices idx = GetSyncInfoColumnIndices(executeHelper->resultSet);
    if (idx.colMode < 0 || idx.colSyncId < 0 || idx.colBundleName < 0) {
        HILOG_WARN("HandleQueryContactSyncInfoResult some required columns not found");
    }

    uint32_t arrayIndex = 0;
    do {
        napi_value syncInfoObj = nullptr;
        napi_create_object(env, &syncInfoObj);
        BuildContactSyncInfoFromRow(env, executeHelper, syncInfoObj, idx);
        napi_set_element(env, result, arrayIndex++, syncInfoObj);
    } while (executeHelper->resultSet->GoToNextRow() == OHOS::NativeRdb::E_OK);
    HILOG_INFO("HandleQueryContactSyncInfoResult array size=%{public}u", arrayIndex);
}

/**
 * @brief Parse sync mode and progress from Scheduling's pre-converted params
 */
void BuildSyncContactsConvertParams(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsBuild contactsBuild;
    bool isFA = (executeHelper->abilityContext == nullptr);
    int modeIndex = isFA ? 1 : 0;
    if (executeHelper->argc > static_cast<unsigned int>(modeIndex) &&
        executeHelper->argv[modeIndex] != nullptr) {
        executeHelper->syncMode = contactsBuild.GetInt(env, executeHelper->argv[modeIndex]);
    }
    int progressIndex = modeIndex + 1;
    if (executeHelper->argc > static_cast<unsigned int>(progressIndex) &&
        executeHelper->argv[progressIndex] != nullptr) {
        napi_value progressObj = executeHelper->argv[progressIndex];
        napi_value propValue = nullptr;
        napi_valuetype valueType = napi_undefined;
        if (napi_get_named_property(env, progressObj, "syncId", &propValue) == napi_ok && propValue != nullptr) {
            if (napi_typeof(env, propValue, &valueType) == napi_ok && valueType == napi_number) {
                executeHelper->syncId = contactsBuild.GetInt(env, propValue);
            }
        }
        propValue = nullptr;
        if (napi_get_named_property(env, progressObj, "currentBatch", &propValue) == napi_ok && propValue != nullptr) {
            if (napi_typeof(env, propValue, &valueType) == napi_ok && valueType == napi_number) {
                executeHelper->currentBatch = contactsBuild.GetInt(env, propValue);
            }
        }
        propValue = nullptr;
        if (napi_get_named_property(env, progressObj, "totalBatches", &propValue) == napi_ok && propValue != nullptr) {
            if (napi_typeof(env, propValue, &valueType) == napi_ok && valueType == napi_number) {
                executeHelper->totalBatches = contactsBuild.GetInt(env, propValue);
            }
        }
    }
    HILOG_INFO("BuildSyncContactsConvertParams syncMode:%{public}d, syncId:%{public}d, "
        "currentBatch:%{public}d, totalBatches:%{public}d, isFA:%{public}d",
        executeHelper->syncMode, executeHelper->syncId, executeHelper->currentBatch,
        executeHelper->totalBatches, isFA);
}

void afterExecuteSyncContacts(napi_env env, ExecuteHelper *executeHelper, bool userConfirmed)
{
    std::string completedBatchesStr = "";
    querySyncInfoRecord(executeHelper, completedBatchesStr);
    HILOG_INFO("UpdateSyncInfoRecord before: syncId=%{public}d, batch=%{public}d, existing=%{public}s,"
        "total=%{public}d", executeHelper->syncId, executeHelper->currentBatch, completedBatchesStr.c_str(),
        executeHelper->totalBatches);
    bool mergedSuccess = false;
    std::string newCompletedBatchesStr = MergeCompletedBatches(completedBatchesStr,
        executeHelper->currentBatch, mergedSuccess);
    if (!mergedSuccess) {
        // Existing completed_batches is corrupt (non-array or non-integer elements).
        // Retry with empty base so current batch is at least recorded.
        // This gracefully degrades: we lose prior batch progress but sync can continue.
        HILOG_ERROR("afterExecuteSyncContacts: existing completed_batches is corrupt '%{public}s', "
            "retrying with empty base for batch=%{public}d",
            completedBatchesStr.c_str(), executeHelper->currentBatch);
        newCompletedBatchesStr = MergeCompletedBatches("", executeHelper->currentBatch, mergedSuccess);
    }
    bool isdeleteSyncInfo = AreAllBatchesCompleted(newCompletedBatchesStr, executeHelper->totalBatches);
    if (isdeleteSyncInfo) {
        CheckDeleteSyncInfoRecord(executeHelper);
        return;
    }

    if (executeHelper->isFirstSync) {
        InsertSyncInfoRecord(executeHelper, userConfirmed);
    } else {
        UpdateSyncInfoRecord(executeHelper, newCompletedBatchesStr);
    }
    HILOG_INFO("UpdateSyncInfoRecord after: newCompletedBatches=%{public}s", newCompletedBatchesStr.c_str());
}

} // namespace ContactsApi
} // namespace OHOS