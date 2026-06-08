/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#define private public
#include "contactfuzz_test.h"

#include <vector>
#include <string>
#include <unistd.h>
#include "access_token.h"
#include "accesstoken_kit.h"
#include "access_token_error.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "datashare_values_bucket.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "predicates_convert.h"
#include "contacts_data_ability.h"
#include "datashare_predicates.h"
#include "datashare_helper.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "singleton.h"
#include "contacts_columns.h"
#include "rdb_predicates.h"
#include "contacts_database.h"
#include "contact_connect_ability.h"
#include "common_tool_type.h"
#include "privacy_contacts_manager.h"
#include "iservice_registry.h"
#include "contacts_manager.h"
#include "os_account_manager.h"

using namespace OHOS::Security::AccessToken;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::DataShare;
using namespace OHOS::Security::AccessToken;
namespace Contacts {
namespace Test {
static constexpr const char *CONTACT_DATA = "datashare:///com.ohos.contactsdataability/contacts/contact_data";
static constexpr const char *CONTACT_BLOCKLIST = "datashare:///com.ohos.contactsdataability/contacts/contact_blocklist";
static constexpr const char *RAW_CONTACT_FUZZ = "datashare:///com.ohos.contactsdataability/contacts/raw_contact";
static OHOS::AbilityRuntime::ContactsDataAbility contactsDataAbility;

using namespace OHOS::Contacts;
static std::vector<int> updateExecuteCommons = {
    RDB_EXECUTE_OK, RDB_EXECUTE_FAIL, RDB_PERMISSION_ERROR, CONTACTS_CONTACT, PROFILE_CONTACT, MERGE_CONTACT,
    CONTACTS_RAW_CONTACT, CONTACTS_CONTACT_DATA, PROFILE_GROUPS, PROFILE_CONTACT_DATA, CONTACTS_GROUPS,
    CLOUD_DATA, QUERY_IS_REFRESH_CONTACT, RESTORE_CONTACT_BY_DOUBLE_TO_SINGLE_UPGRADE_DATA, CONTACTS_BLOCKLIST,
    PROFILE_BLOCKLIST, UPLOAD_DATA_TO_CLOUD, TRIGGER_UPLOAD_CLOUD, SPLIT_AGGREGATION_CONTACT, HW_ACCOUNT, SPLIT_CONTACT,
    CONTACT_BACKUP, PROFILE_BACKUP, CONTACT_RECOVER, PROFILE_RECOVER
};
// REBUILD_CONTACT_SERACH_INDEX, CLEAR_AND_RECREATE_TRIGGER_SEARCH_TABLE, SET_FULL_SEARCH_SWITCH_OPEN,
// MANUAL_MERGE, AUTO_MERGE SET_FULL_SEARCH_SWITCH_CLOSE

static std::vector<int> insertExecuteCommons = {
    RDB_EXECUTE_OK, RDB_EXECUTE_FAIL, RDB_PERMISSION_ERROR, CONTACTS_RAW_CONTACT, PROFILE_RAW_CONTACT,
    CONTACTS_CONTACT_DATA, PROFILE_CONTACT_DATA, CONTACTS_GROUPS, PROFILE_GROUPS, PRIVACY_CONTACTS_BACKUP
};
// PROFILE_BLOCKLIST, CONTACTS_BLOCKLIST, CLOUD_DATA, HW_ACCOUNT, CONTACTS_DELETE

static std::vector<int> deleteExecuteCommons = {
    RDB_EXECUTE_OK, RDB_EXECUTE_FAIL, RDB_PERMISSION_ERROR, CONTACTS_CONTACT, PROFILE_CONTACT, CONTACTS_RAW_CONTACT,
    PROFILE_RAW_CONTACT, CONTACTS_CONTACT_DATA, PROFILE_CONTACT_DATA, CONTACTS_GROUPS, PROFILE_GROUPS,
    CONTACTS_BLOCKLIST, PROFILE_BLOCKLIST, CONTACTS_DELETE, PROFILE_DELETE, CONTACTS_DELETE_RECORD,
    PROFILE_DELETE_RECORD, CLOUD_DATA
};
// RECYCLE_RESTORE, CONTACTS_DELETE_MERGE

static std::vector<int> queryExecuteCommons = {
    RDB_EXECUTE_OK, RDB_EXECUTE_FAIL, RDB_PERMISSION_ERROR, CONTACTS_CONTACT, PROFILE_CONTACT, CONTACTS_RAW_CONTACT,
    PROFILE_RAW_CONTACT, CONTACTS_CONTACT_DATA, PROFILE_CONTACT_DATA, CONTACTS_GROUPS, PROFILE_GROUPS, CLOUD_DATA,
    QUERY_UUID_NOT_IN_RAW_CONTACT, QUERY_BIG_LENGTH_NAME, HW_ACCOUNT, QUERY_IS_REFRESH_CONTACT, CONTACTS_PLACE_HOLDER,
    CONTACTS_BLOCKLIST, PROFILE_BLOCKLIST, PULLDOWN_SYNC_CONTACT_DATA, SYNC_BIRTHDAY_TO_CALENDAR, BACKUP_RESTORE,
    CONTACTS_DELETE, PROFILE_DELETE, CONTACTS_SEARCH_CONTACT, PROFILE_SEARCH_CONTACT, CONTACT_TYPE,
    PROFILE_TYPE, ACCOUNT
};
// QUERY_MERGE_LIST,

void NativeTokenGet(const uint8_t *data, size_t size)
{
    const char *perms[2] = {
        "ohos.permission.WRITE_CONTACTS",
        "ohos.permission.READ_CONTACTS",
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "contacts_data_fuzzer",
        .aplStr = "system_core",
    };
    uint64_t tokenId = 0;
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    AccessTokenKit::ReloadNativeTokenInfo();
}
void ContactsInsert(const uint8_t *data, size_t size)
{
    std::string detailInfo(reinterpret_cast<const char *>(data), size);
    std::string typeId(reinterpret_cast<const char *>(data), size);
    OHOS::DataShare::DataShareValuesBucket value;
    value.Put("detail_info", detailInfo);
    value.Put("type_id", typeId);
    OHOS::Uri uriContactData(CONTACT_DATA);
    contactsDataAbility.Insert(uriContactData, value);
}
void ContactsBatchInsert(const uint8_t *data, size_t size)
{
    std::string detailInfo(reinterpret_cast<const char *>(data), size);
    OHOS::DataShare::DataShareValuesBucket value;
    value.Put("detail_info", detailInfo);
    int typeId = 2;
    value.Put("type_id", typeId);
    std::vector<OHOS::DataShare::DataShareValuesBucket> values;
    values.push_back(value);
    OHOS::Uri uriContactData(CONTACT_DATA);
    contactsDataAbility.BatchInsert(uriContactData, values);
}
void ContactsUpdate(const uint8_t *data, size_t size)
{
    std::string detailInfo(reinterpret_cast<const char *>(data), size);
    OHOS::DataShare::DataShareValuesBucket value;
    value.Put("name", detailInfo);
    OHOS::Uri uriContactData(CONTACT_BLOCKLIST);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", "1");
    contactsDataAbility.Update(uriContactData, predicates, value);
}
void ContactsDelete(const uint8_t *data, size_t size)
{
    OHOS::Uri uriRawContact(RAW_CONTACT_FUZZ);
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    std::string colomName(reinterpret_cast<const char *>(data), size);
    OHOS::DataShare::DataSharePredicates predicates;
    contactsDataAbility.Delete(uriRawContact, predicates);
    contactsDataAbility.getIntValueFromRdbBucket(valuesBucket, colomName);
}

void ContactsDataAbilityCreate(const uint8_t *data, size_t size)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    std::string colomName(reinterpret_cast<const char *>(data), size);
    contactsDataAbility.getIntValueFromRdbBucket(valuesBucket, colomName);
    ContactsDataAbility::Create();
}

void ContactsDataAbilityIsCommitOK(const uint8_t *data, size_t size)
{
    std::mutex mutex;
    std::string str(reinterpret_cast<const char *>(data), size);
    int code = str.length();
    contactsDataAbility.IsCommitOK(code, mutex);
    std::mutex mutex1;
    contactsDataAbility.IsCommitOK(0, mutex1);
}

void ContactsDataAbilityIsBeginTransactionOK(const uint8_t *data, size_t size)
{
    std::mutex mutex;
    std::string str(reinterpret_cast<const char *>(data), size);
    int code = str.length();
    contactsDataAbility.IsBeginTransactionOK(code, mutex);
    std::mutex mutex1;
    contactsDataAbility.IsBeginTransactionOK(0, mutex1);
}

void ContactsDataAbilityInsertExecute(const uint8_t *data, size_t size)
{
    OHOS::DataShare::DataShareValuesBucket value;
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    OHOS::Uri uriContactData(std::string(reinterpret_cast<const char *>(data), size).c_str());
    std::string isSyncFromCloud(reinterpret_cast<const char *>(data), size);
    int code = CommonToolType::UnsignedLongIntToInt(size);
    contactsDataAbility.InsertExecute(code, valuesBucket, isSyncFromCloud);
    for (auto element : insertExecuteCommons) {
        valuesBucket = OHOS::NativeRdb::ValuesBucket();
        OHOS::AbilityRuntime::ContactsDataAbility ability;
        ability.contactDataBase_ = std::make_shared<ContactsDataBase>();
        ability.profileDataBase_ = std::make_shared<ProfileDatabase>();
        ability.contactsConnectAbility_ = std::make_shared<ContactConnectAbility>();
        ability.InsertExecute(element, valuesBucket, isSyncFromCloud);
    }
    valuesBucket.PutString("phone_number", "13900001111");
    valuesBucket.PutInt("types", 5);
    for (auto element : insertExecuteCommons) {
        OHOS::AbilityRuntime::ContactsDataAbility ability;
        ability.contactDataBase_ = std::make_shared<ContactsDataBase>();
        ability.profileDataBase_ = std::make_shared<ProfileDatabase>();
        ability.contactsConnectAbility_ = std::make_shared<ContactConnectAbility>();
        ability.InsertExecute(element, valuesBucket, isSyncFromCloud);
    }
}

void ContactsDataAbilityTransferTypeInfo(const uint8_t *data, size_t size)
{
    std::vector<OHOS::DataShare::DataShareValuesBucket> values;
    std::vector<OHOS::NativeRdb::ValuesBucket> valuesRdb;
    contactsDataAbility.transferTypeInfo(values, valuesRdb);
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    std::string colomName(reinterpret_cast<const char *>(data), size);
    contactsDataAbility.getIntValueFromRdbBucket(valuesBucket, colomName);
}

void ContactsDataAbilityPutDisplayNameInfo(const uint8_t *data, size_t size)
{
    OHOS::DataShare::DataShareValuesBucket displayNameBucket;
    std::string displayName(reinterpret_cast<const char *>(data), size);
    int rawContactId = size;
    contactsDataAbility.putDisplayNameInfo(displayNameBucket, displayName, rawContactId);
}

void ContactsDataAbilityGenerateDisplayName(const uint8_t *data, size_t size)
{
    std::vector<OHOS::NativeRdb::ValuesBucket> valuesRdb;
    contactsDataAbility.generateDisplayName(valuesRdb);
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    std::string colomName(reinterpret_cast<const char *>(data), size);
    contactsDataAbility.getIntValueFromRdbBucket(valuesBucket, colomName);
}

void ContactsDataAbilityGenerateDisplayNameNew(const uint8_t *data, size_t size)
{
    std::string fromDetailInfo(reinterpret_cast<const char *>(data), size);
    int typeId = size;
    contactsDataAbility.generateDisplayName(typeId, fromDetailInfo);
}

void ContactsDataAbilityGetStringValueFromRdbBucket(const uint8_t *data, size_t size)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    const std::string colName(reinterpret_cast<const char *>(data), size);
    std::string fromDetailInfo;
    contactsDataAbility.getStringValueFromRdbBucket(valuesBucket, colName);
}

