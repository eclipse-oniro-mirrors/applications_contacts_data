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

namespace OHOS {
namespace ContactsApi {
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
} // namespace ContactsApi
} // namespace OHOS
