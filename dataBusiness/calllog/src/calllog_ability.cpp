/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "calllog_ability.h"

#include <mutex>

#include "board_report_util.h"
#include "common.h"
#include "contacts_columns.h"
#include "contacts_common_event.h"
#include "contacts_datashare_stub_impl.h"
#include "datashare_ext_ability_context.h"
#include "datashare_predicates.h"
#include "file_utils.h"
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
std::shared_ptr<Contacts::CallLogDataBase> CallLogAbility::callLogDataBase_ = nullptr;
std::map<std::string, int> CallLogAbility::uriValueMap_ = {
    {"/com.ohos.calllogability/calls/calllog", Contacts::CALLLOG}
};

CallLogAbility* CallLogAbility::Create()
{
    return new CallLogAbility();
}

CallLogAbility::CallLogAbility() : DataShareExtAbility()
{
}

CallLogAbility::~CallLogAbility()
{
}

sptr<IRemoteObject> CallLogAbility::OnConnect(const AAFwk::Want &want)
{
    HILOG_INFO("CallLogAbility OnConnect begin, ts = %{public}lld", (long long) time(NULL));
    Extension::OnConnect(want);
    sptr<DataShare::ContactsDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::ContactsDataShareStubImpl();
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetCallLogAbility(std::static_pointer_cast<CallLogAbility>(shared_from_this()));
    HILOG_INFO("CallLogAbility OnConnect end, ts = %{public}lld", (long long) time(NULL));
    Contacts::CallLogDataBase::GetInstance();
    return remoteObject->AsObject();
}

void CallLogAbility::OnStart(const Want &want)
{
    HILOG_INFO("CallLogAbility OnStart begin, ts = %{public}lld", (long long) time(NULL));
    Extension::OnStart(want);
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context != nullptr) {
        context->SwitchArea(0);
        std::string basePath = context->GetDatabaseDir();
        Contacts::ContactsPath::RDB_PATH = basePath + "/";
        Contacts::ContactsPath::RDB_BACKUP_PATH = basePath + "/backup/";
    }
}

int CallLogAbility::UriParse(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    int parseCode = uriUtils.UriParse(uri, uriValueMap_);
    return parseCode;
}

std::string CallLogAbility::UriParseBatchParam(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    std::string isFromBatch;
    std::map<std::string, std::string> mapParams;
    mapParams = uriUtils.getQueryParameter(uri);
    if (!mapParams.empty()) {
        isFromBatch = mapParams["isFromBatch"];
    }
    return isFromBatch;
}

/**
 * @brief CallLogAbility BeginTransaction emptiness problems
 *
 * @param code the return number of BeginTransaction
 * @param mutex transmission parameter : lock
 *
 * @return BeginTransaction emptiness true or false
 */
bool CallLogAbility::IsBeginTransactionOK(int code, std::mutex &mutex)
{
    bool ret = mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsBeginTransactionOK failed");
        if (ret) {
            mutex.unlock();
        }
        return false;
    }
    return true;
}

/**
 * @brief CallLogAbility Commit emptiness problems
 *
 * @param code the return number of Commit
 * @param mutex transmission parameter : lock
 *
 * @return Commit emptiness true or false
 */
bool CallLogAbility::IsCommitOk(int code, std::mutex &mutex)
{
    bool ret = mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsCommitOk failed");
        if (ret) {
            mutex.unlock();
        }
        return false;
    }
    return true;
}

/**
 * @brief CallLogAbility Insert database
 *
 * @param uri Determine the data table name based on the URI
 * @param value Insert the data value of the database
 *
 * @return Insert database results code
 */
int CallLogAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    HILOG_INFO("CallLogAbility Insert begin, ts = %{public}lld", (long long) time(NULL));
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CALL_LOG)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("CallLogAbility CheckValuesBucket is error");
        return Contacts::RDB_EXECUTE_FAIL;
    }

    if (uri.ToString().find("isPrivacy") == std::string::npos) {
        if (Contacts::PrivacyContactsManager::GetInstance()->TryToMigratePrivateCallLog(value)) {
            HILOG_INFO("TryToMigratePrivateCallLog success");
            int callDirection = -1;
            if (valuesBucket.HasColumn(Contacts::CallLogColumns::CALL_DIRECTION)) {
                OHOS::NativeRdb::ValueObject value;
                valuesBucket.GetObject(Contacts::CallLogColumns::CALL_DIRECTION, value);
                value.GetInt(callDirection);
            }
            Contacts::BoardReportUtil::InsertCallLogReport(Contacts::ExecuteResult::SUCCESS, "1",
                callDirection, "Update PrivacyTag success", 100);
            return Contacts::OPERATION_OK;
        }
        HILOG_WARN("TryToMigratePrivateCallLog failed, continue");
    }

    g_mutex.lock();
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    int resultId = InsertExecute(uri, valuesBucket);
    if (resultId == Contacts::OPERATION_ERROR) {
        g_mutex.unlock();
        HILOG_ERROR("CallLogAbility InsertExecute error,result:%{public}d", resultId);
        return Contacts::OPERATION_ERROR;
    }
    g_mutex.unlock();
    CheckNotifyCallLogChange(valuesBucket);
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
    HILOG_INFO("CallLogAbility Insert end, ts = %{public}lld", (long long) time(NULL));
    return resultId;
}

