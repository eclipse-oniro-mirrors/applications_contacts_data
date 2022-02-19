/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef CONTACTSDATAABILITY_BASE_TEST_H
#define CONTACTSDATAABILITY_BASE_TEST_H

#include <gtest/gtest.h>

#include "calllog_ability.h"
#include "contacts_data_ability.h"
#include "data_ability_result.h"
#include "hilog_wrapper.h"
#include "voicemail_ability.h"

namespace Contacts {
namespace Test {
class BaseTest : public testing::Test {
public:
    BaseTest();
    ~BaseTest();
    OHOS::AppExecFwk::CallLogAbility calllogAbility;
    OHOS::AppExecFwk::VoiceMailAbility voicemailAbility;
    OHOS::AppExecFwk::ContactsDataAbility contactsDataAbility;
    void CheckResultSet(OHOS::NativeRdb::ValuesBucket &rawContactValues,
        const std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet, std::string testName);
    void CheckData(OHOS::NativeRdb::ValuesBucket &rawContactValues,
        const std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet, std::string &columnName,
        std::string testName);
    void CheckResultSetList(std::vector<OHOS::NativeRdb::ValuesBucket> &valuesVector,
        std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet, std::string testName);
    OHOS::NativeRdb::ValuesBucket GetAllColumnsValues(
        std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr);
    int ContactsRand();
    void InitAbility();
};
} // namespace Test
} // namespace Contacts
#endif // CONTACTSDATAABILITY_BASE_TEST_H
