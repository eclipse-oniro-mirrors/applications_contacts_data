/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "napi/native_api.h"
#include "calllog_backup_napi.h"
#include "contacts_database.h"
#include "application_context.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace Contacts {
/*
 * Function registering all props and functions of ohos.medialibrary module
 */
static napi_value BackupNapiInit(napi_env env, napi_value exports)
{
    HILOG_INFO("Init, BackupNapiInit has been used.");
    CallLogBackupNapi::Init(env, exports);
    return exports;
}

/*
 * module register for mediabackup
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    HILOG_INFO("Init attribute has been used.");

    auto moduleManager = NativeModuleManager::GetInstance();
    NativeModule colorSpaceManagerModule = {
        .name = "contactsdataability",
        .fileName = nullptr,
        .registerCallback = OHOS::Contacts::BackupNapiInit,
    };
    moduleManager->Register(&colorSpaceManagerModule);
}
} // namespace Contacts
} // namespace OHOS