void ContactsDataAbilityTransferDataShareToRdbBucket(const uint8_t *data, size_t size)
{
    std::vector<OHOS::DataShare::DataShareValuesBucket> values;
    std::vector<OHOS::NativeRdb::ValuesBucket> valuesRdb;
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    std::string colomName(reinterpret_cast<const char *>(data), size);
    contactsDataAbility.transferDataShareToRdbBucket(values, valuesRdb);
    contactsDataAbility.getIntValueFromRdbBucket(valuesBucket, colomName);
}

void ContactsDataAbilityGetIntValueFromRdbBucket(const uint8_t *data, size_t size)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    std::string colName(reinterpret_cast<const char *>(data), size);
    contactsDataAbility.getIntValueFromRdbBucket(valuesBucket, colName);
}

void ContactsDataAbilityBatchInsertByMigrate(const uint8_t *data, size_t size)
{
    OHOS::Uri uri(std::string(reinterpret_cast<const char *>(data), size).c_str());
    const std::vector<OHOS::DataShare::DataShareValuesBucket> values;
    contactsDataAbility.BatchInsertByMigrate(OHOS::Contacts::CONTACTS_RAW_CONTACT, uri, values);
    contactsDataAbility.BatchInsertByMigrate(size, uri, values);
}

void ContactsDataAbilityUpdateExecute(const uint8_t *data, size_t size)
{
    auto ptr = std::make_shared<OHOS::AbilityRuntime::ContactsDataAbility>();
    for (auto element : updateExecuteCommons) {
        int retCode;
        OHOS::NativeRdb::ValuesBucket value;
        OHOS::DataShare::DataSharePredicates dataSharePredicates;
        std::string isSync(reinterpret_cast<const char *>(data), size);
        OHOS::AbilityRuntime::ContactsDataAbility ability;
        ability.contactDataBase_ = std::make_shared<ContactsDataBase>();
        ability.profileDataBase_ = std::make_shared<ProfileDatabase>();
        ability.contactsConnectAbility_ = std::make_shared<ContactConnectAbility>();
        ability.UpdateExecute(retCode, element, value, dataSharePredicates, isSync);
        ability.SwitchUpdate(retCode, element, value, dataSharePredicates);
        ability.SwitchUpdateOthers(retCode, element, value, dataSharePredicates);
    }
}

