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

#ifndef CONTACTS_SYNC_API_H
#define CONTACTS_SYNC_API_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "contacts_napi_common.h"

namespace OHOS {
namespace ContactsApi {

/**
 * @brief NAPI entry: sync contacts with cloud
 * @param env NAPI environment
 * @param info callback info
 * @return NAPI value (promise or callback result)
 */
napi_value SyncContacts(napi_env env, napi_callback_info info);

/**
 * @brief NAPI entry: query contact sync info
 * @param env NAPI environment
 * @param info callback info
 * @return NAPI value (promise or callback result)
 */
napi_value QueryContactSyncInfo(napi_env env, napi_callback_info info);

/**
 * @brief Trigger sync confirmation dialog (modal UI extension)
 * Called when first batch sync requires user confirmation.
 * @param env NAPI environment
 * @param executeHelper execution context (holds sync params and deferred promise)
 * @param deferred promise to resolve/reject after dialog result
 */
void TriggerSyncConfirmDialog(napi_env env, ExecuteHelper *executeHelper, napi_deferred deferred);

/**
 * @brief Check if current app is in foreground
 * @return true if current process is in foreground, false otherwise
 */
bool IsCurAppForeground();

} // namespace ContactsApi
} // namespace OHOS

#endif // CONTACTS_SYNC_API_H