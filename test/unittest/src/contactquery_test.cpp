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

#include "contactquery_test.h"

#include "construction_name.h"
#include "test_common.h"

namespace Contacts {
namespace Test {
ContactQueryTest::ContactQueryTest()
{
}

ContactQueryTest::~ContactQueryTest()
{
}

int64_t ContactQueryTest::RawContactInsert(std::string displayName,
    OHOS::DataShare::DataShareValuesBucket &rawContactValues)
{
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    rawContactValues.Put("display_name", displayName);
    int64_t code = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    return code;
}

int64_t ContactQueryTest::ContactDataInsert(int64_t rawContactId, std::string contentType, std::string detailInfo,
    std::string position, OHOS::DataShare::DataShareValuesBucket &contactDataValues)
{
    OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
    contactDataValues.Put("raw_contact_id", rawContactId);
    contactDataValues.Put("content_type", contentType);
    contactDataValues.Put("detail_info", detailInfo);
    contactDataValues.Put("position", position);
    int64_t code = contactsDataAbility.Insert(uriContactData, contactDataValues);
    return code;
}

std::shared_ptr<OHOS::DataShare::DataShareResultSet> ContactQueryTest::ContactQuery(
    const std::string &tableName, std::vector<std::string> columns, OHOS::DataShare::DataSharePredicates predicates)
{
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet;
    if (tableName == ContactTabName::RAW_CONTACT) {
        OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
        resultSet = contactsDataAbility.Query(uriRawContact, predicates, columns);
    } else if (tableName == ContactTabName::CONTACT_DATA) {
        OHOS::Uri uriContactData(ContactsUri::CONTACT_DATA);
        resultSet = contactsDataAbility.Query(uriContactData, predicates, columns);
    } else if (tableName == ContactTabName::CONTACT) {
        OHOS::Uri uriContact(ContactsUri::CONTACT);
        resultSet = contactsDataAbility.Query(uriContact, predicates, columns);
    } else if (tableName == ContactTabName::GROUPS) {
        OHOS::Uri uriGroups(ContactsUri::GROUPS);
        resultSet = contactsDataAbility.Query(uriGroups, predicates, columns);
    } else if (tableName == ContactTabName::CONTACT_BLOCKLIST) {
        OHOS::Uri uriBlocklist(ContactsUri::BLOCKLIST);
        resultSet = contactsDataAbility.Query(uriBlocklist, predicates, columns);
    } else if (tableName == ContactTabName::DELETED_RAW_CONTACT) {
        OHOS::Uri uriDeletedRawContact(ContactsUri::RAW_CONTACT);
        resultSet = contactsDataAbility.Query(uriDeletedRawContact, predicates, columns);
    } else if (tableName == ContactTabName::SEARCH_CONTACT) {
        OHOS::Uri uriSearchContact(ContactsUri::SEARCH);
        resultSet = contactsDataAbility.Query(uriSearchContact, predicates, columns);
    } else {
        HILOG_ERROR("ContactsDataAbility ====>no match uri action");
    }
    return resultSet;
}

void ContactQueryTest::ClearData()
{
    OHOS::DataShare::DataSharePredicates predicates;
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    predicates.NotEqualTo("id", "0");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    contactsDataAbility.Delete(uriRawContact, predicates);
    int time = 2000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    OHOS::DataShare::DataSharePredicates predicates2;
    OHOS::Uri uriRawContactComplete(ContactsUri::DELETED_RAW_CONTACT);
    predicates2.NotEqualTo("id", "0");
    contactsDataAbility.Delete(uriRawContactComplete, predicates2);
}

/*
* @tc.number  contact_data_Insert_test_100
* @tc.name    Insert contact details, including
                (name, company, position, mobile phone number, nickname, email, home, remarks, aim)
* @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
              email address, residence, remarks, AIM fuzzy search capabilities
* @tc.level   Level1
* @tc.size    MediumTest
* @tc.type    Function
*/
HWTEST_F(ContactQueryTest, contact_data_Insert_test_100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----contact_data_Insert_test_100-----");
    OHOS::Contacts::ConstructionName::local = "zh-CN";
    OHOS::DataShare::DataShareValuesBucket values;
    OHOS::DataShare::DataShareValuesBucket valuesOne;
    OHOS::DataShare::DataShareValuesBucket valuesTwo;
    OHOS::DataShare::DataShareValuesBucket valuesThree;
    OHOS::DataShare::DataShareValuesBucket valuesFour;
    OHOS::DataShare::DataShareValuesBucket valuesFive;
    OHOS::DataShare::DataShareValuesBucket valuesSix;
    OHOS::DataShare::DataShareValuesBucket valuesSeven;
    OHOS::DataShare::DataShareValuesBucket valuesEight;
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    int64_t rawContactId = RawContactInsert("买键盘", values);
    EXPECT_GT(rawContactId, 0);

    int64_t contactDataIdOne = ContactDataInsert(rawContactId, "name", "买键盘", "", valuesOne);
    EXPECT_GT(contactDataIdOne, 0);

    int64_t contactDataIdTwo =
        ContactDataInsert(rawContactId, "organization", "你好天天向上hsk", "安全测试", valuesTwo);
    EXPECT_GT(contactDataIdTwo, 0);

    int64_t contactDataIdThree = ContactDataInsert(rawContactId, "phone", "18552504058", "", valuesThree);
    EXPECT_GT(contactDataIdThree, 0);

    int64_t contactDataIdFour = ContactDataInsert(rawContactId, "nickname", "买键盘", "", valuesFour);
    EXPECT_GT(contactDataIdFour, 0);

    int64_t contactDataIdFive = ContactDataInsert(rawContactId, "email", "1667894561@163.com", "", valuesFive);
    EXPECT_GT(contactDataIdFive, 0);

    int64_t contactDataIdSix = ContactDataInsert(rawContactId, "postal_address", "南京市", "", valuesSix);
    EXPECT_GT(contactDataIdSix, 0);

    int64_t contactDataIdSeven = ContactDataInsert(rawContactId, "note", "LanguagemjavaC++", "", valuesSeven);
    EXPECT_GT(contactDataIdSeven, 0);

    int64_t contactDataIdEight = ContactDataInsert(rawContactId, "im", "14487956@1.com", "", valuesEight);
    EXPECT_GT(contactDataIdEight, 0);
    ClearData();
}

/*
 * @tc.number  query_by_name_test_200
 * @tc.name    Fuzzy search by name
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_name_test_200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_name_test_200 is starting!-----");
    std::string tag("query_by_name_test_200");
    std::vector<std::string> columns;
    columns.push_back("display_name");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("display_name", "%买%");
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(searchContact, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("display_name", "买键盘");
    CheckResultSet(values, resultSet, "query_by_name_test_200");
    ClearData();
}

/*
 * @tc.number  query_by_pinyin_test_300
 * @tc.name    Fuzzy search based on name Pinyin
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_pinyin_test_300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_pinyin_test_300 is starting!-----");
    std::string tag("query_by_pinyin_test_300");
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("search_name");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("search_name", "%mai%");
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(searchContact, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("search_name", "买键盘||maijianpan||mjp");
    CheckResultSet(values, resultSet, "query_by_pinyin_test_300");
    ClearData();
}

/*
 * @tc.number  query_by_pinyin_test_400
 * @tc.name    Fuzzy search based on Pinyin name
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_pinyin_test_400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_pinyin_test_400 is starting!-----");
    std::string tag("query_by_pinyin_test_400");
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("search_name");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("search_name", "%mjp%");
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(searchContact, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("search_name", "买键盘||maijianpan||mjp");
    CheckResultSet(values, resultSet, "query_by_pinyin_test_400");
    ClearData();
}

/*
 * @tc.number  query_by_company_test_500
 * @tc.name    Fuzzy search by company
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_company_test_500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_company_test_500 is starting!-----");
    std::string tag("query_by_company_test_500");
    std::vector<std::string> columns;
    columns.push_back("detail_info");
    columns.push_back("position");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("detail_info", "%天天向上%");
    predicates.And();
    predicates.EqualTo("type_id", "4");
    std::string contactData = "contact_data";
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "你好天天向上hsk");
    values.Put("position", "安全测试");
    CheckResultSet(values, resultSet, "query_by_company_test_500");
    ClearData();
}

/*
 * @tc.number  query_by_position_test_600
 * @tc.name    Fuzzy search by position
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_position_test_600, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_position_test_600 is starting!-----");
    std::string tag("query_by_position_test_600");
    std::vector<std::string> columns;
    columns.push_back("detail_info");
    columns.push_back("position");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("position", "%测试%");
    predicates.And();
    predicates.EqualTo("type_id", "4");
    std::string contactData = "contact_data";
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "你好天天向上hsk");
    values.Put("position", "安全测试");
    CheckResultSet(values, resultSet, "query_by_position_test_600");
    ClearData();
}

/*
 * @tc.number  query_by_phone_test_700
 * @tc.name    Fuzzy search according to mobile phone number
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_phone_test_700, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_phone_test_700 is starting!-----");
    std::string tag("query_by_phone_test_700");
    std::vector<std::string> columns;
    columns.push_back("detail_info");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("detail_info", "1855250%");
    predicates.And();
    predicates.EqualTo("type_id", "5");
    std::string contactData = "contact_data";
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "18552504058");
    CheckResultSet(values, resultSet, "query_by_phone_test_700");
    ClearData();
}

/*
 * @tc.number  query_by_nick_name_test_800
 * @tc.name    Fuzzy search by nickname
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_nick_name_test_800, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_nick_name_test_800 is starting!-----");
    std::string tag("query_by_nick_name_test_800");
    std::vector<std::string> columns;
    columns.push_back("detail_info");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("detail_info", "%买%");
    predicates.And();
    predicates.EqualTo("type_id", "3");
    std::string contactData = "contact_data";
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "买键盘");
    CheckResultSet(values, resultSet, "query_by_nick_name_test_800");
    ClearData();
}

/*
 * @tc.number  query_by_email_test_900
 * @tc.name    Fuzzy search by mailbox
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_email_test_900, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_email_test_900 is starting!-----");
    std::string tag("query_by_email_test_900");
    std::vector<std::string> columns;
    columns.push_back("detail_info");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("detail_info", "166%");
    predicates.And();
    predicates.EqualTo("type_id", "1");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    std::string contactData = "contact_data";
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "1667894561@163.com");
    CheckResultSet(values, resultSet, "query_by_email_test_900");
    ClearData();
}

/*
 * @tc.number  query_by_address_test_1000
 * @tc.name    Fuzzy search based on house
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_address_test_1000, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_address_test_1000 is starting!-----");
    std::string tag("query_by_address_test_1000");
    std::vector<std::string> columns;
    columns.push_back("detail_info");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("detail_info", "%南京%");
    predicates.And();
    predicates.EqualTo("type_id", "7");
    std::string contactData = "contact_data";
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "南京市");
    CheckResultSet(values, resultSet, "query_by_address_test_1000");
    ClearData();
}

/*
 * @tc.number  query_by_note_test_1100
 * @tc.name    Fuzzy search based on comments
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_note_test_1100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_note_test_1100 is starting!-----");
    std::string tag("query_by_note_test_1100");
    std::vector<std::string> columns;
    columns.push_back("detail_info");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("detail_info", "%mjava%");
    predicates.And();
    predicates.EqualTo("type_id", "10");
    std::string contactData = "contact_data";
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "LanguagemjavaC++");
    CheckResultSet(values, resultSet, "query_by_note_test_1100");
    ClearData();
}

/*
 * @tc.number  query_by_aim_test_1200
 * @tc.name    Fuzzy search according to aim
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, query_by_aim_test_1200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----query_by_aim_test_1200 is starting!-----");
    std::string tag("query_by_aim_test_1200");
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("raw_contact_id");
    columns.push_back("detail_info");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("detail_info", "144%");
    predicates.And();
    predicates.EqualTo("type_id", "2");
    std::string contactData = "contact_data";
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(contactData, columns, predicates);

    OHOS::DataShare::DataShareValuesBucket values;
    values.Put("detail_info", "14487956@1.com");
    CheckResultSet(values, resultSet, "query_by_aim_test_1200");
    ClearData();
}

/*
 * @tc.number  abnormal_contact_Query_test_1300
 * @tc.name    An incorrect field name was passed in during retrieval
 * @tc.desc    Based on name, name pinyin, pinyin name, company, position, mobile phone number, nickname,
               email address, residence, remarks, AIM fuzzy search capabilities
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactQueryTest, abnormal_contact_Query_test_1300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-----abnormal_contact_Query_test_1300 is starting!-----");
    std::string tag("abnormal_contact_Query_test_1300");
    std::vector<std::string> columns;
    columns.push_back("id");
    columns.push_back("search_name");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.Like("search_namesss", "%mjp%");
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet = ContactQuery(searchContact, columns, predicates);
    // resultSet count 1
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(-1, rowCount);
    ClearData();
}
} // namespace Test
} // namespace Contacts