void ContactsDataAbilitySwitchUpdate(const uint8_t *data, size_t size)
{
    int retCode;
    OHOS::NativeRdb::ValuesBucket value;
    OHOS::DataShare::DataSharePredicates dataSharePredicates;
    std::string str(reinterpret_cast<const char *>(data), size);
    int code = str.length();
    contactsDataAbility.SwitchUpdate(retCode, code, value, dataSharePredicates);
}

void ContactsDataAbilityDeleteExecuteSwitchSplit(const uint8_t *data, size_t size)
{
    int retCode;
    OHOS::NativeRdb::ValuesBucket value;
    OHOS::DataShare::DataSharePredicates dataSharePredicates;
    std::string isSync(reinterpret_cast<const char *>(data), size);
    contactsDataAbility.DeleteExecuteSwitchSplit(retCode, size, dataSharePredicates, isSync, "");
    for (auto element : deleteExecuteCommons) {
        dataSharePredicates = OHOS::DataShare::DataSharePredicates();
        OHOS::AbilityRuntime::ContactsDataAbility ability;
        ability.contactDataBase_ = std::make_shared<ContactsDataBase>();
        ability.profileDataBase_ = std::make_shared<ProfileDatabase>();
        ability.contactsConnectAbility_ = std::make_shared<ContactConnectAbility>();
        ability.DeleteExecute(retCode, element, dataSharePredicates, isSync, "");
        ability.DeleteExecuteSwitchSplit(retCode, element, dataSharePredicates, isSync, "");
    }
}

