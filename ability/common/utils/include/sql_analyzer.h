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

#ifndef SQL_ANALYZER_H
#define SQL_ANALYZER_H

#include <string>

#include "values_bucket.h"

#include "common.h"

namespace OHOS {
namespace Contacts {
class SqlAnalyzer {
public:
    SqlAnalyzer();
    ~SqlAnalyzer();

    bool CheckValuesBucket(const OHOS::NativeRdb::ValuesBucket &value);
    bool FindIllegalWords(std::string sql);
    bool StrCheck(char &ch, std::size_t strlen, std::string sql, std::size_t &pos);
    bool CharCheck(char &ch, std::string sql, std::size_t &pos);

private:
    inline bool IsNumber(char ch)
    {
        return (ch >= '0' && ch <= '9');
    }
    inline bool IsLetter(char ch)
    {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_');
    }
    inline bool IsLetterNumber(char ch)
    {
        return IsNumber(ch) || IsLetter(ch);
    }
    inline char PickChar(std::string str, std::size_t index)
    {
        if (index < str.length()) {
            return str.at(index);
        }
        return '\0';
    }
    inline int IsInStr(char ch, std::string str)
    {
        std::size_t pos = str.find(ch);
        if (pos == std::string::npos) {
            return OPERATION_ERROR;
        }
        return 0;
    }
    std::string ParseSpecial(std::string originString)
    {
        std::vector<char> needsTransform = {'\'', '\"', ';', '_', '-', '\\', '%', '[', ']', '/', '*'};
        std::string parsedString;
        for (unsigned int i = 0; i < originString.size(); i++) {
            char curChar = originString.at(i);
            if (std::find(needsTransform.begin(), needsTransform.end(), curChar) != needsTransform.end()) {
                parsedString += '\\' + curChar;
            } else {
                parsedString += curChar;
            }
        }
        return parsedString;
    }
};
} // namespace Contacts
} // namespace OHOS
#endif // SQL_ANALYZER_H
