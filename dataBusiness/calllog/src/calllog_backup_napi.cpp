/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#define MLOG_TAG "CallLogBackupNapi"

#include "calllog_backup_napi.h"
#include "napi/native_api.h"
#include "application_context.h"
#include "js_native_api.h"
#include "calllog_database.h"
#include "contacts_database.h"


namespace OHOS {
namespace Contacts {

napi_value CallLogBackupNapi::Init(napi_env env, napi_value exports)
{
    HILOG_INFO("Init, CallLogBackupNapi has been used, ts = %{public}lld", (long long) time(NULL));
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getCallLogTotal", CallLogBackupNapi::JSGetCallLogTotal),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value CallLogBackupNapi::JSGetCallLogTotal(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JSGetCallLogTotal start, ts = %{public}lld", (long long) time(NULL));
    napi_value result = nullptr;
    static std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    if (contactsDataBase == nullptr || contactsDataBase->contactStore_ == nullptr) {
        HILOG_ERROR("ContactsDataBase is nullptr or ContactsDataBase->contactStore_ is nullptr");
        napi_create_int32(env, -1, &result);
        return result;
    }
    std::string sql = "SELECT * FROM ";
    sql.append(CallsTableName::CALLLOG);
    std::vector<std::string> selectionArgs;
    auto resultSet = contactsDataBase->contactStore_->QuerySql(sql, selectionArgs);
    if (resultSet == nullptr) {
        HILOG_ERROR("JSGetCallLogTotal QuerySqlResult is null");
        return result;
    }
    int count = 0;
    resultSet->GetRowCount(count);
    resultSet->Close();

    napi_create_int32(env, count, &result);
    return result;
}

} // namespace Contacts
} // namespace OHOS
