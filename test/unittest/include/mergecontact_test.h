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

#ifndef MERGECONTACT_TEST_H
#define MERGECONTACT_TEST_H

#include "base_test.h"

namespace Contacts {
namespace Test {
class MergeContactTest : public BaseTest {
public:
    int64_t RawContactInsert(std::string displayName);
    int64_t ContactDataInsert(
        int64_t rawContactId, std::string contentType, std::string detailInfo, std::string position);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> ContactQuery(
        std::vector<std::string> columns, OHOS::DataShare::DataSharePredicates predicates);
    void CheckMergeResultId(std::vector<int64_t> ids, bool isMerge);
    int GetMergeResultContactId(const std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet);
    std::vector<int> GetMergeResultRawContactId(const std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet);
    std::vector<int> GetMergeRawContactId(const std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet);
    void DeleteRawContact();
};
} // namespace Test
} // namespace Contacts
#endif // MERGECONTACT_TEST_H
