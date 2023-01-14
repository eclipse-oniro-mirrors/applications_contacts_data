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

#ifndef CONTACTABILITY_TEST_H
#define CONTACTABILITY_TEST_H

#include "base_test.h"
#include "test_common.h"

namespace Contacts {
namespace Test {
namespace Lock {
std::mutex contactsMtx_;
}

class ContactAsync {
public:
    OHOS::DataShare::DataShareValuesBucket values;
    std::map<int, OHOS::DataShare::DataShareValuesBucket> result;
    OHOS::DataShare::DataShareValuesBucket updateValues;
    int predicatesId;
    std::vector<std::shared_ptr<OHOS::DataShare::DataShareResultSet>> resultSet;
    std::vector<int64_t> predicatesQueryId;
    int predicatesDeleteId;
    ContactAsync(OHOS::DataShare::DataShareValuesBucket &values, std::map<int,
        OHOS::DataShare::DataShareValuesBucket> &result)
    {
        this->values = values;
        this->result = result;
        this->predicatesId = -1;
        this->predicatesDeleteId = -1;
    }
    ContactAsync(OHOS::DataShare::DataShareValuesBucket &updateValues, int &predicatesId)
    {
        this->updateValues = updateValues;
        this->predicatesId = predicatesId;
        this->predicatesDeleteId = -1;
    }

    ContactAsync(std::vector<std::shared_ptr<OHOS::DataShare::DataShareResultSet>> resultSet,
        std::vector<int64_t> predicatesQueryId)
    {
        this->resultSet = resultSet;
        this->predicatesQueryId = predicatesQueryId;
        this->predicatesId = -1;
        this->predicatesDeleteId = -1;
    }

    explicit ContactAsync(int &predicatesDeleteId)
    {
        this->predicatesId = -1;
        this->predicatesDeleteId = predicatesDeleteId;
    }
    void Insert()
    {
        OHOS::AbilityRuntime::ContactsDataAbility contactsDataAbility;
        OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
        int64_t code = contactsDataAbility.Insert(uriRawContact, this->values);
        int rawContactId = code;
        EXPECT_GT(rawContactId, 0);
        Lock::contactsMtx_.lock();
        this->result.insert(std::map<int, OHOS::DataShare::DataShareValuesBucket>::value_type(rawContactId,
            this->values));
        Lock::contactsMtx_.unlock();
        HILOG_INFO("--- VoicemailAsync Insert---%{public}s", ContactsUri::RAW_CONTACT);
    }
    void Update()
    {
        OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
        OHOS::DataShare::DataSharePredicates predicates;
        predicates.EqualTo("id", std::to_string(this->predicatesId));
        OHOS::AbilityRuntime::ContactsDataAbility contactsDataAbility;
        int resultCode = contactsDataAbility.Update(uriRawContact, predicates, this->updateValues);
        EXPECT_EQ(0, resultCode);
        HILOG_INFO("--- VoicemailAsync Update---%{public}s", ContactsUri::RAW_CONTACT);
    }
    void Query()
    {
        OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
        OHOS::DataShare::DataSharePredicates predicates;
        int size = this->predicatesQueryId.size();
        for (int i = 0; i < size; i++) {
            predicates.EqualTo("id", std::to_string(this->predicatesQueryId[i]));
            if (i < size - 1) {
                predicates.Or();
            }
        }
        OHOS::AbilityRuntime::ContactsDataAbility contactsDataAbility;
        std::vector<std::string> columns;
        std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
            contactsDataAbility.Query(uriRawContact, predicates, columns);
        Lock::contactsMtx_.lock();
        this->resultSet.push_back(resultSet);
        Lock::contactsMtx_.unlock();
        HILOG_INFO("--- VoicemailAsync Query---%{public}s", ContactsUri::RAW_CONTACT);
    }

    void Delete()
    {
        OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
        OHOS::DataShare::DataSharePredicates predicates;
        predicates.EqualTo("id", std::to_string(this->predicatesDeleteId));
        OHOS::AbilityRuntime::ContactsDataAbility contactsDataAbility;
        int resultCode = contactsDataAbility.Delete(uriRawContact, predicates);
        EXPECT_EQ(0, resultCode);
        HILOG_INFO("--- VoicemailAsync Delete---%{public}s", ContactsUri::RAW_CONTACT);
    }
};

class ContactAbilityTest : public BaseTest {
public:
    static constexpr int SLEEP_TIME = 2;

    ContactAbilityTest();
    ~ContactAbilityTest();
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
#endif // CONTACTABILITY_TEST_H
