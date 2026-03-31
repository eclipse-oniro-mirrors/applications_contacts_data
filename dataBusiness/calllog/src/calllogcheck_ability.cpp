/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "calllogcheck_ability.h"

#include <mutex>

#include "board_report_util.h"
#include "common.h"
#include "contacts_columns.h"
#include "contacts_common_event.h"
#include "contacts_datashare_stub_impl.h"
#include "datashare_ext_ability_context.h"
#include "datashare_predicates.h"
#include "file_utils.h"
#include "log_utils.h"
#include "predicates_convert.h"
#include "rdb_predicates.h"
#include "rdb_utils.h"
#include "sql_analyzer.h"
#include "uri_utils.h"
#include "privacy_contacts_manager.h"
#include "calllog_common.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
std::mutex g_mutex;
}
std::shared_ptr<Contacts::CallLogDataBase> CallLogCheckAbility::callLogDataBase_ = nullptr;
std::map<std::string, int> CallLogCheckAbility::uriValueMap_ = {
    {"/com.ohos.calllogcheckability/calls/calllog", Contacts::CALLLOG}
};

CallLogCheckAbility::CallLogCheckAbility() : DataShareExtAbility()
{
}

CallLogCheckAbility::~CallLogCheckAbility()
{
}

sptr<IRemoteObject> CallLogCheckAbility::OnConnect(const AAFwk::Want &want)
{
    HILOG_INFO("CallLogCheckAbility OnConnect begin, ts = %{public}ld", time(NULL));
    Extension::OnConnect(want);
    sptr<DataShare::ContactsDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::ContactsDataShareStubImpl();
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetCallLogCheckAbility(std::static_pointer_cast<CallLogCheckAbility>(shared_from_this()));
    HILOG_INFO("CallLogCheckAbility OnConnect end, ts = %{public}ld", time(NULL));
    Contacts::CallLogDataBase::GetInstance();
    return remoteObject->AsObject();
}

/**
 * @brief CallLogCheckAbility Query database
 *
 * @param uri Determine the data table name based on the URI
 * @param columns Columns returned by query
 * @param predicates Query the data values of the condition
 *
 * @return Query database results
 */
std::shared_ptr<DataShare::DataShareResultSet> CallLogCheckAbility::Query(const Uri &uri,
    const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    if (!(Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::CHECK_CALL_LOG))) {
        HILOG_ERROR("Permission denied!");
        return nullptr;
    }
    HILOG_INFO("CallLogCheckAbility ====>Query start, ts = %{public}ld", time(NULL));
    g_mutex.lock();
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    Contacts::PredicatesConvert predicatesConvert;
    std::shared_ptr<OHOS::NativeRdb::ResultSet> result;
    OHOS::Uri uriTemp = uri;
    Contacts::UriUtils uriUtils;
    int parseCode = uriUtils.UriParse(uriTemp, uriValueMap_);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<std::string> columnsTemp = columns;
    bool isUriMatch = true;
    switch (parseCode) {
        case Contacts::CALLLOG:
            rdbPredicates = predicatesConvert.ConvertPredicates(Contacts::CallsTableName::CALLLOG, dataSharePredicates);
            AddQueryNotPrivacyCondition(rdbPredicates);
            result = callLogDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        default:
            isUriMatch = false;
            HILOG_ERROR("CallLogCheckAbility ====>no match uri action");
            break;
    }
    if (!isUriMatch) {
        g_mutex.unlock();
        return nullptr;
    }
    auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(result);
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult =
        std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
    g_mutex.unlock();
    int resultCount;
    sharedPtrResult->GetRowCount(resultCount);
    HILOG_INFO("CallLogCheckAbility ====>Query end, resultCount = %{public}d, ts = %{public}ld", 
        resultCount, time(NULL));
    return sharedPtrResult;
}

void CallLogCheckAbility::OnStart(const Want &want)
{
    HILOG_INFO("CallLogCheckAbility OnStart begin, ts = %{public}ld", time(NULL));
    Extension::OnStart(want);
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context != nullptr) {
        context->SwitchArea(0);
        std::string basePath = context->GetDatabaseDir();
        Contacts::ContactsPath::RDB_PATH = basePath + "/";
        Contacts::ContactsPath::RDB_BACKUP_PATH = basePath + "/backup/";
    }
}

int CallLogCheckAbility::UriParse(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    int parseCode = uriUtils.UriParse(uri, uriValueMap_);
    return parseCode;
}

/**
 * 添加查询非隐私记录的信息条件
 * @param rdbPredicates
 */
void CallLogCheckAbility::AddQueryNotPrivacyCondition(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    std::string whereSql = rdbPredicates.GetWhereClause();
    if (whereSql.find(Contacts::CallLogColumns::PRIVACY_TAG) == std::string::npos) {
        HILOG_INFO("query calllog, not has privacy_tag conditoin, add condition, ts = %{public}ld", time(NULL));
        // 不存在是否隐私记录条件，需要设置为非隐私
        rdbPredicates.And();
        rdbPredicates.NotEqualTo(
            Contacts::CallLogColumns::PRIVACY_TAG, static_cast<int32_t>(Contacts::PrivacyTag::CALL_LOG_PRIVACY_TAG));
    }
}
} // namespace AbilityRuntime
} // namespace OHOS