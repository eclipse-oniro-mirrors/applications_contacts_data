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
    int64_t RawContactInsert(std::string displayName, OHOS::DataShare::DataShareValuesBucket &alues);
    int64_t RawContactExpandInsert(
        std::vector<std::string> valueVector, int isFavorite, OHOS::DataShare::DataShareValuesBucket &rawContactValues);
    int64_t RawContactLastContactedInsert(
        std::string displayName, int lastestContactedTime, OHOS::DataShare::DataShareValuesBucket &rawContactValues);
    int64_t ContactDataInsert(int64_t rawContactId, std::string contentType, std::string detailInfo,
        std::string position, OHOS::DataShare::DataShareValuesBucket &contactDataValues);
    int64_t GroupsInsert(std::string groupName, OHOS::DataShare::DataShareValuesBucket &groupValues);
    int64_t ContactBlocklistInsert(std::string phoneNumber, OHOS::DataShare::DataShareValuesBucket &rawContactValues);
    int ContactUpdate(const std::string &tableName, OHOS::DataShare::DataShareValuesBucket updateValues,
        OHOS::DataShare::DataSharePredicates predicates);
    int ContactDelete(const std::string &tableName, OHOS::DataShare::DataSharePredicates predicates);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> ContactQuery(const std::string &tableName,
        std::vector<std::string> &columns, OHOS::DataShare::DataSharePredicates predicates);
    void QueryAndExpectResult(std::string &tableName, OHOS::DataShare::DataSharePredicates predicates,
        OHOS::DataShare::DataShareValuesBucket &values, std::string testName);
    OHOS::DataShare::DataShareValuesBucket GetAllColumnsValues(
        std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    void GetAllRawContactColumns(std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    void GetAllContactDataColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr);
    void GetAllGroupsColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr);
    void GetAllContactBlocklistColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr);
    void GetDetailsContactDataColumns(std::vector<std::string> &columns);
    void MergeColumns(
        std::vector<std::string> &columns, std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    int64_t RawContactInsertValues(OHOS::DataShare::DataShareValuesBucket &values);
    int64_t ContactDataInsertValues(OHOS::DataShare::DataShareValuesBucket &values);
    int64_t GroupsInsertValues(OHOS::DataShare::DataShareValuesBucket &values);
    int64_t ContactBlocklistInsertValues(OHOS::DataShare::DataShareValuesBucket &values);
    std::vector<OHOS::DataShare::DataShareValuesBucket> GetBatchList(int64_t rawContactId);
    void ClearContacts();
};
} // namespace Test
} // namespace Contacts
#endif // CONTACTPROFILE_TEST_H
