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
    int64_t GroupsInsert(std::string groupName, OHOS::NativeRdb::ValuesBucket &groupValues);
    int ContactUpdate(const std::string &tableName, OHOS::NativeRdb::ValuesBucket updateValues,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
    int ContactDelete(const std::string &tableName, OHOS::NativeRdb::DataAbilityPredicates predicates);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactQuery(const std::string &tableName,
        std::vector<std::string> columns, OHOS::NativeRdb::DataAbilityPredicates predicates);
    void QueryAndExpectResult(const std::string &tableName, OHOS::NativeRdb::DataAbilityPredicates predicates,
        OHOS::NativeRdb::ValuesBucket &values, std::string testName);
    OHOS::NativeRdb::ValuesBucket GetAllColumnsValues(
        std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    int64_t GroupsInsertValues(OHOS::NativeRdb::ValuesBucket &values);
    void GetAllGroupsColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr);
    void MergeColumns(
        std::vector<std::string> &columns, std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    void ClearData();
};
} // namespace Test
} // namespace Contacts
#endif // CONTACTGROUP_TEST_H
