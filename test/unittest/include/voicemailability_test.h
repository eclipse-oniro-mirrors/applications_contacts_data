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

#ifndef VOICEMAILABILITY_TEST_H
#define VOICEMAILABILITY_TEST_H

#include "base_test.h"
#include "test_common.h"

namespace Contacts {
namespace Test {
namespace Lock {
std::mutex mtx_;
}
class VoicemailAsync {
public:
    OHOS::NativeRdb::ValuesBucket values;
    std::map<int, OHOS::NativeRdb::ValuesBucket> result;
    OHOS::NativeRdb::ValuesBucket updateValues;
    int predicatesId;
    std::vector<std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet>> resultSet;
    std::vector<int64_t> predicatesQueryId;
    int predicatesDeleteId;
    VoicemailAsync(OHOS::NativeRdb::ValuesBucket &values, std::map<int, OHOS::NativeRdb::ValuesBucket> &result)
    {
        this->values = values;
        this->result = result;
        this->predicatesId = -1;
        this->predicatesDeleteId = -1;
    }
    VoicemailAsync(OHOS::NativeRdb::ValuesBucket &updateValues, int &predicatesId)
    {
        this->updateValues = updateValues;
        this->predicatesId = predicatesId;
        this->predicatesDeleteId = -1;
    }

    VoicemailAsync(std::vector<std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet>> resultSet,
        std::vector<int64_t> predicatesQueryId)
    {
        this->resultSet = resultSet;
        this->predicatesQueryId = predicatesQueryId;
        this->predicatesId = -1;
        this->predicatesDeleteId = -1;
    }

    explicit VoicemailAsync(int &predicatesDeleteId)
    {
        this->predicatesId = -1;
        this->predicatesDeleteId = predicatesDeleteId;
    }
    void Insert()
    {
        OHOS::AppExecFwk::VoiceMailAbility voicemailAbility;
        OHOS::Uri voicemailUri(VoicemailUri::VOICEMAIL);
        int64_t code = voicemailAbility.Insert(voicemailUri, this->values);
        int callLogId = code;
        EXPECT_GT(callLogId, 0);
        mtx_.lock();
        this->result.insert(std::map<int, OHOS::NativeRdb::ValuesBucket>::value_type(callLogId, this->values));
        mtx_.unlock();
        HILOG_INFO("--- VoicemailAsync insert---%{public}s", VoicemailUri::VOICEMAIL);
    }
    void Update()
    {
        OHOS::Uri voicemailUri(VoicemailUri::VOICEMAIL);
        OHOS::NativeRdb::DataAbilityPredicates predicates;
        predicates.EqualTo("id", std::to_string(this->predicatesId));
        OHOS::AppExecFwk::VoiceMailAbility voicemailAbility;
        int resultCode = voicemailAbility.Update(voicemailUri, this->updateValues, predicates);
        EXPECT_EQ(0, resultCode);
        HILOG_INFO("--- VoicemailAsync Update---%{public}s", VoicemailUri::VOICEMAIL);
    }
    void Query()
    {
        OHOS::Uri voicemailUri(VoicemailUri::VOICEMAIL);
        OHOS::NativeRdb::DataAbilityPredicates predicates;
        int size = this->predicatesQueryId.size();
        for (int i = 0; i < size; i++) {
            predicates.EqualTo("id", std::to_string(this->predicatesQueryId[i]));
            if (i < size - 1) {
                predicates.Or();
            }
        }
        OHOS::AppExecFwk::VoiceMailAbility voicemailAbility;
        std::vector<std::string> columns;
        std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
            voicemailAbility.Query(voicemailUri, columns, predicates);
        mtx_.lock();
        this->resultSet.push_back(resultSet);
        mtx_.unlock();
        HILOG_INFO("--- VoicemailAsync Query---%{public}s", VoicemailUri::VOICEMAIL);
    }

    void Delete()
    {
        OHOS::Uri voicemailUri(VoicemailUri::VOICEMAIL);
        OHOS::NativeRdb::DataAbilityPredicates predicates;
        predicates.EqualTo("id", std::to_string(this->predicatesDeleteId));
        OHOS::AppExecFwk::VoiceMailAbility voicemailAbility;
        int resultCode = voicemailAbility.Delete(voicemailUri, predicates);
        HILOG_INFO("--- VoicemailAsync Delete---%{public}s", VoicemailUri::VOICEMAIL);
        EXPECT_EQ(0, resultCode);
    }
};

class VoicemailAbilityTest : public BaseTest {
public:
    VoicemailAbilityTest();
    ~VoicemailAbilityTest();
    int64_t VoicemailStatusInsert(std::string phoneNumber, int status);
    int VoicemailUpdate(OHOS::NativeRdb::ValuesBucket updateValues, OHOS::NativeRdb::DataAbilityPredicates predicates);
    int VoicemailDelete(OHOS::NativeRdb::DataAbilityPredicates predicates);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> VoicemailQuery(
        std::vector<std::string> columns, OHOS::NativeRdb::DataAbilityPredicates predicates);
    OHOS::NativeRdb::ValuesBucket GetVoiceMailValues(
        int columnsStart, int columnsEnd, std::vector<std::string> &columns);
    void GetVoiceMailValuesColumn(std::vector<std::string> &columns);
    int VoicemailInsertValues(OHOS::NativeRdb::ValuesBucket &values);
    int64_t VoicemailInsertValue(std::string displayName, OHOS::NativeRdb::ValuesBucket &values);
    void CheckResultSetBatch(OHOS::NativeRdb::DataAbilityPredicates predicates, std::vector<std::string> &columns,
        OHOS::NativeRdb::ValuesBucket values, std::string testName);
    void ClearVoicemail();
};
} // namespace Test
} // namespace Contacts
#endif // VOICEMAILABILITY_TEST_H