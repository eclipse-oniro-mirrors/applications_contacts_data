/*
 * Copyright (C) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef COMMON_TOOL_TYPE_H
#define COMMON_TOOL_TYPE_H

#include <iostream>
#include <charconv>
#include <string>

namespace OHOS {
namespace Contacts {
class CommonToolType {
public:
static bool ConvertToInt(const std::string& str, int& value)
{
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

static int UnsignedLongIntToInt(const size_t& size)
{
    int result = 0;
    ConvertToInt(std::to_string(size), result);
    return result;
}
};
}  // namespace Contacts
}  // namespace OHOS

#endif  // COMMON_TOOL_TYPE_H
