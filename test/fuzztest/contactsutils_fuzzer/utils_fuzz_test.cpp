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
#include <cstdio>
#include "utils_fuzz_test.h"
#define private public
#include "contacts_common_event.h"
#include "contacts_json_utils.h"
#include "contacts_path.h"
#include "contacts_string_utils.h"
#include "file_utils.h"
#include "merge_utils.h"
#include "predicates_convert.h"
#include "sql_analyzer.h"
#include "telephony_permission.h"
#include "uri_utils.h"
#undef private

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

void StringFuzz(const uint8_t *data, size_t size)
{
    std::string dataStr(reinterpret_cast<const char *>(data), size);
    std::string str = "  1234  " + dataStr;
    OHOS::Contacts::ContactsStringUtils csu;
    csu.SplitStr("12341", "1");
    csu.Trim(str);
}

void FileFuzz(const uint8_t *data, size_t size)
{
    std::string filePath = "/data/tmp/xxdfdaeee.log";
    OHOS::Contacts::FileUtils fu;
    fu.IsFolderExist(filePath);
    fu.Mkdir(filePath);
    remove(filePath.c_str());
    std::string dataStr(reinterpret_cast<const char *>(data), size);
    fu.Mkdir(dataStr);
    remove(dataStr.c_str());
}

void MergeFuzz(const uint8_t *data, size_t size)
{
    std::set<int> rawIds;
    std::set<std::string> sets;
    OHOS::Contacts::MergeUtils mu;
    OHOS::NativeRdb::ValuesBucket value;
    rawIds.insert(static_cast<int32_t>(*data));
    mu.QueryRawContactByType(gRdb, 1, 1);
    mu.QueryDataExecute(gRdb, rawIds, 1);
    mu.SetEqual(sets, sets);
    mu.GetRawIdsByRawId(gRdb, 1, rawIds);
}

void PredicatesConvertFuzz(const uint8_t *data, size_t size)
{
    std::string tableName(reinterpret_cast<const char *>(data), size);
    OHOS::DataShare::DataSharePredicates dataSharePredicates;
    OHOS::NativeRdb::RdbPredicates oldRdbPredicates("");
    OHOS::Contacts::PredicatesConvert pc;
    pc.ConvertPredicates(tableName, dataSharePredicates);
    pc.CopyPredicates(tableName, oldRdbPredicates);
}
void SqlAnalyzerFuzz(const uint8_t *data, size_t size)
{
    char ch = static_cast<char>(*data);
    std::size_t pos = 0;
    std::string sql = "SELECT * FROM raw_contact";
    OHOS::NativeRdb::ValuesBucket value;
    OHOS::Contacts::SqlAnalyzer sa;
    sa.CheckValuesBucket(value);
    sa.FindIllegalWords(sql);
    sa.StrCheck(ch, sql.length(), sql, pos);
    sa.CharCheck(ch, sql, pos);
    sa.CheckColumnExists(*gRdb.get(), "raw_contact", "photo_file_id");
}

void TelephonyPermissionFuzz(const uint8_t *data, size_t size)
{
    std::string key(reinterpret_cast<const char *>(data), size);
    std::string permissionName = "ohos.permission.READ_CONTACTS";
    OHOS::Telephony::TelephonyPermission::getCountGroup(key);
    OHOS::Telephony::TelephonyPermission::CheckPermission(permissionName);
}
void UriFuzz(const uint8_t *data, size_t size)
{
    std::string str = "datashare:///com.ohos.contactsdataability/contacts/contact";
    std::string split(reinterpret_cast<const char *>(data), size);
    Uri uri("datashare:///com.ohos.contactsdataability/contacts/contact");
    std::map<std::string, int> keyMap;
    OHOS::Contacts::UriUtils uu;
    uu.split(str, split);
    uu.getQueryParameter(uri);
    uu.UriParse(uri, keyMap);
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
        {Contacts::Test::StringFuzz, "StringFuzz"},
        {Contacts::Test::FileFuzz, "FileFuzz"},
        {Contacts::Test::MergeFuzz, "MergeFuzz"},
        {Contacts::Test::PredicatesConvertFuzz, "PredicatesConvertFuzz"},
        {Contacts::Test::SqlAnalyzerFuzz, "SqlAnalyzerFuzz"},
        {Contacts::Test::TelephonyPermissionFuzz, "TelephonyPermissionFuzz"},
        {Contacts::Test::UriFuzz, "UriFuzz"},
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

        // 调用函数
        functions[i].func(segment_data, segment_size);
    }
    return 0;
}