int CallLogAbility::InsertExecute(const Uri &uri, const OHOS::NativeRdb::ValuesBucket &value)
{
    int rowId = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParse(uriTemp);
    switch (parseCode) {
        case Contacts::CALLLOG:
            rowId = callLogDataBase_->InsertCallLog(value);
            break;
        default:
            HILOG_ERROR("CallLogAbility ====>no match uri action");
            break;
    }
    return rowId;
}

int CallLogAbility::BatchInsertSplit(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    int ret = callLogDataBase_->BeginTransaction();
    if (!IsBeginTransactionOK(ret, g_mutex)) {
        g_mutex.unlock();
        HILOG_ERROR("BatchInsertSplit BeginTransaction failed");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    int retCode = Contacts::RDB_EXECUTE_OK;
    int count = 0;
    unsigned int size = values.size();
    HILOG_INFO("BatchInsertSplit start insert, ts = %{public}lld", (long long) time(NULL));
    for (unsigned int i = 0; i < size; i++) {
        ++count;
        DataShare::DataShareValuesBucket rawContactValues = values[i];
        OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(rawContactValues);
        int code = InsertExecute(uri, value);
        if (code == Contacts::RDB_EXECUTE_FAIL) {
            callLogDataBase_->RollBack();
            g_mutex.unlock();
            retCode = code;
            return retCode;
        }
        if (count % Contacts::BATCH_INSERT_COUNT == 0) {
            int markRet = callLogDataBase_->Commit();
            int beginRet = callLogDataBase_->BeginTransaction();
            if (!IsCommitOk(markRet, g_mutex) || !IsBeginTransactionOK(beginRet, g_mutex)) {
                callLogDataBase_->RollBack();
                g_mutex.unlock();
                retCode = Contacts::RDB_EXECUTE_FAIL;
                return retCode;
            }
        }
    }
    int markRet = callLogDataBase_->Commit();
    if (!IsCommitOk(markRet, g_mutex)) {
        callLogDataBase_->RollBack();
        g_mutex.unlock();
        HILOG_ERROR("BatchInsertSplit RDB_EXECUTE_FAIL");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    HILOG_INFO("BatchInsertSplit end insert, ts = %{public}lld", (long long) time(NULL));
    return retCode;
}

/**
 * @brief CallLogAbility BatchInsert database
 *
 * @param uri Determine the data table name based on the URI
 * @param value Insert the data values of the database
 *
 * @return Insert database results code
 */
int CallLogAbility::BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CALL_LOG)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    int rowRet = Contacts::RDB_EXECUTE_FAIL;
    unsigned int size = values.size();
    if (size < 1) {
        HILOG_ERROR("BatchInsert value is error");
        return rowRet;
    }
    OHOS::Uri uriTemp = uri;
    std::vector<DataShare::DataShareValuesBucket> insertValues = values;
    std::string isFromBatch = UriParseBatchParam(uriTemp);
    if (isFromBatch == "true") {
        std::vector<DataShare::DataShareValuesBucket> unprocessedValues;
        if (uri.ToString().find("isPrivacy") == std::string::npos) {
            Contacts::PrivacyContactsManager::GetInstance()->ProcessPrivacyCallLog(insertValues, unprocessedValues);
        } else {
            unprocessedValues = insertValues;
        }
        return BatchInsertByMigrate(uri, unprocessedValues);
    }
    g_mutex.lock();
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    int retCode = BatchInsertSplit(uri, insertValues);
    if (retCode != Contacts::RDB_EXECUTE_OK) {
        return retCode;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
    callLogDataBase_->NotifyCallLogChange();
    HILOG_INFO("BatchInsert end DataBaseNotifyChange, ts = %{public}lld", (long long) time(NULL));
    return Contacts::RDB_EXECUTE_OK;
}

/**
 * @brief CallLogAbility BatchInsertByMigrate database
 *
 * @param uri Determine the data table name based on the URI
 * @param value Insert the data values of the database
 *
 * @return BatchInsertByMigrate database results code
 */
int CallLogAbility::BatchInsertByMigrate(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    HILOG_INFO("CallLogAbility BatchInsertByMigrate begin, ts = %{public}lld", (long long) time(NULL));
    std::vector<OHOS::NativeRdb::ValuesBucket> batchInsertValues;
    unsigned int size = values.size();
    for (unsigned int i = 0; i < size; i++) {
        DataShare::DataShareValuesBucket valuesElement = values[i];
        OHOS::NativeRdb::ValuesBucket callDataValues = RdbDataShareAdapter::RdbUtils::ToValuesBucket(valuesElement);
        batchInsertValues.push_back(callDataValues);
    }
    g_mutex.lock();
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    int code = callLogDataBase_->BatchInsertCallLog(batchInsertValues);
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
    HILOG_INFO("CallLogAbility BatchInsertByMigrate end, ts = %{public}lld", (long long) time(NULL));
    return code;
}

/**
 * @brief CallLogAbility Update database
 *
 * @param uri Determine the data table name based on the URI
 * @param predicates Update the data value of the condition
 *
 * @return Update database results code
 */
int CallLogAbility::Update(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, const DataShare::DataShareValuesBucket &value)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CALL_LOG)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("CallLogAbility CheckValuesBucket is error");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    Contacts::PredicatesConvert predicatesConvert;
    int ret = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParse(uriTemp);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (parseCode) {
        case Contacts::CALLLOG:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::CALLLOG, dataSharePredicates);
            ret = callLogDataBase_->UpdateCallLog(valuesBucket, rdbPredicates);
            break;
        default:
            HILOG_ERROR("CallLogAbility ====>no match uri action");
            break;
    }
    g_mutex.unlock();
    if (ret > 0) {
        HILOG_INFO("CallLogAbility ====>update row is %{public}d", ret);
        DataBaseNotifyChange(Contacts::CONTACT_UPDATE, uri);
        callLogDataBase_->NotifyCallLogChange();
    } else {
        HILOG_INFO("CallLogAbility ====>update row is %{public}d, skill notify", ret);
    }
    return ret;
}

