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

#ifndef CONTACTPROFILE_TEST_H
#define CONTACTPROFILE_TEST_H

#include "base_test.h"

namespace Contacts {
namespace Test {
class ContactProfileTest : public BaseTest {
public:
    static constexpr int SLEEP_TIME = 2;

    ContactProfileTest();
    ~ContactProfileTest();
    int64_t RawContactInsert(std::string displayName, OHOS::NativeRdb::ValuesBucket &alues);
    int64_t RawContactExpandInsert(
        std::vector<std::string> valueVector, int isFavorite, OHOS::NativeRdb::ValuesBucket &rawContactValues);
    int64_t RawContactLastContactedInsert(
        std::string displayName, int lastestContactedTime, OHOS::NativeRdb::ValuesBucket &rawContactValues);
    int64_t ContactDataInsert(int64_t rawContactId, std::string contentType, std::string detailInfo,
        std::string position, OHOS::NativeRdb::ValuesBucket &contactDataValues);
    int64_t GroupsInsert(std::string groupName, OHOS::NativeRdb::ValuesBucket &groupValues);
    int64_t ContactBlocklistInsert(std::string phoneNumber, OHOS::NativeRdb::ValuesBucket &rawContactValues);
    int ContactUpdate(const std::string &tableName, OHOS::NativeRdb::ValuesBucket updateValues,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
    int ContactDelete(const std::string &tableName, OHOS::NativeRdb::DataAbilityPredicates predicates);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactQuery(const std::string &tableName,
        std::vector<std::string> &columns, OHOS::NativeRdb::DataAbilityPredicates predicates);
    void QueryAndExpectResult(std::string &tableName, OHOS::NativeRdb::DataAbilityPredicates predicates,
        OHOS::NativeRdb::ValuesBucket &values, std::string testName);
    OHOS::NativeRdb::ValuesBucket GetAllColumnsValues(
        std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    void GetAllRawContactColumns(std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    void GetAllContactDataColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr);
    void GetAllGroupsColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr);
    void GetAllContactBlocklistColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr);
    void GetDetailsContactDataColumns(std::vector<std::string> &columns);
    void MergeColumns(
        std::vector<std::string> &columns, std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    int64_t RawContactInsertValues(OHOS::NativeRdb::ValuesBucket &values);
    int64_t ContactDataInsertValues(OHOS::NativeRdb::ValuesBucket &values);
    int64_t GroupsInsertValues(OHOS::NativeRdb::ValuesBucket &values);
    int64_t ContactBlocklistInsertValues(OHOS::NativeRdb::ValuesBucket &values);
    std::vector<OHOS::NativeRdb::ValuesBucket> GetBatchList(int64_t rawContactId);
    void ClearContacts();
};
} // namespace Test
} // namespace Contacts
#endif // CONTACTPROFILE_TEST_H