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

#include "contactability_test.h"

#include "data_ability_operation_builder.h"

namespace Contacts {
namespace Test {
ContactAbilityTest::ContactAbilityTest()
{
}

ContactAbilityTest::~ContactAbilityTest()
{
}

int64_t ContactAbilityTest::RawContactInsert(std::string displayName, OHOS::NativeRdb::ValuesBucket &rawContactValues)
{
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    rawContactValues.PutString("display_name", displayName);
    int64_t code = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    return code;
}

int64_t ContactAbilityTest::RawContactExpandInsert(
    std::vector<std::string> valueVector, int isFavorite, OHOS::NativeRdb::ValuesBucket &rawContactValues)
{
    int indexZero = 0;
    int indexOne = 1;
    int indexTwo = 2;
    int indexThree = 3;
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    rawContactValues.PutString("display_name", valueVector[indexZero]);
    rawContactValues.PutString("company", valueVector[indexOne]);
    rawContactValues.PutString("position", valueVector[indexTwo]);
    rawContactValues.PutInt("favorite", isFavorite);
    rawContactValues.PutString("phonetic_name", valueVector[indexThree]);
    int64_t code = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    return code;
}

int64_t ContactAbilityTest::RawContactLastContactedInsert(
    std::string displayName, int lastestContactedTime, OHOS::NativeRdb::ValuesBucket &rawContactValues)
{
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    rawContactValues.PutString("display_name", displayName);
    rawContactValues.PutInt("lastest_contacted_time", lastestContactedTime);
    int64_t code = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    return code;
}

int64_t ContactAbilityTest::ContactDataInsert(int64_t rawContactId, std::string contentType, std::string detailInfo,
    std::string position, OHOS::NativeRdb::ValuesBucket &contactDataValues)
{
    OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
    contactDataValues.PutInt("raw_contact_id", rawContactId);
    contactDataValues.PutString("content_type", contentType);
    contactDataValues.PutString("detail_info", detailInfo);
    contactDataValues.PutString("position", position);
    int64_t code = contactsDataAbility.Insert(uriContactData, contactDataValues);
    return code;
}

int64_t ContactAbilityTest::GroupsInsert(std::string groupName, OHOS::NativeRdb::ValuesBucket &groupValues)
{
    OHOS::Uri uriGroups(ContactsUri::GROUPS);
    groupValues.PutString("group_name", groupName);
    int64_t code = contactsDataAbility.Insert(uriGroups, groupValues);
    return code;
}

int64_t ContactAbilityTest::ContactBlocklistInsert(
    std::string phoneNumber, OHOS::NativeRdb::ValuesBucket &contactBlocklistValues)
{
    OHOS::Uri uriBlacklist(ContactsUri::BLOCKLIST);
    contactBlocklistValues.PutString("phone_number", phoneNumber);
    int64_t code = contactsDataAbility.Insert(uriBlacklist, contactBlocklistValues);
    return code;
}

int ContactAbilityTest::ContactUpdate(const std::string &tableName, OHOS::NativeRdb::ValuesBucket updateValues,
    OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    int code = 0;
    if (tableName == ContactTabName::RAW_CONTACT) {
        OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
        code = contactsDataAbility.Update(uriRawContact, updateValues, predicates);
    } else if (tableName == ContactTabName::CONTACT_DATA) {
        OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
        code = contactsDataAbility.Update(uriContactData, updateValues, predicates);
    } else if (tableName == ContactTabName::GROUPS) {
        OHOS::Uri uriGroups(ContactsUri::GROUPS);
        code = contactsDataAbility.Update(uriGroups, updateValues, predicates);
    } else if (tableName == ContactTabName::CONTACT_BLOCKLIST) {
        OHOS::Uri uriBlocklist(ContactsUri::BLOCKLIST);
        code = contactsDataAbility.Update(uriBlocklist, updateValues, predicates);
    } else {
        HILOG_ERROR("ContactsDataAbility ====>no match uri action");
    }
    return code;
}

int ContactAbilityTest::ContactDelete(const std::string &tableName, OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    int code = 0;
    if (tableName == ContactTabName::RAW_CONTACT) {
        OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
        code = contactsDataAbility.Delete(uriRawContact, predicates);
    } else if (tableName == ContactTabName::CONTACT_DATA) {
        OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
        code = contactsDataAbility.Delete(uriContactData, predicates);
    } else if (tableName == ContactTabName::GROUPS) {
        OHOS::Uri uriGroups(ContactsUri::GROUPS);
        code = contactsDataAbility.Delete(uriGroups, predicates);
    } else if (tableName == ContactTabName::CONTACT_BLOCKLIST) {
        OHOS::Uri uriBlocklist(ContactsUri::BLOCKLIST);
        code = contactsDataAbility.Delete(uriBlocklist, predicates);
    } else if (tableName == ContactTabName::CONTACT) {
        OHOS::Uri uriContact(ContactsUri::CONTACT);
        code = contactsDataAbility.Delete(uriContact, predicates);
    } else if (tableName == ContactTabName::DELETED_RAW_CONTACT_RECORD) {
        OHOS::Uri deleteRawContact(ContactsUri::DELETED_RAW_CONTACT_RECORD);
        code = contactsDataAbility.Delete(deleteRawContact, predicates);
    } else {
        HILOG_ERROR("ContactsDataAbility ====>no match uri action");
    }
    return code;
}

std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactAbilityTest::ContactQuery(
    const std::string &tableName, std::vector<std::string> &columns, OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet;

    if (tableName == ContactTabName::RAW_CONTACT) {
        OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
        resultSet = contactsDataAbility.Query(uriRawContact, columns, predicates);
    } else if (tableName == ContactTabName::CONTACT_DATA) {
        OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
        resultSet = contactsDataAbility.Query(uriContactData, columns, predicates);
    } else if (tableName == ContactTabName::CONTACT) {
        OHOS::Uri uriContact(ContactsUri::CONTACT);
        resultSet = contactsDataAbility.Query(uriContact, columns, predicates);
    } else if (tableName == ContactTabName::GROUPS) {
        OHOS::Uri uriGroups(ContactsUri::GROUPS);
        resultSet = contactsDataAbility.Query(uriGroups, columns, predicates);
    } else if (tableName == ContactTabName::CONTACT_BLOCKLIST) {
        OHOS::Uri uriBlocklist(ContactsUri::BLOCKLIST);
        resultSet = contactsDataAbility.Query(uriBlocklist, columns, predicates);
    } else if (tableName == ContactTabName::DELETED_RAW_CONTACT) {
        OHOS::Uri uriDeletedRawContact(ContactsUri::DELETED_RAW_CONTACT);
        resultSet = contactsDataAbility.Query(uriDeletedRawContact, columns, predicates);
    } else if (tableName == ContactTabName::SEARCH_CONTACT) {
        OHOS::Uri uriSearchContact(ContactsUri::SEARCH);
        resultSet = contactsDataAbility.Query(uriSearchContact, columns, predicates);
    } else {
        HILOG_ERROR("ContactsDataAbility ====>no match uri action");
    }
    return resultSet;
}

void ContactAbilityTest::QueryAndExpectResult(std::string &tableName, OHOS::NativeRdb::DataAbilityPredicates predicates,
    OHOS::NativeRdb::ValuesBucket &values, std::string testName)
{
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(tableName, columns, predicates);
    CheckResultSet(values, resultSet, testName);
}

int64_t ContactAbilityTest::RawContactInsertValues(OHOS::NativeRdb::ValuesBucket &values)
{
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    int64_t code = contactsDataAbility.Insert(uriRawContact, values);
    return code;
}

int64_t ContactAbilityTest::ContactDataInsertValues(OHOS::NativeRdb::ValuesBucket &values)
{
    OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
    int64_t code = contactsDataAbility.Insert(uriContactData, values);
    return code;
}

int64_t ContactAbilityTest::GroupsInsertValues(OHOS::NativeRdb::ValuesBucket &values)
{
    OHOS::Uri uriGroups(ContactsUri::GROUPS);
    int64_t code = contactsDataAbility.Insert(uriGroups, values);
    return code;
}

int64_t ContactAbilityTest::ContactBlocklistInsertValues(OHOS::NativeRdb::ValuesBucket &values)
{
    OHOS::Uri uriContactBolcklist(ContactsUri::BLOCKLIST);
    int64_t code = contactsDataAbility.Insert(uriContactBolcklist, values);
    return code;
}

void ContactAbilityTest::GetAllRawContactColumns(
    std::vector<std::string> &columnInt, std::vector<std::string> &columnStr)
{
    columnInt.push_back("photo_id");
    columnInt.push_back("photo_file_id");
    columnInt.push_back("is_transfer_voicemail");
    columnInt.push_back("account_id");
    columnInt.push_back("version");
    columnInt.push_back("contacted_count");
    columnInt.push_back("lastest_contacted_time");
    columnInt.push_back("favorite");
    columnInt.push_back("phonetic_name_type");
    columnInt.push_back("read_only");
    columnInt.push_back("merge_mode");
    columnInt.push_back("is_need_merge");
    columnInt.push_back("merge_status");
    columnInt.push_back("is_merge_target");
    columnInt.push_back("vibration_setting");
    columnInt.push_back("sync_id");

    columnStr.push_back("personal_ringtone");
    columnStr.push_back("personal_notification_ringtone");
    columnStr.push_back("photo_first_name");
    columnStr.push_back("display_name");
    columnStr.push_back("sort");
    columnStr.push_back("favorite_order");
    columnStr.push_back("phonetic_name");
    columnStr.push_back("company");
    columnStr.push_back("position");
    columnStr.push_back("sort_first_letter");
    columnStr.push_back("syn_1");
    columnStr.push_back("syn_2");
    columnStr.push_back("syn_3");
}

void ContactAbilityTest::GetAllContactDataColumns(
    std::vector<std::string> &columnInt, std::vector<std::string> &columnStr)
{
    columnInt.push_back("read_only");
    columnInt.push_back("version");
    columnInt.push_back("is_preferred_number");

    columnStr.push_back("detail_info");
    columnStr.push_back("family_name");
    columnStr.push_back("middle_name_phonetic");
    columnStr.push_back("given_name");
    columnStr.push_back("given_name_phonetic");
    columnStr.push_back("alias_detail_info");
    columnStr.push_back("phonetic_name");
    columnStr.push_back("position");
    columnStr.push_back("extend1");
    columnStr.push_back("extend2");
    columnStr.push_back("extend3");
    columnStr.push_back("extend4");
    columnStr.push_back("city");
    columnStr.push_back("country");
    columnStr.push_back("neighborhood");
    columnStr.push_back("pobox");
    columnStr.push_back("region");
    columnStr.push_back("street");
    columnStr.push_back("alpha_name");
    columnStr.push_back("other_lan_last_name");
    columnStr.push_back("other_lan_first_name");
    columnStr.push_back("lan_style");
    columnStr.push_back("custom_data");
    columnStr.push_back("extend6");
    columnStr.push_back("extend7");
    columnStr.push_back("syn_1");
    columnStr.push_back("syn_2");
    columnStr.push_back("syn_3");
}

void ContactAbilityTest::GetAllGroupsColumns(std::vector<std::string> &columnInt, std::vector<std::string> &columnStr)
{
    columnInt.push_back("account_id");
    columnInt.push_back("ringtone_modify_time");
    columnInt.push_back("lastest_modify_time");
    columnStr.push_back("group_name");
    columnStr.push_back("group_notes");
    columnStr.push_back("group_ringtone");
}

void ContactAbilityTest::GetAllContactBlocklistColumns(
    std::vector<std::string> &columnInt, std::vector<std::string> &columnStr)
{
    columnInt.push_back("types");
    columnStr.push_back("phone_number");
    columnStr.push_back("content");
    columnStr.push_back("time_stamp");
}

void ContactAbilityTest::GetDetailsContactDataColumns(std::vector<std::string> &columns)
{
    columns.push_back("email");
    columns.push_back("im");
    columns.push_back("nickname");
    columns.push_back("organization");
    columns.push_back("phone");
    columns.push_back("name");
    columns.push_back("postal_address");
    columns.push_back("photo");
    columns.push_back("group_membership");
    columns.push_back("note");
    columns.push_back("contact_event");
    columns.push_back("website");
    columns.push_back("relation");
    columns.push_back("contact_misc");
    columns.push_back("hicall_device");
    columns.push_back("camcard");
    columns.push_back("sip_address");
}

void ContactAbilityTest::MergeColumns(
    std::vector<std::string> &columns, std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr)
{
    int columnsIntSize = columnsInt.size();
    for (int i = 0; i < columnsIntSize; i++) {
        columns.push_back(columnsInt[i]);
    }
    int columnsStrSize = columnsStr.size();
    for (int i = 0; i < columnsStrSize; i++) {
        columns.push_back(columnsStr[i]);
    }
}

/**
 * @brief get ValuesBucket
 * @params columnsStart column start index
 * @params columnsEnd column end index
 * @return ValuesBucket
 */
OHOS::NativeRdb::ValuesBucket ContactAbilityTest::GetAllColumnsValues(
    std::vector<std::string> &columnsInt, std::vector<std::string> &columnsStr)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    int randomInt = 0;
    int columnsIntSize = columnsInt.size();
    for (int i = 0; i < columnsIntSize; i++) {
        randomInt = ContactsRand();
        HILOG_INFO("rand=%{public}d", randomInt);
        valuesBucket.PutInt(columnsInt[i], randomInt);
    }
    std::string randomStr = "";
    int columnsStringSize = columnsStr.size();
    for (int i = 0; i < columnsStringSize; i++) {
        randomStr = columnsStr[i] + std::to_string(ContactsRand());
        valuesBucket.PutString(columnsStr[i], randomStr);
    }

