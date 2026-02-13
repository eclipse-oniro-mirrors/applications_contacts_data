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

#include "character_transliterate.h"

#include <codecvt>
#include <iostream>

#include "common.h"
#include "hilog_wrapper.h"
#include <unicode/translit.h>
#include <unicode/unistr.h>
#include <unicode/stringpiece.h>

namespace OHOS {
namespace Contacts {
std::map<std::string, std::string> CharacterTransliterate::multiPronunciationMap = {
    {"\u8983", "qin"}, // qin tan
    {"\u6c88", "shen"}, // SHEN yang
    {"\u66fe", "zeng"}, // ZENG zu fu
    {"\u8d3e", "jia"}, // JIA
    {"\u4fde", "yu"},
    {"\u513F", "er"},
    {"\u5475", "he"},
    {"\u957f", "chang"},
    {"\u7565", "lue"},
    {"\u63a0", "lue"},
    {"\u4e7e", "qian"}, // qian
    {"\u79d8", "bi"},
    {"\u8584", "bo"},
    {"\u79cd", "chong"},
    {"\u891a", "chu"},
    {"\u555c", "chuai"},
    {"\u53e5", "gou"},
    {"\u839e", "guan"},
    {"\u7094", "gui"},
    {"\u85c9", "ji"},
    {"\u5708", "juan"},
    {"\u89d2", "jue"},
    {"\u961a", "kan"},
    {"\u9646", "lu"},
    {"\u7f2a", "miao"},
    {"\u4f74", "nai"},
    {"\u5152", "ni"},
    {"\u4e5c", "nie"},
    {"\u533a", "ou"},
    {"\u6734", "piao"},
    {"\u7e41", "po"},
    {"\u4ec7", "qiu"},
    {"\u5355", "shan"},
    {"\u76db", "sheng"},
    {"\u6298", "she"},
    {"\u5bbf", "su"},
    {"\u6d17", "xian"},
    {"\u89e3", "xie"},
    {"\u5458", "yun"},
    {"\u7b2e", "ze"},
    {"\u7fdf", "zhai"},
    {"\u796d", "zhai"},
    {"\u963f", "a"},
    {"\u5b93", "fu"},
    {"\u90a3", "na"},
    {"\u5c09", "yu"},
    {"\u86fe", "yi"},
    {"\u67e5", "zha"},
    {"\u5200", "dao"},
};

CharacterTransliterate::CharacterTransliterate(void)
{
    UErrorCode status = U_ZERO_ERROR;
    transliteratorLationToAscii = icu::Transliterator::createInstance (
        icu::UnicodeString(LATIN_TO_ASCII_ICU_TRANSLITE_ID), UTransDirection::UTRANS_FORWARD, status);
    transliteratorHanziToPinyin = icu::Transliterator::createInstance (
        icu::UnicodeString(HANZI_TO_PINYIN_ICU_TRANSLITE_ID), UTransDirection::UTRANS_FORWARD, status);
}

CharacterTransliterate::~CharacterTransliterate()
{
    if (transliteratorLationToAscii != nullptr) {
        delete transliteratorLationToAscii;
    }
    if (transliteratorHanziToPinyin != nullptr) {
        delete transliteratorHanziToPinyin;
    }
}

bool CharacterTransliterate::IsChineseCharacter(wchar_t chineseCharacter)
{
    return (uint16_t)chineseCharacter >= 0x4E00 && (uint16_t)chineseCharacter <= 0x9FCF;
}

std::string CharacterTransliterate::getMultiPronunciation(std::string chineseCharacter)
{
    auto it = multiPronunciationMap.find(chineseCharacter);
    if (it != multiPronunciationMap.end()) {
        return it->second;
    }
    return "";
}

Container CharacterTransliterate::GetContainer(std::wstring wChinese)
{
    Container container;
    // 字符串长度
    size_t count = wChinese.size();
    std::vector<std::vector<std::wstring>> initialsVectors;
    std::vector<std::vector<std::wstring>> nameFullFightsVectors;
    bool isMultiPronunciation = true;
    // 遍历字符串的每个字符
    for (size_t index = 0; index < count; index++) {
        // initials记录首字母，nameFullFights记录名称拼音全字母
        std::vector<std::wstring> initials;
        std::vector<std::wstring> nameFullFights;
        std::wstring childwChineseCharacter = wChinese.substr(index, 1);
        if (index == 0 && isMultiPronunciation) {
            // 根据多音字获取
            std::string pronunciation = getMultiPronunciation(WstringToString(childwChineseCharacter));
            if (!pronunciation.empty()) {
                HILOG_INFO("This is a multi-pronunciation Chinese character, ts = %{public}lld", (long long) time(NULL));
                std::wstring wPronunciation = StringToWstring(pronunciation);
                // 记录首字母
                initials.push_back(wPronunciation.substr(0, 1));
                nameFullFights.push_back(wPronunciation);
            } else {
                isMultiPronunciation = false;
                index--;
            }
        } else {
            // 如果是汉字，解析出拼音
            // 获得汉字对应的拼音，拼音首字母放到initials；拼音全字母放到nameFullFights
            // 如果不是汉字，按原始字符放到initials，nameFullFights
            GetCommonPronunciation(childwChineseCharacter, initials, nameFullFights);
        }
        if (!initials.empty() && !nameFullFights.empty()) {
            initialsVectors.push_back(initials);
            nameFullFightsVectors.push_back(nameFullFights);
        }
    }
    container.initialsContainer_ = initialsVectors;
    container.nameFullFightContainer_ = nameFullFightsVectors;
    return container;
}

/**
 * 添加汉字
 * @param sortKey
 * @param hanzi
 * @param pinyin
 */
void AppendNameInfoPinYin(std::wstring &sortKey, std::wstring &hanzi, std::wstring &pinyin)
{
    if (sortKey.size() > 0) {
        sortKey.append(L" ");
    }
    // pinyin + " " + hanzi
    std::transform(pinyin.begin(), pinyin.end(), pinyin.begin(), towupper);
    sortKey.append(pinyin);
    sortKey.append(L" ");
    sortKey.append(hanzi);
}

/**
 * 添加同类型的字符串
 * @param sortKey
 * @param sameTypeStr
 */
void AppendNameInfo(std::wstring &sortKey, std::wstring &sameTypeStr)
{
    if (!sameTypeStr.empty()) {
        if (sortKey.size() > 0) {
            sortKey.append(L" ");
        }

        std::transform(sameTypeStr.begin(), sameTypeStr.end(), sameTypeStr.begin(), towupper);
        sortKey.append(sameTypeStr);
        sameTypeStr.clear();
    }
}

const int LATIN = 1;
const int UNKNOWN = 3;
const int ASCII_MAX_NUM = 128;

/**
 * 处理中文的sortKey
 * @param childwChineseCharacter
 * @param sortKey
 */
void CharacterTransliterate::handleChineseSortKey(std::wstring &childwChineseCharacter, std::wstring &sortKey)
{
    // 姓氏可能是多音字，多音字需要按姓的拼音来处理
    std::string pronunciation = getMultiPronunciation(WstringToString(childwChineseCharacter));
    // 多音字
    if (!pronunciation.empty()) {
        // 拼音转为wstring字符串
        std::wstring wPronunciation = StringToWstring(pronunciation);
        AppendNameInfoPinYin(sortKey, childwChineseCharacter, wPronunciation);

        return;
    }

    // 不是多音字，查对应汉字
    std::string sourcestr = WstringToString(childwChineseCharacter);
    std::string targetstr;
    transferByTranslite(transliteratorHanziToPinyin, sourcestr, targetstr);
    std::wstring targetWstr = StringToWstring (targetstr);
    AppendNameInfoPinYin(sortKey, childwChineseCharacter, targetWstr);
}

void CharacterTransliterate::handleExtendedLatin(std::wstring &childwChineseCharacter, std::wstring &sameTypeStr)
{
    std::string sourcestr = WstringToString(childwChineseCharacter);
    std::string targetstr;
    transferByTranslite(transliteratorLationToAscii, sourcestr, targetstr);
    std::wstring targetwstr = StringToWstring(targetstr);
    sameTypeStr.append(targetwstr);
}

/**
 * 根据displayName，获取排序key
 * @param displayName
 * @return
 */
std::wstring CharacterTransliterate::getSortKey(std::wstring displayName)
{
    // 字符串长度，用于遍历字符串
    size_t count = displayName.size();
    // 记录相同类型的字符串
    std::wstring sameTypeStr = L"";
    // 记录相同类型字符串的类型
    int type = LATIN;
    // 最后解析出的排序key
    std::wstring sortKey = L"";

    // 遍历字符串
    for (size_t index = 0; index < count; index++) {
        // 获取当前字符
        std::wstring childwChineseCharacter = displayName.substr(index, 1);
        // 中文判断
        if (IsChineseCharacter(childwChineseCharacter[0])) {
            // 如果中文字符前有连续的同类型字符串, 添加同类型字符串
            AppendNameInfo(sortKey, sameTypeStr);
            // 处理中文
            handleChineseSortKey(childwChineseCharacter, sortKey);
            continue;
        }

        // 如果是字符，特殊字符，按原字符排序或进行Latin-Ascii转换后排序，但是连续的同类型非中文字符，需要一起
        // 如果是空格，将空格前的相同类型字符拼接到sortkey
        if (childwChineseCharacter[0] == L' ') {
            AppendNameInfo(sortKey, sameTypeStr);
            continue;
        }

        // 如果是ASCII，转换后字符和原始相同；
        if (childwChineseCharacter[0] < ASCII_MAX_NUM) {
            // 不是LATIN类型的加入转换
            if (type != LATIN && !sameTypeStr.empty()) {
                AppendNameInfo(sortKey, sameTypeStr);
            }
            // sameTypeStr 为LATIN类型字符串
            type = LATIN;
            sameTypeStr.append(childwChineseCharacter);
            continue;
        }

        // Latin-Ascii
        if (childwChineseCharacter[0] < 0x250 ||
            (0x1e00 <= childwChineseCharacter[0] && childwChineseCharacter[0] < 0x1eff)) {
            // 不是LATIN类型的加入转换
            if (type != LATIN) {
                AppendNameInfo(sortKey, sameTypeStr);
            }

            // sameTypeStr 为LATIN类型字符串
            // Extended Latin. Transcode these to ASCII equivalents.
            type = LATIN;
            handleExtendedLatin(childwChineseCharacter, sameTypeStr);
            continue;
        }

        // 可能是中文字符，中文字符作为中文处理的，转为 targetstr+“ ”+sourcestr
        // 。，“ ？ 等等
        std::string sourcestr = WstringToString(childwChineseCharacter);
        std::string targetstr;
        transferByTranslite(transliteratorLationToAscii, sourcestr, targetstr);
        // 字符串不相同，作为中文处理
        if (sourcestr.compare(targetstr) != 0) {
            // 添加中文前同类型字符串
            AppendNameInfo(sortKey, sameTypeStr);
            std::wstring targetwstr = StringToWstring(targetstr);
            AppendNameInfoPinYin(sortKey, childwChineseCharacter, targetwstr);
            continue;
        }

        // 其他类型
        // 不是LATIN类型的加入转换
        if (type != UNKNOWN) {
            AppendNameInfo(sortKey, sameTypeStr);
        }
        // sameTypeStr 为UNKNOWN类型字符串
        type = UNKNOWN;
        sameTypeStr.append(childwChineseCharacter);
    }

    AppendNameInfo(sortKey, sameTypeStr);
    return sortKey;
}

void CharacterTransliterate::transferByTranslite(icu::Transliterator* transliterator,
    std::string &sourcestr, std::string &targetstr)
{
    if (transliterator == nullptr) {
        HILOG_ERROR("transliterator is null, ts = %{public}lld", (long long) time(NULL));
        return;
    }
    icu::UnicodeString unicodeString(sourcestr.c_str());

    transliterator -> transliterate(unicodeString);
    unicodeString.toUTF8String(targetstr);
}

void CharacterTransliterate::GetCommonPronunciation(
    std::wstring &chineseCharacter, std::vector<std::wstring> &initials, std::vector<std::wstring> &nameFullFights)
{
    // Only one character can be entered
    if (chineseCharacter.size() > 1) {
        return;
    }
    initials.clear();
    nameFullFights.clear();
    std::string chineseCharacterString = WstringToString(chineseCharacter);
    // If it is a Chinese character, there is no need to transcode Unicode here.
    // The system will transcode it. Only the Chinese character needs to be matched directly.
    if (IsChineseCharacter(chineseCharacter[0])) {
        std::string targetstr;
        transferByTranslite(transliteratorHanziToPinyin, chineseCharacterString, targetstr);
        if (!targetstr.empty()) {
            initials.push_back(StringToWstring(targetstr.substr(0, 1)));
            nameFullFights.push_back(StringToWstring(targetstr));
        }
    } else {
        // 不是中文
        initials.push_back(chineseCharacter);
        nameFullFights.push_back(chineseCharacter);
    }
}

// sort Chinese Pinyin combinations
std::vector<std::vector<std::wstring>> CharacterTransliterate::GetCombinedVector(
    std::vector<std::vector<std::wstring>> sourceVector)
{
    // gets the first set of attributes in the sourcevector collection
    std::vector<std::vector<std::wstring>> targetVector;
    targetVector.push_back(sourceVector[0]);
    // start the traversal from the second set of attributes in the sourcevector collection
    unsigned int size = sourceVector.size();
    for (unsigned int i = 1; i < size; i++) {
        std::vector<std::wstring> nextVector = sourceVector[i];
        // traverse the combination of the second set of attributes and the first set of attributes
        // set an intermediate array to store the data after attribute combination
        std::vector<std::vector<std::wstring>> tempVector;
        for (std::wstring nextStr : nextVector) {
            for (std::vector<std::wstring> targetStrVector : targetVector) {
                for (std::wstring targetStr : targetStrVector) {
                    // put the combined attributes into a temporary array
                    std::wstring tempString = targetStr + nextStr;
                    std::vector<std::wstring> temp;
                    temp.push_back(tempString);
                    tempVector.push_back(temp);
                }
            }
        }
        targetVector = tempVector;
    }
    return targetVector;
}

// Split Chinese Pinyin with '|'
std::wstring CharacterTransliterate::Join(std::vector<std::vector<std::wstring>> strVector, std::wstring split)
{
    std::wstring str;
    for (std::vector<std::wstring> childVector : strVector) {
        size_t len = childVector.size();
        for (size_t i = 0; i < len; i++) {
            str.append(childVector[i]);
            if (i < len - 1) {
                str.append(split);
            }
        }
    }
    return str;
}

std::string CharacterTransliterate::WstringToString(std::wstring wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}

// convert string to wsstring
std::wstring CharacterTransliterate::StringToWstring(std::string str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    try {
        return converterX.from_bytes(str);
    } catch (const std::range_error& e) {
        // 处理范围错误，可能是由于无效的UTF-8序列
        HILOG_ERROR("StringToWstring range_error: %{public}s", e.what());
        return L"";
    } catch (const std::out_of_range& e) {
        // 处理超出范围的错误
        HILOG_ERROR("StringToWstring out_of_rangeError: %{public}s", e.what());
        return L"";
    } catch (const std::exception& e) {
        // 处理其他标准库异常
        HILOG_ERROR("StringToWstring exceptionError: %{public}s", e.what());
        return L"";
    }
}
} // namespace Contacts
} // namespace OHOS
