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
    // 中文环境
    if (local == "zh-CN") {
        // 转拼音格式，生成名称拼音首字母的集合，拼音全字母的集合
        Container container =
            characterTransliterate.GetContainer(characterTransliterate.StringToWstring(chineseCharacter));
        std::wstring split(L"||");
        std::wstring initials = characterTransliterate.Join(container.initialsContainer_, split);
        std::wstring nameFullFight = characterTransliterate.Join(container.nameFullFightContainer_, split);
        constructionName.initials_ = characterTransliterate.WstringToString(initials);
        constructionName.nameFullFight_ = characterTransliterate.WstringToString(nameFullFight);
    } else {
        constructionName.initials_ = chineseCharacter;
        constructionName.nameFullFight_ = chineseCharacter;
    }

    // 排序sortKey关键字
    std::wstring sortKeyWstring = characterTransliterate.getSortKey(
        characterTransliterate.StringToWstring(chineseCharacter));
    constructionName.sortKey = characterTransliterate.WstringToString(sortKeyWstring);
    // 根据sortkey首字母，生成sortFirstLetter_，sortFirstLetterCode_
    const wchar_t *sortKeyCharArrW = sortKeyWstring.c_str();
    if ((sortKeyCharArrW[0] >= L'a' && sortKeyCharArrW[0] <= L'z') ||
        (sortKeyCharArrW[0] >= L'A' && sortKeyCharArrW[0] <= L'Z')) {
        std::string sortFirstLetterTemp = characterTransliterate.WstringToString(sortKeyWstring.substr(0, 1));
        // 转大写
        std::transform(
            sortFirstLetterTemp.begin(), sortFirstLetterTemp.end(), sortFirstLetterTemp.begin(), std::toupper);
        constructionName.sortFirstLetter_ = sortFirstLetterTemp;
        int code = constructionName.sortFirstLetter_.c_str()[0];
        constructionName.sortFirstLetterCode_ = code;
    } else {
        std::string sortFirstLetter("#");
        constructionName.sortFirstLetter_ = sortFirstLetter;
        constructionName.sortFirstLetterCode_ = -1;
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