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

#include "voicemail_ability.h"

#include <mutex>

#include "ability_loader.h"
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

namespace OHOS {
namespace AbilityRuntime {
namespace {
std::mutex g_mutex;
}
std::shared_ptr<Contacts::VoiceMailDataBase> VoiceMailAbility::voiceMailDataBase_ = nullptr;
std::map<std::string, int> VoiceMailAbility::uriValueMap_ = {
    {"/com.ohos.voicemailability/calls/voicemail", Contacts::VOICEMAIL},
    {"/com.ohos.voicemailability/calls/replaying", Contacts::REPLAYING}
};

VoiceMailAbility* VoiceMailAbility::Create()
{
    return new VoiceMailAbility();
}

VoiceMailAbility::VoiceMailAbility() : DataShareExtAbility()
{
}

VoiceMailAbility::~VoiceMailAbility()
{
}

sptr<IRemoteObject> VoiceMailAbility::OnConnect(const AAFwk::Want &want)
{
    HILOG_INFO("VoiceMailAbility %{public}s begin.", __func__);
    Extension::OnConnect(want);
    sptr<DataShare::ContactsDataShareStubImpl> remoteObject = new (std::nothrow) DataShare::ContactsDataShareStubImpl(
        std::static_pointer_cast<VoiceMailAbility>(shared_from_this()));
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    HILOG_INFO("VoiceMailAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void VoiceMailAbility::OnStart(const Want &want)
{
    HILOG_INFO("VoiceMailAbility %{public}s begin.", __func__);
    Extension::OnStart(want);
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context != nullptr) {
        std::string basePath = context->GetDatabaseDir();
        Contacts::ContactsPath::RDB_PATH = basePath + "/";
        Contacts::ContactsPath::RDB_BACKUP_PATH = basePath + "/backup/";
    }
}

/**
 * @brief VoiceMailAbility BeginTransaction emptiness problems
 *
 * @param code the return number of BeginTransaction
 * @param mutex transmission parameter : lock
 *
 * @return BeginTransaction emptiness true or false
 */
bool VoiceMailAbility::IsBeginTransactionOK(int code, std::mutex &mutex)
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
 * @brief VoiceMailAbility Commit emptiness problems
 *
 * @param code the return number of Commit
 * @param mutex transmission parameter : lock
 *
 * @return Commit emptiness true or false
 */
bool VoiceMailAbility::IsCommitOK(int code, std::mutex &mutex)
{
    mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsCommitOK fail");
        mutex.unlock();
        return false;
    }
    return true;
}

/**
 * @brief VoiceMailAbility Insert database
 *
 * @param uri Determine the data table name based on the URI
 * @param value Insert the data value of the database
 *
 * @return Insert database results code
 */
int VoiceMailAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("VoiceMailAbility CheckValuesBucket is error");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    voiceMailDataBase_ = Contacts::VoiceMailDataBase::GetInstance();
    int rowRet = Contacts::RDB_EXECUTE_FAIL;
    int ret = voiceMailDataBase_->BeginTransaction();
    if (!IsBeginTransactionOK(ret, g_mutex)) {
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    rowRet = InsertExecute(uri, valuesBucket);
    if (rowRet == Contacts::OPERATION_ERROR) {
        voiceMailDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::OPERATION_ERROR;
    }
    ret = voiceMailDataBase_->Commit();
    if (!IsCommitOK(ret, g_mutex)) {
        voiceMailDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
    return rowRet;
}

int VoiceMailAbility::UriParse(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    int parseCode = uriUtils.UriParse(uri, uriValueMap_);
    return parseCode;
}

int VoiceMailAbility::InsertExecute(const OHOS::Uri &uri, const OHOS::NativeRdb::ValuesBucket &initialValues)
{
    int rowId = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int code = UriParse(uriTemp);
    switch (code) {
        case Contacts::VOICEMAIL:
            rowId = voiceMailDataBase_->InsertVoiceMail(Contacts::CallsTableName::VOICEMAIL, initialValues);
            break;
        case Contacts::REPLAYING:
            rowId = voiceMailDataBase_->InsertVoiceMail(Contacts::CallsTableName::REPLYING, initialValues);
            break;
        default:
            HILOG_ERROR("VoiceMailAbility ====>no match uri action");
            break;
    }
    return rowId;
}

/**
 * @brief VoiceMailAbility BatchInsert database
 *
 * @param uri Determine the data table name based on the URI
 * @param value Insert the data values of the database
 *
 * @return BatchInsert database results code
 */
int VoiceMailAbility::BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    unsigned int size = values.size();
    if (size <= 0) {
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    voiceMailDataBase_ = Contacts::VoiceMailDataBase::GetInstance();
    int ret = voiceMailDataBase_->BeginTransaction();
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
        if (code == Contacts::OPERATION_ERROR) {
            voiceMailDataBase_->RollBack();
            g_mutex.unlock();
            return code;
        }
        if (count % Contacts::BATCH_INSERT_COUNT == 0) {
            int markRet = voiceMailDataBase_->Commit();
            int beginRet = voiceMailDataBase_->BeginTransaction();
            if (!IsCommitOK(markRet, g_mutex) || !IsBeginTransactionOK(beginRet, g_mutex)) {
                voiceMailDataBase_->RollBack();
                g_mutex.unlock();
                return Contacts::RDB_EXECUTE_FAIL;
            }
        }
    }
    int markRet = voiceMailDataBase_->Commit();
    if (!IsCommitOK(markRet, g_mutex)) {
        voiceMailDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri);
    return Contacts::RDB_EXECUTE_OK;
}

/**
 * @brief VoiceMailAbility Update database
 *
 * @param uri Determine the data table name based on the URI
 * @param predicates Update the data value of the condition
 *
 * @return Update database results code
 */
int VoiceMailAbility::Update(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, const DataShare::DataShareValuesBucket &value)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("VoiceMailAbility CheckValuesBucket is error");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    voiceMailDataBase_ = Contacts::VoiceMailDataBase::GetInstance();
    Contacts::PredicatesConvert predicatesConvert;
    int ret = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int code = UriParse(uriTemp);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (code) {
        case Contacts::VOICEMAIL:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::VOICEMAIL, dataSharePredicates);
            ret = voiceMailDataBase_->UpdateVoiceMail(valuesBucket, rdbPredicates);
            break;
        case Contacts::REPLAYING:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::REPLYING, dataSharePredicates);
            ret = voiceMailDataBase_->UpdateVoiceMail(valuesBucket, rdbPredicates);
            break;
        default:
            HILOG_ERROR("VoiceMailAbility ====>no match uri action");
            break;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_UPDATE, uri);
    return ret;
}

