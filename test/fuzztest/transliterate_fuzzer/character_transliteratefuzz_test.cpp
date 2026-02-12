/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "character_transliteratefuzz_test.h"

#include <vector>
#include <string>

#include "access_token.h"
#include "accesstoken_kit.h"
#include "access_token_error.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "datashare_predicates.h"
#include "datashare_helper.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "singleton.h"
#include "character_transliterate.h"
#include "construction_name.h"

using namespace OHOS::Security::AccessToken;
namespace Contacts {
namespace Test {
static OHOS::Contacts::CharacterTransliterate characterTransliterate;
static OHOS::Contacts::ConstructionName constructionName;
void ContactsGetContainer(const uint8_t *data, size_t size)
{
    std::string value("trans");
    std::wstring value2 = characterTransliterate.StringToWstring(value);
    characterTransliterate.GetContainer(value2);
    OHOS::Contacts::ConstructionName constructionNameObj;
    std::string str(reinterpret_cast<const char *>(data), size);
    constructionNameObj.initials_ = str;
    constructionName.GetConstructionName(value, constructionNameObj);
}
void ContactsGetSortKey(const uint8_t *data, size_t size)
{
    std::string value("trans");
    std::wstring value2 = characterTransliterate.StringToWstring(value);
    characterTransliterate.getSortKey(value2);
    OHOS::Contacts::ConstructionName constructionNameObj;
    std::string str(reinterpret_cast<const char *>(data), size);
    constructionNameObj.initials_ = str;
    constructionName.GetConstructionName(value, constructionNameObj);
}
void ContactsGetCombinedVector(const uint8_t *data, size_t size)
{
    std::string detailInfo("trans");
    std::wstring detailInfo2 = characterTransliterate.StringToWstring(detailInfo);
    std::vector<std::wstring> valueChild;
    valueChild.push_back(detailInfo2);
    std::vector<std::vector<std::wstring>> value;
    value.push_back(valueChild);
    characterTransliterate.GetCombinedVector(value);
    OHOS::Contacts::ConstructionName constructionNameObj;
    std::string str(reinterpret_cast<const char *>(data), size);
    constructionNameObj.initials_ = str;
    constructionName.GetConstructionName(detailInfo, constructionNameObj);
}
void ContactsJoin(const uint8_t *data, size_t size)
{
    std::string detailInfoOne("trans");
    std::wstring detailInfo = characterTransliterate.StringToWstring(detailInfoOne);
    std::vector<std::wstring> valueChild;
    valueChild.push_back(detailInfo);
    std::vector<std::vector<std::wstring>> value;
    value.push_back(valueChild);
    characterTransliterate.Join(value, detailInfo);
    OHOS::Contacts::ConstructionName constructionNameObj;
    std::string str(reinterpret_cast<const char *>(data), size);
    constructionNameObj.initials_ = str;
    constructionName.GetConstructionName(detailInfoOne, constructionNameObj);
}
void ContactsGetConstructionName(const uint8_t *data, size_t size)
{
    std::string chineseCharacter("trans");
    OHOS::Contacts::ConstructionName constructionNameObj;
    std::string str(reinterpret_cast<const char *>(data), size);
    constructionNameObj.initials_ = str;
    constructionName.GetConstructionName(chineseCharacter, constructionNameObj);
}
void ContactsGetPhotoFirstName(const uint8_t *data, size_t size)
{
    std::string disPlayName("trans");
    OHOS::Contacts::ConstructionName constructionNameObj;
    constructionNameObj.disPlayName_ = disPlayName;
    std::string str(reinterpret_cast<const char *>(data), size);
    constructionNameObj.photoFirstName_ = str;
    constructionName.GetPhotoFirstName(constructionNameObj);
}
}  // namespace Test
}  // namespace Contacts

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return -1;
    }
    /* Run your code on data */
    struct FunctionInfo {
        void (*func)(const uint8_t *, size_t);
        const char *func_name;
    };

    const FunctionInfo functions[] = {
        {Contacts::Test::ContactsGetContainer, "ContactsGetContainer"},
        {Contacts::Test::ContactsGetSortKey, "ContactsGetSortKey"},
        {Contacts::Test::ContactsGetCombinedVector, "ContactsGetCombinedVector"},
        {Contacts::Test::ContactsJoin, "ContactsJoin"},
        {Contacts::Test::ContactsGetConstructionName, "ContactsGetConstructionName"},
        {Contacts::Test::ContactsGetPhotoFirstName, "ContactsGetPhotoFirstName"},
    };

    const size_t num_functions = sizeof(functions) / sizeof(functions[0]);
    
    std::vector<std::pair<const uint8_t*, size_t>> segments;
    size_t current_pos = 0;

    for (size_t i = 0; i < num_functions; ++i) {
        size_t segment_size = size / num_functions;
        if (i == num_functions - 1) {
            segment_size = size - current_pos;
        }

        if (segment_size > 0) {
            segments.emplace_back(data + current_pos, segment_size);
            current_pos += segment_size;
        } else {
            segments.emplace_back(nullptr, 0);
        }
    }

    for (size_t i = 0; i < num_functions; ++i) {
        const auto& segment = segments[i];
        const uint8_t* segment_data = segment.first;
        size_t segment_size = segment.second;

        if (segment_data == nullptr || segment_size == 0) {
            continue;
        }

        functions[i].func(segment_data, segment_size);
    }
    return 0;
}