    return valuesBucket;
}

void ContactAbilityTest::ClearContacts()
{
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    predicates.NotEqualTo("id", "0");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    contactsDataAbility.Delete(uriRawContact, predicates);
    int time = 1000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    predicates.Clear();
    OHOS::Uri uriRawContactComplete(ContactsUri::DELETED_RAW_CONTACT);
    predicates.NotEqualTo("id", "0");
    contactsDataAbility.Delete(uriRawContactComplete, predicates);
}

/*
 * @tc.number  contact_Insert_test_100
 * @tc.name    Add the basic information of a single contact and verify whether the insertion is successful
 * @tc.desc    New capabilities for basic contact data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_100 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    int64_t rawContactId = RawContactInsert("liming", values);
    HILOG_INFO("contact_Insert_test_100 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    QueryAndExpectResult(rawContacts, predicates, values, "contact_Insert_test_100");
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_200
 * @tc.name    Add the basic information of multiple contacts and verify whether the insertion is successful
 * @tc.desc    New capabilities for basic contact data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_200 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    int64_t rawContactIdOne = RawContactInsert("xiaoqian", values);
    HILOG_INFO("contact_Insert_test_200 : rawContactIdOne = %{public}lld", rawContactIdOne);
    EXPECT_GT(rawContactIdOne, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactIdOne));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    QueryAndExpectResult(rawContacts, predicates, values, "contact_Insert_test_200");
    values.Clear();

    int64_t rawContactIdTwo = RawContactInsert("xiaowang", values);
    HILOG_INFO("contact_Insert_test_200 : rawContactIdTwo = %{public}lld", rawContactIdTwo);
    EXPECT_GT(rawContactIdTwo, 0);
    predicates.EqualTo("id", std::to_string(rawContactIdTwo));
    QueryAndExpectResult(rawContacts, predicates, values, "contact_Insert_test_200");
    predicates.Clear();

    int64_t rawContactIdThree = RawContactInsert("xiaozhou", values);
    HILOG_INFO("contact_Insert_test_200 : rawContactIdThree = %{public}lld", rawContactIdThree);
    EXPECT_GT(rawContactIdThree, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactIdThree));
    QueryAndExpectResult(rawContacts, predicates, values, "contact_Insert_test_200");
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_300
 * @tc.name    Add a full field data to the raw_contact table and verify whether the insertion is successful
 * @tc.desc    Added ability to raw_contact
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Insert_test_300 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    GetAllRawContactColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    int rawId = RawContactInsertValues(valuesBucket);
    EXPECT_GT(rawId, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);

    // resultSet count 1
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(valuesBucket, resultSet, "contact_Insert_test_300");
    ClearContacts();
}

/*
* @tc.number  contact_Insert_test_400
* @tc.name    Add the basic information of a single contact and verify whether the insertion is successful
                (name, name Pinyin, Pinyin name, company, position, favorite or not)
* @tc.desc    Add basic information of a single contact
* @tc.level   Level1
* @tc.size    MediumTest
* @tc.type    Function
*/
HWTEST_F(ContactAbilityTest, contact_Insert_test_400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_400 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    std::vector<std::string> vectorValue;
    vectorValue.push_back("liming");
    vectorValue.push_back("tiantianxaingshang");
    vectorValue.push_back("Test");
    vectorValue.push_back("liming||lm");
    int64_t rawContactId = RawContactExpandInsert(vectorValue, 1, values);
    HILOG_INFO("contact_Insert_test_400 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    QueryAndExpectResult(rawContacts, predicates, values, "contact_Insert_test_400");
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_500
 * @tc.name    Add individual contact details and verify that the insertion was successful
 *              (including name, nickname, company, position, mobile phone number and email address)
 * @tc.desc    New ability to contact detailed data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_500 is starting! ---");
    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("ligang", rawContactValues);
    HILOG_INFO("contact_Insert_test_500 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);
    OHOS::NativeRdb::ValuesBucket values;
    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "name", "ligang", "", values);
    HILOG_INFO("contact_Insert_test_500 : contactDataIdOne = %{public}lld", contactDataIdOne);
    EXPECT_GT(contactDataIdOne, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_500");
    values.Clear();

    int64_t contactDataIdTwo = ContactDataInsert(rawContactId, "organization", "tiantianxaingshang", "Test", values);
    HILOG_INFO("contact_Insert_test_500 : contactDataIdTwo = %{public}lld", contactDataIdTwo);
    EXPECT_GT(contactDataIdTwo, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_500");
    values.Clear();

    int64_t contactDataIdThree = ContactDataInsert(rawContactId, "email", "8523@163.com", "", values);
    HILOG_INFO("contact_Insert_test_500 : contactDataIdThree = %{public}lld", contactDataIdThree);
    EXPECT_GT(contactDataIdThree, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdThree));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_500");
    values.Clear();

    int64_t contactDataIdFour = ContactDataInsert(rawContactId, "phone", "188520314", "", values);
    HILOG_INFO("contact_Insert_test_500 : contactDataIdFour = %{public}lld", contactDataIdFour);
    EXPECT_GT(contactDataIdFour, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdFour));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_500");
    values.Clear();

    int64_t contactDataIdFive = ContactDataInsert(rawContactId, "nickname", "xiaogang", "", values);
    HILOG_INFO("contact_Insert_test_500 : contactDataIdFive = %{public}lld", contactDataIdFive);
    EXPECT_GT(contactDataIdFive, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdFive));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_500");
    values.Clear();
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_600
 * @tc.name    Add individual contact details and verify that the insertion was successful
 *              (including home, group, aim, notes, and URL)
 * @tc.desc    New ability to contact detailed data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_600 is starting! ---");
    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::ValuesBucket values;
    int64_t rawContactId = RawContactInsert("chengshao", values);
    HILOG_INFO("contact_Insert_test_600 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);
    values.Clear();

    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "postal_address", "NanJingCity", "", values);
    HILOG_INFO("contact_Insert_test_600 : contactDataIdOne = %{public}lld", contactDataIdOne);
    EXPECT_GT(contactDataIdOne, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_600");
    values.Clear();

    int64_t contactDataIdTwo = ContactDataInsert(rawContactId, "group_membership", "1", "", values);
    HILOG_INFO("contact_Insert_test_600 : contactDataIdTwo = %{public}lld", contactDataIdTwo);
    EXPECT_GT(contactDataIdTwo, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_600");
    values.Clear();

    int64_t contactDataIdThree = ContactDataInsert(rawContactId, "note", "dalao", "", values);
    HILOG_INFO("contact_Insert_test_600 : contactDataIdThree = %{public}lld", contactDataIdThree);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdThree));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_600");
    values.Clear();

    int64_t contactDataIdFour = ContactDataInsert(rawContactId, "im", "aaaa", "", values);
    HILOG_INFO("contact_Insert_test_600 : contactDataIdFour = %{public}lld", contactDataIdFour);
    EXPECT_GT(contactDataIdFour, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdFour));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_600");
    values.Clear();