/**
 * @brief VoiceMailAbility Delete database
 *
 * @param uri Determine the data table name based on the URI
 * @param predicates Delete the data values of the condition
 *
 * @return Delete database results code
 */
int VoiceMailAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    g_mutex.lock();
    voiceMailDataBase_ = Contacts::VoiceMailDataBase::GetInstance();
    Contacts::PredicatesConvert predicatesConvert;
    int ret = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParse(uriTemp);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (parseCode) {
        case Contacts::VOICEMAIL:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::VOICEMAIL, dataSharePredicates);
            ret = voiceMailDataBase_->DeleteVoiceMail(rdbPredicates);
            break;
        case Contacts::REPLAYING:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::REPLYING, dataSharePredicates);
            ret = voiceMailDataBase_->DeleteVoiceMail(rdbPredicates);
            break;
        default:
            HILOG_ERROR("VoiceMailAbility ====>no match uri action");
            break;
    }
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_DELETE, uri);
    return ret;
}

/**
 * @brief VoiceMailAbility Query database
 *
 * @param uri Determine the data table name based on the URI
 * @param columns Columns returned by query
 * @param predicates Query the data values of the condition
 *
 * @return Query database results
 */
std::shared_ptr<DataShare::DataShareResultSet> VoiceMailAbility::Query(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns)
{
    HILOG_ERROR("VoiceMailAbility ====>Query start");
    voiceMailDataBase_ = Contacts::VoiceMailDataBase::GetInstance();
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParse(uriTemp);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> result;
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<std::string> columnsTemp = columns;
    bool isUriMatch = true;
    switch (parseCode) {
        case Contacts::VOICEMAIL:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::VOICEMAIL, dataSharePredicates);
            result = voiceMailDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::REPLAYING:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::REPLYING, dataSharePredicates);
            result = voiceMailDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        default:
            isUriMatch = false;
            HILOG_ERROR("VoiceMailAbility ====>no match uri action");
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
    HILOG_ERROR("VoiceMailAbility ====>Query end");
    return sharedPtrResult;
}

void VoiceMailAbility::DataBaseNotifyChange(int code, Uri uri)
{
    Contacts::ContactsCommonEvent::SendCallLogChange(code);
}
} // namespace AbilityRuntime
} // namespace OHOS
