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

#include "mergecontact_test.h"

#include <sys/time.h>

#include "match_candidate.h"
#include "merger_contacts.h"
#include "test_common.h"

namespace Contacts {
namespace Test {
void MergeContactTest::DeleteRawContact()
{
    OHOS::DataShare::DataSharePredicates predicates;
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    predicates.NotEqualTo("id", "0");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    contactsDataAbility.Delete(uriRawContact, predicates);
    int time = Time::SLEEP_TIME_MERGE_DELETE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::DataShare::DataSharePredicates predicates2;
    OHOS::Uri uriRawContactComplete(ContactsUri::DELETED_RAW_CONTACT);
    predicates2.NotEqualTo("id", "0");
    contactsDataAbility.Delete(uriRawContactComplete, predicates2);
}

void MergeContactTest::CheckMergeResultId(std::vector<int64_t> ids, bool merge)
{
    std::vector<int64_t> resultIdVector;
    int mergeIdSize = ids.size();
    for (int i = 0; i < mergeIdSize; i++) {
        std::vector<std::string> columns;
        columns.push_back("contact_id");
        OHOS::DataShare::DataSharePredicates predicates;
        predicates.EqualTo("id", std::to_string(ids[i]));
        std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultOne = ContactQuery(columns, predicates);
        int contactIdOne = GetMergeResultContactId(resultOne);
        resultIdVector.push_back(contactIdOne);
    }
    int size = resultIdVector.size();
    int64_t contactId = resultIdVector[0];
    for (int i = 1; i < size; i++) {
        if (merge) {
            EXPECT_EQ(contactId, resultIdVector[i]);
            HILOG_INFO("merge contact_id = %{public}ld, contact_idTwo %{public}ld", contactId, resultIdVector[i]);
        } else {
            EXPECT_NE(contactId, resultIdVector[i]);
            HILOG_INFO("split contact_id = %{public}ld", resultIdVector[i]);
        }
    }
}

int MergeContactTest::GetMergeResultContactId(const std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet)
{
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    int resultSetIntValue = 0;
    if (resultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
        std::string typeValue = "contact_id";
        int columnIndex = 0;
        resultSet->GetColumnIndex(typeValue, columnIndex);
        resultSet->GetInt(columnIndex, resultSetIntValue);
    }
    resultSet->Close();
    return resultSetIntValue;
}

std::vector<int> MergeContactTest::GetMergeResultRawContactId(
    const std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet)
{
    std::vector<int> rawContactIdVector;
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int resultSetIntValue = 0;
        std::string typeValue = "raw_contact_id";
        int columnIndex = 0;
        resultSet->GetColumnIndex(typeValue, columnIndex);
        resultSet->GetInt(columnIndex, resultSetIntValue);
        rawContactIdVector.push_back(resultSetIntValue);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return rawContactIdVector;
}

std::vector<int> MergeContactTest::GetMergeRawContactId(
    const std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet)
{
    std::vector<int> rawContactIdVector;
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int resultSetIntValue = 0;
        std::string typeValue = "id";
        int columnIndex = 0;
        resultSet->GetColumnIndex(typeValue, columnIndex);
        resultSet->GetInt(columnIndex, resultSetIntValue);
        rawContactIdVector.push_back(resultSetIntValue);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return rawContactIdVector;
}

std::shared_ptr<OHOS::DataShare::DataShareResultSet> MergeContactTest::ContactQuery(
    std::vector<std::string> columns, OHOS::DataShare::DataSharePredicates predicates)
{
    OHOS::Uri uri(ContactsUri::RAW_CONTACT);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        contactsDataAbility.Query(uri, predicates, columns);
    return resultSet;
}

int64_t MergeContactTest::RawContactInsert(std::string displayName)
{
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    OHOS::DataShare::DataShareValuesBucket rawContactValues;
    rawContactValues.Put("display_name", displayName);
    int64_t code = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    rawContactValues.Clear();
    return code;
}

int64_t MergeContactTest::ContactDataInsert(
    int64_t rawContactId, std::string contentType, std::string detailInfo, std::string position)
{
    OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
    OHOS::DataShare::DataShareValuesBucket contactDataValues;
    contactDataValues.Put("raw_contact_id", rawContactId);
    contactDataValues.Put("content_type", contentType);
    contactDataValues.Put("detail_info", detailInfo);
    contactDataValues.Put("position", position);
    int64_t code = contactsDataAbility.Insert(uriContactData, contactDataValues);
    contactDataValues.Clear();
    return code;
}

HWTEST_F(MergeContactTest, merge_test_start, testing::ext::TestSize.Level1)
{
    DeleteRawContact();
}

/*
 * @tc.number  merge_Query_test_100
 * @tc.name    Insert contacts with the same name and different mobile numbers,
 *              and check whether the database can find the contacts that can be merged
 * @tc.desc    A: xiaowuwuwu 123456, B: xiaowuwuwu 1234567
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Query_test_100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Query_test_100 query is starting! ---");
    int64_t rawOne = RawContactInsert("xiaowuww");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaowuww", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Query_test_100 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaowuww");
    EXPECT_GT(rawTwo, 0);
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaowuww", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "1234567", "");
    HILOG_INFO("merge_Query_test_100 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriQueryMergeList(ContactsUri::MERGE_LIST);
    std::vector<std::string> columns;
    OHOS::DataShare::DataSharePredicates predicates;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        contactsDataAbility.Query(uriQueryMergeList, predicates, columns);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<int> rawVector = GetMergeResultRawContactId(resultSet);
    EXPECT_EQ(rawOne, rawVector[0]);
    EXPECT_EQ(rawTwo, rawVector[1]);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_200
 * @tc.name    Insert two contacts with the same name to check whether the database can be merged successfully
 * @tc.desc    A: xiaochenc, B: xiaochenc
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_200 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaochenc");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaochenc", "");
    HILOG_INFO("merge_Update_test_200 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaochenc");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaochenc", "");
    HILOG_INFO("merge_Update_test_200 :  = %{public}ld", dataIdTwo);
    int time = 2000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_300
 * @tc.name    Insert two contacts with the same name. One of them has a mobile phone number.
 *              Check whether the database can be merged
 * @tc.desc    A: xiaomingmingming 123456, B: xiaomingmingming
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_300 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaomingmm");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaomingmm", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_300 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaomingmm");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaomingmm", "");
    HILOG_INFO("merge_Update_test_300 :  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    value.Clear();
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriManualMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_400
 * @tc.name    Insert two contacts with the same name. The mobile phone numbers of contacts a and B are the same.
 *              Check whether the database can be merged
 * @tc.desc    A: xiaohong、123456, B: xiaohong、123456
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_400 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaohong");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaohong", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_400 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaohong");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaohong", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_400 :  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_500
 * @tc.name    Insert two contacts with the same name. The mobile phone numbers of contacts a and B are different.
 *              Check whether the database can be merged
 * @tc.desc    A: xiaozhang、123456, B: xiaozhang、1234567
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_500 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaozhang");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaozhang", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_500 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaozhang");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaozhang", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "1234567", "");
    HILOG_INFO("merge_Update_test_500 :  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, false);

    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    value.Clear();
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriManualMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_600
 * @tc.name    Insert two contacts with the same name. Contact a has two mobile phone numbers
 *              and contact B has no mobile phone number. Check whether the database can be merged
 * @tc.desc    A: xiaozhou 123456/1234567, B: xiaozhou
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_600 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaozhou");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaozhou", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_600 dataIdOne_1  = %{public}ld", dataIdOne);
    dataIdOne = ContactDataInsert(rawOne, "phone", "1234567", "");
    HILOG_INFO("merge_Update_test_600 dataIdOne_2  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaozhou");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaozhou", "");
    HILOG_INFO("merge_Update_test_600 :  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, false);

    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    value.Clear();
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriManualMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_700
 * @tc.name    Insert two contacts with the same name. Both contacts a and B have the same mobile phone numbers.
 *              Check whether the database can be merged
 * @tc.desc    A: xiaobai、123456/1234567, B: xiaobai、123456/1234567
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_700 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaobai");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaobai", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_700 dataIdOne_1  = %{public}ld", dataIdOne);
    dataIdOne = ContactDataInsert(rawOne, "phone", "1234567", "");
    HILOG_INFO("merge_Update_test_700 dataIdOne_2  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaobai");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaobai", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_700 dataIdTwo_1  = %{public}ld", dataIdTwo);
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "1234567", "");
    HILOG_INFO("merge_Update_test_700 dataIdTwo_2  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_800
 * @tc.name    Insert two contacts with the same name. Each contact a and B has two mobile phone numbers, one of which
 * is the same. Check whether the database can be merged
 * @tc.desc    A: xiaomi、123456/1234567, B: xiaomi、123456/12345678
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_800 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaomi");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaomi", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_800 dataIdOne_1  = %{public}ld", dataIdOne);
    dataIdOne = ContactDataInsert(rawOne, "phone", "1234567", "");
    HILOG_INFO("merge_Update_test_800 dataIdOne_2  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaomi");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaomi", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_800 dataIdTwo_1  = %{public}ld", dataIdTwo);
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "12345678", "");
    HILOG_INFO("merge_Update_test_800 dataIdTwo_2  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, false);

    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    value.Clear();
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriManualMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_900
 * @tc.name    Insert two contacts with the same name. Contact a has two mobile phone numbers, and contact B has one
 * mobile phone number and is the same as one of the mobile phone numbers of A. check whether the database can be merged
 * @tc.desc    A: daniu、123456/1234567, B: daniu、123456
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_900 Add is starting! ---");
    int64_t rawOne = RawContactInsert("daniu");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "daniu", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_900 dataIdOne_1  = %{public}ld", dataIdOne);
    dataIdOne = ContactDataInsert(rawOne, "phone", "1234567", "");
    HILOG_INFO("merge_Update_test_900 dataIdOne_2  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("daniu");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "daniu", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_900 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    value.Clear();
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriManualMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_1000
 * @tc.name    Insert two contacts with the same name. Contact a has two mobile phone numbers
 *              and contact B has one mobile phone number. Check whether the database can be merged
 * @tc.desc    A: mazi、123456/1234567, B: mazi、123456789
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_1000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_1000 Add is starting! ---");
    int64_t rawOne = RawContactInsert("mazi");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "mazi", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_1000 dataIdOne_1  = %{public}ld", dataIdOne);
    dataIdOne = ContactDataInsert(rawOne, "phone", "1234567", "");
    HILOG_INFO("merge_Update_test_1000 dataIdOne_2  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("mazi");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "mazi", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456789", "");
    HILOG_INFO("merge_Update_test_1000 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    value.Clear();
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriManualMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  abnormal_merge_Update_test_1100
 * @tc.name    After consolidation, it is transferred to - 1 for splitting
 * @tc.desc    Exception use case: Merge and pass -1 to split
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, abnormal_merge_Update_test_1100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_merge_Update_test_1100 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaocai");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaocai", "");
    HILOG_INFO("abnormal_merge_Update_test_1100 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaocai");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaocai", "");
    HILOG_INFO("abnormal_merge_Update_test_1100 :  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket valueOne;
    OHOS::DataShare::DataSharePredicates predicates;
    contactsDataAbility.Update(uriAutoMerge, predicates, valueOne);
    OHOS::Uri uriSplitContact(ContactsUri::SPLIT_CONTACT);
    OHOS::DataShare::DataShareValuesBucket valueTwo;
    std::vector<std::string> ids;
    ids.push_back("-1");
    predicates.In("raw_contact_id", ids);
    int ret = contactsDataAbility.Update(uriSplitContact, predicates, valueTwo);
    EXPECT_EQ(ret, -1);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_1200
 * @tc.name    Insert multiple automatically merged contacts to check whether the database can be merged
 * @tc.desc    A: xiaoliu、123456, B: xiaoliu、123456, C:xiaoliu、123456
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_1200, testing::ext::TestSize.Level1)
{
    int64_t rawOne = RawContactInsert("xiaoliu");
    int64_t dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    dataIdOne = ContactDataInsert(rawOne, "name", "xiaoliu", "");
    HILOG_INFO("merge_Update_test_1200 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaoliu");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaoliu", "");
    HILOG_INFO("merge_Update_test_1200 :  = %{public}ld", dataIdTwo);
    int64_t rawThree = RawContactInsert("xiaoliu");
    int64_t dataIdThree = ContactDataInsert(rawThree, "phone", "123456", "");
    dataIdThree = ContactDataInsert(rawThree, "name", "xiaoliu", "");
    HILOG_INFO("merge_Update_test_1200 :  = %{public}ld", dataIdThree);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    ids.push_back(std::to_string(rawThree));
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    HILOG_INFO("ret : %{public}d ", ret);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_1300
 * @tc.name    Insert two automatically merged contacts, modify a's name,
 *              and then automatically merge to see whether the database can be merged
 * @tc.desc    A: xiaohua、123456, B: xiaohua、123456 --> A: xiaohua、123456, B: dayu、123456
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_1300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_1300 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaohua");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaohua", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_1300 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaohua");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaohua", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    HILOG_INFO("merge_Update_test_1300 dataIdTwo  = %{public}ld", dataIdTwo);

    OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "dayuy");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawTwo));
    predicates.And();
    predicates.EqualTo("type_id", "6");
    contactsDataAbility.Update(uriContactData, predicates, values);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    values.Clear();
    OHOS::DataShare::DataSharePredicates predicates2;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates2, values);
    EXPECT_EQ(ret, -1);
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, false);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_1400
 * @tc.name    Insert two automatically merged contacts, modify a's mobile phone number,
 *              and then manually merge contacts to see whether the database can be merged
 * @tc.desc    A: xiaobei、123456, B: xiaobei、123456 --> A: xiaobei、123456, B: xiaobei、1234567
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_1400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_1400 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaobei");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaobei", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "17612345689", "");
    HILOG_INFO("merge_Update_test_1400 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaobei");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaobei", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "17612345689", "");
    HILOG_INFO("merge_Update_test_1400 dataIdTwo  = %{public}ld", dataIdTwo);

    OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "18355421566");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawTwo));
    predicates.And();
    predicates.EqualTo("type_id", "5");
    contactsDataAbility.Update(uriContactData, predicates, values);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    values.Clear();
    OHOS::DataShare::DataSharePredicates predicates2;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates2, values);
    EXPECT_EQ(ret, -1);
    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    values.Clear();
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates2.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriManualMerge, predicates2, values);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_1500
 * @tc.name    After consolidation, it is transferred to rowid1 for splitting
 * @tc.desc    After merging, pass rowId1 to split
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */

HWTEST_F(MergeContactTest, merge_Update_test_1500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_1500 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaolil");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaolil", "");
    HILOG_INFO("merge_Update_test_1500 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaoli");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaolil", "");
    HILOG_INFO("merge_Update_test_1500 :  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    int ret;
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket valueOne;
    OHOS::DataShare::DataSharePredicates predicates;
    ret = contactsDataAbility.Update(uriAutoMerge, predicates, valueOne);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);

