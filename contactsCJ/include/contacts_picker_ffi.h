/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef CONTACTS_PICKER_FFI_H
#define CONTACTS_PICKER_FFI_H

#include <cstdint>

namespace OHOS {
namespace ContactsFfi {

struct CContactSelectionOptions {
    bool hasIsMultiSelect;
    bool isMultiSelect;
    bool hasMaxSelectable;
    int32_t maxSelectable;
    bool hasFilter;
    char* filterJson;
    char* displayType;
};

struct CPickerResult {
    char* pickerData;
    int32_t total;
    int32_t resultCode;
};

CPickerResult* CJSelectContacts(int64_t contextId, CContactSelectionOptions* options, int32_t* errCode);
void CJFreePickerResult(CPickerResult* result);

} // namespace ContactsFfi
} // namespace OHOS

#endif // CONTACTS_PICKER_FFI_H