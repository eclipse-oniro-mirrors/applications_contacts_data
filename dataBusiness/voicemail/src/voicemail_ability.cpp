/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "data_ability_predicates.h"
#include "file_utils.h"
#include "predicates_convert.h"
#include "rdb_predicates.h"
#include "sql_analyzer.h"
#include "uri_utils.h"

namespace OHOS {
namespace AppExecFwk {
REGISTER_AA(VoiceMailAbility);
namespace {
std::mutex g_mutex;
}
std::shared_ptr<Contacts::VoiceMailDataBase> VoiceMailAbility::voiceMailDataBase_ = nullptr;
std::map<std::string, int> VoiceMailAbility::uriValueMap_ = {
    {"/com.ohos.voicemailability/calls/voicemail", Contacts::VOICEMAIL},
    {"/com.ohos.voicemailability/calls/replaying", Contacts::REPLAYING}
};

VoiceMailAbility::VoiceMailAbility()
{
}

VoiceMailAbility::~VoiceMailAbility()
{
}

void VoiceMailAbility::Dump(const std::string &extra)
{
    HILOG_ERROR("VoiceMailAbility ====>Dump:%{public}s", extra.c_str());
    Contacts::FileUtils fileUtils;
    std::string dirStr = Contacts::ContactsPath::DUMP_PATH;
    fileUtils.WriteStringToFileAppend(dirStr, extra);
}

void VoiceMailAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
    std::string basePath = GetDatabaseDir();
    Contacts::ContactsPath::RDB_PATH = basePath + "/";
    Contacts::ContactsPath::RDB_BACKUP_PATH = basePath + "/backup/";
    Contacts::ContactsPath::DUMP_PATH = GetFilesDir() + "/";
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
int VoiceMailAbility::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(value);
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
    rowRet = InsertExecute(uri, value);
    if (rowRet == Contacts::OPERATION_ERROR) {
        ret = voiceMailDataBase_->RollBack();
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

int VoiceMailAbility::InsertExecute(const OHOS::Uri &uri, const NativeRdb::ValuesBucket &initialValues)
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
int VoiceMailAbility::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int size = values.size();
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
    for (int i = 0; i < size; i++) {
        ++count;
        OHOS::NativeRdb::ValuesBucket rawContactValues = values[i];
        int code = InsertExecute(uri, rawContactValues);
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
    if (!IsCommitOK(markRet, g_mutex) ) {
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
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(value);
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
    OHOS::NativeRdb::DataAbilityPredicates dataAbilityPredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (code) {
        case Contacts::VOICEMAIL:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::VOICEMAIL, dataAbilityPredicates);
            ret = voiceMailDataBase_->UpdateVoiceMail(value, rdbPredicates);
            break;
        case Contacts::REPLAYING:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::REPLYING, dataAbilityPredicates);
            ret = voiceMailDataBase_->UpdateVoiceMail(value, rdbPredicates);
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
int VoiceMailAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    g_mutex.lock();
    voiceMailDataBase_ = Contacts::VoiceMailDataBase::GetInstance();
    Contacts::PredicatesConvert predicatesConvert;
    int ret = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParse(uriTemp);
    OHOS::NativeRdb::DataAbilityPredicates dataAbilityPredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (parseCode) {
        case Contacts::VOICEMAIL:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::VOICEMAIL, dataAbilityPredicates);
            ret = voiceMailDataBase_->DeleteVoiceMail(rdbPredicates);
            break;
        case Contacts::REPLAYING:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::REPLYING, dataAbilityPredicates);
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
std::shared_ptr<NativeRdb::AbsSharedResultSet> VoiceMailAbility::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_ERROR("VoiceMailAbility ====>Query start");
    voiceMailDataBase_ = Contacts::VoiceMailDataBase::GetInstance();
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParse(uriTemp);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> result;
    OHOS::NativeRdb::DataAbilityPredicates dataAbilityPredicates = predicates;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::vector<std::string> columnsTemp = columns;
    switch (parseCode) {
        case Contacts::VOICEMAIL:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::VOICEMAIL, dataAbilityPredicates);
            result = voiceMailDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::REPLAYING:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::CallsTableName::REPLYING, dataAbilityPredicates);
            result = voiceMailDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        default:
            HILOG_ERROR("VoiceMailAbility ====>no match uri action");
            break;
    }
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> sharedPtrResult = std::move(result);
    HILOG_ERROR("VoiceMailAbility ====>Query end");
    return sharedPtrResult;
}

void VoiceMailAbility::DataBaseNotifyChange(int code, Uri uri)
{
    Contacts::ContactsCommonEvent::SendCallLogChange(code);
    std::string path = uri.GetPath();
}
} // namespace AppExecFwk
} // namespace OHOS
