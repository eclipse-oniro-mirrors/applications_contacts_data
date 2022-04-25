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

#include "construction_name.h"

#include "character_transliterate.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
std::string ConstructionName::local = "zh-CN";
ConstructionName::ConstructionName()
{
}

ConstructionName::~ConstructionName()
{
}

ConstructionName ConstructionName::GetConstructionName(
    std::string &chineseCharacter, ConstructionName &constructionName)
{
    CharacterTransliterate characterTransliterate;
    if (strcmp(local.c_str(), "zh-CN") == 0) {
        Container container =
            characterTransliterate.GetContainer(characterTransliterate.StringToWstring(chineseCharacter));
        std::wstring split(L"||");
        std::wstring initials = characterTransliterate.Join(container.initialsContainer_, split);
        std::wstring nameFullFight = characterTransliterate.Join(container.nameFullFightContainer_, split);
        constructionName.initials_ = characterTransliterate.WstringToString(initials);
        constructionName.nameFullFight_ = characterTransliterate.WstringToString(nameFullFight);
        const wchar_t *nameFullFightInitials = nameFullFight.c_str();
        if ((nameFullFightInitials[0] >= L'a' && nameFullFightInitials[0] <= L'z') ||
            (nameFullFightInitials[0] >= L'A' && nameFullFightInitials[0] <= L'Z')) {
            std::string sortFirstLetterTemp = characterTransliterate.WstringToString(nameFullFight.substr(0, 1));
            std::transform(
                sortFirstLetterTemp.begin(), sortFirstLetterTemp.end(), sortFirstLetterTemp.begin(), std::toupper);
            constructionName.sortFirstLetter_ = sortFirstLetterTemp;
            int code = constructionName.sortFirstLetter_.c_str()[0];
            constructionName.sortFirstLetterCode_ = code;
            HILOG_INFO(" GetConstructionName sortFirstLetterCode :%{public}d", code);
        } else {
            std::wstring sortFirstLetter(L"#");
            constructionName.sortFirstLetter_ = characterTransliterate.WstringToString(sortFirstLetter);
            constructionName.sortFirstLetterCode_ = -1;
        }
    } else {
        constructionName.initials_ = chineseCharacter;
        constructionName.nameFullFight_ = chineseCharacter;
        if ((chineseCharacter[0] >= 'a' && chineseCharacter[0] <= 'z') ||
            (chineseCharacter[0] >= 'A' && chineseCharacter[0] <= 'Z')) {
            std::string sortFirstLetterTemp = chineseCharacter.substr(0, 1);
            std::transform(
                sortFirstLetterTemp.begin(), sortFirstLetterTemp.end(), sortFirstLetterTemp.begin(), std::toupper);
            constructionName.sortFirstLetter_ = sortFirstLetterTemp;
            int code = constructionName.sortFirstLetter_.c_str()[0];
            constructionName.sortFirstLetterCode_ = code;
            HILOG_INFO(" GetConstructionName sortFirstLetterCode1 :%{public}d", code);
        } else {
            std::string sortFirstLetter("#");
            constructionName.sortFirstLetter_ = sortFirstLetter;
            constructionName.sortFirstLetterCode_ = -1;
        }
    }
    constructionName.disPlayName_ = chineseCharacter;
    return GetPhotoFirstName(constructionName);
}

ConstructionName ConstructionName::GetPhotoFirstName(ConstructionName &constructionName)
{
    constructionName.photoFirstName_.clear();
    if (!constructionName.disPlayName_.empty()) {
        CharacterTransliterate characterTransliterate;
        std::wstring nameWstr = characterTransliterate.StringToWstring(constructionName.disPlayName_);
        unsigned int len = nameWstr.size();
        for (unsigned int index = 0; index < len; index++) {
            if (characterTransliterate.IsChineseCharacter(nameWstr[index])) {
                std::wstring childwChineseCharacter = nameWstr.substr(index, 1);
                constructionName.photoFirstName_ = characterTransliterate.WstringToString(childwChineseCharacter);
                return constructionName;
            }
        }
        if ((constructionName.disPlayName_[0] >= 'a' && constructionName.disPlayName_[0] <= 'z') ||
            (constructionName.disPlayName_[0] >= 'A' && constructionName.disPlayName_[0] <= 'Z')) {
            constructionName.photoFirstName_ = constructionName.disPlayName_[0];
        }
    }
    return constructionName;
}
} // namespace Contacts
} // namespace OHOS