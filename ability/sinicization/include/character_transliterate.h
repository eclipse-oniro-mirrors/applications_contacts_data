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

#ifndef CHARACTER_TRANSLITERATE_H
#define CHARACTER_TRANSLITERATE_H

#include <map>
#include <string>
#include <vector>
#include <unicode/unistr.h>
#include <unicode/translit.h>

namespace OHOS {
namespace Contacts {
struct Container {
    // The container of first letter
    std::vector<std::vector<std::wstring>> initialsContainer_;
    // Pinyin container
    std::vector<std::vector<std::wstring>> nameFullFightContainer_;
};

class CharacterTransliterate {
public:
    CharacterTransliterate();
    ~CharacterTransliterate();
    bool IsChineseCharacter(wchar_t chineseCharacter);
    std::wstring getSortKey(std::wstring displayName);

    void transferByTranslite(icu::Transliterator* transliterator, std::string &sourcestr, std::string &targetstr);

    Container GetContainer(std::wstring wChinese);
    std::wstring Join(std::vector<std::vector<std::wstring>> strVector, std::wstring split);
    void GetCommonPronunciation(
        std::wstring &chineseCharacter, std::vector<std::wstring> &initials, std::vector<std::wstring> &nameFullFights);
    std::vector<std::vector<std::wstring>> GetCombinedVector(std::vector<std::vector<std::wstring>> sourceVector);
    std::wstring StringToWstring(std::string str);
    std::string WstringToString(std::wstring str);
private:
    static std::map<std::string, std::string> multiPronunciationMap;

    icu::Transliterator* transliteratorLationToAscii;
    icu::Transliterator* transliteratorHanziToPinyin;

    std::string getMultiPronunciation(std::string chineseCharacter);
    void handleChineseSortKey(std::wstring &childwChineseCharacter, std::wstring &sortKey);
    void handleExtendedLatin(std::wstring &childwChineseCharacter, std::wstring &sameTypeStr);
};
} // namespace Contacts
} // namespace OHOS
#endif // CHARACTER_TRANSLITERATE_H
