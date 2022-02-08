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

#include "contactpinyin_test.h"

#include "construction_name.h"
#include "test_common.h"

namespace Contacts {
namespace Test {
ContactPinyinTest::ContactPinyinTest()
{
}

ContactPinyinTest::~ContactPinyinTest()
{
}

int64_t ContactPinyinTest::RawContactInsert(std::string displayName, OHOS::NativeRdb::ValuesBucket &rawContactValues)
{
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    rawContactValues.PutString("display_name", displayName);
    int64_t code = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    return code;
}

int64_t ContactPinyinTest::ContactDataInsert(int64_t rawContactId, std::string contentType, std::string detailInfo,
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

std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactPinyinTest::ContactQuery(
    const std::string &tableName, std::vector<std::string> columns, OHOS::NativeRdb::DataAbilityPredicates predicates)
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

void ContactPinyinTest::QueryAndExpectResult(std::string &tableName, OHOS::NativeRdb::DataAbilityPredicates predicates,
    OHOS::NativeRdb::ValuesBucket &values, std::string testName)
{
    std::vector<std::string> columns;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = ContactQuery(tableName, columns, predicates);
    CheckResultSet(values, resultSet, testName);
}

void ContactPinyinTest::ClearData()
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
 * @tc.number  name_convert_to_pinyin_test_100
 * @tc.name    Insert a simplified Chinese name to view the converted Pinyin
 * @tc.desc    Contacts name conversion pinyin ability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactPinyinTest, pinyin_conversion_Insert_test_100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-------pinyin_conversion_Insert_test_100 is starting!------");
    std::string tag("pinyin_conversion_Insert_test_100");
    OHOS::Contacts::ConstructionName::local = "zh-CN";
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("李想", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket values;
    int64_t contactDataId = ContactDataInsert(rawContactId, "name", "李想", "", values);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    QueryAndExpectResult(searchContact, predicates, values, "pinyin_conversion_Insert_test_100");
    ClearData();
}

/*
 * @tc.number  pinyin_conversion_Insert_test_200
 * @tc.name    Insert a traditional Chinese name and view the converted Pinyin
 * @tc.desc    Contacts name conversion pinyin ability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactPinyinTest, pinyin_conversion_Insert_test_200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-------pinyin_conversion_Insert_test_200 is starting!------");
    std::string tag("pinyin_conversion_Insert_test_200");
    OHOS::Contacts::ConstructionName::local = "zh-CN";
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("張三", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket values;
    int64_t contactDataId = ContactDataInsert(rawContactId, "name", "張三", "", values);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    QueryAndExpectResult(searchContact, predicates, values, "pinyin_conversion_Insert_test_200");
    ClearData();
}

/*
 * @tc.number  pinyin_conversion_Insert_test_300
 * @tc.name    Insert an English name to view the converted Pinyin
 * @tc.desc    Contacts name conversion pinyin ability
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactPinyinTest, pinyin_conversion_Insert_test_300, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-------pinyin_conversion_Insert_test_300 is starting!------");
    OHOS::Contacts::ConstructionName::local = "other";
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("Tom", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket values;
    int64_t contactDataId = ContactDataInsert(rawContactId, "name", "Tom", "", values);
    HILOG_INFO("pinyin_conversion_Insert_test_300 : contactDataId = %{public}lld", contactDataId);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    QueryAndExpectResult(searchContact, predicates, values, "Tom||Tom||Tom");
    ClearData();
}

/*
 * @tc.number  abnormal_pinyin_conversion_Insert_test_400
 * @tc.name    Insert a name with special characters to view the converted Pinyin
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactPinyinTest, abnormal_pinyin_conversion_Insert_test_400, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-------abnormal_pinyin_conversion_Insert_test_400 is starting!------");
    OHOS::Contacts::ConstructionName::local = "zh-CN";
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("李%^玉@成", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket values;
    int64_t contactDataId = ContactDataInsert(rawContactId, "name", "李%^玉@成", "", values);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    QueryAndExpectResult(searchContact, predicates, values, "李%^玉@成||li%^yu@cheng||l%^y@c");
    ClearData();
}

/*
 * @tc.number  abnormal_pinyin_conversion_Insert_test_500
 * @tc.name    Insert a name in multiple languages and view the converted Pinyin
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(ContactPinyinTest, abnormal_pinyin_conversion_Insert_test_500, testing::ext::TestSize.Level1)
{
    HILOG_INFO("-------abnormal_pinyin_conversion_Insert_test_500 is starting!------");
    OHOS::Contacts::ConstructionName::local = "zh-CN";
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    int64_t rawContactId = RawContactInsert("李bp玉成욱", rawContactValues);
    EXPECT_GT(rawContactId, 0);

    OHOS::NativeRdb::ValuesBucket values;
    int64_t contactDataId = ContactDataInsert(rawContactId, "name", "李bp玉成욱", "", values);
    EXPECT_GT(contactDataId, 0);

    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo("raw_contact_id", std::to_string(rawContactId));
    std::string searchContact = ContactTabName::SEARCH_CONTACT;
    QueryAndExpectResult(searchContact, predicates, values, "李bp玉成욱||libpyucheng욱||lbpyc욱");
    ClearData();
}
} // namespace Test
} // namespace Contacts