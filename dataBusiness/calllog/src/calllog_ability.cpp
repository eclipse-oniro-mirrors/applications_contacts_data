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

#include "calllog_ability.h"

#include <mutex>

#include "ability_loader.h"
#include "common.h"
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
    HILOG_INFO("CallLogAbility %{public}s begin.", __func__);
    Extension::OnConnect(want);
    sptr<DataShare::ContactsDataShareStubImpl> remoteObject = new (std::nothrow) DataShare::ContactsDataShareStubImpl(
        std::static_pointer_cast<CallLogAbility>(shared_from_this()));
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    HILOG_INFO("CallLogAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void CallLogAbility::OnStart(const Want &want)
{
    HILOG_INFO("CallLogAbility %{public}s begin.", __func__);
    Extension::OnStart(want);
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context != nullptr) {
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
    mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsBeginTransactionOK fail");
        mutex.unlock();
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
    mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsCommitOk fail");
        mutex.unlock();
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
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("CallLogAbility CheckValuesBucket is error");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    int ret = callLogDataBase_->BeginTransaction();
    if (!IsBeginTransactionOK(ret, g_mutex)) {
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    int resultId = InsertExecute(uri, valuesBucket);
    if (resultId == Contacts::OPERATION_ERROR) {
        callLogDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::OPERATION_ERROR;
    }
    ret = callLogDataBase_->Commit();
    if (!IsCommitOk(ret, g_mutex)) {
        callLogDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
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
    int rowRet = Contacts::RDB_EXECUTE_FAIL;
    unsigned int size = values.size();
    if (size <= 0) {
        HILOG_ERROR("BatchInsert value is error");
        return rowRet;
    }
    g_mutex.lock();
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    int ret = callLogDataBase_->BeginTransaction();
    if (!IsBeginTransactionOK(ret, g_mutex)) {
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    int count = 0;
    for (unsigned int i = 0; i < size; i++) {
        ++count;
        DataShare::DataShareValuesBucket rawContactValues = values[i];
        OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(rawContactValues);
        int code = InsertExecute(uri, value);
        if (code == Contacts::RDB_EXECUTE_FAIL) {
            callLogDataBase_->RollBack();
            g_mutex.unlock();
            return code;
        }
        if (count % Contacts::BATCH_INSERT_COUNT == 0) {
            int markRet = callLogDataBase_->Commit();
            int beginRet = callLogDataBase_->BeginTransaction();
            if (!IsCommitOk(markRet, g_mutex) || !IsBeginTransactionOK(beginRet, g_mutex)) {
                callLogDataBase_->RollBack();
                g_mutex.unlock();
                return Contacts::RDB_EXECUTE_FAIL;
            }
        }
    }
    int markRet = callLogDataBase_->Commit();
    if (!IsCommitOk(markRet, g_mutex)) {
        callLogDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
    return Contacts::RDB_EXECUTE_OK;
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
    DataBaseNotifyChange(Contacts::CONTACT_UPDATE, uri);
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
std::shared_ptr<DataShare::DataShareResultSet> CallLogAbility::Query(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns)
{
    HILOG_INFO("CallLogAbility ====>Query start");
    callLogDataBase_ = Contacts::CallLogDataBase::GetInstance();
    if (callLogDataBase_ == nullptr) {
        HILOG_ERROR("AbsSharedResultSet is nullptr");
        return nullptr;
    }
    Contacts::PredicatesConvert predicatesConvert;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> result;
    OHOS::Uri uriTemp = uri;
    Contacts::UriUtils uriUtils;
    int parseCode = uriUtils.UriParse(uriTemp, uriValueMap_);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<std::string> columnsTemp = columns;
    bool isUriMatch = true;
    switch (parseCode) {
        case Contacts::CALLLOG:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::CALLLOG, dataSharePredicates);
            result = callLogDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        default:
            isUriMatch = false;
            HILOG_ERROR("CallLogAbility ====>no match uri action");
            break;
    }
    if (!isUriMatch) {
        return nullptr;
    }
    if (result == nullptr) {
        HILOG_ERROR("AbsSharedResultSet is nullptr");
        return nullptr;
    }
    auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(result);
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult =
        std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
    HILOG_INFO("CallLogAbility ====>Query end");
    return sharedPtrResult;
}

void CallLogAbility::DataBaseNotifyChange(int code, Uri uri)
{
    Contacts::ContactsCommonEvent::SendCallLogChange(code);
}
} // namespace AbilityRuntime
} // namespace OHOS