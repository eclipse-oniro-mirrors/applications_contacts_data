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

#ifndef CONTACTGROUP_TEST_H
#define CONTACTGROUP_TEST_H

#include "base_test.h"

namespace Contacts {
namespace Test {
class ContactGroupTest : public BaseTest {
public:
    ContactGroupTest();
    ~ContactGroupTest();
    int64_t GroupsInsert(std::string groupName, OHOS::DataShare::DataShareValuesBucket &groupValues);
    int ContactUpdate(const std::string &tableName, OHOS::DataShare::DataShareValuesBucket updateValues,
        OHOS::DataShare::DataSharePredicates predicates);
    int ContactDelete(const std::string &tableName, OHOS::DataShare::DataSharePredicates predicates);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> ContactQuery(const std::string &tableName,
        std::vector<std::string> columns, OHOS::DataShare::DataSharePredicates predicates);
    void QueryAndExpectResult(const std::string &tableName, OHOS::DataShare::DataSharePredicates predicates,
        OHOS::DataShare::DataShareValuesBucket &values, std::string testName);
    OHOS::DataShare::DataShareValuesBucket GetAllColumnsValues(
        std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    int64_t GroupsInsertValues(OHOS::DataShare::DataShareValuesBucket &values);
    void GetAllGroupsColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr);
    void MergeColumns(
        std::vector<std::string> &columns, std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    void ClearData();
};
} // namespace Test
} // namespace Contacts
#endif // CONTACTGROUP_TEST_H
