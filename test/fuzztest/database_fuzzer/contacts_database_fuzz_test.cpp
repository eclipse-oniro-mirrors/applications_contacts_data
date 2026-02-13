/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include <vector>
#include <string>
#include <unistd.h>
#include <iostream>
#include "contacts_database_fuzz_test.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "access_token_error.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "datashare_values_bucket.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "predicates_convert.h"
#define private public
#include "contacts_database.h"
#undef private
#include "datashare_predicates.h"
#include "datashare_helper.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "singleton.h"
#include "contacts_columns.h"
#include "rdb_predicates.h"
#include "common.h"
#include "contacts_path.h"
#include "construction_name.h"
#include "common_tool_type.h"

using namespace OHOS::DataShare;
namespace Contacts {
namespace Test {
using namespace OHOS::Contacts;
static OHOS::Contacts::ContactsDataBase gCdb;
std::shared_ptr<OHOS::NativeRdb::RdbStore> gRdb = nullptr;
void ContactsStore()
{
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    // 调用rdb方法拼接数据库路径
    std::string databaseName = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
        OHOS::Contacts::ContactsPath::RDB_PATH, "contacts.db", getDataBasePathErrCode);
    OHOS::Contacts::SqliteOpenHelperContactCallback sqliteOpenHelperCallback;
    OHOS::NativeRdb::RdbStoreConfig config(databaseName);
    config.SetBundleName("com.ohos.contactsdataability");
    config.SetName("contacts.db");
    config.SetArea(1);
    config.SetSearchable(true);
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S3);
    config.SetAllowRebuild(true);
    config.SetAutoClean(false);
    int errCode = OHOS::NativeRdb::E_OK;
    gRdb = OHOS::NativeRdb::RdbHelper::GetRdbStore(
        config, OHOS::Contacts::DATABASE_CONTACTS_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
}

void OnContactsCreate(const uint8_t *data, size_t size)
{
    OHOS::Contacts::ConstructionName name;
    OHOS::Contacts::SqliteOpenHelperContactCallback sqliteOpenHelperCallback;
    OHOS::NativeRdb::ValueObject value;
    int contactValue = static_cast<int32_t>(*data);
    if (gRdb == nullptr) {
        return;
    }
    OHOS::NativeRdb::RdbStore& store = *gRdb.get();
    sqliteOpenHelperCallback.OnCreate(store);
    int oldVersion = 1;
    int newVersion = OHOS::Contacts::DATABASE_CONTACTS_OPEN_VERSION;
    name.sortKey = "key";
    name.sortFirstLetter_ = "123";
    sqliteOpenHelperCallback.OnUpgrade(store, oldVersion, newVersion);
    sqliteOpenHelperCallback.OnDowngrade(store, oldVersion, newVersion);
    sqliteOpenHelperCallback.UpdateSortInfo(store, name, 1);
    sqliteOpenHelperCallback.UpdateFormatPhoneNumber(store);
    sqliteOpenHelperCallback.UpdateSrotInfoByDisplayName(store);
    sqliteOpenHelperCallback.UpdateAnonymousSortInfo(store, 1);
    gCdb.GetContactByValue(contactValue, value);
}

void ContactsDataBasePublic(const uint8_t *data, size_t size)
{
    std::vector<std::string> columns;
    std::string isSync("true");
    columns.push_back(isSync);
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    if (gCdb.store_ == nullptr || gRdb == nullptr) {
        return;
    }
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet = gRdb->Query(rdbPredicates, columns);
    gCdb.ResultSetToValuesBucket(resultSet);
    std::string table = "contact_data";
    OHOS::DataShare::DataSharePredicates predicates;
    std::vector<OHOS::DataShare::DataShareValuesBucket> vectValues;
    OHOS::NativeRdb::ValueObject value;
    int contactValue = static_cast<int32_t>(*data);
    std::vector<int> rawContactIdVector;
    std::vector<int> contactIds;
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    gCdb.GetContactByValue(contactValue, value);
    gCdb.InsertRawContact(table, valuesBucket);
    gCdb.InsertContactData(table, valuesBucket, isSync);
    gCdb.BatchInsert(table, vectValues, isSync);
    gCdb.BatchInsertRawContacts(vectValues);
    gCdb.InsertGroup(table, valuesBucket);
    gCdb.InsertBlockList(table, valuesBucket);
    gCdb.InsertCloudRawContacts(table, valuesBucket);
    gCdb.UpdateContactData(valuesBucket, rdbPredicates, isSync);
    gCdb.UpdateRawContact(valuesBucket, rdbPredicates, isSync);
    gCdb.UpdateCloudRawContacts(valuesBucket, rdbPredicates);
    gCdb.Update(valuesBucket, rdbPredicates);
    gCdb.UpdateTable(valuesBucket, rdbPredicates);
    gCdb.UpdateBlockList(valuesBucket, rdbPredicates);
    gCdb.DeleteGroup(rdbPredicates);
    gCdb.DeleteRecord(rdbPredicates);
    gCdb.DeleteBlockList(rdbPredicates);
    gCdb.DeleteCloudRawContact(rdbPredicates);
    gCdb.Query(rdbPredicates, columns);
    gCdb.Split(predicates);
    gCdb.RecycleRestore(rdbPredicates, isSync);
    gCdb.InsertDeleteRawContact(table, valuesBucket);
    gCdb.QueryDeleteRawContact(rdbPredicates);
    gCdb.DeleteAll(gRdb, rawContactIdVector);
    gCdb.DeleteAllMerged(gRdb, rawContactIdVector, contactIds);
    gCdb.DestroyInstanceAndRestore(table);
}

void ContactsDataBasePublicEx(const uint8_t *data, size_t size)
{
    if (gCdb.store_ == nullptr) {
        return;
    }
    OHOS::NativeRdb::ValueObject value;
    int contactValue = static_cast<int32_t>(*data);
    gCdb.UpdateContactTimeStamp();
    gCdb.RecycleUpdateCloud(1);
    gCdb.QueryUuidNotInRawContact();
    gCdb.QueryBigLengthNameContact();
    gCdb.GetTypeId("phone");
    gCdb.SetFullSearchSwitch(false);
    gCdb.UpdateUUidNull();
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    gCdb.UpdateRawContactSeq(rawContactValues);
    gCdb.ClearRetryCloudSyncCount();
    gCdb.RetryCloudSync(0);
	gCdb.GetContactByValue(contactValue, value);
}

void ContactsDataBasePrivate(const uint8_t *data, size_t size)
{
    if (gCdb.store_ == nullptr) {
        return;
    }
    OHOS::NativeRdb::ValueObject idValue;
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<OHOS::NativeRdb::ValuesBucket> queryValuesBucket;
    std::vector<OHOS::NativeRdb::ValuesBucket> batchInsertValues;
    std::string isSync = "true";
    std::string rawContactIdColumn = "";
    std::string typeText = "";
    int ret = 0;
    int rawContactId = CommonToolType::UnsignedLongIntToInt(size);
    int contactId = CommonToolType::UnsignedLongIntToInt(size);
    int isDeletedValue = CommonToolType::UnsignedLongIntToInt(size);
    size_t deleteSize = 0;
    std::vector<int> rawContactIdVector;
    rawContactIdVector.push_back(1);
    std::vector<std::string> phoneNumbers;
    std::string phoneNumber(reinterpret_cast<const char *>(data), size);
    phoneNumbers.push_back("122");
    phoneNumbers.push_back(phoneNumber);
    gCdb.StructureDeleteContactJson(rawContactValues, rawContactIdColumn, rawContactId);
    gCdb.DeleteExecute(queryValuesBucket, isSync, deleteSize);
    gCdb.DeleteExecuteUpdate(rawContactIdVector);
    gCdb.QueryDeletedRelationData(rawContactId);
    gCdb.DeleteRawContactLocal(contactId, rawContactId, "", "", 100, "", batchInsertValues);
    gCdb.DeleteContactQuery(rdbPredicates, phoneNumbers);
    gCdb.DeleteRawContactQuery(rdbPredicates);
    gCdb.DeleteLocal(rawContactId, std::to_string(contactId), isDeletedValue);
    gCdb.DeleteCloud(rawContactId, ret);
    gCdb.DeleteLocalMerged(std::to_string(contactId), std::to_string(rawContactId));
    gCdb.Restore("contact_data");
    gCdb.UpdateDeleted(rawContactIdVector);
    gCdb.DeleteExecuteRetUpdateCloud("122");

    gCdb.GetTypeText(rawContactValues, ret, rawContactId, typeText);
    gCdb.CompletelyDeleteCommit(rawContactId);
    gCdb.GetUpdateDisplayRet(typeText, rawContactIdVector, rawContactValues);
    gCdb.BatchInsertPushBack(rawContactId, rawContactValues, queryValuesBucket, 1, "true");
    gCdb.RecordUpdateContactId(rawContactId, rawContactValues, "true");
}
}  // namespace Test
}  // namespace Contacts

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (!data || size <= 0) {
        return 0;
    }

    struct FunctionInfo {
        void (*func)(const uint8_t *, size_t);
        const char *func_name;
    };

    const FunctionInfo functions[] = {
        {Contacts::Test::OnContactsCreate, "OnContactsCreate"},
        {Contacts::Test::ContactsDataBasePublic, "ContactsDataBasePublic"},
        {Contacts::Test::ContactsDataBasePublicEx, "ContactsDataBasePublicEx"},
        {Contacts::Test::ContactsDataBasePrivate, "ContactsDataBasePrivate"},
    };

    const size_t num_functions = sizeof(functions) / sizeof(functions[0]);
    
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

    Contacts::Test::ContactsStore();
    for (size_t i = 0; i < num_functions; ++i) {
        const auto& segment = segments[i];
        const uint8_t* segment_data = segment.first;
        size_t segment_size = segment.second;

        if (segment_data == nullptr || segment_size == 0) {
            continue;
        }
        functions[i].func(segment_data, segment_size);
    }
    return 0;
}