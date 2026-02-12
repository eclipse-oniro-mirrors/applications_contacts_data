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
#define private public
#include "access_token.h"
#include "accesstoken_kit.h"
#include "access_token_error.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "datashare_predicates.h"
#include "datashare_helper.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "singleton.h"
#include "voicemailfuzz_test.h"
#include "voicemail_ability.h"
#include "contacts_database.h"
#include "voicemail_database.h"
#include "calllog_database.h"
#include "predicates_convert.h"
#include "rdb_utils.h"
#include "common_tool_type.h"

using namespace OHOS::Security::AccessToken;
namespace VoiceMail {
namespace Sa {
namespace Test {
using namespace OHOS::Contacts;
static constexpr const char *VOICEMAIL = "datashare:///com.ohos.voicemailability/calls/voicemail";
static OHOS::AbilityRuntime::VoiceMailAbility gVoicemailAbility;

void NativeTokenGet(const uint8_t *data, size_t size)
{
    const char *perms[1] = {"ohos.permission.MANAGE_VOICEMAIL"};

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "voicemail_sa_fuzzer",
        .aplStr = "system_core",
    };
    uint64_t tokenId = 0;
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    AccessTokenKit::ReloadNativeTokenInfo();
}

void SaInsert(const uint8_t *data, size_t size)
{
    std::string detailInfo(reinterpret_cast<const char *>(data), size);
    OHOS::DataShare::DataShareValuesBucket value;
    value.Put("detail_info", detailInfo);
    OHOS::Uri uriVoicemail(VOICEMAIL);
    gVoicemailAbility.Insert(uriVoicemail, value);
}

void SaBatchInsert(const uint8_t *data, size_t size)
{
    std::vector<OHOS::DataShare::DataShareValuesBucket> values;
    std::string detailInfo("123");
    int count = CommonToolType::UnsignedLongIntToInt(size % 5);
    for (int i = 0; i < count; i++) {
        OHOS::DataShare::DataShareValuesBucket value;
        value.Put("detail_info", detailInfo.append(std::to_string(i)));
        values.push_back(value);
    }
    OHOS::Uri uriVoicemail(VOICEMAIL);
    std::string str(reinterpret_cast<const char *>(data), size);
    OHOS::Uri uri(str.c_str());
    gVoicemailAbility.DataBaseNotifyChange(0, uri);
    gVoicemailAbility.BatchInsert(uriVoicemail, values);
}

void SaUpdate(const uint8_t *data, size_t size)
{
    std::string detailInfo("123");
    OHOS::DataShare::DataShareValuesBucket value;
    value.Put("detail_info", detailInfo);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("account_name", detailInfo);
    OHOS::Uri uriVoicemail(VOICEMAIL);
    std::string str(reinterpret_cast<const char *>(data), size);
    OHOS::Uri uri(str.c_str());
    gVoicemailAbility.DataBaseNotifyChange(0, uri);
    gVoicemailAbility.Update(uriVoicemail, predicates, value);
}

void SaDelete(const uint8_t *data, size_t size)
{
    std::string detailInfo("123");
    OHOS::DataShare::DataShareValuesBucket value;
    value.Put("detail_info", detailInfo);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("account_name", detailInfo);
    OHOS::Uri uriVoicemail(VOICEMAIL);
    std::string str(reinterpret_cast<const char *>(data), size);
    OHOS::Uri uri(str.c_str());
    gVoicemailAbility.DataBaseNotifyChange(0, uri);
    gVoicemailAbility.Delete(uriVoicemail, predicates);
}

void SaQuery(const uint8_t *data, size_t size)
{
    std::string detailInfo("123");
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo("account_name", detailInfo);
    OHOS::Uri uriVoicemail(VOICEMAIL);
    std::vector<std::string> columns;
    int count = CommonToolType::UnsignedLongIntToInt(size % 5);
    for (int i = 0; i < count; i++) {
        columns.push_back(detailInfo.append(std::to_string(i)));
    }
    OHOS::DataShare::DatashareBusinessError businessError;
    std::string str(reinterpret_cast<const char *>(data), size);
    OHOS::Uri uri(str.c_str());
    gVoicemailAbility.DataBaseNotifyChange(0, uri);
    gVoicemailAbility.Query(uriVoicemail, predicates, columns, businessError);
}

}  // namespace Test
}  // namespace Sa
}  // namespace Contacts

namespace VoiceMail {
namespace DataBase {
namespace   Test {
void DbDelete(const uint8_t *data, size_t size)
{
    std::string detailInfo(reinterpret_cast<const char *>(data), size);
    OHOS::NativeRdb::RdbPredicates predicates(detailInfo);
    OHOS::Contacts::VoiceMailDataBase::GetInstance()->DeleteVoiceMail(predicates);
}

void DbQuery(const uint8_t *data, size_t size)
{
    std::string detailInfo(reinterpret_cast<const char *>(data), size);
    std::vector<std::string> columns;
    columns.push_back("quicksearch_key");
    columns.push_back("phone_number");
    columns.push_back("display_name");
    OHOS::NativeRdb::RdbPredicates predicates(detailInfo);
    OHOS::Contacts::VoiceMailDataBase::GetInstance()->Query(predicates, columns);
}

void DbBeginTransaction(const uint8_t *data, size_t size)
{
    OHOS::Contacts::VoiceMailDataBase::GetInstance()->BeginTransaction();
}

void DbCommit(const uint8_t *data, size_t size)
{
    OHOS::Contacts::VoiceMailDataBase::GetInstance()->Commit();
}

void DbRollBack(const uint8_t *data, size_t size)
{
    OHOS::Contacts::VoiceMailDataBase::GetInstance()->RollBack();
}
}  // namespace Test
}  // namespace DataBase
}  // namespace Contacts


/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return -1;
    }

    /* Run your code on data */
    struct FunctionInfo {
        void (*func)(const uint8_t *, size_t);
        const char *func_name;
    };

    const FunctionInfo functions[] = {
        {VoiceMail::Sa::Test::SaInsert, "SaInsert"},
        {VoiceMail::Sa::Test::SaBatchInsert, "SaBatchInsert"},
        {VoiceMail::Sa::Test::SaUpdate, "SaUpdate"},
        {VoiceMail::Sa::Test::SaDelete, "SaDelete"},
        {VoiceMail::Sa::Test::SaQuery, "SaQuery"},
        {VoiceMail::Sa::Test::NativeTokenGet, "NativeTokenGet"},
        {VoiceMail::Sa::Test::SaInsert, "SaInsert"},
        {VoiceMail::Sa::Test::SaBatchInsert, "SaBatchInsert"},
        {VoiceMail::Sa::Test::SaUpdate, "SaUpdate"},
        {VoiceMail::Sa::Test::SaDelete, "SaDelete"},
        {VoiceMail::Sa::Test::SaQuery, "SaQuery"},
        {VoiceMail::DataBase::Test::DbBeginTransaction, "DbBeginTransaction"},
        {VoiceMail::DataBase::Test::DbCommit, "DbCommit"},
        {VoiceMail::DataBase::Test::DbQuery, "DbQuery"},
        {VoiceMail::DataBase::Test::DbBeginTransaction, "DbBeginTransaction"},
        {VoiceMail::DataBase::Test::DbDelete, "DbDelete"},
        {VoiceMail::DataBase::Test::DbRollBack, "DbRollBack"},
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