/**
 * @brief CallLogAbility Delete database
 *
 * @param uri Determine the data table name based on the URI
 * @param predicates Delete the data values of the condition
 *
 * @return Delete database results code
 */
int CallLogAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CALL_LOG)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    g_mutex.lock();
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    Contacts::PredicatesConvert predicatesConvert;
    int ret = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParse(uriTemp);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (parseCode) {
        case Contacts::CALLLOG:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::CALLLOG, dataSharePredicates);
            ret = callLogDataBase_->DeleteCallLog(rdbPredicates);
            break;
        default:
            HILOG_ERROR("CallLogAbility ====>no match uri action");
            break;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_DELETE, uri);
    callLogDataBase_->NotifyCallLogChange();
    return ret;
}

/**
 * @brief CallLogAbility Query database
 *
 * @param uri Determine the data table name based on the URI
 * @param columns Columns returned by query
 * @param predicates Query the data values of the condition
 *
 * @return Query database results
 */
std::shared_ptr<DataShare::DataShareResultSet> CallLogAbility::Query(const Uri &uri,
    const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::READ_CALL_LOG)) {
        HILOG_ERROR("Permission denied!");
        return nullptr;
    }
    HILOG_INFO("CallLogAbility ====>Query start, ts = %{public}lld", (long long) time(NULL));
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
            HILOG_ERROR("CallLogAbility ====>no match uri action");
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
    HILOG_WARN("CallLogAbility ====>Query end, resultCount = %{public}d, ts = %{public}lld", resultCount, (long long) time(NULL));
    return sharedPtrResult;
}

void CallLogAbility::DataBaseNotifyChange(int code, Uri uri)
{
    HILOG_INFO("Calllog DataBaseNotifyChange start.");
    Contacts::ContactsCommonEvent::SendCallLogChange(code);
}

void CallLogAbility::CheckNotifyCallLogChange(OHOS::NativeRdb::ValuesBucket &valuesBucket)
{
    int call_direction = 1;
    int answer_state = 1;
    if (valuesBucket.HasColumn(Contacts::CallLogColumns::CALL_DIRECTION)) {
        OHOS::NativeRdb::ValueObject value;
        valuesBucket.GetObject(Contacts::CallLogColumns::CALL_DIRECTION, value);
        value.GetInt(call_direction);
    }
    if (valuesBucket.HasColumn(Contacts::CallLogColumns::ANSWER_STATE)) {
        OHOS::NativeRdb::ValueObject value;
        valuesBucket.GetObject(Contacts::CallLogColumns::ANSWER_STATE, value);
        value.GetInt(answer_state);
    }

    if (call_direction == Contacts::CALL_DIRECTION_IN && answer_state == Contacts::CALL_ANSWER_MISSED) {
        callLogDataBase_->NotifyCallLogChange();
    } else {
        HILOG_INFO("skip NotifyCallLogChange, call_direction is  %{public}d, answer_state is  %{public}d",
            call_direction, answer_state);
    }
}

/**
 * 添加查询非隐私记录的信息条件
 * @param rdbPredicates
 */
void CallLogAbility::AddQueryNotPrivacyCondition(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    std::string whereSql = rdbPredicates.GetWhereClause();
    if (whereSql.find(Contacts::CallLogColumns::PRIVACY_TAG) == std::string::npos) {
        HILOG_INFO("query calllog, not has privacy_tag conditoin, add condition, ts = %{public}lld", (long long) time(NULL));
        // 不存在是否隐私记录条件，需要设置为非隐私
        rdbPredicates.And();
        rdbPredicates.NotEqualTo(
            Contacts::CallLogColumns::PRIVACY_TAG, static_cast<int32_t>(Contacts::PrivacyTag::CALL_LOG_PRIVACY_TAG));
    }
}
} // namespace AbilityRuntime
} // namespace OHOS