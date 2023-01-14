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

#ifndef CALLLOGABILITY_TEST_H
#define CALLLOGABILITY_TEST_H

#include "base_test.h"
#include "test_common.h"

namespace Contacts {
namespace Test {
namespace Lock {
std::mutex calllogMtx_;
}

class CallLogAsync {
public:
    OHOS::DataShare::DataShareValuesBucket values;
    std::map<int, OHOS::DataShare::DataShareValuesBucket> result;
    OHOS::DataShare::DataShareValuesBucket updateValues;
    int predicatesId;
    std::vector<std::shared_ptr<OHOS::DataShare::DataShareResultSet>> resultSet;
    std::vector<int64_t> predicatesQueryId;
    int predicatesDeleteId;
    CallLogAsync(OHOS::DataShare::DataShareValuesBucket &values,
        std::map<int, OHOS::DataShare::DataShareValuesBucket> &result)
    {
        this->values = values;
        this->result = result;
        this->predicatesId = -1;
        this->predicatesDeleteId = -1;
    }
    CallLogAsync(OHOS::DataShare::DataShareValuesBucket &updateValues, int &predicatesId)
    {
        this->updateValues = updateValues;
        this->predicatesId = predicatesId;
        this->predicatesDeleteId = -1;
    }

    CallLogAsync(std::vector<std::shared_ptr<OHOS::DataShare::DataShareResultSet>> resultSet,
        std::vector<int64_t> predicatesQueryId)
    {
        this->resultSet = resultSet;
        this->predicatesQueryId = predicatesQueryId;
        this->predicatesId = -1;
        this->predicatesDeleteId = -1;
    }

    explicit CallLogAsync(int &predicatesDeleteId)
    {
        this->predicatesId = -1;
        this->predicatesDeleteId = predicatesDeleteId;
    }
    void Insert()
    {
        OHOS::AbilityRuntime::CallLogAbility calllogAbility;
        OHOS::Uri callLogUri(CallLogUri::CALL_LOG);
        int64_t code = calllogAbility.Insert(callLogUri, this->values);
        int callLogId = code;
        EXPECT_GT(callLogId, 0);
        Lock::calllogMtx_.lock();
        this->result.insert(std::map<int, OHOS::DataShare::DataShareValuesBucket>::value_type(callLogId, this->values));
        Lock::calllogMtx_.unlock();
        HILOG_INFO("--- VoicemailAsync Insert---%{public}s", CallLogUri::CALL_LOG);
    }
    void Update()
    {
        OHOS::Uri callLogUri(CallLogUri::CALL_LOG);
        OHOS::DataShare::DataSharePredicates predicates;
        predicates.EqualTo("id", std::to_string(this->predicatesId));
        OHOS::AbilityRuntime::CallLogAbility calllogAbility;
        int resultCode = calllogAbility.Update(callLogUri, predicates, this->updateValues);
        EXPECT_EQ(0, resultCode);
        HILOG_INFO("--- VoicemailAsync Update---%{public}s", CallLogUri::CALL_LOG);
    }
    void Query()
    {
        OHOS::Uri callLogUri(CallLogUri::CALL_LOG);
        OHOS::DataShare::DataSharePredicates predicates;
        int size = this->predicatesQueryId.size();
        for (int i = 0; i < size; i++) {
            predicates.EqualTo("id", std::to_string(this->predicatesQueryId[i]));
            if (i < size - 1) {
                predicates.Or();
            }
        }
        OHOS::AbilityRuntime::CallLogAbility calllogAbility;
        std::vector<std::string> columns;
        std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
            calllogAbility.Query(callLogUri, predicates, columns);
        Lock::calllogMtx_.lock();
        this->resultSet.push_back(resultSet);
        Lock::calllogMtx_.unlock();
        HILOG_INFO("--- VoicemailAsync Query---%{public}s", CallLogUri::CALL_LOG);
    }

    void Delete()
    {
        OHOS::Uri callLogUri(CallLogUri::CALL_LOG);
        OHOS::DataShare::DataSharePredicates predicates;
        predicates.EqualTo("id", std::to_string(this->predicatesDeleteId));
        OHOS::AbilityRuntime::CallLogAbility calllogAbility;
        int resultCode = calllogAbility.Delete(callLogUri, predicates);
        EXPECT_EQ(0, resultCode);
        HILOG_INFO("--- VoicemailAsync Delete---%{public}s", CallLogUri::CALL_LOG);
    }
};

class CalllogAbilityTest : public BaseTest {
public:
    CalllogAbilityTest();
    ~CalllogAbilityTest();
    int64_t CalllogInsert(std::string phoneNumber);
    int CalllogUpdate(OHOS::DataShare::DataShareValuesBucket updateValues,
        OHOS::DataShare::DataSharePredicates predicates);
    int CalllogDelete(OHOS::DataShare::DataSharePredicates predicates);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> CalllogQuery(
        std::vector<std::string> columns, OHOS::DataShare::DataSharePredicates predicates);
    OHOS::DataShare::DataShareValuesBucket GetCallLogValues(int columnsStart, int columnsEnd,
        std::vector<std::string> &columns);
    void GetAllValuesColumn(std::vector<std::string> &columns);
    int64_t CalllogInsertValues(OHOS::DataShare::DataShareValuesBucket &values);
    int64_t CalllogInsertValue(std::string displayName, OHOS::DataShare::DataShareValuesBucket &values);
    void ClearCallLog();
};
} // namespace Test
} // namespace Contacts
#endif // CALLLOGABILITY_TEST_H
