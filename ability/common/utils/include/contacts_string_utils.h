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

#ifndef CONTACT_STRING_UTILS_H
#define CONTACT_STRING_UTILS_H

#include <string>
#include <vector>

namespace OHOS {
namespace Contacts {
class ContactsStringUtils {
public:
    ContactsStringUtils();
    ~ContactsStringUtils();
    static std::string &Trim(std::string &str);
    static std::vector<std::string> SplitStr(std::string str, std::string spSymbol);
};
} // namespace Contacts
} // namespace OHOS

#endif // STRING_UTILS_H
