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
#include "other_fuzz_test.h"
#define private public
#include "contacts_account.h"
#include "contacts_datashare_stub_impl.h"
#include "contacts_update_helper.h"
#include "contacts.h"
#include "profile_database.h"
#include "raw_contacts.h"
#undef private
#include "contacts_path.h"
#include "contacts_database.h"
#include "common.h"
#include "account_data_collection.h"
#include "data_ability_observer_interface.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace Contacts {
namespace Test {
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

namespace Account {
void AccountPublic(const uint8_t *data, size_t size)
{
    OHOS::Contacts::ContactsAccount ca;
    OHOS::Contacts::AccountDataCollection collection("1", "1", "1");
    ca.Insert(gRdb, "account", "1");
    ca.LookupAccountTypeId(gRdb, "account", "1");
    ca.PrepopulateCommonAccountTypes(gRdb);
    ca.GetAccountFromLocal(gRdb);
    ca.GetNotExistAccount(gRdb, collection);
    int32_t num = static_cast<int32_t>(*data) % 1;
    ca.DeleteDataByRawId(gRdb, num + 1);
    ca.DeleteGroupsByAccountId(gRdb, 1);
    ca.DeleteAccountByAccountId(gRdb, 1);
    ca.StopForegin(gRdb);
    ca.OpenForegin(gRdb);
}
}

namespace Datashare {
void DatasharePubilc(const uint8_t *data, size_t size)
{
    std::string mode = "r";
    std::string mimeTypeFilter = "1";
    OHOS::DataShare::ContactsDataShareStubImpl cdss;
    std::vector<std::string> columns;
    std::string column(reinterpret_cast<const char *>(data), size);
    columns.push_back(column);
    std::shared_ptr<OHOS::DataShare::DataShareExtAbility> extension;
    OHOS::Uri uri("datashare:///com.ohos.contactsdataability/contacts/contact_data");
    OHOS::DataShare::DataSharePredicates predicates;
    OHOS::DataShare::DataShareValuesBucket dsValue;
    OHOS::DataShare::DatashareBusinessError businessError;
    std::vector<OHOS::DataShare::DataShareValuesBucket> dsValues;
    dsValue.Put("detail_info", "123456789");
    dsValue.Put("type_id", "1");
    cdss.Insert(uri, dsValue);
    cdss.Update(uri, predicates, dsValue);
    cdss.Delete(uri, predicates);
    cdss.Query(uri, predicates, columns, businessError);
    cdss.BatchInsert(uri, dsValues);
    cdss.GetFileTypes(uri, mimeTypeFilter);
    cdss.OpenFile(uri, mode);
    cdss.OpenRawFile(uri, mode);
    cdss.GetType(uri);
    cdss.NotifyChange(uri);
    cdss.NormalizeUri(uri);
    cdss.DenormalizeUri(uri);
    cdss.SetContactsDataAbility(extension);
    cdss.SetCallLogAbility(extension);
    cdss.SetVoiceMailAbility(extension);
    cdss.GetBundleNameByUid(1, mimeTypeFilter);
    cdss.GetOwner(uri);
    cdss.GetContactsDataAbility();
    cdss.GetCallLogAbility();
    cdss.GetVoiceMailAbility();
}
}  // namespace Account

namespace Helper {
void HelperPubilc(const uint8_t *data, size_t size)
{
    std::string name = "1";
    std::string quickSearch = "1";
    std::string extra3 = "1";
    std::vector<int> vectorInt;
    std::vector<std::string> vectorStr;
    std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet;
    OHOS::NativeRdb::ValuesBucket vb;
    OHOS::Contacts::ContactsUpdateHelper cuh;
    vectorStr.push_back("organization");
    int32_t num = static_cast<int32_t>(*data) % 1;
    vectorInt.push_back(num + 1);
    if (gRdb != nullptr) {
        cuh.UpdateDisplay(vectorInt, vectorStr, gRdb, vb, false);
        cuh.GetUpdateDisPlayNameValuesBucket(vb, false);
        cuh.GetUpdateSearchNameValuesBucket(vb, false);
        cuh.GetUpdateCompanyValuesBucket(vb, false);
        cuh.UpdateName(vb, false, 1, "1", gRdb);
        cuh.UpdateNameResult(vb, 1, "1", gRdb);
        cuh.UpdateCallLogByPhoneNumNotifyChange(1);
        cuh.QueryDataForCallLog(gRdb, 1);
        cuh.UpdateCallLog(vectorStr, vectorStr, name, quickSearch, extra3, false);
        cuh.UpdateCallLogWhenContactChange(vectorStr, vectorStr, name, quickSearch, extra3);
        cuh.UpdateCallLogWhenDeleteContact(vectorStr, vectorStr, quickSearch);
        cuh.UpdateCallLogNameNull(name, quickSearch);
        cuh.UpdateCallLogAddPhoneNumber(vectorStr, name, quickSearch, extra3, false);
        cuh.UpdateCallLogDelPhoneNumber(vectorStr, vectorStr, name, quickSearch);
    }
}
}  // namespace Helper

namespace Table {
void TablePubilc(const uint8_t *data, size_t size)
{
    int32_t num = static_cast<int32_t>(*data) % 1;
    int64_t id64 = 1;
    std::string columnName = "company";
    std::string columnValue = "hw";
    OHOS::Contacts::Contacts table;
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    rawContactValues.PutString(columnName, columnValue);
    if (gRdb != nullptr) {
        table.InsertContact(gRdb, id64, rawContactValues, id64);
        table.StructureContactDataValueBucket(rawContactValues);
        int id = num + 1;
        table.UpdateContact(id, gRdb, rawContactValues);
        table.DeleteContactById(gRdb, id);
    }
    table.ContactValueBucketGetInt(rawContactValues, columnName);
    table.ContactValueBucketGetString(rawContactValues, columnName);
}
}  // namespace Table

namespace Profile {
void ProfilePubilc(const uint8_t *data, size_t size)
{
    int oldVersion = static_cast<int32_t>(*data) % OHOS::Contacts::DATABASE_NEW_VERSION;
    int newVersion = OHOS::Contacts::DATABASE_NEW_VERSION;
    OHOS::Contacts::SqliteOpenHelperProfileCallback sopc;
    if (gRdb != nullptr) {
        OHOS::NativeRdb::RdbStore &store = *gRdb.get();
        sopc.OnCreate(store);
        sopc.OnUpgrade(store, oldVersion, newVersion);
        sopc.OnDowngrade(store, oldVersion, newVersion);
    }
}
}  // namespace Profile

namespace Raw {
void RawPubilc(const uint8_t *data, size_t size)
{
    int32_t num = static_cast<int32_t>(*data) % 1;
    int64_t id64 = 1;
    OHOS::Contacts::RawContacts rc;
    std::string columnName = "company";
    std::string columnValue = "hw";
    std::vector<std::string> whereArgs;
    OHOS::NativeRdb::ValuesBucket rawContactValues;
    rawContactValues.PutString(columnName, columnValue);
    if (gRdb != nullptr) {
        rc.InsertRawContact(gRdb, id64, rawContactValues);
        rc.UpdateRawContact(gRdb, rawContactValues, "display_name", whereArgs);
        int id = num + 1;
        rc.UpdateRawContactById(id, "1", gRdb, rawContactValues);
        rc.GetDeleteContactIdByAccountId(gRdb, 1);
        rc.GetDeleteRawContactIdByAccountId(gRdb, 1);
        rc.DeleteRawcontactByRawId(gRdb, 1);
    }
}
}  // namespace Raw
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
        {Contacts::Test::Account::AccountPublic, "AccountPublic"},
        {Contacts::Test::Datashare::DatasharePubilc, "DatasharePubilc"},
        {Contacts::Test::Helper::HelperPubilc, "HelperPubilc"},
        {Contacts::Test::Table::TablePubilc, "TablePubilc"},
        {Contacts::Test::Profile::ProfilePubilc, "ProfilePubilc"},
        {Contacts::Test::Raw::RawPubilc, "RawPubilc"},
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