void ContactsDataAbilityQuery(const uint8_t *data, size_t size)
{
    OHOS::Uri uri(std::string(reinterpret_cast<const char *>(data), size).c_str());
    OHOS::DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    OHOS::DataShare::DatashareBusinessError businessError;
    contactsDataAbility.Query(uri, predicates, columns, businessError);
}

void ContactsDataAbilityQueryExecute(const uint8_t *data, size_t size)
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> result;
    OHOS::DataShare::DataSharePredicates dataSharePredicates;
    std::vector<std::string> columnsTemp;
    std::string str(reinterpret_cast<const char *>(data), size);
    int code = str.length();
    std::string uriStr = "";
    OHOS::Uri uri(uriStr.c_str());
    contactsDataAbility.QueryExecute(result, dataSharePredicates, columnsTemp, code, uri);
    for (auto element : queryExecuteCommons) {
        dataSharePredicates = OHOS::DataShare::DataSharePredicates();
        OHOS::AbilityRuntime::ContactsDataAbility ability;
        ability.contactDataBase_ = std::make_shared<ContactsDataBase>();
        ability.profileDataBase_ = std::make_shared<ProfileDatabase>();
        ability.contactsConnectAbility_ = std::make_shared<ContactConnectAbility>();
        ability.QueryExecute(result, dataSharePredicates, columnsTemp, element, uri);
        ability.QueryExecuteSwitchSplit(result, dataSharePredicates, columnsTemp, element, uri);
        ability.QueryExecuteSwitchSplitOthers(result, dataSharePredicates, columnsTemp, element, uri);
    }
}

