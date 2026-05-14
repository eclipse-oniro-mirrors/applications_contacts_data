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

#ifndef PORTRAIT_FFI_H
#define PORTRAIT_FFI_H

#include <cstdint>
#include <string>
#include <memory>
#include "datashare_helper.h"
#include "contacts_control.h"

namespace OHOS {
namespace ContactsFfi {

struct CPortrait {
    bool hasUri = false;
    char* uri = nullptr;
    bool hasPhoto = false;
    int64_t photoId = 0;
};

struct PortraitContext {
    ContactsControl& contactsControl;
    std::shared_ptr<DataShare::DataShareHelper>& dataShareHelper;
};

struct ContactIdentity {
    std::string contactId;
    int64_t rawContactId = 0;
    std::string fileName;
};

struct ImageSize {
    int32_t height = 0;
    int32_t width = 0;
};

int32_t CJInsertPortrait(int64_t contextId, int64_t rawContactId, CPortrait* portrait,
    bool isAddType);

} // namespace ContactsFfi
} // namespace OHOS

#endif // PORTRAIT_FFI_H