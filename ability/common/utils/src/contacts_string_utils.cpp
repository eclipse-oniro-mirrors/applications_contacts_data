/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "contacts_string_utils.h"

namespace OHOS {
namespace Contacts {
ContactsStringUtils::ContactsStringUtils()
{
}

ContactsStringUtils::~ContactsStringUtils()
{
}

std::vector<std::string> ContactsStringUtils::SplitStr(std::string str, std::string spSymbol)
{
    std::vector<std::string> result;
    std::string::size_type i = 0;
    Trim(str);
    std::string::size_type found = str.find(spSymbol);
    if (found != std::string::npos) {
        while (found != std::string::npos) {
            std::string value = str.substr(i, found - i);
            Trim(value);
            if (!value.empty()) {
                result.push_back(value);
            }
            i = found + spSymbol.size();
            found = str.find(spSymbol, i);
        }
        std::string lastv = str.substr(i, str.size() - i);
        Trim(lastv);
        if (!lastv.empty()) {
            result.push_back(lastv);
        }
    }
    return result;
}

std::string &ContactsStringUtils::Trim(std::string &str)
{
    if (str.empty()) {
        return str;
    }
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
}
} // namespace Contacts
} // namespace OHOS