void ContactsDataAbilityQueryExecuteSwitchSplit(const uint8_t *data, size_t size)
{
    std::shared_ptr<OHOS::NativeRdb::ResultSet> result;
    OHOS::DataShare::DataSharePredicates dataSharePredicates;
    std::vector<std::string> columnsTemp;
    std::string str(reinterpret_cast<const char *>(data), size);
    int code = str.length();
    std::string uriStr = "";
    OHOS::Uri uri(uriStr.c_str());
    contactsDataAbility.QueryExecuteSwitchSplit(result, dataSharePredicates, columnsTemp, code, uri);
}

void ContactsDataAbilitySwitchProfile(const uint8_t *data, size_t size)
{
    std::string uid(reinterpret_cast<const char *>(data), size);
    OHOS::Uri uri(uid.c_str());
    contactsDataAbility.SwitchProfile(uri);
}

void ContactsDataAbilityBackUp(const uint8_t *data, size_t size)
{
    contactsDataAbility.BackUp();
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    std::string colName(reinterpret_cast<const char *>(data), size);
    contactsDataAbility.getIntValueFromRdbBucket(valuesBucket, colName);
}

void ContactsDataAbilityRecover(const uint8_t *data, size_t size)
{
    std::string str(reinterpret_cast<const char *>(data), size);
    int code = str.length();
    contactsDataAbility.Recover(code);
}

void ContactsDataAbilityDataBaseNotifyChange(const uint8_t *data, size_t size)
{
    std::string uid(reinterpret_cast<const char *>(data), size);
    OHOS::Uri uri(uid.c_str());
    contactsDataAbility.DataBaseNotifyChange(size, uri);
}

