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

#include "voicemailability_test.h"

#include "data_ability_operation_builder.h"
#include "random_number_utils.h"

using namespace OHOS::Contacts;

namespace Contacts {
namespace Test {
VoicemailAbilityTest::VoicemailAbilityTest()
{
}

VoicemailAbilityTest::~VoicemailAbilityTest()
{
}

int64_t VoicemailAbilityTest::VoicemailStatusInsert(std::string phoneNumber, int status)
{
    OHOS::Uri uriVoicemail(VoicemailUri::VOICEMAIL);
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    voicemailValues.Put("phone_number", phoneNumber);
    voicemailValues.Put("voice_status", status);
    int64_t code = voicemailAbility.Insert(uriVoicemail, voicemailValues);
    return code;
}

int VoicemailAbilityTest::VoicemailUpdate(
    OHOS::DataShare::DataShareValuesBucket updateValues, OHOS::DataShare::DataSharePredicates predicates)
{
    OHOS::Uri uriVoicemail(VoicemailUri::VOICEMAIL);
    int code = voicemailAbility.Update(uriVoicemail, predicates, updateValues);
    return code;
}

int VoicemailAbilityTest::VoicemailDelete(OHOS::DataShare::DataSharePredicates predicates)
{
    OHOS::Uri uriVoicemail(VoicemailUri::VOICEMAIL);
    int code = voicemailAbility.Delete(uriVoicemail, predicates);
    return code;
}

std::shared_ptr<OHOS::DataShare::DataShareResultSet> VoicemailAbilityTest::VoicemailQuery(
    std::vector<std::string> columns, OHOS::DataShare::DataSharePredicates predicates)
{
    OHOS::Uri uriVoicemail(VoicemailUri::VOICEMAIL);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        voicemailAbility.Query(uriVoicemail, predicates, columns);
    return resultSet;
}

void VoicemailAbilityTest::CheckResultSetBatch(OHOS::DataShare::DataSharePredicates predicates,
    std::vector<std::string> &columns, OHOS::DataShare::DataShareValuesBucket values, std::string testName)
{
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, resultSet, testName);
}

int VoicemailAbilityTest::VoicemailInsertValues(OHOS::DataShare::DataShareValuesBucket &values)
{
    OHOS::Uri uriVoicemail(VoicemailUri::VOICEMAIL);
    int64_t code = voicemailAbility.Insert(uriVoicemail, values);
    int result = code;
    return result;
}

int64_t VoicemailAbilityTest::VoicemailInsertValue(std::string displayName,
    OHOS::DataShare::DataShareValuesBucket &values)
{
    OHOS::Uri uriVoicemail(VoicemailUri::VOICEMAIL);
    values.Put("display_name", displayName);
    int64_t code = voicemailAbility.Insert(uriVoicemail, values);
    return code;
}

OHOS::DataShare::DataShareValuesBucket VoicemailAbilityTest::GetVoiceMailValues(
    int columnsStart, int columnsEnd, std::vector<std::string> &columns)
{
    std::string voiceMailTestStringValue = std::to_string(ContactsRand());
    OHOS::DataShare::DataShareValuesBucket valuesBucket;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(8);
    valuesBucket.Put("phone_number", phoneNumber + voiceMailTestStringValue);
    valuesBucket.Put("quicksearch_key", "dfquicksearch_key" + voiceMailTestStringValue);
    valuesBucket.Put("display_name", "dfName" + voiceMailTestStringValue);
    valuesBucket.Put("voicemail_uri", "dfUri::voicemail_uri" + voiceMailTestStringValue);
    valuesBucket.Put("voicemail_type", ContactsRand());
    valuesBucket.Put("voice_file_size", ContactsRand());
    valuesBucket.Put("voice_duration", ContactsRand());
    valuesBucket.Put("voice_status", ContactsRand());
    valuesBucket.Put("origin_type", "origin_type" + voiceMailTestStringValue);
    valuesBucket.Put("create_time", ContactsRand());
    std::vector<std::string> columnsTemp;
    GetVoiceMailValuesColumn(columnsTemp);
    int allSize = columnsTemp.size();
    for (int i = 0; i < allSize; i++) {
        if (i >= columnsStart && i < columnsEnd) {
            columns.push_back(columnsTemp[i]);
        } else {
            valuesBucket.Put(columnsTemp[i], "");
        }
    }
    return valuesBucket;
}

void VoicemailAbilityTest::GetVoiceMailValuesColumn(std::vector<std::string> &columns)
{
    columns.push_back("phone_number");
    columns.push_back("quicksearch_key");
    columns.push_back("display_name");
    columns.push_back("voicemail_uri");
    columns.push_back("voicemail_type");
    columns.push_back("voice_file_size");
    columns.push_back("voice_duration");
    columns.push_back("voice_status");
    columns.push_back("origin_type");
    columns.push_back("create_time");
}

void VoicemailAbilityTest::ClearVoicemail()
{
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.GreaterThan("id", "0");
    int deleteCode = VoicemailDelete(predicates);
    EXPECT_EQ(0, deleteCode);
}

/*
 * @tc.number  voicemail_Insert_test_100
 * @tc.name    Add a voice mailbox with status 1 and verify that the insertion was successful
 * @tc.desc    Added ability to voice mailbox status
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Insert_test_100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Insert_test_100 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(6);
    values.Put("phone_number", phoneNumber);
    values.Put("voice_status", 1);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("voicemail_Insert_test_100 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "voicemail_Insert_test_100");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Insert_test_200
 * @tc.name    Add a single piece of voicemail data and verify whether the insertion is successful
 * @tc.desc    New capabilities for voicemail data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Insert_test_200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Insert_test_200 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(6);
    values.Put("phone_number", phoneNumber);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("voicemail_Insert_test_200 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("id");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "voicemail_Insert_test_200");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Insert_test_300
 * @tc.name    Add a full field data to voicemail and verify that the insertion was successful
 * @tc.desc    Added ability to call record
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Insert_test_300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----voicemail_Insert_test_300 is starting!-----");
    std::vector<std::string> columns;
    GetVoiceMailValuesColumn(columns);
    int columnsSize = 10;
    OHOS::DataShare::DataShareValuesBucket valuesBucket = GetVoiceMailValues(0, columnsSize, columns);
    int rawId = VoicemailInsertValues(valuesBucket);
    EXPECT_GT(rawId, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(rawId));
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, predicates);
    // resultSet count 1
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);

    CheckResultSet(valuesBucket, resultSet, "voicemail_Insert_test_300");
    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(rawId));
    // test end delete data
    int deleteCode = VoicemailDelete(predicates2);
    EXPECT_EQ(deleteCode, 0);
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Update_test_400
 * @tc.name    Delete the voicemail status and verify that the deletion was successful
 * @tc.desc    Voicemail status deletion capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Update_test_400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Update_test_400 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(4);
    values.Put("phone_number", phoneNumber);
    values.Put("voice_status", 1);
    int voicemailId = VoicemailInsertValues(values);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataShareValuesBucket updateValues;
    updateValues.Put("voice_status", 0);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    int updateCode = VoicemailUpdate(updateValues, predicates);
    EXPECT_EQ(updateCode, 0);

    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("voice_status", "0");
    predicates2.And();
    predicates2.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(updateValues, result, "voicemail_Update_test_400");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Delete_test_500
 * @tc.name    Delete a single voice mailbox data and verify whether the deletion is successful
 * @tc.desc    Voicemail data deletion capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Delete_test_500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Delete_test_500 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(6);
    values.Put("phone_number", phoneNumber);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("voicemail_Delete_test_500 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    int deleteCode = VoicemailDelete(predicates);
    HILOG_INFO("voicemail_Delete_test_500: deleteCode = %{public}d", deleteCode);
    EXPECT_EQ(deleteCode, 0);

    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("id");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    result->Close();
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Delete_test_600
 * @tc.name    Insert 3 pieces of voicemail data first, then delete 2 of them, and verify whether the deletion is
 * successful
 * @tc.desc    Voicemail data deletion capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Delete_test_600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Delete_test_600 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket valuesOne;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(7);
    valuesOne.Put("phone_number", phoneNumber);
    int voicemailIdOne = VoicemailInsertValues(valuesOne);
    HILOG_INFO("voicemail_Delete_test_600 : voicemailIdOne = %{public}d", voicemailIdOne);
    EXPECT_GT(voicemailIdOne, 0);

    OHOS::DataShare::DataShareValuesBucket valuesTwo;
    valuesTwo.Put("phone_number", phoneNumber);
    int voicemailIdTwo = VoicemailInsertValues(valuesTwo);
    HILOG_INFO("voicemail_Delete_test_600 : voicemailIdTwo = %{public}d", voicemailIdTwo);
    EXPECT_GT(voicemailIdTwo, 0);

    OHOS::DataShare::DataShareValuesBucket valuesThree;
    valuesThree.Put("phone_number", phoneNumber);
    int voicemailIdThree = VoicemailInsertValues(valuesThree);
    HILOG_INFO("voicemail_Delete_test_600 : voicemailIdThree = %{public}d", voicemailIdThree);
    EXPECT_GT(voicemailIdThree, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(voicemailIdThree));
    int deleteCode = VoicemailDelete(predicates);
    HILOG_INFO("voicemail_Delete_test_600: deleteCode = %{public}d", deleteCode);
    EXPECT_EQ(deleteCode, 0);

    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("id");
    valuesTwo.Put("id", voicemailIdTwo);
    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(voicemailIdOne));
    predicates2.Or();
    predicates2.EqualTo("id", std::to_string(voicemailIdTwo));
    predicates2.Or();
    predicates2.EqualTo("id", std::to_string(voicemailIdThree));
    predicates2.OrderByAsc("id");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(valuesTwo, result, "voicemail_Delete_test_600");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Delete_test_700
 * @tc.name    Delete all fields of single voice mailbox data, and verify whether the deletion is successful
 * @tc.desc    Call log deletion capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Delete_test_700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----voicemail_Delete_test_700 is starting!-----");
    // insert
    std::vector<std::string> columns;
    GetVoiceMailValuesColumn(columns);
    int columnsSize = 10;
    OHOS::DataShare::DataShareValuesBucket valuesBucket = GetVoiceMailValues(0, columnsSize, columns);
    int rawId = VoicemailInsertValues(valuesBucket);
    EXPECT_GT(rawId, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(rawId));
    // test end delete data
    int deleteCode = VoicemailDelete(predicates);
    EXPECT_EQ(deleteCode, 0);

    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSetDeleteQuery = VoicemailQuery(columns, predicates);
    // resultSet count 0
    int rowCount = 0;
    resultSetDeleteQuery->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    resultSetDeleteQuery->Close();
    HILOG_INFO("-----voicemail_Delete_test_700 is edn!-----");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Delete_test_800
 * @tc.name    Delete voice mailbox data in batch and verify whether the deletion is successful
 * @tc.desc    delete ability to call records in batches
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Delete_test_800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Delete_test_800 is starting!---");
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(6);
    voicemailValues.Put("phone_number", phoneNumber);
    int voicemailIdOne = VoicemailInsertValues(voicemailValues);
    EXPECT_GT(voicemailIdOne, 0);
    voicemailValues.Put("phone_number", phoneNumber);
    int voicemailIdTwo = VoicemailInsertValues(voicemailValues);
    EXPECT_GT(voicemailIdTwo, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.GreaterThan("id", "0");
    std::shared_ptr<OHOS::DataShare::DataSharePredicates> executePredicates =
        std::make_shared<OHOS::DataShare::DataSharePredicates>(predicates);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(VoicemailUri::VOICEMAIL);
    std::shared_ptr<OHOS::DataShare::DataShareOperation> operation =
        OHOS::DataShare::DataShareOperation::NewDeleteBuilder(uri)
            ->WithPredicatesBackReference(0, 0)
            ->WithPredicates(executePredicates)
            ->WithInterruptionAllowed(true)
            ->Build();
    std::vector<std::shared_ptr<OHOS::DataShare::DataShareOperation>> executeBatchOperations;
    executeBatchOperations.push_back(operation);
    InitAbility();
    std::vector<std::shared_ptr<OHOS::DataShare::DataShareResult>> dataShareResult =
        voicemailAbility.ExecuteBatch(executeBatchOperations);
    EXPECT_EQ(0, dataShareResult[0]->GetCount());
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSetInsertQuery = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    resultSetInsertQuery->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Update_test_900
 * @tc.name    Update the voice mailbox status and verify whether the update is successful (voice_status changes from 0
 * to 1)
 * @tc.desc    Voicemail status update capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Update_test_900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Update_test_900 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(9);
    values.Put("phone_number", phoneNumber);
    values.Put("voice_status", 0);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("voicemail_Update_test_900 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataShareValuesBucket updateValues;
    updateValues.Put("voice_status", 1);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    int updateCode = VoicemailUpdate(updateValues, predicates);
    EXPECT_EQ(updateCode, 0);

    updateValues.Put("phone_number", phoneNumber);
    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(updateValues, result, "voicemail_Update_test_900");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Update_test_1000
 * @tc.name    FUpdate single voicemail data and verify whether the update is successful
 * @tc.desc    Voicemail data update capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Update_test_1000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Update_test_1000 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(5);
    values.Put("phone_number", phoneNumber);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("voicemail_Update_test_1000 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataShareValuesBucket updateValues;
    std::string phoneNumber_test = randomNumberUtils.Generating(11);
    updateValues.Put("phone_number", std::string(phoneNumber_test));
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    int updateCode = VoicemailUpdate(updateValues, predicates);
    HILOG_INFO("voicemail_Update_test_1000: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);

    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(updateValues, result, "voicemail_Update_test_1000");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Update_test_1100
 * @tc.name    Insert 3 pieces of voicemail data, modify 2 of them, and verify whether the modification is successful
 * @tc.desc    Voicemail data update capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Update_test_1100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Update_test_1100 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket valuesOne;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(9);
    valuesOne.Put("phone_number", phoneNumber);
    int voicemailIdOne = VoicemailInsertValues(valuesOne);
    EXPECT_GT(voicemailIdOne, 0);

    OHOS::DataShare::DataShareValuesBucket valuesTwo;
    valuesTwo.Put("phone_number", phoneNumber);
    int voicemailIdTwo = VoicemailInsertValues(valuesTwo);
    EXPECT_GT(voicemailIdTwo, 0);

    OHOS::DataShare::DataShareValuesBucket valuesThree;
    valuesThree.Put("phone_number", phoneNumber);
    int voicemailIdThree = VoicemailInsertValues(valuesThree);
    EXPECT_GT(voicemailIdThree, 0);

    OHOS::DataShare::DataShareValuesBucket updateValues;
    std::string phoneNumber_test = randomNumberUtils.Generating(8);
    updateValues.Put("phone_number", std::string(phoneNumber_test));
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailIdOne));
    int updateCode = VoicemailUpdate(updateValues, predicates);
    EXPECT_EQ(updateCode, 0);

    OHOS::DataShare::DataShareValuesBucket updateValuesTwo;
    std::string phoneNumber_test_name = randomNumberUtils.Generating(7);
    updateValuesTwo.Put("phone_number", std::string(phoneNumber_test_name));
    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(voicemailIdTwo));
    updateCode = VoicemailUpdate(updateValuesTwo, predicates2);
    EXPECT_EQ(updateCode, 0);

    std::vector<std::string> columns;
    columns.push_back("phone_number");
    OHOS::DataShare::DataSharePredicates predicates3;
    predicates3.EqualTo("id", std::to_string(voicemailIdOne));
    predicates3.Or();
    predicates3.EqualTo("id", std::to_string(voicemailIdTwo));
    predicates3.OrderByAsc("id");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates3);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<OHOS::DataShare::DataShareValuesBucket> valuesVector;
    valuesVector.push_back(updateValues);
    valuesVector.push_back(updateValuesTwo);
    CheckResultSetList(valuesVector, result, "voicemail_Update_test_1100");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Update_test_1200
 * @tc.name    Update the full field data of voice mailbox and verify whether the update is successful
 * @tc.desc    Support all call log update capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Update_test_1200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----voicemail_Update_test_1200 is starting!-----");
    std::vector<std::string> columns;
    GetVoiceMailValuesColumn(columns);
    int columnsSize = 10;
    OHOS::DataShare::DataShareValuesBucket valuesBucket = GetVoiceMailValues(0, columnsSize, columns);
    int rawId = VoicemailInsertValues(valuesBucket);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(rawId));
    OHOS::DataShare::DataShareValuesBucket upDateValuesBucket = GetVoiceMailValues(0, columnsSize, columns);
    int upDateCode = VoicemailUpdate(upDateValuesBucket, predicates);
    EXPECT_EQ(upDateCode, 0);

    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, predicates);
    // resultSet count 1
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);

    CheckResultSet(upDateValuesBucket, resultSet, "voicemail_Update_test_1200");
    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(rawId));
    // test end delete data
    int deleteCode = VoicemailDelete(predicates2);
    EXPECT_EQ(deleteCode, 0);
    HILOG_INFO("-----voicemail_Update_test_1200 is edn!-----");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Update_test_1300
 * @tc.name    Batch update voice mailbox data and verify whether the update is successful
 * @tc.desc    update ability to call records in batches
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Update_test_1300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Update_test_1300 is starting!---");
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(6);
    voicemailValues.Put("phone_number", phoneNumber);
    int voicemailIdOne = VoicemailInsertValues(voicemailValues);
    EXPECT_GT(voicemailIdOne, 0);
    voicemailValues.Put("phone_number", phoneNumber);
    int voicemailIdTwo = VoicemailInsertValues(voicemailValues);
    EXPECT_GT(voicemailIdTwo, 0);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(voicemailIdTwo));
    std::shared_ptr<OHOS::DataShare::DataShareValuesBucket> values =
        std::make_shared<OHOS::DataShare::DataShareValuesBucket>(voicemailValues);
    std::shared_ptr<OHOS::DataShare::DataSharePredicates> executePredicates =
        std::make_shared<OHOS::DataShare::DataSharePredicates>(predicates);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(VoicemailUri::VOICEMAIL);
    std::shared_ptr<OHOS::DataShare::DataShareOperation> operation =
        OHOS::DataShare::DataShareOperation::NewUpdateBuilder(uri)
            ->WithValuesBucket(values)
            ->WithPredicatesBackReference(0, 0)
            ->WithPredicates(executePredicates)
            ->WithInterruptionAllowed(true)
            ->Build();
    std::vector<std::shared_ptr<OHOS::DataShare::DataShareOperation>> executeBatchOperations;
    executeBatchOperations.push_back(operation);
    InitAbility();
    std::vector<std::shared_ptr<OHOS::DataShare::DataShareResult>> dataShareResult =
        voicemailAbility.ExecuteBatch(executeBatchOperations);
    EXPECT_EQ(0, dataShareResult[0]->GetCount());
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSetInsertQuery = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    resultSetInsertQuery->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<OHOS::DataShare::DataShareValuesBucket> listValue;
    listValue.push_back(voicemailValues);
    listValue.push_back(voicemailValues);
    CheckResultSetList(listValue, resultSetInsertQuery, "voicemail_Update_test_1300");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Query_test_1400
 * @tc.name    Query the voice mailbox in status 1 and return the query results
 * @tc.desc    Voicemail status query capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Query_test_1400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Query_test_1400 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(9);
    values.Put("phone_number", phoneNumber);
    values.Put("voice_status", 1);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("voicemail_Query_test_1400 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    values.Put("id", voicemailId);
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("voice_status", "1");
    predicates.And();
    predicates.EqualTo("id", std::to_string(voicemailId));
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "voicemail_Query_test_1400");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Query_test_1500
 * @tc.name    Insert a single voice mailbox and query the inserted voice mailbox data according to the returned ID
 * @tc.desc    Voicemail data query capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Query_test_1500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Query_test_1500 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(5);
    values.Put("phone_number", phoneNumber);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("voicemail_Query_test_1500 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_number");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "voicemail_Query_test_1500");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Query_test_1600
 * @tc.name    Insert multiple voice mailboxes, and query the inserted voice mailbox data according to the returned ID
 * @tc.desc    Voicemail data query capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Query_test_1600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Query_test_1600 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(5);
    values.Put("phone_number", phoneNumber);
    int voicemailIdOne = VoicemailInsertValues(values);
    HILOG_INFO("voicemail_Query_test_1600 : voicemailIdOne = %{public}d", voicemailIdOne);
    EXPECT_GT(voicemailIdOne, 0);

    OHOS::DataShare::DataShareValuesBucket valuesTwo;
    std::string phoneNumber_test = randomNumberUtils.Generating(6);
    valuesTwo.Put("phone_number", phoneNumber_test);
    int voicemailIdTwo = VoicemailInsertValues(valuesTwo);
    HILOG_INFO("voicemail_Query_test_1600 : voicemailIdTwo = %{public}d", voicemailIdTwo);
    EXPECT_GT(voicemailIdTwo, 0);

    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_number");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(voicemailIdTwo));
    predicates.OrderByAsc("id");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<OHOS::DataShare::DataShareValuesBucket> valuesVector;
    valuesVector.push_back(values);
    valuesVector.push_back(valuesTwo);
    CheckResultSetList(valuesVector, result, "voicemail_Query_test_1600");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Query_test_1700
 * @tc.name    Query all voice mail data and return query results
 * @tc.desc    Voicemail data query capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Query_test_1700, testing::ext::TestSize.Level1)
{
    // query database count  There are two test cases above ,the number is 4
    HILOG_INFO("--- voicemail_Query_test_1700 is staring! ---");
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_number");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.GreaterThan("id", "0");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    result->Close();
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_BatchInsert_test_1800
 * @tc.name    Add 3 voice mail messages with status  and verify whether the insertion is successful
 * @tc.desc    Added ability to batch voicemail status
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_BatchInsert_test_1800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_BatchInsert_test_1800 is starting! ---");

    OHOS::Uri uriData(VoicemailUri::VOICEMAIL);
    OHOS::DataShare::DataShareValuesBucket voicemailValuesOne;
    int number = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    voicemailValuesOne.Put("phone_number", std::string(phoneNumber));
    voicemailValuesOne.Put("voice_status", 1);
    OHOS::DataShare::DataShareValuesBucket voicemailValuesTwo;
    voicemailValuesTwo.Put("phone_number", std::string(phoneNumber));
    voicemailValuesTwo.Put("voice_status", 0);
    OHOS::DataShare::DataShareValuesBucket voicemailValuesThree;
    voicemailValuesThree.Put("phone_number", std::string(phoneNumber));
    voicemailValuesThree.Put("voice_status", 0);

    std::vector<OHOS::DataShare::DataShareValuesBucket> listAddBluk;
    listAddBluk.push_back(voicemailValuesOne);
    listAddBluk.push_back(voicemailValuesTwo);
    listAddBluk.push_back(voicemailValuesThree);
    int batchInsertCode = voicemailAbility.BatchInsert(uriData, listAddBluk);
    EXPECT_EQ(batchInsertCode, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("phone_number", phoneNumber);
    predicates.Or();
    predicates.EqualTo("phone_number", phoneNumber);
    predicates.OrderByAsc("id");
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<OHOS::DataShare::DataShareValuesBucket> listValue;
    listValue.push_back(voicemailValuesOne);
    listValue.push_back(voicemailValuesThree);
    CheckResultSetList(listValue, result, "voicemail_BatchInsert_test_1800");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_BatchInsert_test_1900
 * @tc.name    Add 5 voice mail data and verify whether the insertion is successful
 * @tc.desc    New capabilities for voice mail data in batches
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_BatchInsert_test_1900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_BatchInsert_test_1900 is staring! ---");
    OHOS::Uri uriData(VoicemailUri::VOICEMAIL);
    OHOS::DataShare::DataShareValuesBucket voicemailValuesOne;
    int number = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    voicemailValuesOne.Put("phone_number", std::string(phoneNumber));
    voicemailValuesOne.Put("display_name", std::string("xiaoming"));
    OHOS::DataShare::DataShareValuesBucket voicemailValuesTwo;
    voicemailValuesTwo.Put("phone_number", std::string(phoneNumber));
    voicemailValuesTwo.Put("display_name", std::string("xiaozi"));
    OHOS::DataShare::DataShareValuesBucket voicemailValuesThree;
    voicemailValuesThree.Put("phone_number", std::string(phoneNumber));
    voicemailValuesThree.Put("display_name", std::string("xiaozhi"));
    OHOS::DataShare::DataShareValuesBucket voicemailValuesFour;
    voicemailValuesFour.Put("phone_number", std::string(phoneNumber));
    voicemailValuesFour.Put("display_name", std::string("xiaohei"));
    OHOS::DataShare::DataShareValuesBucket voicemailValuesFive;
    voicemailValuesFive.Put("phone_number", std::string(phoneNumber));
    voicemailValuesFive.Put("display_name", std::string("xiaohong"));
    std::vector<OHOS::DataShare::DataShareValuesBucket> listAddBluk;
    listAddBluk.push_back(voicemailValuesOne);
    listAddBluk.push_back(voicemailValuesTwo);
    listAddBluk.push_back(voicemailValuesThree);
    listAddBluk.push_back(voicemailValuesFour);
    listAddBluk.push_back(voicemailValuesFive);
    int batchInsertCode = voicemailAbility.BatchInsert(uriData, listAddBluk);
    EXPECT_EQ(0, batchInsertCode);

    int size = listAddBluk.size();
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("display_name");
    for (int i = 0; i < size; i++) {
        OHOS::DataShare::DataSharePredicates predicates;
        bool isValid = false;
        std::string name = listAddBluk[i].Get("display_name", isValid);
        predicates.EqualTo("display_name", name);
        predicates.And();
        std::string phoneNumber = listAddBluk[i].Get("phone_number", isValid);
        predicates.EqualTo("phone_number", phoneNumber);
        CheckResultSetBatch(predicates, columns, listAddBluk[i], "voicemail_BatchInsert_test_1900");
    }
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Insert_test_2000
 * @tc.name    When voice mail status is added, a non-existent field is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Insert_test_2000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Insert_test_2000 is starting! ---");

    OHOS::Uri uriVoicemail(VoicemailUri::VOICEMAIL);
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    int number = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    voicemailValues.Put("phone_numbers", phoneNumber);
    int64_t voicemailId = voicemailAbility.Insert(uriVoicemail, voicemailValues);
    HILOG_INFO("abnormal_voicemail_Insert_test_2000: voicemailId = %{public}ld", voicemailId);
    EXPECT_EQ(voicemailId, -1);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("phone_number", phoneNumber);
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    result->Close();
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Insert_test_2100
 * @tc.name    When adding voice mail data, a nonexistent table name is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Insert_test_2100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Insert_test_2100 is starting! ---");

    OHOS::Uri errorUriVoicemails(VoicemailUri::ERROR_URI);
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    int number = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    voicemailValues.Put("phone_number", phoneNumber);
    int64_t voicemailId = voicemailAbility.Insert(errorUriVoicemails, voicemailValues);
    HILOG_INFO("abnormal_voicemail_Insert_test_2100: voicemailId = %{public}ld", voicemailId);
    EXPECT_EQ(voicemailId, -1);

    OHOS::DataShare::DataSharePredicates predicates;
    std::string phoneNumber2 = randomNumberUtils.Generating(number);
    predicates.EqualTo("phone_number", phoneNumber2);
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    result->Close();
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Update_test_2200
 * @tc.name    When deleting voicemail status, a non-existent field is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Update_test_2200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Update_test_2200 is starting! ---");
    int64_t voicemailId = VoicemailStatusInsert("123456", 1);
    HILOG_INFO("abnormal_voicemail_Update_test_2200: voicemailId = %{public}ld", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataShareValuesBucket updateValues;
    updateValues.Put("voice_status", 0);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("ids", std::to_string(voicemailId));
    int updateCode = VoicemailUpdate(updateValues, predicates);
    HILOG_INFO("abnormal_voicemail_Update_test_2200 : updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, -1);

    OHOS::DataShare::DataSharePredicates predicates2;
    int number = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    predicates2.EqualTo("phone_number", phoneNumber);
    predicates2.And();
    predicates2.EqualTo("voice_status", "0");
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    result->Close();
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Delete_test_2300
 * @tc.name    When deleting voicemail data, a non-existent field is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Delete_test_2300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Delete_test_2300 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    int number = 8;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    values.Put("phone_number", phoneNumber);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("abnormal_voicemail_Delete_test_2300 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("ids", std::to_string(voicemailId));
    int deleteCode = VoicemailDelete(predicates);
    HILOG_INFO("abnormal_voicemail_Delete_test_2300: deleteCode = %{public}d", deleteCode);
    EXPECT_EQ(deleteCode, -1);
    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "abnormal_voicemail_Delete_test_2300");
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Delete_test_2400
 * @tc.name    When deleting voicemail data, a nonexistent table name is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Delete_test_2400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Delete_test_2400 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    int number = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    values.Put("phone_number", phoneNumber);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("abnormal_voicemail_Delete_test_2400 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::Uri uriVoicemails(VoicemailUri::ERROR_URI);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    int deleteCode = voicemailAbility.Delete(uriVoicemails, predicates);
    HILOG_INFO("abnormal_voicemail_Delete_test_2400: deleteCode = %{public}d", deleteCode);
    EXPECT_NE(deleteCode, 0);

    std::vector<std::string> columns;
    columns.push_back("phone_number");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "abnormal_voicemail_Delete_test_2400");
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Update_test_2500
 * @tc.name    When updating voicemail status, a non-existent field was passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Update_test_2500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Update_test_2500 is starting! ---");
    int64_t voicemailId = VoicemailStatusInsert("123456", 1);
    HILOG_INFO("abnormal_voicemail_Update_test_2500: voicemailId = %{public}ld", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataShareValuesBucket updateValues;
    updateValues.Put("voice_status", 0);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("ids", std::to_string(voicemailId));
    int updateCode = VoicemailUpdate(updateValues, predicates);
    HILOG_INFO("abnormal_voicemail_Update_test_2500 : updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, -1);

    OHOS::DataShare::DataSharePredicates predicates2;
    int number = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    predicates2.EqualTo("phone_number", phoneNumber);
    predicates2.And();
    predicates2.EqualTo("voice_status", "0");
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    result->Close();
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Update_test_2600
 * @tc.name    When updating voicemail data, a non-existent field was passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Update_test_2600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Update_test_2600 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    int number = 11;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    values.Put("phone_number", phoneNumber);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("abnormal_voicemail_Update_test_2600 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataShareValuesBucket updateValues;
    updateValues.Put("phone_number", std::string(phoneNumber));
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("ids", std::to_string(voicemailId));
    int updateCode = VoicemailUpdate(updateValues, predicates);
    HILOG_INFO("abnormal_voicemail_Update_test_2600: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, -1);
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Update_test_2700
 * @tc.name    When updating voicemail data, a nonexistent table name is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Update_test_2700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Update_test_2700 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    int number_test = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber_test = randomNumberUtils.Generating(number_test);
    values.Put("phone_number", phoneNumber_test);
    int voicemailId = VoicemailInsertValues(values);
    HILOG_INFO("abnormal_voicemail_Update_test_2700 : voicemailId = %{public}d", voicemailId);
    EXPECT_GT(voicemailId, 0);

    OHOS::Uri uriVoicemails(VoicemailUri::ERROR_URI);
    OHOS::DataShare::DataShareValuesBucket updateValues;
    int number = 11;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    updateValues.Put("phone_number", std::string(phoneNumber));
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    int updateCode = voicemailAbility.Update(uriVoicemails, predicates, updateValues);
    HILOG_INFO("abnormal_voicemail_Update_test_2700: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, -1);
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "abnormal_voicemail_Update_test_2700");
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Query_test_2800
 * @tc.name    When querying voicemail status, a non-existent field is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Query_test_2800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_Query_test_2800 is starting! ---");
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_numbers");
    columns.push_back("voice_status");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("voice_status", "1");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(-1, rowCount);
    resultSet->Close();
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Query_test_2900
 * @tc.name    An incorrect field was passed in when querying voicemail data
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Query_test_2900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----abnormal_voicemail_Query_test_2900 is starting!-----");
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_numbers");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.GreaterThan("id", "0");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, predicates);
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(-1, rowCount);
    resultSet->Close();
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Query_test_3000
 * @tc.name    When querying voicemail data, a non-existent field value is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Query_test_3000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----abnormal_voicemail_Query_test_3000 is starting!-----");
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_number");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", "10000000");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    result->Close();
    EXPECT_EQ(0, rowCount);
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_Query_test_3100
 * @tc.name    When querying voicemail data, a nonexistent table name was passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_Query_test_3100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----abnormal_voicemail_Query_test_3100 is starting!-----");
    OHOS::Uri errorUri(VoicemailUri::ERROR_URI);
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_number");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.GreaterThan("id", "0");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        voicemailAbility.Query(errorUri, predicates, columns);
    EXPECT_EQ(resultSet, nullptr);
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_BatchInsert_test_3200
 * @tc.name    When adding voice mail status in batch, an error field is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_BatchInsert_test_3200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_BatchInsert_test_3200 is starting! ---");
    OHOS::Uri uriData(VoicemailUri::VOICEMAIL);
    OHOS::DataShare::DataShareValuesBucket voicemailValuesOne;
    int number = 15;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    int number_test = 6;
    std::string phoneNumber_test = randomNumberUtils.Generating(number_test);
    voicemailValuesOne.Put("phone_number", std::string(phoneNumber));
    voicemailValuesOne.Put("voice_status", 1);
    OHOS::DataShare::DataShareValuesBucket voicemailValuesTwo;
    voicemailValuesTwo.Put("phone_numbers", std::string(phoneNumber_test));
    voicemailValuesTwo.Put("voice_statuss", 0);
    OHOS::DataShare::DataShareValuesBucket voicemailValuesThree;
    voicemailValuesThree.Put("phone_number", std::string(phoneNumber_test));
    voicemailValuesThree.Put("voice_status", 0);

    std::vector<OHOS::DataShare::DataShareValuesBucket> listAddBluk;
    listAddBluk.push_back(voicemailValuesOne);
    listAddBluk.push_back(voicemailValuesTwo);
    listAddBluk.push_back(voicemailValuesThree);
    int batchInsertCode = voicemailAbility.BatchInsert(uriData, listAddBluk);
    EXPECT_EQ(batchInsertCode, -1);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("phone_number", phoneNumber);
    predicates.Or();
    predicates.EqualTo("phone_numbers", phoneNumber_test);
    predicates.Or();
    predicates.EqualTo("phone_number", phoneNumber_test);
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_number");
    columns.push_back("phone_numbers");
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(-1, rowCount);
    resultSet->Close();
    ClearVoicemail();
}

/*
 * @tc.number  abnormal_voicemail_BatchInsert_test_3300
 * @tc.name    When adding voice mail data in batch, several of them fail. Check the processing logic
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, abnormal_voicemail_BatchInsert_test_3300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_voicemail_BatchInsert_test_3300 is starting! ---");
    OHOS::Uri uriData(VoicemailUri::VOICEMAIL);
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    std::vector<OHOS::DataShare::DataShareValuesBucket> listAddBluk;
    int batchInserCode = 0;
    for (int i = 0; i < 10; i++) {
        listAddBluk.clear();
        for (int j = 10 * i + 1; j <= 10 * (i + 1); j++) {
            voicemailValues.Clear();
            voicemailValues.Put("phone_number", std::to_string(j));
            std::string name("zhangsan");
            name.append(std::to_string(j));
            if (j == 14 || j == 27 || j == 57) {
                voicemailValues.Put("display_names", name);
            } else {
                voicemailValues.Put("display_name", name);
            }
            listAddBluk.push_back(voicemailValues);
        }
        batchInserCode = voicemailAbility.BatchInsert(uriData, listAddBluk);
        HILOG_INFO("abnormal_voicemail_BatchInsert_test_3300 : batchInserCode = %{public}d", batchInserCode);
        if (batchInserCode == 0) {
            HILOG_INFO("abnormal_voicemail_BatchInsert_test_3300 batch insert success!");
        } else {
            HILOG_INFO("abnormal_voicemail_BatchInsert_test_3300 batch insert fail!");
            EXPECT_EQ(batchInserCode, -1);
        }
    }
    // clear data
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.GreaterThan("id", "0");
    int deleteCode = VoicemailDelete(predicates);
    EXPECT_EQ(0, deleteCode);
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Insert_test_3400
 * @tc.name    Add a voice mailbox with status 0 and verify that the insertion was successful
 * @tc.desc    Added ability to voice mailbox status
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Insert_test_3400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Insert_test_3400 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    int number = 6;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    values.Put("phone_number", phoneNumber);
    values.Put("voice_status", 0);
    int voicemailId = VoicemailInsertValues(values);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "voicemail_Insert_test_3400");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Update_test_3500
 * @tc.name    Update the voice mailbox status and verify whether the update is successful (voice_status changes from 1
 * to 0)
 * @tc.desc    Voicemail status update capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Update_test_3500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Update_test_3500 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    int number = 9;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    values.Put("phone_number", phoneNumber);
    values.Put("voice_status", 1);
    int voicemailId = VoicemailInsertValues(values);
    EXPECT_GT(voicemailId, 0);

    OHOS::DataShare::DataShareValuesBucket updateValues;
    updateValues.Put("voice_status", 0);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", std::to_string(voicemailId));
    int updateCode = VoicemailUpdate(updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    std::string phoneNumber2 = randomNumberUtils.Generating(number);
    updateValues.Put("phone_number", phoneNumber2);
    OHOS::DataShare::DataSharePredicates predicates2;
    predicates2.EqualTo("id", std::to_string(voicemailId));
    std::vector<std::string> columns;
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates2);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(updateValues, result, "voicemail_Update_test_3500");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_Query_test_3600
 * @tc.name    Query the voice mailbox with status 0 and return the query results
 * @tc.desc    Voicemail status query capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_Query_test_3600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_Query_test_3600 is starting! ---");
    OHOS::DataShare::DataShareValuesBucket values;
    int number = 9;
    RandomNumberUtils randomNumberUtils;
    std::string phoneNumber = randomNumberUtils.Generating(number);
    values.Put("phone_number", phoneNumber);
    values.Put("voice_status", 0);
    int voicemailId = VoicemailInsertValues(values);
    EXPECT_GT(voicemailId, 0);

    values.Put("id", voicemailId);
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("phone_number");
    columns.push_back("voice_status");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("voice_status", "0");
    predicates.And();
    predicates.EqualTo("id", std::to_string(voicemailId));
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> result = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    result->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(values, result, "voicemail_Query_test_3600");
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_async_insert_test_3800
 * @tc.name    Add voicemail async
 * @tc.desc    Ability to join or add voicemail
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_async_insert_test_3700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_async_insert_test_3700 is staring! ---");
    std::map<int, OHOS::DataShare::DataShareValuesBucket> result;
    std::vector<VoicemailAsync *> voicemailAsyncVector;
    int threadNum = 6;
    for (int i = 0; i < threadNum; ++i) {
        OHOS::DataShare::DataShareValuesBucket values;
        std::string name;
        name.append("asyncTest");
        name.append(std::to_string(i));
        values.Put("display_name", name);
        VoicemailAsync *contactAsync = new VoicemailAsync(values, result);
        std::thread asyncThread(&VoicemailAsync::Insert, contactAsync);
        voicemailAsyncVector.push_back(contactAsync);
        asyncThread.detach();
    }
    std::chrono::milliseconds dura(Time::ASYNC_SLEEP_TIME);
    std::this_thread::sleep_for(dura);
    std::vector<std::string> columns;
    columns.push_back("display_name");
    std::map<int, OHOS::DataShare::DataShareValuesBucket>::iterator it;
    for (it = result.begin(); it != result.end(); it++) {
        OHOS::DataShare::DataSharePredicates predicates;
        predicates.EqualTo("id", std::to_string(it->first));
        std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, predicates);
        CheckResultSet(it->second, resultSet, "voicemail_async_insert_test_3700");
    }
    for (int i = 0; i < threadNum; ++i) {
        delete voicemailAsyncVector[i];
    }
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_async_update_test_3900
 * @tc.name    update voicemail async
 * @tc.desc    Ability to join or update voicemail
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_async_update_test_3800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_async_update_test_3800 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    std::vector<int64_t> voicemailId;
    voicemailId.push_back(VoicemailInsertValue("async_update1", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update2", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update3", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update4", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update5", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update6", voicemailValues));
    voicemailValues.Clear();
    std::vector<OHOS::DataShare::DataShareValuesBucket> upDateValues;
    OHOS::DataShare::DataSharePredicates queryPredicates;
    std::vector<VoicemailAsync *> voicemailAsyncVector;
    int size = voicemailId.size();
    for (int i = 0; i < size; ++i) {
        OHOS::DataShare::DataShareValuesBucket values;
        std::string name;
        name.append("asyncTest");
        values.Put("display_name", name);
        upDateValues.push_back(values);
        int id = voicemailId[i];
        VoicemailAsync *contactAsync = new VoicemailAsync(values, id);
        voicemailAsyncVector.push_back(contactAsync);
        std::thread asyncThread(&VoicemailAsync::Update, contactAsync);
        asyncThread.detach();
        queryPredicates.EqualTo("id", std::to_string(voicemailId[i]));
        if (i < size - 1) {
            queryPredicates.Or();
        }
    }
    std::chrono::milliseconds dura(Time::ASYNC_SLEEP_TIME);
    std::this_thread::sleep_for(dura);
    std::vector<std::string> columns;
    columns.push_back("display_name");
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, queryPredicates);
    CheckResultSetList(upDateValues, resultSet, "voicemail_async_update_test_3800");
    for (int i = 0; i < size; ++i) {
        delete voicemailAsyncVector[i];
    }
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_async_query_test_3900
 * @tc.name    query voicemail async
 * @tc.desc    Ability to join or query
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_async_query_test_3900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_async_query_test_4000 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    std::vector<int64_t> voicemailId;
    voicemailId.push_back(VoicemailInsertValue("async_query1", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_query2", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_query3", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_query4", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_query5", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_query6", voicemailValues));
    voicemailValues.Clear();
    std::vector<VoicemailAsync *> voicemailAsyncVector;
    std::vector<std::shared_ptr<OHOS::DataShare::DataShareResultSet>> resultSetVector;
    int threadNum = 6;
    for (int i = 0; i < threadNum; ++i) {
        VoicemailAsync *contactAsync = new VoicemailAsync(resultSetVector, voicemailId);
        voicemailAsyncVector.push_back(contactAsync);
        std::thread asyncThread(&VoicemailAsync::Query, contactAsync);
        asyncThread.detach();
    }
    int queryCount = 6;
    int size = resultSetVector.size();
    for (int i = 0; i < size; ++i) {
        int rowCount = 0;
        resultSetVector[i]->GetRowCount(rowCount);
        EXPECT_EQ(queryCount, rowCount);
        resultSetVector[i]->Close();
    }
    for (int i = 0; i < size; ++i) {
        delete voicemailAsyncVector[i];
    }
    ClearVoicemail();
}

/*
 * @tc.number  voicemail_async_delete_test_4100
 * @tc.name    delete voicemail async
 * @tc.desc    Ability to join or delete voicemail
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(VoicemailAbilityTest, voicemail_async_delete_test_4000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- voicemail_async_delete_test_4000 is staring! ---");
    OHOS::DataShare::DataShareValuesBucket voicemailValues;
    std::vector<int64_t> voicemailId;
    voicemailId.push_back(VoicemailInsertValue("async_update1", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update2", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update3", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update4", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update5", voicemailValues));
    voicemailValues.Clear();
    voicemailId.push_back(VoicemailInsertValue("async_update6", voicemailValues));
    voicemailValues.Clear();
    int size = voicemailId.size();
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.BeginWrap();
    std::vector<VoicemailAsync *> voicemailAsyncVector;
    for (int i = 0; i < size; ++i) {
        int id = voicemailId[i];
        VoicemailAsync *contactAsync = new VoicemailAsync(id);
        voicemailAsyncVector.push_back(contactAsync);
        std::thread asyncThread(&VoicemailAsync::Delete, contactAsync);
        asyncThread.detach();
        predicates.EqualTo("id", std::to_string(id));
        if (i < size - 1) {
            predicates.Or();
        }
    }
    predicates.EndWrap();
    std::chrono::milliseconds dura(Time::ASYNC_SLEEP_TIME);
    std::this_thread::sleep_for(dura);
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = VoicemailQuery(columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    resultSet->Close();
    for (int i = 0; i < size; ++i) {
        delete voicemailAsyncVector[i];
    }
    ClearVoicemail();
}
} // namespace Test
} // namespace Contacts