    OHOS::Uri uriSplitContact(ContactsUri::SPLIT_CONTACT);
    OHOS::DataShare::DataShareValuesBucket valueTwo;
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    predicates.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriSplitContact, predicates, valueTwo);
    EXPECT_EQ(ret, 0);
    CheckMergeResultId(resultIdVector, false);
    DeleteRawContact();
}

/*
 * @tc.number  abnormal_merge_Query_test_1600
 * @tc.name    Insert contacts with the same name and different mobile numbers.
 *              The URI address is wrong. Check whether the database can find the contacts that can be merged
 * @tc.desc    A: xiaoma 123456, B: xiaoma 1234567
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, abnormal_merge_Query_test_1600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_merge_Query_test_1600 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaoma");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaoma", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("abnormal_merge_Query_test_1600 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaoma");
    EXPECT_GT(rawTwo, 0);
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaoma", "");
    dataIdOne = ContactDataInsert(rawTwo, "phone", "1234567", "");
    HILOG_INFO("abnormal_merge_Query_test_1600 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriQueryMergeList(ContactsUri::MERGE_LIST_ERROR);
    std::vector<std::string> columns;
    OHOS::DataShare::DataSharePredicates predicates;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        contactsDataAbility.Query(uriQueryMergeList, predicates, columns);
    EXPECT_EQ(resultSet, nullptr);
    DeleteRawContact();
}

/*
 * @tc.number  abnormal_merge_Update_test_1700
 * @tc.name    Parameter URI address error
 * @tc.desc    Passing an error auto_merge uri
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, abnormal_merge_Update_test_1700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_merge_Update_test_1700 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaoqian");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaoqian", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    int64_t rawTwo = RawContactInsert("xiaoqian");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaoqian", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    HILOG_INFO("abnormal_merge_Update_test_1700 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::MERGE_ERROR);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, false);

    HILOG_INFO("ret : %{public}d ", ret);
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    predicates.EqualTo("id", std::to_string(rawOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawTwo));
    contactsDataAbility.Delete(uriRawContact, predicates);
    DeleteRawContact();
}

/*
 * @tc.number  abnormal_merge_Update_test_1800
 * @tc.name    Insert two contacts with different names and merge them automatically to see if the database can be
 * merged
 * @tc.desc    A: xiaosun、123456, B: xiaomei、123456
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, abnormal_merge_Update_test_1800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_merge_Update_test_1800 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaosun");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaosun", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("abnormal_merge_Update_test_1800 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaomei");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaomei", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    HILOG_INFO("abnormal_merge_Update_test_1800 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    HILOG_INFO("ret : %{public}d ", ret);
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, false);
    DeleteRawContact();
}

/*
 * @tc.number  abnormal_merge_Update_test_1900
 * @tc.name    Insert two contacts with different names for automatic merging and manual merging
 * @tc.desc    A: bob2300, B: lileibob2300 --> A: bob2300, B: lileibob2300
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, abnormal_merge_Update_test_1900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_merge_Update_test_1900 Add is starting! ---");
    int64_t rawOne = RawContactInsert("bob2300");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "bob2300", "");
    HILOG_INFO("abnormal_merge_Update_test_1900 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("lileibob2300");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "lileibob2300", "");
    HILOG_INFO("abnormal_merge_Update_test_1900 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);

    OHOS::DataShare::DataShareValuesBucket values;
    OHOS::DataShare::DataSharePredicates predicates;
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, values);
    EXPECT_EQ(ret, -1);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    values.Clear();
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates.In("raw_contact_id", ids);
    ret = contactsDataAbility.Update(uriManualMerge, predicates, values);
    EXPECT_EQ(ret, -1);
    DeleteRawContact();
}

/*
 * @tc.number  abnormal_merge_Update_test_2000
 * @tc.name    Pass in the wrong field name -- nameee, name
 * @tc.desc    A: xiaowang, B: xiaowang
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, abnormal_merge_Update_test_2000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_merge_Update_test_2000 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaowang");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdOne = ContactDataInsert(rawOne, "nameee", "xiaowang", "");
    HILOG_INFO("abnormal_merge_Update_test_2000 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaowang");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaowang", "");
    HILOG_INFO("abnormal_merge_Update_test_2000 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    int ret = contactsDataAbility.Update(uriAutoMerge, predicates, value);
    EXPECT_EQ(ret, -1);
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, false);

    HILOG_INFO("ret : %{public}d ", ret);
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    predicates.EqualTo("id", std::to_string(dataIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(dataIdTwo));
    contactsDataAbility.Delete(uriRawContact, predicates);
    DeleteRawContact();
}

/*
 * @tc.number  abnormal_merge_Update_test_2100
 * @tc.name    After consolidation, it is transferred to 0 for splitting
 * @tc.desc    Exception use case: Merge and pass 0 to split
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, abnormal_merge_Update_test_2100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_merge_Update_test_2100 Add is starting! ---");
    int64_t rawOne = RawContactInsert("xiaocheng");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaocheng", "");
    HILOG_INFO("abnormal_merge_Update_test_2100 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaocheng");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaocheng", "");
    HILOG_INFO("abnormal_merge_Update_test_2100 :  = %{public}ld", dataIdTwo);
    std::chrono::milliseconds dura(Time::SLEEP_TIME_MERGE);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriAutoMerge(ContactsUri::AUTO_MERGE);
    OHOS::DataShare::DataShareValuesBucket valueOne;
    OHOS::DataShare::DataSharePredicates predicates;
    contactsDataAbility.Update(uriAutoMerge, predicates, valueOne);
    OHOS::Uri uriSplitContact(ContactsUri::SPLIT_CONTACT);
    OHOS::DataShare::DataShareValuesBucket valueTwo;
    std::vector<std::string> ids;
    ids.push_back("0");
    predicates.In("raw_contact_id", ids);
    int ret = contactsDataAbility.Update(uriSplitContact, predicates, valueTwo);
    EXPECT_EQ(ret, -1);

    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_2200
 * @tc.name    Insert multiple manually merged contacts to check whether the database can be merged
 * @tc.desc    A: xiaocai、1234567, B: xiaocai、123456, C:xiaocai、12345678
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_2200, testing::ext::TestSize.Level1)
{
    int64_t rawOne = RawContactInsert("xiaocai");
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaocai", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "1234567", "");
    HILOG_INFO("merge_Update_test_2200 :  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaocai");
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaocai", "");
    HILOG_INFO("merge_Update_test_2200 :  = %{public}ld", dataIdTwo);
    int64_t rawThree = RawContactInsert("xiaocai");
    int64_t dataIdThree = ContactDataInsert(rawThree, "phone", "12345678", "");
    dataIdThree = ContactDataInsert(rawThree, "name", "xiaocai", "");
    HILOG_INFO("merge_Update_test_2200 :  = %{public}ld", dataIdThree);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    ids.push_back(std::to_string(rawThree));
    predicates.In("raw_contact_id", ids);
    int ret = contactsDataAbility.Update(uriManualMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    HILOG_INFO("ret : %{public}d ", ret);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Query_test_2300
 * @tc.name    Insert two contacts with the same name to check whether the database
 *             can find the contacts that can be merged
 * @tc.desc    A: xsiaowuwwwww  B: xsiaowuwwwww
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Query_test_2300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Query_test_2300 query is starting! ---");
    int64_t rawOne = RawContactInsert("xsiaowuwwwww");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xsiaowuwwwww", "");
    HILOG_INFO("merge_Query_test_2300 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xsiaowuwwwww");
    EXPECT_GT(rawTwo, 0);
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xsiaowuwwwww", "");
    HILOG_INFO("merge_Query_test_2300 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    std::vector<std::string> columns;
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.BeginWrap();
    predicates.EqualTo("id", std::to_string(rawOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawTwo));
    predicates.EndWrap();
    predicates.And();
    predicates.EqualTo("merge_mode", "2");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        contactsDataAbility.Query(uriRawContact, predicates, columns);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<int> rawVector = GetMergeRawContactId(resultSet);
    EXPECT_EQ(rawOne, rawVector[0]);
    EXPECT_EQ(rawTwo, rawVector[1]);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Query_test_2400
 * @tc.name    Insert two contacts with different names to check whether the database
 *             can find the contacts that can be merged
 * @tc.desc    A: jackxsiaowuwwwww, B: xsiaowuwwwww
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Query_test_2400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Query_test_2400 query is starting! ---");
    int64_t rawOne = RawContactInsert("jackxsiaowuwwwww");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "jackxsiaowuwwwww", "");
    HILOG_INFO("merge_Query_test_2400 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xsiaowuwwwww");
    EXPECT_GT(rawTwo, 0);
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xsiaowuwwwww", "");
    HILOG_INFO("merge_Query_test_2400 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriQueryMergeList(ContactsUri::MERGE_LIST);
    std::vector<std::string> columns;
    OHOS::DataShare::DataSharePredicates predicates;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        contactsDataAbility.Query(uriQueryMergeList, predicates, columns);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Query_test_2500
 * @tc.name    Insert contacts with the same name and phone number, and check whether the database
 *              can find contacts that can be merged
 * @tc.desc    A: xiaowuwwl 123456, B: xiaowuwwl 123456
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Query_test_2500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Query_test_2500 query is starting! ---");
    int64_t rawOne = RawContactInsert("xiaowuwwl");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "xiaowuwwl", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456", "");
    HILOG_INFO("merge_Query_test_2500 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("xiaowuww");
    EXPECT_GT(rawTwo, 0);
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "xiaowuwwl", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "123456", "");
    HILOG_INFO("merge_Query_test_2500 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    std::vector<std::string> columns;
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.BeginWrap();
    predicates.EqualTo("id", std::to_string(rawOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawTwo));
    predicates.EndWrap();
    predicates.And();
    predicates.EqualTo("merge_mode", "2");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        contactsDataAbility.Query(uriRawContact, predicates, columns);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<int> rawVector = GetMergeRawContactId(resultSet);
    EXPECT_EQ(rawOne, rawVector[0]);
    EXPECT_EQ(rawTwo, rawVector[1]);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Query_test_2600
 * @tc.name    Insert contacts with different names and phone numbers, and check whether the
 *             database can find the contacts that can be merged
 * @tc.desc    A: cxiaowuwwl 123456565454, B: nxiaowuwwl 1234564488
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Query_test_2600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Query_test_2600 query is starting! ---");
    int64_t rawOne = RawContactInsert("cxiaowuwwl");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "cxiaowuwwl", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456565454", "");
    HILOG_INFO("merge_Query_test_2600 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("nxiaowuww");
    EXPECT_GT(rawTwo, 0);
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "nxiaowuwwl", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "1234564488", "");
    HILOG_INFO("merge_Query_test_2600 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriQueryMergeList(ContactsUri::MERGE_LIST);
    std::vector<std::string> columns;
    OHOS::DataShare::DataSharePredicates predicates;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet =
        contactsDataAbility.Query(uriQueryMergeList, predicates, columns);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    DeleteRawContact();
}

/*
 * @tc.number  merge_Update_test_2700
 * @tc.name    Insert two contacts with the same name. Contacts A and B have two different mobile
 *             phone numbers, and check whether the database can be merged
 * @tc.desc    A: jack12345 123456565454 1234565654546 , B: jack12345 1234564488 12345644888
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(MergeContactTest, merge_Update_test_2700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- merge_Update_test_2700 query is starting! ---");
    int64_t rawOne = RawContactInsert("jack12345");
    EXPECT_GT(rawOne, 0);
    int64_t dataIdOne = ContactDataInsert(rawOne, "name", "jack12345", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "123456565454", "");
    dataIdOne = ContactDataInsert(rawOne, "phone", "1234565654546", "");
    HILOG_INFO("merge_Update_test_2700 dataIdOne  = %{public}ld", dataIdOne);
    int64_t rawTwo = RawContactInsert("jack12345");
    EXPECT_GT(rawTwo, 0);
    int64_t dataIdTwo = ContactDataInsert(rawTwo, "name", "jack12345", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "1234564488", "");
    dataIdTwo = ContactDataInsert(rawTwo, "phone", "12345644888", "");
    HILOG_INFO("merge_Update_test_2700 dataIdTwo  = %{public}ld", dataIdTwo);
    int time = Time::SLEEP_TIME_MERGE;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::Uri uriManualMerge(ContactsUri::MANUAL_MERGE);
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::DataShare::DataSharePredicates predicates;
    std::vector<std::string> ids;
    ids.push_back(std::to_string(rawOne));
    ids.push_back(std::to_string(rawTwo));
    predicates.In("raw_contact_id", ids);
    int ret = contactsDataAbility.Update(uriManualMerge, predicates, value);
    EXPECT_EQ(ret, 0);
    std::vector<int64_t> resultIdVector;
    resultIdVector.push_back(rawOne);
    resultIdVector.push_back(rawTwo);
    CheckMergeResultId(resultIdVector, true);
    DeleteRawContact();
}
} // namespace Test
} // namespace Contacts