void PrivacyContactPubilc(const uint8_t *data, size_t size)
{
    int32_t num = static_cast<int32_t>(*data);
    auto privacyContactsManager = OHOS::Contacts::PrivacyContactsManager::GetInstance();
    privacyContactsManager->SyncPrivacyContactsBackup();
    OHOS::DataShare::DataShareValuesBucket value;
    value.Put("type_id", 5);
    OHOS::NativeRdb::ValuesBucket contactData;
    contactData.PutInt("type_id", 5);
    std::vector<OHOS::DataShare::DataShareValuesBucket> values;
    values.emplace_back(value);
    privacyContactsManager->InsertContactDataToPrivacyBackup(contactData);
    privacyContactsManager->InsertContactDataToPrivacyBackup(values);
    std::vector<int32_t> rawContactIdsEmpty;
    std::vector<int32_t> rawContactIds = {num, 1, 2};
    privacyContactsManager->DeleteContactsFromPrivacyBackup(rawContactIdsEmpty);
    privacyContactsManager->DeleteContactsFromPrivacyBackup(rawContactIds);
    privacyContactsManager->restorePrivacContacts(rawContactIds);
    privacyContactsManager->TryToMigratePrivateCallLog(value);
    std::vector<OHOS::DataShare::DataShareValuesBucket> unprocessedDataShareValues;
    privacyContactsManager->ProcessPrivacyCallLog(values, unprocessedDataShareValues);
    std::vector<OHOS::NativeRdb::ValuesBucket> unprocessedValues;
    std::vector<OHOS::NativeRdb::ValuesBucket> valueBuckets;
    valueBuckets.emplace_back(contactData);
    privacyContactsManager->ProcessPrivacyCallLog(valueBuckets, unprocessedValues);
    privacyContactsManager->MigratePrivacyCallLog();
    OHOS::Contacts::PrivacyContactsManager::IsMainSpace();
    OHOS::Contacts::PrivacyContactsManager::IsPrivacySpace();
    privacyContactsManager->InsertContactDataToPrivacyBackup(valueBuckets);
    privacyContactsManager->InsertPrivacContactsByContactDatas(valueBuckets);
    OHOS::Contacts::PrivacyContactsManager::ToDataShareValuesBucket(contactData);
    OHOS::NativeRdb::ValuesBucket outData;
    OHOS::NativeRdb::ValuesBucket contactData1;
    contactData1.PutInt("type_id", 6);
    privacyContactsManager->GetValuesBucketFromContactData(contactData1, outData);
    privacyContactsManager->GetValuesBucketFromContactData(contactData, outData);
    privacyContactsManager->QueryCallLogWithPhoneNumber("111", values);
    privacyContactsManager->UpdatePrivacyTag(values);
    privacyContactsManager->InsertPrivacyCallLog(values);
    std::string id(reinterpret_cast<const char *>(data), size);
    std::vector<std::string> rawContactIdStrs = {id, "1", "2"};
    privacyContactsManager->DeleteCallLogAfterMigrate(rawContactIdStrs);
    privacyContactsManager->QueryCallLogWithPrivacyTag(values, rawContactIdStrs);
    privacyContactsManager->QueryContactsFromPrivacyContactData(values);
    auto resultSet = std::make_shared<OHOS::DataShare::DataShareResultSet>();
    OHOS::Contacts::PrivacyContactsManager::ConvertResultSetToValuesBucket(resultSet, values);
    privacyContactsManager->GetPrivacyCallLogPredicates("111");
    privacyContactsManager->UpdatePrivacyTagOfValueBucket(value);
    value.Put("is_read", 1);
    privacyContactsManager->UpdatePrivacyTagOfValueBucket(value);
    privacyContactsManager->DeleteAllContactsFromPrivacyBackup();
    int32_t userId = 100;
    privacyContactsManager->GetSpecialTypeUserId(OHOS::AccountSA::OsAccountType::ADMIN, userId);
}

void OnReceiveEvent(const uint8_t *data, size_t size)
{
    auto contactMnager = OHOS::Contacts::ContactManager::GetInstance();
    OHOS::EventFwk::CommonEventData event;
    OHOS::EventFwk::Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    event.SetCode(1);
    event.SetWant(want);
    contactMnager->screenSwitchSubscriber_->OnReceiveEvent(event);
}

