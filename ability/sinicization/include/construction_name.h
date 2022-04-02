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

#ifndef CONSTRUCTION_NAME_H
#define CONSTRUCTION_NAME_H

#include "common.h"

namespace OHOS {
namespace Contacts {
class ConstructionName {
public:
    ConstructionName();
    ~ConstructionName();
    int sortFirstLetterCode_ = 0;
    // The first letter of Pinyin
    std::string initials_;
    // Full-Pinyin
    std::string nameFullFight_;
    std::string sortFirstLetter_;
    std::string disPlayName_;
    std::string photoFirstName_;
    static std::string local;
    ConstructionName GetConstructionName(std::string &chineseCharacter, ConstructionName &constructionName);
    ConstructionName GetPhotoFirstName(ConstructionName &constructionName);
};
} // namespace Contacts
} // namespace OHOS
#endif // CONSTRUCTION_NAME_H
