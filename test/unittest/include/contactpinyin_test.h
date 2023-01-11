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

#ifndef CONTACTPINYIN_TEST_H
#define CONTACTPINYIN_TEST_H

#include "base_test.h"

namespace Contacts {
namespace Test {
class ContactPinyinTest : public BaseTest {
public:
    ContactPinyinTest();
    ~ContactPinyinTest();
    int64_t RawContactInsert(std::string displayName, OHOS::DataShare::DataShareValuesBucket &alues);
    int64_t ContactDataInsert(int64_t rawContactId, std::string contentType, std::string detailInfo,
        std::string position, OHOS::DataShare::DataShareValuesBucket &contactDataValues);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> ContactQuery(const std::string &tableName,
        std::vector<std::string> columns, OHOS::DataShare::DataSharePredicates predicates);
    void QueryAndExpectResult(std::string &tableName, OHOS::DataShare::DataSharePredicates predicates,
        OHOS::DataShare::DataShareValuesBucket &values, std::string testName);
    void ClearData();
};
} // namespace Test
} // namespace Contacts
#endif // CONTACTPINYIN_TEST_H