    int64_t contactDataIdFive = ContactDataInsert(rawContactId, "website", "www.48236.com", "", values);
    HILOG_INFO("contact_Insert_test_600 : contactDataIdFive = %{public}lld", contactDataIdFive);
    EXPECT_GT(contactDataIdFive, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdFive));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_600");
    values.Clear();
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_700
 * @tc.name    Add individual contact details and verify that the insertion was successful
 *              (including phone ring tones, birthdays, and assistants)
 * @tc.desc    New ability to contact detailed data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_700 is starting! ---");
    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::ValuesBucket values;
    int64_t rawContactId = RawContactInsert("yanshao", values);
    HILOG_INFO("contact_Insert_test_700 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);
    values.Clear();

    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "contact_event", "19960229", "", values);
    HILOG_INFO("contact_Insert_test_700 : contactDataIdOne = %{public}lld", contactDataIdOne);
    EXPECT_GT(contactDataIdOne, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_700");
    values.Clear();

    int64_t contactDataIdTwo = ContactDataInsert(rawContactId, "relation", "Secretary", "", values);
    HILOG_INFO("contact_Insert_test_700 : contactDataIdTwo = %{public}lld", contactDataIdTwo);
    EXPECT_GT(contactDataIdTwo, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_700");
    values.Clear();

    int64_t contactDataIdThree = ContactDataInsert(rawContactId, "contact_misc", "1314", "", values);
    HILOG_INFO("contact_Insert_test_700 : contactDataIdThree = %{public}lld", contactDataIdThree);
    EXPECT_GT(contactDataIdThree, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdThree));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_700");
    values.Clear();
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_800
 * @tc.name    Add all details of a single contact and verify whether the insertion is successful
 *              (name, mobile number, company, position, nickname, email, home, remarks, aim, birthday, website,
 * assistant, group, phone ring)
 * @tc.desc    New ability to contact detailed data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_800 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    int64_t rawContactId = RawContactInsert("insert_detail_contactdata", rawValuesBucket);
    EXPECT_GT(rawContactId, 0);

    std::vector<std::string> columns;
    std::vector<std::string> columnQuery;
    columnQuery.push_back("detail_info");
    GetDetailsContactDataColumns(columns);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    OHOS::NativeRdb::ValuesBucket valuesBucket;

    std::string randomStr = "";
    std::string tableName = ContactTabName::CONTACT_DATA;
    int columnSize = columns.size();
    for (int i = 0; i < columnSize; i++) {
        randomStr = columns[i] + std::to_string(ContactsRand());
        valuesBucket.PutInt("raw_contact_id", rawContactId);
        valuesBucket.PutString("content_type", columns[i]);
        valuesBucket.PutString("detail_info", randomStr);
        int contactDataId = ContactDataInsertValues(valuesBucket);
        EXPECT_GT(contactDataId, 0);

        predicates.Clear();
        predicates.EqualTo("id", std::to_string(contactDataId));
        std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
            ContactQuery(tableName, columnQuery, predicates);
        int rowCount = -1;
        resultSet->GetRowCount(rowCount);
        EXPECT_EQ(1, rowCount);
        CheckResultSet(valuesBucket, resultSet, "contact_Insert_test_800");
        valuesBucket.Clear();
    }
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_900
 * @tc.name    Add multiple contact details and verify that the insertion was successful
 * @tc.desc    New ability to contact detailed data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_900 is starting! ---");
    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::ValuesBucket values;
    int64_t rawContactId = RawContactInsert("xiaoming", values);
    EXPECT_GT(rawContactId, 0);
    values.Clear();

    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "name", "xiaoming", "", values);
    EXPECT_GT(contactDataIdOne, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_900");
    values.Clear();

    int64_t contactDataIdTwo = ContactDataInsert(rawContactId, "organization", "tiantianxaingshang", "Test", values);
    EXPECT_GT(contactDataIdTwo, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_900");
    values.Clear();

    rawContactId = RawContactInsert("lihong", values);
    HILOG_INFO("contact_Insert_test_900 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);
    values.Clear();

    int64_t contactDataIdThree = ContactDataInsert(rawContactId, "name", "lihong", "", values);
    HILOG_INFO("contact_Insert_test_900 : contactDataIdThree = %{public}lld", contactDataIdThree);
    EXPECT_GT(contactDataIdThree, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdThree));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_900");
    values.Clear();

    int64_t contactDataIdFour = ContactDataInsert(rawContactId, "organization", "tiantianxaingshang", "Test", values);
    HILOG_INFO("contact_Insert_test_900 : contactDataIdFour = %{public}lld", contactDataIdFour);
    EXPECT_GT(contactDataIdFour, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdFour));
    QueryAndExpectResult(contactData, predicates, values, "contact_Insert_test_900");
    values.Clear();
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_1000
 * @tc.name    Add a full field data to the contact_data table and verify that the insertion was successful
 * @tc.desc    Added ability to contact_data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_1000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Insert_test_1000 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string contactData = ContactTabName::CONTACT_DATA;
    GetAllContactDataColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("feimaomao", rawContactValues);
    valuesBucket.PutInt("raw_contact_id", rawContactId);
    // type 6 is name
    valuesBucket.PutInt("type_id", 6);
    int ContactDataId = ContactDataInsertValues(valuesBucket);
    EXPECT_GT(ContactDataId, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    CheckResultSet(valuesBucket, resultSet, "contact_Insert_test_1000");
    // resultSet count 1
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(valuesBucket, resultSet, "contact_Insert_test_1000");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1100
 * @tc.name    Modify several basic information of the contact, and verify whether the modification is successful
 *              (including name, name Pinyin, Pinyin name, company, position and favorite)
 * @tc.desc    Contacts basic data table update ability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_1100 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    std::vector<std::string> vectorValue;
    vectorValue.push_back("zhangming");
    vectorValue.push_back("tiantianxaingshang");
    vectorValue.push_back("Test");
    vectorValue.push_back("zhangming||zm");
    int64_t rawContactId = RawContactExpandInsert(vectorValue, 1, values);
    HILOG_INFO("contact_Update_test_1100 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("display_name", "dongming");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_1100");

    updateValues.Clear();
    updateValues.PutString("company", "XXXX");
    updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_1100");

    updateValues.Clear();
    updateValues.PutString("position", "Secretary");
    updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_1100");

    updateValues.Clear();
    updateValues.PutString("favorite", "0");
    updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_1100");

    updateValues.Clear();
    updateValues.PutString("phonetic_name", "dongming||dm");
    updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_1100");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1200
 * @tc.name    Modify the basic information of multiple contacts and verify whether the modification is successful
 * @tc.desc    Contacts basic data table update ability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_1200 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    int64_t rawContactIdOne = RawContactInsert("zhangming", values);
    EXPECT_GT(rawContactIdOne, 0);
    values.Clear();

    int64_t rawContactIdTwo = RawContactInsert("ligang", values);
    EXPECT_GT(rawContactIdTwo, 0);
    values.Clear();

    int64_t rawContactIdThree = RawContactInsert("wanghong", values);
    EXPECT_GT(rawContactIdThree, 0);
    values.Clear();

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("display_name", "dongming");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactIdOne));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_1200");

    updateValues.Clear();
    updateValues.PutString("display_name", std::string("laoliu"));
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactIdThree));
    updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_1200");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1300
 * @tc.name    Update the full field data of the raw_contact table and verify whether the modification is successful
 * @tc.desc    Support all raw_contact update capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Update_test_1300 is starting!-----");
    std::vector<std::string> columns;
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    GetAllRawContactColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    int rawId = RawContactInsertValues(valuesBucket);
    EXPECT_GT(rawId, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetOne = ContactQuery(rawContacts, columns, predicates);
    CheckResultSet(valuesBucket, resultSetOne, "contact_Update_test_1300");

    OHOS::NativeRdb::ValuesBucket upDateValuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    int upDateCode = ContactUpdate(rawContacts, upDateValuesBucket, predicates);
    EXPECT_EQ(upDateCode, 0);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);

    // resultSet count 1
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(upDateValuesBucket, resultSet, "contact_Update_test_1300");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1400
 * @tc.name    Modify the details of a single contact and verify whether the modification is successful
 *              (name, company, position and email address)
 * @tc.desc    The ability to update the contact detail data table
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_1400 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    int64_t rawContactId = RawContactInsert("ligang", values);
    EXPECT_GT(rawContactId, 0);
    values.Clear();

    OHOS::NativeRdb::ValuesBucket valuesOne;
    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "name", "ligang", "", valuesOne);
    EXPECT_GT(contactDataIdOne, 0);

    OHOS::NativeRdb::ValuesBucket valuesTwo;
    int64_t contactDataIdTwo = ContactDataInsert(rawContactId, "organization", "tiantianxaingshang", "Test", valuesTwo);
    EXPECT_GT(contactDataIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket valuesThree;
    int64_t contactDataIdThree = ContactDataInsert(rawContactId, "email", "8523@163.com", "", valuesThree);
    EXPECT_GT(contactDataIdThree, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("detail_info", "dongming");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    std::string contactData = ContactTabName::CONTACT_DATA;
    int updateCode = ContactUpdate(contactData, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1400");

    updateValues.Clear();
    updateValues.PutString("detail_info", "vivo");
    updateValues.PutString("position", "Developer");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1400");

    updateValues.Clear();
    updateValues.PutString("detail_info", "1220369@qq.com");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdThree));
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1400");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1500
 * @tc.name    Modify the details of a single contact and verify that the modification is successful
 *              (mobile phone number, address and nickname)
 * @tc.desc    The ability to update the contact detail data table
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_1500 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaomi", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValueOne;
    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "phone", "1665230", "", contactDataValueOne);
    EXPECT_GT(contactDataIdOne, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValueTwo;
    int64_t contactDataIdTwo = ContactDataInsert(rawContactId, "nickname", "xiaomi", "", contactDataValueTwo);
    EXPECT_GT(contactDataIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValueThree;
    int64_t contactDataIdThree =
        ContactDataInsert(rawContactId, "postal_address", "BeiJingFir", "", contactDataValueThree);
    EXPECT_GT(contactDataIdThree, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("detail_info", "33996652");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    std::string contactData = ContactTabName::CONTACT_DATA;
    int updateCode = ContactUpdate(contactData, updateValues, predicates);
    HILOG_INFO("contact_Update_test_1500: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1500");

    updateValues.Clear();
    updateValues.PutString("detail_info", "mimi");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1500");

    updateValues.Clear();
    updateValues.PutString("detail_info", "NanJIngGulou");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdThree));
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1500");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1600
 * @tc.name    Modify the details of individual contacts and verify whether the modification is successful
 *              (remarks, goals and birthdays)
 * @tc.desc    The ability to update the contact detail data table
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_1600 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaocai", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValueOne;
    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "note", "dalao", "", contactDataValueOne);
    EXPECT_GT(contactDataIdOne, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValueTwo;
    int64_t contactDataIdTwo = ContactDataInsert(rawContactId, "im", "aaaaaa", "", contactDataValueTwo);
    EXPECT_GT(contactDataIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValueThree;
    int64_t contactDataIdThree =
        ContactDataInsert(rawContactId, "contact_event", "19820314", "", contactDataValueThree);
    EXPECT_GT(contactDataIdThree, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("detail_info", "God");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    std::string contactData = ContactTabName::CONTACT_DATA;
    int updateCode = ContactUpdate(contactData, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1600");

    updateValues.Clear();
    updateValues.PutString("detail_info", "bcade");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    HILOG_INFO("contact_Update_test_1600: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1600");

    updateValues.Clear();
    updateValues.PutString("detail_info", "19820328");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdThree));
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    HILOG_INFO("contact_Update_test_1600: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1600");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1700
 * @tc.name    Modify the details of a single contact and verify that the modification was successful
 *              (URL, assistant, and group)
 * @tc.desc    The ability to update the contact detail data table
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_1700 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaocai", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket contactValuesOne;
    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "website", "www.aaa.com", "", contactValuesOne);
    EXPECT_GT(contactDataIdOne, 0);

    OHOS::NativeRdb::ValuesBucket contactValuesTwo;
    int64_t contactDataIdTwo = ContactDataInsert(rawContactId, "relation", "fuzi", "", contactValuesTwo);
    EXPECT_GT(contactDataIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket contactValuesThree;
    int64_t contactDataIdThree = ContactDataInsert(rawContactId, "group_membership", "1", "", contactValuesThree);
    EXPECT_GT(contactDataIdThree, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("detail_info", "www.bbb.com");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    std::string contactData = ContactTabName::CONTACT_DATA;
    int updateCode = ContactUpdate(contactData, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1700");

    updateValues.Clear();
    updateValues.PutString("detail_info", "laozhang");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    HILOG_INFO("contact_Update_test_1700: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1700");

    updateValues.Clear();
    updateValues.PutString("detail_info", "2");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdThree));
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    HILOG_INFO("contact_Update_test_1700: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1700");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1800
 * @tc.name    Modify the details of a single contact and verify that the modification was successful (phone ringing)
 * @tc.desc    The ability to update the contact detail data table
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_1800 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaocai", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValues;
    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "contact_misc", "111", "", contactDataValues);
    EXPECT_GT(contactDataIdOne, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("detail_info", "222");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    std::string contactData = ContactTabName::CONTACT_DATA;
    int updateCode = ContactUpdate(contactData, updateValues, predicates);
    HILOG_INFO("contact_Update_test_1800: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1800");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_1900
 * @tc.name    Modify multiple contact details and verify whether the modification is successful
 * @tc.desc    The ability to update the contact detail data table
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_1900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_1900 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactIdOne = RawContactInsert("ligang", rawContactValues);
    EXPECT_GT(rawContactIdOne, 0);
    OHOS::NativeRdb::ValuesBucket contactValues;
    int64_t contactDataId = ContactDataInsert(rawContactIdOne, "name", "ligang", "", contactValues);
    EXPECT_GT(contactDataId, 0);
    contactValues.Clear();

    contactDataId = ContactDataInsert(rawContactIdOne, "organization", "tiantianxaingshang", "Test", contactValues);
    EXPECT_GT(contactDataId, 0);
    contactValues.Clear();

    rawContactValues.Clear();
    int64_t rawContactIdTwo = RawContactInsert("zhangming", rawContactValues);
    EXPECT_GT(rawContactIdTwo, 0);

    contactDataId = ContactDataInsert(rawContactIdTwo, "name", "zhangming", "", contactValues);
    EXPECT_GT(contactDataId, 0);
    contactValues.Clear();

    contactDataId = ContactDataInsert(rawContactIdTwo, "organization", "tiantianxaingshang", "Test", contactValues);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("detail_info", "lixiang");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactIdOne));
    predicates.And();
    // type 6 is phone
    predicates.EqualTo("type_id", "6");
    std::string contactData = ContactTabName::CONTACT_DATA;
    int updateCode = ContactUpdate(contactData, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1900");

    updateValues.Clear();
    updateValues.PutString("detail_info", "zhangsan");
    predicates.Clear();
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactIdTwo));
    predicates.And();
    // type 6 is phone
    predicates.EqualTo("type_id", "6");
    updateCode = ContactUpdate(contactData, updateValues, predicates);
    HILOG_INFO("contact_Update_test_1900: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(contactData, predicates, updateValues, "contact_Update_test_1900");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_2000
 * @tc.name    Update the full field data of the contact_data table and verify whether the modification is successful
 * @tc.desc    Update ability to contact_data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_2000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Update_test_2000 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string tableName = ContactTabName::CONTACT_DATA;
    GetAllContactDataColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("feimaotui", rawContactValues);
    valuesBucket.PutInt("raw_contact_id", rawContactId);
    // type_id 6 is name
    valuesBucket.PutInt("type_id", 6);
    int ContactDataId = ContactDataInsertValues(valuesBucket);
    EXPECT_GT(ContactDataId, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    OHOS::NativeRdb::ValuesBucket upDateValuesBucket = GetAllColumnsValues(columnsInt, columnsStr);

    int upDateCode = ContactUpdate(tableName, upDateValuesBucket, predicates);
    EXPECT_EQ(upDateCode, 0);

    OHOS::NativeRdb::ValueObject value;
    valuesBucket.GetObject("version", value);
    int versionCode = -1;
    value.GetInt(versionCode);
    versionCode += 1;
    upDateValuesBucket.Delete("version");
    upDateValuesBucket.PutInt("version", versionCode);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(tableName, columns, predicates);

    // resultSet count 1
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(upDateValuesBucket, resultSet, "contact_Update_test_2000");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_2100
 * @tc.name    Update all contact details and verify that the modification was successful
 * @tc.desc    New ability to contact detailed data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_2100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_2100 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    int64_t rawContactId = RawContactInsert("update_detail_contactdata", rawValuesBucket);
    EXPECT_GT(rawContactId, 0);

    std::vector<std::string> columns;
    std::vector<std::string> columnQuery;
    columnQuery.push_back("detail_info");
    GetDetailsContactDataColumns(columns);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    OHOS::NativeRdb::ValuesBucket upDateValuesBucket;

    std::string randomStr = "";
    std::string updateStrValue = "";
    std::string tableName = ContactTabName::CONTACT_DATA;
    int columnSize = columns.size();
    for (int i = 0; i < columnSize; i++) {
        randomStr = columns[i] + std::to_string(ContactsRand());
        valuesBucket.PutInt("raw_contact_id", rawContactId);
        valuesBucket.PutString("content_type", columns[i]);
        valuesBucket.PutString("detail_info", randomStr);
        int contactDataId = ContactDataInsertValues(valuesBucket);
        EXPECT_GT(contactDataId, 0);

        predicates.Clear();
        predicates.EqualTo("id", std::to_string(contactDataId));
        updateStrValue = "update_all" + randomStr;
        upDateValuesBucket.PutString("detail_info", updateStrValue);
        int upDateCode = ContactUpdate(tableName, upDateValuesBucket, predicates);
        EXPECT_EQ(upDateCode, 0);

        std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
            ContactQuery(tableName, columnQuery, predicates);
        int rowCount = -1;
        resultSet->GetRowCount(rowCount);
        EXPECT_EQ(1, rowCount);
        CheckResultSet(upDateValuesBucket, resultSet, "contact_Update_test_2100");

        valuesBucket.Clear();
        upDateValuesBucket.Clear();
    }
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_2200
 * @tc.name    Add a single contact to your favorites and verify that the favorites field has changed
 * @tc.desc    Contacts favorites and unfavorite capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_2200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_2200 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaozong", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutInt("favorite", 1);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_2200");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_2300
 * @tc.name    Add multiple contacts to favorites and verify that the favorites field is changed
 * @tc.desc    Contacts favorites and unfavorite capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_2300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_2300 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactIdOne = RawContactInsert("xiaowang", rawContactValues);
    EXPECT_GT(rawContactIdOne, 0);

    rawContactValues.Clear();
    int64_t rawContactIdTwo = RawContactInsert("xiaozhou", rawContactValues);
    EXPECT_GT(rawContactIdTwo, 0);
    rawContactValues.Clear();
    int64_t rawContactIdThree = RawContactInsert("laowei", rawContactValues);
    EXPECT_GT(rawContactIdThree, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutInt("favorite", 1);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawContactIdTwo));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawContactIdThree));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);

    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactIdOne));
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_2300");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactIdTwo));
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_2300");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactIdThree));
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_2300");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_2400
 * @tc.name    Unfriend individual contacts and verify that the favorites field has changed
 * @tc.desc    Contacts favorites and unfavorite capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_2400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_2400 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    std::vector<std::string> vectorValue;
    vectorValue.push_back("zhangming");
    vectorValue.push_back("tiantianxaingshang");
    vectorValue.push_back("Test");
    vectorValue.push_back("zhangming||zm");
    int64_t rawContactId = RawContactExpandInsert(vectorValue, 1, values);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutInt("favorite", 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_2400");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_2500
 * @tc.name    Remove favorites from multiple contacts and verify that the favorites field has changed
 * @tc.desc    Contacts favorites and unfavorite capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_2500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_2500 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    std::vector<std::string> vectorValue;
    vectorValue.push_back("zhangming");
    vectorValue.push_back("tiantianxaingshang");
    vectorValue.push_back("Test");
    vectorValue.push_back("zhangming||zm");
    int64_t rawContactId = RawContactExpandInsert(vectorValue, 1, values);
    EXPECT_GT(rawContactId, 0);

    vectorValue.clear();
    values.Clear();
    vectorValue.push_back("laozhou");
    vectorValue.push_back("tiantianxaingshang");
    vectorValue.push_back("Test");
    vectorValue.push_back("laozhou||lz");
    int64_t rawContactIdTwo = RawContactExpandInsert(vectorValue, 1, values);
    EXPECT_GT(rawContactIdTwo, 0);

    vectorValue.clear();
    values.Clear();
    vectorValue.push_back("abiao");
    vectorValue.push_back("tiantianxaingshang");
    vectorValue.push_back("Test");
    vectorValue.push_back("abiao||ab");
    int64_t rawContactIdThree = RawContactExpandInsert(vectorValue, 1, values);
    HILOG_INFO("contact_Update_test_2500 : rawContactIdThree = %{public}lld", rawContactIdThree);
    EXPECT_GT(rawContactIdThree, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutInt("favorite", 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawContactIdTwo));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawContactIdThree));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    EXPECT_EQ(updateCode, 0);

    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactId));
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_2500");

    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactIdTwo));
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_2500");

    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactIdThree));
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_2500");
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_2600
 * @tc.name    Add a single record to the blocklist and verify that the insertion was successful
 * @tc.desc    The ability to add and remove contacts from the blocklist
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_2600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_2600 is starting! ---");
    OHOS::NativeRdb::ValuesBucket blocklistValues;
    int64_t blocklistId = ContactBlocklistInsert("10086", blocklistValues);
    EXPECT_GT(blocklistId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(blocklistId));
    std::string contactBlocklist = ContactTabName::CONTACT_BLOCKLIST;
    QueryAndExpectResult(contactBlocklist, predicates, blocklistValues, "contact_Insert_test_2600");
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_2700
 * @tc.name    Add multiple records to the blocklist and verify whether the insertion is successful
 * @tc.desc    The ability to add and remove contacts from the blocklist
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_2700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_2700 is starting! ---");
    std::string contactBlocklist = ContactTabName::CONTACT_BLOCKLIST;
    OHOS::NativeRdb::ValuesBucket blocklistValues;
    int64_t blocklistIdOne = ContactBlocklistInsert("188520", blocklistValues);
    EXPECT_GT(blocklistIdOne, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(blocklistIdOne));
    QueryAndExpectResult(contactBlocklist, predicates, blocklistValues, "contact_Insert_test_2700");

    blocklistValues.Clear();
    predicates.Clear();
    int64_t blocklistIdTwo = ContactBlocklistInsert("130269", blocklistValues);
    EXPECT_GT(blocklistIdTwo, 0);
    predicates.EqualTo("id", std::to_string(blocklistIdTwo));
    QueryAndExpectResult(contactBlocklist, predicates, blocklistValues, "contact_Insert_test_2700");

    blocklistValues.Clear();
    predicates.Clear();
    int64_t blocklistIdThree = ContactBlocklistInsert("772501", blocklistValues);
    EXPECT_GT(blocklistIdThree, 0);
    predicates.EqualTo("id", std::to_string(blocklistIdThree));
    QueryAndExpectResult(contactBlocklist, predicates, blocklistValues, "contact_Insert_test_2700");
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_2800
 * @tc.name    Add a full field data to the contact_blocklist table and verify whether the insertion is successful
 * @tc.desc    Added ability to ContactBlocklist
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_2800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Insert_test_2800 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string tableName = ContactTabName::CONTACT_BLOCKLIST;
    GetAllContactBlocklistColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    int contactBlockId = ContactBlocklistInsertValues(valuesBucket);
    EXPECT_GT(contactBlockId, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactBlockId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(tableName, columns, predicates);
    // resultSet count 1
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(valuesBucket, resultSet, "contact_Insert_test_2800");
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_2900
 * @tc.name    Delete a contact from the blocklist and verify whether the deletion is successful
 * @tc.desc    The ability to add and remove contacts from the blocklist
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_2900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_test_2900 is starting! ---");
    OHOS::NativeRdb::ValuesBucket blocklistValues;
    int64_t blocklistId = ContactBlocklistInsert("147852369", blocklistValues);
    EXPECT_GT(blocklistId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(blocklistId));
    std::string contactBlocklist = ContactTabName::CONTACT_BLOCKLIST;
    int deleteCode = ContactDelete(contactBlocklist, predicates);
    EXPECT_EQ(deleteCode, 0);
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(contactBlocklist, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    resultSet->Close();
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_3000
 * @tc.name    Delete multiple contacts from the blocklist and verify whether the deletion is successful
 * @tc.desc    The ability to add and remove contacts from the blocklist
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_3000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_test_3000 is starting! ---");
    OHOS::NativeRdb::ValuesBucket blocklistValues;
    int64_t blocklistIdOne = ContactBlocklistInsert("111228855", blocklistValues);
    EXPECT_GT(blocklistIdOne, 0);

    blocklistValues.Clear();
    int64_t blocklistIdTwo = ContactBlocklistInsert("11335566", blocklistValues);
    EXPECT_GT(blocklistIdTwo, 0);

    blocklistValues.Clear();
    int64_t blocklistIdThree = ContactBlocklistInsert("9933220011", blocklistValues);
    EXPECT_GT(blocklistIdThree, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(blocklistIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(blocklistIdTwo));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(blocklistIdThree));
    std::string contactBlocklist = ContactTabName::CONTACT_BLOCKLIST;
    int deleteCode = ContactDelete(contactBlocklist, predicates);
    EXPECT_EQ(deleteCode, 0);

    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(contactBlocklist, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    resultSet->Close();
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_3100
 * @tc.name    Delete a full field data of contact_blocklist table and verify whether the deletion is successful
 * @tc.desc    ContactBlocklist deletion capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_3100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Delete_test_3100 is starting!-----");
    // insert
    std::vector<std::string> columns;
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    GetAllContactBlocklistColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    int contacBlockId = ContactBlocklistInsertValues(valuesBucket);
    EXPECT_GT(contacBlockId, 0);

    // test end delete data
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contacBlockId));
    std::string tableName = ContactTabName::CONTACT_BLOCKLIST;
    int deleteCode = ContactDelete(tableName, predicates);
    EXPECT_EQ(deleteCode, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetDeleteQuery =
        ContactQuery(tableName, columns, predicates);
    int rowCount = -1;
    resultSetDeleteQuery->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_3200
 * @tc.name    Join a single contact to the group and verify whether the join is successful
 * @tc.desc    Ability to add and remove contacts from groups
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_3200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_3200 is staring! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaoli", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket groupValues;
    int64_t groupId = GroupsInsert("TestFirstGroup", groupValues);
    EXPECT_GT(groupId, 0);
    OHOS::NativeRdb::ValuesBucket contactDataValue;
    int64_t contactDataId =
        ContactDataInsert(rawContactId, "group_membership", std::to_string(groupId), "", contactDataValue);
    EXPECT_GT(contactDataId, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataId));
    std::string contactData = ContactTabName::CONTACT_DATA;
    QueryAndExpectResult(contactData, predicates, contactDataValue, "contact_Insert_test_3200");
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_3300
 * @tc.name    Multiple contacts join the group and verify whether the joining is successful
 * @tc.desc    Ability to add and remove contacts from groups
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_3300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_3300 is staring! ---");
    OHOS::NativeRdb::ValuesBucket groupValues;
    int64_t groupIdOne = GroupsInsert("TestSecondGroup", groupValues);
    EXPECT_GT(groupIdOne, 0);
    groupValues.Clear();

    int64_t groupIdTwo = GroupsInsert("DeveloperFirstGroup", groupValues);
    EXPECT_GT(groupIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactIdOne = RawContactInsert("xiaoli", rawContactValues);
    EXPECT_GT(rawContactIdOne, 0);
    rawContactValues.Clear();

    int64_t rawContactIdTwo = RawContactInsert("BossCai", rawContactValues);
    EXPECT_GT(rawContactIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValuesOne;
    int64_t contactDataIdOne =
        ContactDataInsert(rawContactIdOne, "group_membership", std::to_string(groupIdOne), "", contactDataValuesOne);
    EXPECT_GT(contactDataIdOne, 0);
    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    QueryAndExpectResult(contactData, predicates, contactDataValuesOne, "contact_Insert_test_3300");

    OHOS::NativeRdb::ValuesBucket contactDataValuesTwo;
    int64_t contactDataIdTwo =
        ContactDataInsert(rawContactIdTwo, "group_membership", std::to_string(groupIdTwo), "", contactDataValuesTwo);
    EXPECT_GT(contactDataIdTwo, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    QueryAndExpectResult(contactData, predicates, contactDataValuesTwo, "contact_Insert_test_3300");
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_3400
 * @tc.name    Add a full field data to the groups table and verify whether the insertion is successful
 * @tc.desc    Added ability to groups
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_3400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Insert_test_3400 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string group = ContactTabName::GROUPS;
    GetAllGroupsColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    int rawId = GroupsInsertValues(valuesBucket);
    EXPECT_GT(rawId, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(group, columns, predicates);
    // resultSet count 1
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(valuesBucket, resultSet, "contact_Insert_test_3400");
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_3500
 * @tc.name    Delete a single contact from the group and verify that the deletion was successful
 * @tc.desc    Ability to add and remove contacts from groups
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_3500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_test_3500 is starting! ---");
    OHOS::NativeRdb::ValuesBucket groupValues;
    int64_t groupId = GroupsInsert("CEO", groupValues);
    EXPECT_GT(groupId, 0);

    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaoli", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValues;
    int64_t contactDataId =
        ContactDataInsert(rawContactId, "group_membership", std::to_string(groupId), "", contactDataValues);
    EXPECT_GT(contactDataId, 0);

    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    predicates.And();
    // type_id 9  is group_membership
    predicates.EqualTo("type_id", "9");
    int deleteCode = ContactDelete(contactData, predicates);
    EXPECT_EQ(deleteCode, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataId));
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    resultSet->Close();
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_3600
 * @tc.name    Delete multiple contacts from the group and verify that the deletion was successful
 * @tc.desc    Ability to add and remove contacts from groups
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_3600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_test_3600 is starting! ---");
    OHOS::NativeRdb::ValuesBucket groupValues;
    int64_t groupIdOne = GroupsInsert("Test", groupValues);
    HILOG_INFO("contact_Delete_test_3600: groupIdOne = %{public}lld", groupIdOne);
    EXPECT_GT(groupIdOne, 0);

    groupValues.Clear();
    int64_t groupIdTwo = GroupsInsert("Developer", groupValues);
    HILOG_INFO("contact_Delete_test_3600: groupIdTwo = %{public}lld", groupIdTwo);
    EXPECT_GT(groupIdTwo, 0);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactIdOne = RawContactInsert("xiaoli", rawContactValues);
    EXPECT_GT(rawContactIdOne, 0);

    rawContactValues.Clear();
    int64_t rawContactIdTwo = RawContactInsert("xiaoyuan", rawContactValues);
    EXPECT_GT(rawContactIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValues;
    int64_t contactDataIdOne =
        ContactDataInsert(rawContactIdOne, "group_membership", std::to_string(groupIdOne), "", contactDataValues);
    EXPECT_GT(contactDataIdOne, 0);

    contactDataValues.Clear();
    int64_t contactDataIdTwo =
        ContactDataInsert(rawContactIdTwo, "group_membership", std::to_string(groupIdTwo), "", contactDataValues);
    EXPECT_GT(contactDataIdTwo, 0);

    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    int deleteCode = ContactDelete(contactData, predicates);
    EXPECT_EQ(deleteCode, 0);
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdOne));
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    resultSet->Close();

    predicates.Clear();
    predicates.EqualTo("id", std::to_string(contactDataIdTwo));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetTwo = ContactQuery(contactData, columns, predicates);
    int rowCountTwo = 0;
    resultSetTwo->GetRowCount(rowCountTwo);
    EXPECT_EQ(0, rowCountTwo);
    resultSetTwo->Close();
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_3700
 * @tc.name    Delete the full field data of a groups table and verify whether the deletion is successful
 * @tc.desc    Groups deletion capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_3700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Delete_test_3700 is starting!-----");
    // insert
    std::vector<std::string> columns;
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    GetAllGroupsColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    int groupId = GroupsInsertValues(valuesBucket);
    EXPECT_GT(groupId, 0);

    // test end delete data
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(groupId));
    EXPECT_GT(groupId, 0);
    std::string tableName = ContactTabName::GROUPS;
    int deleteCode = ContactDelete(tableName, predicates);
    EXPECT_EQ(deleteCode, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetDeleteQuery =
        ContactQuery(tableName, columns, predicates);
    int rowCount = -1;
    resultSetDeleteQuery->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_3800
 * @tc.name    Insert contact information and query contact information according to the returned ID
 * @tc.desc    The ability to query the basic information of a single contact
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_3800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_3800 is starting! ---");
    std::vector<std::string> columns;
    columns.push_back("display_name");
    columns.push_back("company");
    columns.push_back("position");
    columns.push_back("favorite");
    columns.push_back("phonetic_name");
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString("display_name", "xiaoyuan");
    valuesBucket.PutString("company", "tiantianxiangshang");
    valuesBucket.PutString("position", "Test");
    valuesBucket.PutInt("favorite", 1);
    valuesBucket.PutString("phonetic_name", "xiaoyuanxy");

    int64_t rawContactId = RawContactInsertValues(valuesBucket);
    EXPECT_GT(rawContactId, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);

    // resultSet count 1
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(valuesBucket, resultSet, "contact_Query_test_3800");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_3900
 * @tc.name    Insert multiple contact information and query the contact information according to the returned ID
 * @tc.desc    The ability to query the basic information of multiple contacts
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_3900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_3900 is starting! ---");
    OHOS::NativeRdb::ValuesBucket valuesBucketOne;
    valuesBucketOne.PutString("display_name", "xiaohei");
    valuesBucketOne.PutString("company", "tiantianxiangshang");
    valuesBucketOne.PutString("position", "Test");
    valuesBucketOne.PutInt("favorite", 1);
    valuesBucketOne.PutString("phonetic_name", "xiaohei||xh");
    int64_t rawContactIdOne = RawContactInsertValues(valuesBucketOne);
    EXPECT_GT(rawContactIdOne, 0);

    OHOS::NativeRdb::ValuesBucket valuesBucketTwo;
    valuesBucketTwo.PutString("display_name", "xiaobai");
    valuesBucketTwo.PutString("company", "tiantianxiangshang");
    valuesBucketTwo.PutString("position", "Test");
    valuesBucketTwo.PutInt("favorite", 1);
    valuesBucketTwo.PutString("phonetic_name", "xiaohei||xh");
    int64_t rawContactIdTwo = RawContactInsertValues(valuesBucketTwo);
    EXPECT_GT(rawContactIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket valuesBucketThr;
    valuesBucketThr.PutString("display_name", "xiaocai");
    valuesBucketThr.PutString("company", "tiantianxiangshang");
    valuesBucketThr.PutString("position", "Test");
    valuesBucketThr.PutInt("favorite", 1);
    valuesBucketThr.PutString("phonetic_name", "xiaohei||xh");
    int64_t rawContactIdThr = RawContactInsertValues(valuesBucketThr);
    EXPECT_GT(rawContactIdThr, 0);

    std::vector<std::string> columns;
    columns.push_back("display_name");
    columns.push_back("company");
    columns.push_back("position");
    columns.push_back("favorite");
    columns.push_back("phonetic_name");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawContactIdTwo));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawContactIdThr));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);
    std::vector<OHOS::NativeRdb::ValuesBucket> listValue;
    listValue.push_back(valuesBucketOne);
    listValue.push_back(valuesBucketTwo);
    listValue.push_back(valuesBucketThr);
    CheckResultSetList(listValue, resultSet, "contact_Query_test_3900");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4000
 * @tc.name    Query the basic information of all contacts in the raw_contact table
 * @tc.desc    The ability to query the basic information of all contacts
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_4000 is starting! ---");
    std::string tag("contact_Query_test_4000");
    OHOS::NativeRdb::ValuesBucket valuesBucketOne;
    valuesBucketOne.PutString("display_name", "xiaohsaaei");
    valuesBucketOne.PutString("company", "tiantianxiadsjjnngshang");
    valuesBucketOne.PutString("position", "Tests");
    valuesBucketOne.PutInt("favorite", 1);
    valuesBucketOne.PutString("phonetic_name", "xiaohssei||x00h");
    RawContactInsertValues(valuesBucketOne);
    RawContactInsertValues(valuesBucketOne);
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("display_name");
    columns.push_back("company");
    columns.push_back("position");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.GreaterThan("id", "0");
    predicates.EqualTo("is_deleted", "0");
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);
    // resultSet count
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_GT(rowCount, 1);
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4100
 * @tc.name    Insert all contact information and query
 * @tc.desc    The ability to query all information of a single contact
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Query_test_4100 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string contactData = ContactTabName::CONTACT_DATA;
    GetAllContactDataColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    rawContactValues.PutString("favorite", "1");
    int64_t rawContactId = RawContactInsert("feimaomao4100", rawContactValues);
    valuesBucket.PutInt("raw_contact_id", rawContactId);
    // type 6 is name
    valuesBucket.PutInt("type_id", 6);
    int ContactDataId = ContactDataInsertValues(valuesBucket);
    EXPECT_GT(ContactDataId, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    CheckResultSet(valuesBucket, resultSet, "contact_Query_test_4100");
    // resultSet count 1
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(valuesBucket, resultSet, "contact_Query_test_4100");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4200
 * @tc.name    Add a group and add two new contacts to the group to query the members of the group
 * @tc.desc    Query group member ability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_4200 is starting! ---");
    std::string tag("contact_Query_test_4200");
    OHOS::NativeRdb::ValuesBucket valuesBucketGroup;
    int64_t groupId = GroupsInsert("dongshihui", valuesBucketGroup);
    EXPECT_GT(groupId, 0);

    OHOS::NativeRdb::ValuesBucket rawContactValuesBucket;
    int64_t rawContactId = RawContactInsert("licheng", rawContactValuesBucket);
    EXPECT_GT(rawContactId, 0);
    OHOS::NativeRdb::ValuesBucket valuesBucketOne;
    int64_t contactDataId =
        ContactDataInsert(rawContactId, "group_membership", std::to_string(groupId), "", valuesBucketOne);
    EXPECT_GT(contactDataId, 0);

    rawContactValuesBucket.Clear();
    rawContactId = RawContactInsert("xiaoyuan", rawContactValuesBucket);
    EXPECT_GT(rawContactId, 0);
    OHOS::NativeRdb::ValuesBucket valuesBucketTwo;
    contactDataId = ContactDataInsert(rawContactId, "group_membership", std::to_string(groupId), "", valuesBucketTwo);
    EXPECT_GT(contactDataId, 0);

    std::vector<std::string> columns;
    columns.push_back("detail_info");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("detail_info", std::to_string(groupId));
    predicates.And();
    // type_id 9  is group_membership
    predicates.EqualTo("type_id", "9");
    std::string contactData = ContactTabName::CONTACT_DATA;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<OHOS::NativeRdb::ValuesBucket> listValue;
    listValue.push_back(valuesBucketOne);
    listValue.push_back(valuesBucketTwo);
    CheckResultSetList(listValue, resultSet, "contact_Query_test_4200");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4300
 * @tc.name    Query favorite contacts
 * @tc.desc    Query the ability of favorite contacts
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_4300 is starting! ---");
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString("display_name", "xiaoyuan");
    valuesBucket.PutString("company", "tiantianxiangshang");
    valuesBucket.PutString("position", "Test");
    valuesBucket.PutInt("favorite", 1);
    valuesBucket.PutString("phonetic_name", "xiaoyuanxy");

    int64_t rawContactId = RawContactInsertValues(valuesBucket);
    EXPECT_GT(rawContactId, 0);

    std::vector<std::string> columns;
    columns.push_back("display_name");
    columns.push_back("company");
    columns.push_back("position");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("favorite", "1");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    predicates.And();
    predicates.EqualTo("id", std::to_string(rawContactId));

    std::string rawContacts = ContactTabName::RAW_CONTACT;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);

    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(valuesBucket, resultSet, "contact_Query_test_4300");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4400
 * @tc.name    Query recent contacts
 * @tc.desc    Query recent contact ability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_4400 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValuesBucket;
    int64_t rawContactId = RawContactLastContactedInsert("wangwu", 60, rawContactValuesBucket);
    EXPECT_GT(rawContactId, 0);

    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("display_name");
    columns.push_back("lastest_contacted_time");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("lastest_contacted_time", "60");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);

    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(rawContactValuesBucket, resultSet, "contact_Query_test_4400");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4500
 * @tc.name    Query the recently deleted contacts in the deleted contacts table
 * @tc.desc    Ability to query recently deleted contacts
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_4500 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaobai", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int deleteCode = ContactDelete(rawContacts, predicates);
    EXPECT_EQ(deleteCode, 0);

    sleep(SLEEP_TIME);
    std::vector<std::string> columns;
    columns.push_back("display_name");
    predicates.Clear();
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::string deletedRawContact = ContactTabName::DELETED_RAW_CONTACT;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(deletedRawContact, columns, predicates);

    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    CheckResultSet(rawContactValues, resultSet, "contact_Query_test_4500");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4600
 * @tc.name    Query the mobile phone numbers of all contacts
 * @tc.desc    Query the capabilities of all mobile phone numbers
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_4600 is starting! ---");
    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.GreaterThan("id", "0");
    int deleteCode = ContactDelete(contactData, predicates);
    EXPECT_EQ(deleteCode, 0);

    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactIdOne = RawContactInsert("dongming", rawContactValues);
    EXPECT_GT(rawContactIdOne, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValues;
    int64_t contactDataId = ContactDataInsert(rawContactIdOne, "phone", "155825478", "", contactDataValues);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::ValuesBucket rawContactValuesTwo;
    int64_t rawContactIdTwo = RawContactInsert("xiaocai", rawContactValuesTwo);
    EXPECT_GT(rawContactIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket contactDataValuesTwo;
    contactDataId = ContactDataInsert(rawContactIdTwo, "phone", "18853269857", "", contactDataValuesTwo);
    EXPECT_GT(contactDataId, 0);

    std::vector<std::string> columns;
    columns.push_back("detail_info");
    predicates.Clear();
    // type_id 5 is phone
    predicates.EqualTo("type_id", "5");
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    std::vector<OHOS::NativeRdb::ValuesBucket> listValue;
    listValue.push_back(contactDataValues);
    listValue.push_back(contactDataValuesTwo);
    CheckResultSetList(listValue, resultSet, "contact_Query_test_4600");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4700
 * @tc.name    Query mailbox of all contacts
 * @tc.desc    Query all mailbox capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_4700 is starting! ---");
    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.GreaterThan("id", "0");
    int deleteCode = ContactDelete(contactData, predicates);
    EXPECT_EQ(deleteCode, 0);

    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactIdOne = RawContactInsert("dongming", rawContactValues);
    EXPECT_GT(rawContactIdOne, 0);
    rawContactValues.Clear();

    int64_t rawContactIdTwo = RawContactInsert("xiaocai", rawContactValues);
    EXPECT_GT(rawContactIdTwo, 0);

    OHOS::NativeRdb::ValuesBucket contactValues;
    int64_t contactDataId = ContactDataInsert(rawContactIdOne, "email", "166@163.com", "", contactValues);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::ValuesBucket contactValuesTwo;
    contactDataId = ContactDataInsert(rawContactIdTwo, "email", "199@163.com", "", contactValuesTwo);
    EXPECT_GT(contactDataId, 0);

    std::vector<std::string> columns;
    columns.push_back("detail_info");
    predicates.Clear();
    // type_id 1 is   email
    predicates.EqualTo("type_id", "1");
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    std::vector<OHOS::NativeRdb::ValuesBucket> listValue;
    listValue.push_back(contactValues);
    listValue.push_back(contactValuesTwo);
    CheckResultSetList(listValue, resultSet, "contact_Query_test_4700");
    ClearContacts();
}

/*
 * @tc.number  contact_Query_test_4800
 * @tc.name    Query information about a single contact
 * @tc.desc    Ability to query data information of a single contact
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Query_test_4800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Query_test_4800 is starting! ---");
    std::string tag("contact_Query_test_4800");

    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("BossCai", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket contactValues;
    int64_t contactDataId = ContactDataInsert(rawContactId, "name", "BossCai", "", contactValues);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::ValuesBucket contactValuesTwo;
    contactDataId = ContactDataInsert(rawContactId, "organization", "happy500", "Test", contactValuesTwo);
    EXPECT_GT(contactDataId, 0);

    std::vector<std::string> columns;
    columns.push_back("detail_info");
    columns.push_back("position");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::string contactData = ContactTabName::CONTACT_DATA;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    std::vector<OHOS::NativeRdb::ValuesBucket> listValue;
    listValue.push_back(contactValues);
    listValue.push_back(contactValuesTwo);
    CheckResultSetList(listValue, resultSet, "contact_Query_test_4800");
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_4900
 * @tc.name    Query the deleted contact according to the original contact ID
 * @tc.desc    Ability to delete contact data from the basic raw data table and record basic raw data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_4900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_test_4900 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaoli", rawContactValues);
    HILOG_INFO("rawContactId= %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int deleteCode = ContactDelete(rawContacts, predicates);
    EXPECT_EQ(deleteCode, 0);

    sleep(SLEEP_TIME);
    std::vector<std::string> columns;
    columns.push_back("raw_contact_id");
    columns.push_back("display_name");
    std::string deletedRawContact = ContactTabName::DELETED_RAW_CONTACT;
    predicates.Clear();
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(deletedRawContact, columns, predicates);

    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    rawContactValues.PutInt("raw_contact_id", rawContactId);
    CheckResultSet(rawContactValues, resultSet, "contact_Delete_test_4900");
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_5000
 * @tc.name    Delete a single full field data in the raw_contact table
 * @tc.desc    Raw contact deletion capability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_5000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Delete_test_5000 is starting!-----");
    // insert
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    GetAllRawContactColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    int rawId = RawContactInsertValues(valuesBucket);
    EXPECT_GT(rawId, 0);

    // test end delete data
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawId));
    EXPECT_GT(rawId, 0);
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int deleteCode = ContactDelete(rawContacts, predicates);
    EXPECT_EQ(deleteCode, 0);
    std::vector<std::string> columns;
    columns.push_back("is_deleted");
    OHOS::NativeRdb::ValuesBucket valuesBucketDelete;
    valuesBucketDelete.PutInt("is_deleted", 1);
    QueryAndExpectResult(rawContacts, predicates, valuesBucketDelete, "contact_Delete_test_5000");
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_5100
 * @tc.name    Query the details of the newly deleted contact
 * @tc.desc    Ability to delete contact data from the detailed data table and record detailed data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_5100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_test_5100 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("siyuan", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket contactValue;
    int64_t contactDataId = ContactDataInsert(rawContactId, "organization", "tiantianxaingshang", "Test", contactValue);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int deleteCode = ContactDelete(rawContacts, predicates);
    EXPECT_EQ(deleteCode, 0);

    sleep(SLEEP_TIME);
    std::vector<std::string> columns;
    columns.push_back("raw_contact_id");
    columns.push_back("display_name");
    std::string deletedRawContact = ContactTabName::DELETED_RAW_CONTACT;
    predicates.Clear();
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(deletedRawContact, columns, predicates);

    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    rawContactValues.PutInt("raw_contact_id", rawContactId);
    CheckResultSet(rawContactValues, resultSet, "contact_Delete_test_5100");
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_5200
 * @tc.name    Delete a single full field data in the contact_data table
 * @tc.desc    Deleted ability to contact_data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_5200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_Delete_test_5200 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string tableName = ContactTabName::CONTACT_DATA;
    GetAllContactDataColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("feimaotui", rawContactValues);
    valuesBucket.PutInt("raw_contact_id", rawContactId);
    // type id 6 is name
    valuesBucket.PutInt("type_id", 6);
    int ContactDataId = ContactDataInsertValues(valuesBucket);
    EXPECT_GT(ContactDataId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    int deleteCode = ContactDelete(tableName, predicates);
    EXPECT_EQ(deleteCode, 0);

    MergeColumns(columns, columnsInt, columnsStr);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetDeleteQuery =
        ContactQuery(tableName, columns, predicates);
    // resultSet count 0
    int rowCount = -1;
    resultSetDeleteQuery->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_test_5300
 * @tc.name    Query deleted basic data
 * @tc.desc    Ability to delete and record basic data of a single contact
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_5300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_test_5300 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaohong", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("name_raw_contact_id", std::to_string(rawContactId));
    std::string contact = ContactTabName::CONTACT;
    int deleteCode = ContactDelete(contact, predicates);
    EXPECT_EQ(deleteCode, 0);

    sleep(SLEEP_TIME);
    std::vector<std::string> columns;
    columns.push_back("raw_contact_id");
    columns.push_back("display_name");
    predicates.Clear();
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::string deletedRawContact = ContactTabName::DELETED_RAW_CONTACT;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(deletedRawContact, columns, predicates);

    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    rawContactValues.PutInt("raw_contact_id", rawContactId);
    CheckResultSet(rawContactValues, resultSet, "contact_Delete_test_5300");
    ClearContacts();
}

/*
 * @tc.number  contact_BatchInsert_test_5400
 * @tc.name    Add basic contact information in batch and verify whether the insertion is successful
 * @tc.desc    Bulk increase capacity of address book
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_BatchInsert_test_5400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_BatchInsert_test_5400 is starting! ---");
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    OHOS::NativeRdb::ValuesBucket rawContactValuesOne;
    rawContactValuesOne.PutString("display_name", std::string("zhangming"));
    rawContactValuesOne.PutString("company", std::string("tiantainxiangzuo4200000000"));
    rawContactValuesOne.PutString("position", std::string("Test"));

    OHOS::NativeRdb::ValuesBucket rawContactValuesTwo;
    rawContactValuesTwo.PutString("display_name", std::string("ligang"));
    rawContactValuesTwo.PutString("company", std::string("tiantainxiangzuo4200000000"));
    rawContactValuesTwo.PutString("position", std::string("Developer"));

    OHOS::NativeRdb::ValuesBucket rawContactValuesThree;
    rawContactValuesThree.PutString("display_name", std::string("wanghong"));
    rawContactValuesThree.PutString("company", std::string("tiantainxiangzuo4200000000"));
    rawContactValuesThree.PutString("position", std::string("manage"));

    std::vector<OHOS::NativeRdb::ValuesBucket> listAddBluk;
    listAddBluk.push_back(rawContactValuesOne);
    listAddBluk.push_back(rawContactValuesTwo);
    listAddBluk.push_back(rawContactValuesThree);

    int batchInserCode = contactsDataAbility.BatchInsert(uriRawContact, listAddBluk);
    EXPECT_EQ(batchInserCode, 0);

    std::string rawContacts = ContactTabName::RAW_CONTACT;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("company", "tiantainxiangzuo4200000000");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    predicates.OrderByAsc("id");
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);
    int rowCount = 0;
    int queryCount = 3;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(queryCount, rowCount);
    CheckResultSetList(listAddBluk, resultSet, "contact_BatchInsert_test_5400");
    ClearContacts();
}

/*
 * @tc.number  contact_BatchInsert_test_5500
 * @tc.name    Batch add contact details and verify that the insertion was successful
 * @tc.desc    Bulk increase capacity of address book
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_BatchInsert_test_5500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_BatchInsert_test_5500 is starting! ---");

    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    rawContactValues.PutString("display_name", "zhangming");
    int64_t rawContactId = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    EXPECT_GT(rawContactId, 0);

    std::vector<OHOS::NativeRdb::ValuesBucket> listAddBluk = GetBatchList(rawContactId);
    int batchInserCode = contactsDataAbility.BatchInsert(uriContactData, listAddBluk);
    EXPECT_EQ(batchInserCode, 0);

    std::string contactData = ContactTabName::CONTACT_DATA;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    // type_id 1 is email
    predicates.EqualTo("type_id", "1");
    predicates.And();
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    int indexTwo = 2;
    CheckResultSet(listAddBluk[indexTwo], resultSet, "contact_BatchInsert_test_5500");
    predicates.Clear();
    // type 5 is phone
    predicates.EqualTo("type_id", "5");
    predicates.And();
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetFour = ContactQuery(contactData, columns, predicates);
    int rowCountFour = 0;
    resultSetFour->GetRowCount(rowCountFour);
    EXPECT_EQ(1, rowCountFour);
    int indexThree = 3;
    CheckResultSet(listAddBluk[indexThree], resultSetFour, "contact_BatchInsert_test_5500");
    ClearContacts();
}

std::vector<OHOS::NativeRdb::ValuesBucket> ContactAbilityTest::GetBatchList(int64_t rawContactId)
{
    OHOS::NativeRdb::ValuesBucket contactDataValuesOne;
    contactDataValuesOne.PutInt("raw_contact_id", rawContactId);
    contactDataValuesOne.PutString("content_type", "name");
    contactDataValuesOne.PutString("detail_info", "zhangming");
    OHOS::NativeRdb::ValuesBucket contactDataValuesTwo;
    contactDataValuesTwo.PutInt("raw_contact_id", rawContactId);
    contactDataValuesTwo.PutString("content_type", "organization");
    contactDataValuesTwo.PutString("detail_info", "tiantianxaingshang");
    contactDataValuesTwo.PutString("position", "Test");
    OHOS::NativeRdb::ValuesBucket contactDataValuesThree;
    contactDataValuesThree.PutInt("raw_contact_id", rawContactId);
    contactDataValuesThree.PutString("content_type", "email");
    contactDataValuesThree.PutString("detail_info", "199632@163.com");
    OHOS::NativeRdb::ValuesBucket contactDataValuesFour;
    contactDataValuesFour.PutInt("raw_contact_id", rawContactId);
    contactDataValuesFour.PutString("content_type", "phone");
    contactDataValuesFour.PutString("detail_info", "1234567898");
    std::vector<OHOS::NativeRdb::ValuesBucket> listAddBluk;
    listAddBluk.push_back(contactDataValuesOne);
    listAddBluk.push_back(contactDataValuesTwo);
    listAddBluk.push_back(contactDataValuesThree);
    listAddBluk.push_back(contactDataValuesFour);
    return listAddBluk;
}

/*
 * @tc.number  contact_Delete_test_5600
 * @tc.name    Delete contacts in batch and verify whether the deletion is successful
 * @tc.desc    delete ability to call records in batches
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_test_5600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_test_5600 is starting!---");
    int time = 10000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    int64_t rawContactIdOne = RawContactInsert("update_detail_contactdata", rawValuesBucket);
    EXPECT_GT(rawContactIdOne, 0);
    int64_t rawContactIdTwo = RawContactInsert("update_contactdata", rawValuesBucket);
    EXPECT_GT(rawContactIdTwo, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.GreaterThan("id", "0");
    std::shared_ptr<OHOS::NativeRdb::DataAbilityPredicates> executePredicates =
        std::make_shared<OHOS::NativeRdb::DataAbilityPredicates>(predicates);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(ContactsUri::RAW_CONTACT);
    std::shared_ptr<OHOS::AppExecFwk::DataAbilityOperation> operation =
        OHOS::AppExecFwk::DataAbilityOperation::NewDeleteBuilder(uri)
            ->WithPredicatesBackReference(0, 0)
            ->WithPredicates(executePredicates)
            ->WithInterruptionAllowed(true)
            ->Build();
    std::vector<std::shared_ptr<OHOS::AppExecFwk::DataAbilityOperation>> executeBatchOperations;
    executeBatchOperations.push_back(operation);
    InitAbility();
    std::vector<std::shared_ptr<OHOS::AppExecFwk::DataAbilityResult>> dataAbilityResult =
        contactsDataAbility.ExecuteBatch(executeBatchOperations);
    EXPECT_EQ(0, dataAbilityResult[0]->GetCount());
    std::string tableName = ContactTabName::CONTACT_DATA;
    std::vector<std::string> columnQuery;
    columnQuery.push_back("detail_info");
    predicates.EqualTo("is_deleted", "0");
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(tableName, columnQuery, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_5700
 * @tc.name    Batch favorite / cancel favorite contacts
 * @tc.desc    Batch collection and uncollection capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_5700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_5700 is starting! ---");

    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    OHOS::NativeRdb::ValuesBucket rawContactValuesOne;
    rawContactValuesOne.PutString("display_name", "zhangmingming");
    rawContactValuesOne.PutInt("favorite", 1);
    OHOS::NativeRdb::ValuesBucket rawContactValuesTwo;
    rawContactValuesTwo.PutString("display_name", "yuanmoumou");
    rawContactValuesTwo.PutInt("favorite", 1);
    OHOS::NativeRdb::ValuesBucket rawContactValuesThree;
    rawContactValuesThree.PutString("display_name", "xiaofenren");
    rawContactValuesThree.PutInt("favorite", 1);

    std::vector<OHOS::NativeRdb::ValuesBucket> listAddBluk;
    listAddBluk.push_back(rawContactValuesOne);
    listAddBluk.push_back(rawContactValuesTwo);
    listAddBluk.push_back(rawContactValuesThree);
    int batchInserCode = contactsDataAbility.BatchInsert(uriRawContact, listAddBluk);
    HILOG_INFO("contact_Update_test_5700 : batchInserCode = %{public}d", batchInserCode);
    EXPECT_EQ(batchInserCode, 0);
    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutInt("favorite", 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("display_name", "zhangmingming");
    predicates.Or();
    predicates.EqualTo("display_name", "yuanmoumou");
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    HILOG_INFO("contact_Update_test_5700: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);

    predicates.Clear();
    predicates.EqualTo("display_name", "zhangmingming");
    QueryAndExpectResult(rawContacts, predicates, updateValues, "contact_Update_test_5700");

    predicates.Clear();
    predicates.EqualTo("display_name", "xiaofenren");
    QueryAndExpectResult(rawContacts, predicates, rawContactValuesThree, "contact_Update_test_5700");
    ClearContacts();
}

/*
 * @tc.number  contact_Update_test_5800
 * @tc.name    Update the contact in batch and verify whether the modification is successful
 * @tc.desc    update ability to call records in batches
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Update_test_5800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Update_test_5800 is starting!---");
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    int64_t rawContactIdOne = RawContactInsert("update_detail_contactdata", rawValuesBucket);
    EXPECT_GT(rawContactIdOne, 0);
    int64_t rawContactIdTwo = RawContactInsert("update_contactdata", rawValuesBucket);
    EXPECT_GT(rawContactIdTwo, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactIdOne));
    predicates.Or();
    predicates.EqualTo("id", std::to_string(rawContactIdTwo));
    std::shared_ptr<OHOS::NativeRdb::ValuesBucket> values =
        std::make_shared<OHOS::NativeRdb::ValuesBucket>(rawValuesBucket);
    std::shared_ptr<OHOS::NativeRdb::DataAbilityPredicates> executePredicates =
        std::make_shared<OHOS::NativeRdb::DataAbilityPredicates>(predicates);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(ContactsUri::RAW_CONTACT);
    std::shared_ptr<OHOS::AppExecFwk::DataAbilityOperation> operation =
        OHOS::AppExecFwk::DataAbilityOperation::NewUpdateBuilder(uri)
            ->WithValuesBucket(values)
            ->WithPredicatesBackReference(0, 0)
            ->WithPredicates(executePredicates)
            ->WithInterruptionAllowed(true)
            ->Build();
    std::vector<std::shared_ptr<OHOS::AppExecFwk::DataAbilityOperation>> executeBatchOperations;
    executeBatchOperations.push_back(operation);
    InitAbility();
    std::vector<std::shared_ptr<OHOS::AppExecFwk::DataAbilityResult>> dataAbilityResult =
        contactsDataAbility.ExecuteBatch(executeBatchOperations);
    EXPECT_EQ(0, dataAbilityResult[0]->GetCount());
    std::string tableName = ContactTabName::RAW_CONTACT;
    std::vector<std::string> columnQuery;
    columnQuery.push_back("display_name");
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(tableName, columnQuery, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(2, rowCount);
    std::vector<OHOS::NativeRdb::ValuesBucket> listValue;
    listValue.push_back(rawValuesBucket);
    listValue.push_back(rawValuesBucket);
    CheckResultSetList(listValue, resultSet, "contact_Update_test_5800");
    ClearContacts();
}

/*
 * @tc.number  contact_BatchInsertAndDelete_test_5900
 * @tc.name    Batch add / delete contact blocklist
 * @tc.desc    Ability to add and remove contacts from the blocklist in batches
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_BatchInsertAndDelete_test_5900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_BatchInsertAndDelete_test_5900 is starting! ---");
    OHOS::Uri uriContactBlacklist(ContactsUri::BLOCKLIST);
    OHOS::NativeRdb::ValuesBucket contactBlacklistValuesOne;
    string phoneNumber = random_number_utils.Generating(9);
    contactBlacklistValuesOne.PutString("phone_number", phoneNumber);
    OHOS::NativeRdb::ValuesBucket contactBlacklistValuesTwo;
    contactBlacklistValuesTwo.PutString("phone_number", phoneNumber);
    OHOS::NativeRdb::ValuesBucket contactBlacklistValuesThree;
    contactBlacklistValuesThree.PutString("phone_number", phoneNumber);

    std::vector<OHOS::NativeRdb::ValuesBucket> listAddBluk;
    listAddBluk.push_back(contactBlacklistValuesOne);
    listAddBluk.push_back(contactBlacklistValuesTwo);
    listAddBluk.push_back(contactBlacklistValuesThree);
    int batchInsertCode = contactsDataAbility.BatchInsert(uriContactBlacklist, listAddBluk);
    EXPECT_EQ(batchInsertCode, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("phone_number", std::string(phoneNumber));
    predicates.Or();
    predicates.EqualTo("phone_number", std::string(phoneNumber));
    std::string contactBlocklist = ContactTabName::CONTACT_BLOCKLIST;
    int deleteCode = ContactDelete(contactBlocklist, predicates);
    EXPECT_EQ(deleteCode, 0);
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(contactBlocklist, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    ClearContacts();
}

/*
 * @tc.number  contact_BatchInsertAndDelete_test_6000
 * @tc.name    Batch add contacts to the group, and then delete contacts from the group
 * @tc.desc    Ability to join or remove contacts from groups in bulk
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_BatchInsertAndDelete_test_6000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_BatchInsertAndDelete_test_6000 is staring! ---");
    OHOS::NativeRdb::ValuesBucket valuesGroup;
    int64_t groupIdOne = GroupsInsert("TestFourth", valuesGroup);
    EXPECT_GT(groupIdOne, 0);

    valuesGroup.Clear();
    int64_t groupIdTwo = GroupsInsert("TestFifth", valuesGroup);
    EXPECT_GT(groupIdTwo, 0);

    valuesGroup.Clear();
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaocheng", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
    OHOS::NativeRdb::ValuesBucket contactDataValuesOne;
    contactDataValuesOne.PutInt("raw_contact_id", rawContactId);
    contactDataValuesOne.PutString("content_type", std::string("group_membership"));
    contactDataValuesOne.PutString("detail_info", std::to_string(groupIdOne));
    OHOS::NativeRdb::ValuesBucket contactDataValuesTwo;
    contactDataValuesTwo.PutInt("raw_contact_id", rawContactId);
    contactDataValuesTwo.PutString("content_type", std::string("group_membership"));
    contactDataValuesTwo.PutString("detail_info", std::to_string(groupIdTwo));

    std::vector<OHOS::NativeRdb::ValuesBucket> listAddBluk;
    listAddBluk.push_back(contactDataValuesOne);
    listAddBluk.push_back(contactDataValuesTwo);
    int batchInserCode = contactsDataAbility.BatchInsert(uriContactData, listAddBluk);
    EXPECT_EQ(batchInserCode, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    predicates.And();
    predicates.EqualTo("detail_info", std::to_string(groupIdOne));
    std::string contactData = ContactTabName::CONTACT_DATA;
    int deleteCode = ContactDelete(contactData, predicates);
    EXPECT_EQ(deleteCode, 0);

    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    ClearContacts();
}

/*
 * @tc.number  abnormal_contact_Insert_test_6100
 * @tc.name    When inserting a contact, pass in a non-existent field or table name
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_Insert_test_6100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_contact_Insert_test_6100 is starting! ---");

    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    OHOS::Uri errorUri(ContactsUri::ERROR_URI);

    OHOS::NativeRdb::ValuesBucket rawContactValues;
    rawContactValues.PutString("display_names", "liming");
    int64_t rawontactId = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    HILOG_INFO("abnormal_contact_Insert_test_6100 : rawontactId = %{public}lld", rawontactId);
    EXPECT_EQ(rawontactId, -1);

    rawContactValues.Clear();
    rawContactValues.PutString("display_name", "liming");
    rawontactId = contactsDataAbility.Insert(errorUri, rawContactValues);
    EXPECT_EQ(rawontactId, -1);
    ClearContacts();
}

/*
 * @tc.number  abnormal_contact_Insert_test_6200
 * @tc.name    Verification of required fields(type_id) in contact_data table
 * @tc.desc    Added ability to contact_data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_Insert_test_6200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----abnormal_contact_Insert_test_6200 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string contactData = ContactTabName::CONTACT_DATA;
    GetAllContactDataColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    int64_t rawContactId = RawContactInsert("lock_type_id", rawValuesBucket);
    valuesBucket.PutInt("raw_contact_id", rawContactId);
    int ContactDataId = ContactDataInsertValues(valuesBucket);
    EXPECT_EQ(ContactDataId, -1);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    // resultSet count 0
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    CheckResultSet(valuesBucket, resultSet, "abnormal_contact_Insert_test_6200");
    ClearContacts();
}

/*
 * @tc.number  abnormal_contact_Insert_test_6300
 * @tc.name    Verification of required fields(raw_contact_id) in contact_data table
 * @tc.desc    Added ability to contact_data
 * @tc.level   Level1
 * @tc.size    MediumTest
 *
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_Insert_test_6300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----abnormal_contact_Insert_test_6300 is starting!-----");
    std::vector<std::string> columnsInt;
    std::vector<std::string> columnsStr;
    std::vector<std::string> columns;
    std::string contactData = ContactTabName::CONTACT_DATA;
    GetAllContactDataColumns(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket valuesBucket = GetAllColumnsValues(columnsInt, columnsStr);
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    int64_t rawContactId = RawContactInsert("lock_raw_contact_id", rawValuesBucket);
    valuesBucket.PutInt("type_id", 6);
    int ContactDataId = ContactDataInsertValues(valuesBucket);
    EXPECT_EQ(ContactDataId, -1);

    MergeColumns(columns, columnsInt, columnsStr);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    // resultSet count 0
    int rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    CheckResultSet(valuesBucket, resultSet, "abnormal_contact_Insert_test_6300");
    ClearContacts();
}

/*
 * @tc.number  abnormal_contact_Update_test_6400
 * @tc.name    When modifying, an incorrect field or table name or non-existent value is passed in
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_Update_test_6400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_contact_Update_test_6400 is starting! ---");
    OHOS::Uri errorUri(ContactsUri::ERROR_URI);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("zhangming", rawContactValues);
    HILOG_INFO("abnormal_contact_Update_test_6400 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("display_names", "dongming");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    HILOG_INFO("abnormal_contact_Update_test_6400: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, -1);
    QueryAndExpectResult(rawContacts, predicates, rawContactValues, "abnormal_contact_Update_test_6400");

    updateValues.Clear();
    updateValues.PutString("display_name", "dongming");
    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactId));
    updateCode = contactsDataAbility.Update(errorUri, updateValues, predicates);
    HILOG_INFO("abnormal_contact_Update_test_6400: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, -1);
    QueryAndExpectResult(rawContacts, predicates, rawContactValues, "abnormal_contact_Update_test_6400");

    predicates.Clear();
    predicates.EqualTo("id", "100000");
    updateCode = ContactUpdate(rawContacts, updateValues, predicates);
    HILOG_INFO("abnormal_contact_Update_test_6400: updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);
    ClearContacts();
}

/*
 * @tc.number  abnormal_contact_Query_test_6500
 * @tc.name    A wrong field was passed in when querying
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_Query_test_6500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_contact_Query_test_6500 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("dongming", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    std::vector<std::string> columns;
    columns.push_back("display_names");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(rawContacts, columns, predicates);

    int rowCount = -2;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(-1, rowCount);
    ClearContacts();
}

/*
 * @tc.number  abnormal_contact_Query_test_6600
 * @tc.name    Pass in a non-existent table name when querying
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_Query_test_6600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_contact_Query_test_6600 is starting! ---");
    std::string tag("abnormal_contact_Query_test_6600");

    OHOS::Uri errorUri(ContactsUri::ERROR_URI);

    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("display_name");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.GreaterThan("id", "0");
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        contactsDataAbility.Query(errorUri, columns, predicates);
    EXPECT_EQ(resultSet, nullptr);
    ClearContacts();
}

/*
 * @tc.number  abnormal_contact_Delete_test_6700
 * @tc.name    When deleting, pass in an incorrect field or table name or a non-existent value
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_Delete_test_6700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-------abnormal_contact_Delete_test_6700 is starting!-------");
    OHOS::Uri errorUri(ContactsUri::ERROR_URI);
    OHOS::NativeRdb::ValuesBucket values;
    int64_t rawContactId = RawContactInsert("zhangming", values);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("ids", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    int deleteCode = ContactDelete(rawContacts, predicates);
    EXPECT_EQ(deleteCode, -1);

    OHOS::NativeRdb::DataAbilityPredicates predicatesQuery;
    predicatesQuery.EqualTo("id", std::to_string(rawContactId));
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(rawContacts, columns, predicatesQuery);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    resultSet->Close();

    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactId));
    deleteCode = contactsDataAbility.Delete(errorUri, predicates);
    EXPECT_EQ(deleteCode, -1);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetUri =
        ContactQuery(rawContacts, columns, predicatesQuery);
    int rowCountUri = 0;
    resultSetUri->GetRowCount(rowCountUri);
    EXPECT_EQ(1, rowCountUri);
    resultSetUri->Close();

    predicates.Clear();
    predicates.EqualTo("id", "10000000");
    deleteCode = ContactDelete(rawContacts, predicates);
    EXPECT_EQ(deleteCode, -1);

    predicates.Clear();
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetId =
        ContactQuery(rawContacts, columns, predicatesQuery);
    int rowCountId = 0;
    resultSetId->GetRowCount(rowCountId);
    EXPECT_EQ(1, rowCountId);
    resultSetId->Close();
    ClearContacts();
}

/*
 * @tc.number  abnormal_contact_BatchInsert_test_6800
 * @tc.name    When adding contacts in batches, some of them failed, check the processing logic
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_BatchInsert_test_6800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_contact_BatchInsert_test_6800 is starting! ---");
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    std::vector<OHOS::NativeRdb::ValuesBucket> listAddBluk;
    int batchInserCode = 0;
    for (int i = 0; i < 10; i++) {
        listAddBluk.clear();
        for (int j = 10 * i + 1; j <= 10 * (i + 1); j++) {
            rawContactValues.Clear();
            std::string name("zhangming");
            name.append(std::to_string(j));
            if (j == 14 || j == 27 || j == 57) {
                rawContactValues.PutString("display_names", name);
            } else {
                rawContactValues.PutString("display_name", name);
            }
            listAddBluk.push_back(rawContactValues);
        }
        batchInserCode = contactsDataAbility.BatchInsert(uriRawContact, listAddBluk);
        HILOG_INFO("abnormal_contact_BatchInsert_test_6800 : batchInserCode = %{public}d", batchInserCode);
        if (batchInserCode == 0) {
            HILOG_INFO("abnormal_contact_BatchInsert_test_6800 batch insert success!");
        } else {
            HILOG_INFO("abnormal_contact_BatchInsert_test_6800 batch insert fail!");
            EXPECT_EQ(batchInserCode, -1);
        }
    }
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.NotEqualTo("id", "0");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    contactsDataAbility.Delete(uriRawContact, predicates);
    int time = 20000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    predicates.Clear();
    OHOS::Uri uriRawContactComplete(ContactsUri::DELETED_RAW_CONTACT);
    predicates.NotEqualTo("id", "0");
    contactsDataAbility.Delete(uriRawContactComplete, predicates);
}

/*
 * @tc.number  abnormal_contact_BatchInsert_test_6900
 * @tc.name    When adding contacts in batch, item 500 fails. Check the subsequent processing logic
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, abnormal_contact_BatchInsert_test_6900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- abnormal_contact_BatchInsert_test_6900 is starting! ---");
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    std::vector<OHOS::NativeRdb::ValuesBucket> listAddBluk;
    for (int i = 0; i < 1000; i++) {
        rawContactValues.Clear();
        std::string name("xiaoyuan");
        name.append(std::to_string(i));
        if (i == 500) {
            rawContactValues.PutString("display_names", name);
        } else {
            rawContactValues.PutString("display_name", name);
        }
        listAddBluk.push_back(rawContactValues);
    }
    int batchInserCode = contactsDataAbility.BatchInsert(uriRawContact, listAddBluk);
    HILOG_INFO("abnormal_contact_BatchInsert_test_6900 : batchInserCode = %{public}d", batchInserCode);
    if (batchInserCode == 0) {
        HILOG_INFO("abnormal_contact_BatchInsert_test_6900 batch insert success!");
    } else {
        HILOG_INFO("abnormal_contact_BatchInsert_test_6900 batch insert fail!");
        EXPECT_EQ(batchInserCode, -1);
    }
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.NotEqualTo("id", "0");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    contactsDataAbility.Delete(uriRawContact, predicates);
    int time = 20000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    predicates.Clear();
    OHOS::Uri uriRawContactComplete(ContactsUri::DELETED_RAW_CONTACT);
    predicates.NotEqualTo("id", "0");
    contactsDataAbility.Delete(uriRawContactComplete, predicates);
}

/*
 * @tc.number  contact_async_insert_test_7000
 * @tc.name    Add contacts async
 * @tc.desc    Ability to join or add contacts
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_async_insert_test_7000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_async_insert_test_7000 is staring! ---");
    std::map<int, OHOS::NativeRdb::ValuesBucket> result;
    std::vector<ContactAsync *> contactAsyncVector;
    int threadNum = 6;
    for (int i = 0; i < threadNum; ++i) {
        OHOS::NativeRdb::ValuesBucket values;
        std::string name;
        name.append("asyncTest");
        name.append(std::to_string(i));
        values.PutString("display_name", name);
        ContactAsync *contactAsync = new ContactAsync(values, result);
        std::thread asyncThread(&ContactAsync::Insert, contactAsync);
        contactAsyncVector.push_back(contactAsync);
        asyncThread.detach();
    }
    std::chrono::milliseconds dura(Time::ASYNC_SLEEP_TIME);
    std::this_thread::sleep_for(dura);
    std::vector<std::string> columns;
    columns.push_back("display_name");
    std::map<int, OHOS::NativeRdb::ValuesBucket>::iterator it;
    for (it = result.begin(); it != result.end(); it++) {
        OHOS::NativeRdb::DataAbilityPredicates predicates;
        predicates.EqualTo("id", std::to_string(it->first));
        std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
            ContactQuery(ContactTabName::RAW_CONTACT, columns, predicates);
        CheckResultSet(it->second, resultSet, "contact_async_insert_test_7000");
    }
    for (int i = 0; i < threadNum; ++i) {
        delete contactAsyncVector[i];
    }
    ClearContacts();
}

/*
 * @tc.number  contact_async_update_test_7100
 * @tc.name    update contacts async
 * @tc.desc    Ability to join or update contacts
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_async_update_test_7100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_async_update_test_7100 is staring! ---");
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    std::vector<int64_t> rawContactId;
    rawContactId.push_back(RawContactInsert("async_update1", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update2", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update3", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update4", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update5", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update6", rawValuesBucket));
    rawValuesBucket.Clear();
    std::vector<OHOS::NativeRdb::ValuesBucket> upDateValues;
    OHOS::NativeRdb::DataAbilityPredicates queryPredicates;
    std::vector<ContactAsync *> contactAsyncVector;
    int size = rawContactId.size();
    for (int i = 0; i < size; ++i) {
        OHOS::NativeRdb::ValuesBucket values;
        std::string name;
        name.append("asyncTest");
        values.PutString("display_name", name);
        upDateValues.push_back(values);
        int id = rawContactId[i];
        ContactAsync *contactAsync = new ContactAsync(values, id);
        contactAsyncVector.push_back(contactAsync);
        std::thread asyncThread(&ContactAsync::Update, contactAsync);
        asyncThread.detach();
        queryPredicates.EqualTo("id", std::to_string(rawContactId[i]));
        if (i < size - 1) {
            queryPredicates.Or();
        }
    }
    std::chrono::milliseconds dura(Time::ASYNC_SLEEP_TIME);
    std::this_thread::sleep_for(dura);
    std::vector<std::string> columns;
    columns.push_back("display_name");
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(ContactTabName::RAW_CONTACT, columns, queryPredicates);
    CheckResultSetList(upDateValues, resultSet, "contact_async_update_test_7100");
    for (int i = 0; i < size; ++i) {
        delete contactAsyncVector[i];
    }
    ClearContacts();
}

/*
 * @tc.number  contact_async_query_test_7200
 * @tc.name    query contacts async
 * @tc.desc    Ability to join or query
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_async_query_test_7200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_async_query_test_7200 is staring! ---");
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    std::vector<int64_t> rawContactId;
    rawContactId.push_back(RawContactInsert("async_query1", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_query2", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_query3", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_query4", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_query5", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_query6", rawValuesBucket));
    rawValuesBucket.Clear();
    std::vector<ContactAsync *> contactAsyncVector;
    std::vector<std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet>> resultSetVector;
    int threadNum = 6;
    for (int i = 0; i < threadNum; ++i) {
        ContactAsync *contactAsync = new ContactAsync(resultSetVector, rawContactId);
        contactAsyncVector.push_back(contactAsync);
        std::thread asyncThread(&ContactAsync::Query, contactAsync);
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
        delete contactAsyncVector[i];
    }
    ClearContacts();
}

/*
 * @tc.number  contact_async_delete_test_7300
 * @tc.name    delete contacts async
 * @tc.desc    Ability to join or delete contacts
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_async_delete_test_7300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_async_delete_test_7300 is staring! ---");
    OHOS::NativeRdb::ValuesBucket rawValuesBucket;
    std::vector<int64_t> rawContactId;
    rawContactId.push_back(RawContactInsert("async_update1", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update2", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update3", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update4", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update5", rawValuesBucket));
    rawValuesBucket.Clear();
    rawContactId.push_back(RawContactInsert("async_update6", rawValuesBucket));
    rawValuesBucket.Clear();
    int size = rawContactId.size();
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.BeginWrap();
    std::vector<ContactAsync *> contactAsyncVector;
    for (int i = 0; i < size; ++i) {
        int id = rawContactId[i];
        ContactAsync *contactAsync = new ContactAsync(id);
        contactAsyncVector.push_back(contactAsync);
        std::thread asyncThread(&ContactAsync::Delete, contactAsync);
        asyncThread.detach();
        predicates.EqualTo("id", std::to_string(id));
        if (i < size - 1) {
            predicates.Or();
        }
    }
    predicates.EndWrap();
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    std::chrono::milliseconds dura(Time::ASYNC_SLEEP_TIME);
    std::this_thread::sleep_for(dura);
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(ContactTabName::RAW_CONTACT, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(0, rowCount);
    resultSet->Close();
    for (int i = 0; i < size; ++i) {
        delete contactAsyncVector[i];
    }
    ClearContacts();
}

/*
 * @tc.number  contact_Insert_test_7400
 * @tc.name    Add the basic information of a single contact and verify text whether the insertion is successful
 * @tc.desc    New capabilities for basic contact data
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Insert_test_7400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Insert_test_7400 is starting! ---");
    OHOS::NativeRdb::ValuesBucket values;
    int size = 20000;
    std::string name;
    for (int i = 0; i < size; ++i) {
        name.append("l");
    }
    int64_t rawContactId = RawContactInsert(name, values);
    HILOG_INFO("contact_Insert_test_7400 : rawContactId = %{public}lld", rawContactId);
    EXPECT_GT(rawContactId, 0);
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("id", std::to_string(rawContactId));
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    QueryAndExpectResult(rawContacts, predicates, values, "contact_Insert_test_7400");
    ClearContacts();
}

/*
 * @tc.number  contact_Delete_Recover_test_7500
 * @tc.name    recover deleted basic data
 * @tc.desc    Ability to recover  record basic data of a single contact
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactAbilityTest, contact_Delete_Recover_test_7500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- contact_Delete_Recover_test_7500 is starting! ---");
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("xiaohongDelete", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("name_raw_contact_id", std::to_string(rawContactId));
    std::string contact = ContactTabName::CONTACT;
    int deleteCode = ContactDelete(contact, predicates);
    EXPECT_EQ(deleteCode, 0);
    int time = 2000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);

    OHOS::NativeRdb::ValuesBucket updateValues;
    updateValues.PutString("is_deleted", "0");
    std::string rawContacts = ContactTabName::RAW_CONTACT;
    OHOS::NativeRdb::DataAbilityPredicates predicatesUpdate;
    predicatesUpdate.EqualTo("id", std::to_string(rawContactId));
    int updateCode = ContactUpdate(rawContacts, updateValues, predicatesUpdate);
    HILOG_INFO("contact_Delete_Recover_test_7500 : updateCode = %{public}d", updateCode);
    EXPECT_EQ(updateCode, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicatesDelete;
    predicatesDelete.EqualTo("raw_contact_id", std::to_string(rawContactId));
    int deleteCodeOne = ContactDelete(ContactTabName::DELETED_RAW_CONTACT_RECORD, predicatesDelete);
    EXPECT_EQ(deleteCodeOne, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicatesQuery;
    predicatesQuery.EqualTo("id", std::to_string(rawContactId));
    predicatesQuery.And();
    predicatesQuery.EqualTo("is_deleted", "0");
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        ContactQuery(ContactTabName::RAW_CONTACT, columns, predicatesQuery);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(1, rowCount);
    resultSet->Close();
    ClearContacts();
}
} // namespace Test
} // namespace Contacts