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

#include <vector>
#include <string>
#include <unistd.h>
#define private public
#include "calllog_ability_fuzz_test.h"
#include "calllog_ability.h"
#include "calllog_database.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "access_token_error.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "uri.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "access_token_error.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "contacts_columns.h"
#include "datashare_predicates.h"
#include "datashare_helper.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "singleton.h"
#include "rdb_utils.h"
#undef private

using namespace OHOS::Security::AccessToken;

namespace CallLogFuzz {
namespace Test {
static OHOS::Contacts::CallLogDataBase gCDB;
std::shared_ptr<OHOS::NativeRdb::RdbStore> gRdb = nullptr;
void ContactsStore()
{
    int getDataBasePathErrCode = OHOS::NativeRdb::E_OK;
    // 调用rdb方法拼接数据库路径
    std::string databaseName = OHOS::NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(
        "/data/storage/el5/database", "calls.db", getDataBasePathErrCode);
    OHOS::Contacts::SqliteOpenHelperContactCallback sqliteOpenHelperCallback;
    OHOS::NativeRdb::RdbStoreConfig config(databaseName);
    config.SetBundleName("com.ohos.contactsdataability");
    config.SetName("calls.db");
    config.SetArea(0);
    config.SetSearchable(true);
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S3);
    config.SetAllowRebuild(true);
    config.SetAutoClean(false);
    int errCode = OHOS::NativeRdb::E_OK;
    gRdb = OHOS::NativeRdb::RdbHelper::GetRdbStore(
        config, OHOS::Contacts::DATABASE_CONTACTS_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
}

int CallLogCallback(const uint8_t *data, size_t size)
{
    OHOS::Contacts::SqliteOpenHelperCallLogCallback sqliteOpenHelperCallback;
    OHOS::NativeRdb::RdbStore& store = *gRdb.get();
    sqliteOpenHelperCallback.OnCreate(store);
    int num = static_cast<int32_t>(*data);
    int oldVersion = num % OHOS::Contacts::DATABASE_NEW_VERSION;
    int newVersion = OHOS::Contacts::DATABASE_CONTACTS_OPEN_VERSION;
    sqliteOpenHelperCallback.OnUpgrade(store, oldVersion, newVersion);
    sqliteOpenHelperCallback.OnDowngrade(store, oldVersion, newVersion);
    return 0;
}

int CallLogDataBasePublic(const uint8_t *data, size_t size)
{
    OHOS::NativeRdb::ValuesBucket value;
    std::vector<OHOS::NativeRdb::ValuesBucket> values;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<std::string> columns;
    std::string str(reinterpret_cast<const char *>(data), size);
    columns.push_back(str);
    std::string columnName("display_name");
    std::string columnValue("as");
    value.PutString(columnName, columnValue);
    values.push_back(value);
    gCDB.BatchInsertCallLog(values);
    gCDB.UpdateCallLog(value, rdbPredicates);
    gCDB.DeleteCallLog(rdbPredicates);
    gCDB.Query(rdbPredicates, columns);
    gCDB.BeginTransaction();
    gCDB.Commit();
    gCDB.RollBack();
    gCDB.QueryContactsByCallsGetSubPhoneNumberSql(str);
    gCDB.QueryContactsByCallsInsertValues(value, str, str, str, "calllog");
    return 0;
}

int CallLogDataBasePrivate(const uint8_t *data, size_t size)
{
    OHOS::NativeRdb::ValuesBucket value;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultSet;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<std::string> columns;
    std::string str(reinterpret_cast<const char *>(data), size);
    columns.push_back(str);
    gCDB.NotifyCallLogChange();
    value.PutString(OHOS::Contacts::CallLogColumns::DISPLAY_NAME, "as");
    gCDB.MoveDbFile();
    gCDB.Query(rdbPredicates, columns);
    return 0;
}
}  // namespace Test
}  // namespace Contacts

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return -1;
    }
    CallLogFuzz::Test::ContactsStore();
    
    size_t callbackSize = size / 4;
    const uint8_t* callbackData = data;
    size_t publicSize = size / 4;
    const uint8_t* publicData = data + callbackSize;
    size_t privateSize = size - callbackSize - publicSize;
    const uint8_t* privateData = data + callbackSize + publicSize;

    CallLogFuzz::Test::CallLogCallback(callbackData, callbackSize);
    CallLogFuzz::Test::CallLogDataBasePublic(publicData, publicSize);
    CallLogFuzz::Test::CallLogDataBasePrivate(privateData, privateSize);
    return 0;
}