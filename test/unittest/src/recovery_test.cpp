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

#include "recovery_test.h"

#include "database_disaster_recovery.h"
#include "test_common.h"

namespace Contacts {
namespace Test {
void RecoveryTest::ClearData()
{
    OHOS::AppExecFwk::ContactsDataAbility contactsDataAbility;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    predicates.NotEqualTo("id", "0");
    predicates.And();
    predicates.EqualTo("is_deleted", "0");
    int code1 = contactsDataAbility.Delete(uriRawContact, predicates);
    HILOG_INFO("RecoveryTest ClearData code1 is %{public}d", code1);
    int time = 3000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
    predicates.Clear();
    OHOS::Uri uriRawContactComplete(ContactsUri::DELETED_RAW_CONTACT);
    predicates.NotEqualTo("id", "0");
    int code = contactsDataAbility.Delete(uriRawContactComplete, predicates);
    HILOG_INFO("RecoveryTest ClearData code is %{public}d", code);
}

int64_t RecoveryTest::RawContactInsert(std::string displayName)
{
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    rawContactValues.PutString("display_name", displayName);
    int64_t code = contactsDataAbility.Insert(uriRawContact, rawContactValues);
    rawContactValues.Clear();
    return code;
}

/*
 * @tc.number  recovery_test_100
 * @tc.name    Backup database
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(RecoveryTest, recovery_test_100, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- recovery_test_100 is starting! ---");
    std::shared_ptr<OHOS::Contacts::DataBaseDisasterRecovery> instance =
        OHOS::Contacts::DataBaseDisasterRecovery::GetInstance();
    std::shared_ptr<OHOS::Contacts::ContactsDataBase> contactDatabase = OHOS::Contacts::ContactsDataBase::GetInstance();
    int result = instance->SQLiteCheckDb();
    HILOG_INFO("--- recovery_test_100 SQLiteCheckDb ret ! --- %{public}d", result);
    if (result == 0) {
        instance->BackDatabase();
    }
    std::string backupPath = instance->GetBackUpDatabase(contactDatabase->contactStore_);
    HILOG_INFO("--- recovery_test_100 is end! --- %{private}s ", backupPath.c_str());
    ClearData();
}

/*
 * @tc.number  recovery_test_200
 * @tc.name    Backup database after insert two person, then recover
 * @tc.desc    Exception use case
 * @tc.level   Level1
 * @tc.size    MediumTest
 * @tc.type    Function
 */
HWTEST_F(RecoveryTest, recovery_test_200, testing::ext::TestSize.Level1)
{
    HILOG_INFO("--- recovery_test_200 is starting! ---");
    RawContactInsert("liming");
    RawContactInsert("xiaolilili");
    // backup
    OHOS::Uri uriRawContactBackUp(ContactsUri::BACKUP);
    OHOS::NativeRdb::ValuesBucket value;
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    int retCode = contactsDataAbility.Update(uriRawContactBackUp, value, predicates);
    EXPECT_EQ(0, retCode);

    RawContactInsert("xiaobaibaibai");
    RawContactInsert("xiaolitiantian");
    OHOS::Uri uriRawContact(ContactsUri::RAW_CONTACT);
    std::vector<std::string> columns;
    predicates.EqualTo("is_deleted", "0");
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        contactsDataAbility.Query(uriRawContact, columns, predicates);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    EXPECT_EQ(4, rowCount);

    OHOS::Uri uriRawContactRecover(ContactsUri::RECOVER);
    int retCodeRecover = contactsDataAbility.Update(uriRawContactRecover, value, predicates);
    EXPECT_EQ(0, retCodeRecover);

    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSetRecover =
        contactsDataAbility.Query(uriRawContact, columns, predicates);
    int rowCountRecover = 0;
    resultSetRecover->GetRowCount(rowCountRecover);
    resultSetRecover->Close();
    EXPECT_EQ(2, rowCountRecover);
    ClearData();
}
} // namespace Test
} // namespace Contacts
