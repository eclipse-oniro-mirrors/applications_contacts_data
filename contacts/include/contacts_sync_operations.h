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

#ifndef CONTACTS_SYNC_OPERATIONS_H
#define CONTACTS_SYNC_OPERATIONS_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "contacts_napi_common.h"
#include "sync_json_helper.h"

namespace OHOS {
namespace ContactsApi {

/**
 * @brief Execute sync contacts locally
 * Validates syncMode, builds operation statements, executes batch,
 * and updates sync_info records (insert/update/delete).
 * @param env NAPI environment
 * @param executeHelper execution context
 */
void LocalExecuteSyncContacts(napi_env env, ExecuteHelper *executeHelper);

/**
 * @brief Execute query contact sync info locally
 * Queries kit_contacts_sync_info table for current caller's bundle.
 * @param env NAPI environment
 * @param executeHelper execution context
 */
void LocalExecuteQueryContactSyncInfo(napi_env env, ExecuteHelper *executeHelper);

/**
 * @brief Handle sync contacts result for NAPI callback
 * Delegates to HandleSyncContactsResult.
 * @param env NAPI environment
 * @param executeHelper execution context
 * @param result output NAPI value
 */
void HandleSyncContactsResult(napi_env env, ExecuteHelper *executeHelper, napi_value &result);

/**
 * @brief Handle query contact sync info result for NAPI callback
 * Builds JS array of ContactSyncInfo objects from resultSet.
 * @param env NAPI environment
 * @param executeHelper execution context
 * @param result output NAPI value (JS array)
 */
void HandleQueryContactSyncInfoResult(napi_env env, ExecuteHelper *executeHelper, napi_value &result);

/**
 * @brief Handle sync contacts error code for NAPI Stage/FA mode
 * @param env NAPI environment
 * @param executeHelper execution context
 * @param result output NAPI error value
 */
void HandleSyncContactsErrorCode(napi_env env, ExecuteHelper *executeHelper, napi_value &result);

/**
 * @brief Parse sync mode and progress from Scheduling's pre-converted params
 * Reads syncMode, syncId, currentBatch, totalBatches from executeHelper->argv.
 * @param env NAPI environment
 * @param executeHelper execution context
 */
void BuildSyncContactsConvertParams(napi_env env, ExecuteHelper *executeHelper);

/**
 * @brief Handle post-sync operations after executing a sync batch
 * Updates sync_info table (insert/update/delete), reports sync event to DFX,
 * and triggers notification when all batches complete.
 * @param env NAPI environment
 * @param executeHelper execution context
 * @param userConfirmed whether user confirmed the sync
 */
void afterExecuteSyncContacts(napi_env env, ExecuteHelper *executeHelper, bool userConfirmed);
/**
 * @brief Insert sync_info record (for first batch)
 */
void InsertSyncInfoRecord(ExecuteHelper *executeHelper, bool confirmResult);

/**
 * @brief Delete sync_info record when all batches have completed
 * Only deletes if all batches (1 through totalBatches) are in completed_batches
 */
void CheckDeleteSyncInfoRecord(ExecuteHelper *executeHelper);

} // namespace ContactsApi
} // namespace OHOS

#endif // CONTACTS_SYNC_OPERATIONS_H