void BlocklistDataBasePublic(const uint8_t *data, size_t size)
{
    auto blocklistDataBase = OHOS::Contacts::BlocklistDataBase::GetInstance();
    blocklistDataBase->BeginTransaction();
    blocklistDataBase->RollBack();
    blocklistDataBase->BeginTransaction();
    blocklistDataBase->Commit();
    OHOS::NativeRdb::RdbPredicates rdbPredicates(OHOS::Contacts::ContactTableName::CONTACT_BLOCKLIST);
    std::vector<std::string> columns;
    blocklistDataBase->Query(rdbPredicates, columns);
}
}  // namespace Test
}  // namespace Contacts

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (!data || size <= 0) {
        return 0;
    }
    // 定义需要调用的函数列表
    struct FunctionInfo {
        void (*func)(const uint8_t *, size_t);
        const char *func_name;
    };

    const FunctionInfo functions[] = {
        {Contacts::Test::ContactsInsert, "ContactsInsert"},
        {Contacts::Test::ContactsBatchInsert, "ContactsBatchInsert"},
        {Contacts::Test::ContactsUpdate, "ContactsUpdate"},
        {Contacts::Test::ContactsDelete, "ContactsDelete"},
        {Contacts::Test::NativeTokenGet, "NativeTokenGet"},
        {Contacts::Test::ContactsInsert, "ContactsInsert"},
        {Contacts::Test::ContactsBatchInsert, "ContactsBatchInsert"},
        {Contacts::Test::ContactsUpdate, "ContactsUpdate"},
        {Contacts::Test::ContactsDelete, "ContactsDelete"},
        {Contacts::Test::ContactsDataAbilityCreate, "ContactsDataAbilityCreate"},
        {Contacts::Test::ContactsDataAbilityIsCommitOK, "ContactsDataAbilityIsCommitOK"},
        {Contacts::Test::ContactsDataAbilityIsBeginTransactionOK, "ContactsDataAbilityIsBeginTransactionOK"},
        {Contacts::Test::ContactsDataAbilityInsertExecute, "ContactsDataAbilityInsertExecute"},
        {Contacts::Test::ContactsDataAbilityTransferTypeInfo, "ContactsDataAbilityTransferTypeInfo"},
        {Contacts::Test::ContactsDataAbilityPutDisplayNameInfo, "ContactsDataAbilityPutDisplayNameInfo"},
        {Contacts::Test::ContactsDataAbilityGenerateDisplayName, "ContactsDataAbilityGenerateDisplayName"},
        {Contacts::Test::ContactsDataAbilityGenerateDisplayNameNew, "ContactsDataAbilityGenerateDisplayNameNew"},
        {Contacts::Test::ContactsDataAbilityGetStringValueFromRdbBucket, "ContactsDataAbilityGetStringValueFromRdbBucket"},
        {Contacts::Test::ContactsDataAbilityTransferDataShareToRdbBucket, "ContactsDataAbilityTransferDataShareToRdbBucket"},
        {Contacts::Test::ContactsDataAbilityGetIntValueFromRdbBucket, "ContactsDataAbilityGetIntValueFromRdbBucket"},
        {Contacts::Test::ContactsDataAbilityBatchInsertByMigrate, "ContactsDataAbilityBatchInsertByMigrate"},
        {Contacts::Test::ContactsDataAbilityUpdateExecute, "ContactsDataAbilityUpdateExecute"},
        {Contacts::Test::ContactsDataAbilitySwitchUpdate, "ContactsDataAbilitySwitchUpdate"},
        {Contacts::Test::ContactsDataAbilityDeleteExecuteSwitchSplit, "ContactsDataAbilityDeleteExecuteSwitchSplit"},
        {Contacts::Test::ContactsDataAbilityQuery, "ContactsDataAbilityQuery"},
        {Contacts::Test::ContactsDataAbilityQueryExecute, "ContactsDataAbilityQueryExecute"},
        {Contacts::Test::ContactsDataAbilityQueryExecuteSwitchSplit, "ContactsDataAbilityQueryExecuteSwitchSplit"},
        {Contacts::Test::ContactsDataAbilitySwitchProfile, "ContactsDataAbilitySwitchProfile"},
        {Contacts::Test::ContactsDataAbilityBackUp, "ContactsDataAbilityBackUp"},
        {Contacts::Test::ContactsDataAbilityRecover, "ContactsDataAbilityRecover"},
        {Contacts::Test::ContactsDataAbilityDataBaseNotifyChange, "ContactsDataAbilityDataBaseNotifyChange"},
        {Contacts::Test::PrivacyContactPubilc, "PrivacyContactPubilc"},
        {Contacts::Test::OnReceiveEvent, "OnReceiveEvent"},
        {Contacts::Test::BlocklistDataBasePublic, "BlocklistDataBasePublic"},
    };

    const size_t num_functions = sizeof(functions) / sizeof(functions[0]);

    // 分割数据
    std::vector<std::pair<const uint8_t*, size_t>> segments;
    size_t current_pos = 0;

    for (size_t i = 0; i < num_functions; ++i) {
        size_t segment_size = size / num_functions;
        if (i == num_functions - 1) {
            segment_size = size - current_pos;
        }

        if (segment_size > 0) {
            segments.emplace_back(data + current_pos, segment_size);
            current_pos += segment_size;
        } else {
            segments.emplace_back(nullptr, 0);
        }
    }

    for (size_t i = 0; i < num_functions; ++i) {
        const auto& segment = segments[i];
        const uint8_t* segment_data = segment.first;
        size_t segment_size = segment.second;

        if (segment_data == nullptr || segment_size == 0) {
            continue;
        }

        // 调用函数
        functions[i].func(segment_data, segment_size);
    }
    return 0;
}