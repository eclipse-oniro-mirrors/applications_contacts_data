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

#include "voicemail_database.h"

#include "calllog_database.h"
#include "predicates_convert.h"

namespace OHOS {
namespace Contacts {
std::shared_ptr<VoiceMailDataBase> VoiceMailDataBase::voiceMailDataBase_ = nullptr;
std::shared_ptr<OHOS::NativeRdb::RdbStore> VoiceMailDataBase::store_ = nullptr;

VoiceMailDataBase::VoiceMailDataBase()
{
    store_ = CallLogDataBase::GetInstance()->store_;
}

std::shared_ptr<VoiceMailDataBase> VoiceMailDataBase::GetInstance()
{
    if (voiceMailDataBase_ == nullptr) {
        voiceMailDataBase_.reset(new VoiceMailDataBase());
        return voiceMailDataBase_;
    }
    return voiceMailDataBase_;
}

int64_t VoiceMailDataBase::InsertVoiceMail(std::string tableName, OHOS::NativeRdb::ValuesBucket insertValues)
{
    int64_t outRowId = RDB_EXECUTE_FAIL;
    if (store_ == nullptr) {
        HILOG_ERROR("VoiceMailDataBase Insert store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    CallLogDataBase::GetInstance()->QueryContactsByInsertCalls(insertValues);
    int ret = store_->Insert(outRowId, tableName, insertValues);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("VoiceMailDataBase InsertVoiceMail ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return outRowId;
}

int VoiceMailDataBase::UpdateVoiceMail(
    OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    int changedRows = RDB_EXECUTE_FAIL;
    if (store_ == nullptr) {
        HILOG_ERROR("VoiceMailDataBase Update store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (values.HasColumn(CallLogColumns::PHONE_NUMBER)) {
        CallLogDataBase::GetInstance()->QueryContactsByInsertCalls(values);
    }
    int ret = store_->Update(changedRows, values, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("VoiceMailDataBase Update ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

int VoiceMailDataBase::DeleteVoiceMail(OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    int deletedRows = RDB_EXECUTE_FAIL;
    if (store_ == nullptr) {
        HILOG_ERROR("VoiceMailDataBase Delete store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->Delete(deletedRows, rdbPredicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("VoiceMailDataBase Delete ret :%{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> VoiceMailDataBase::Query(
    OHOS::NativeRdb::RdbPredicates &rdbPredicates, const std::vector<std::string> columns)
{
    if (store_ == nullptr) {
        HILOG_ERROR("CallLogDataBase Delete store_ is nullptr");
        return nullptr;
    }
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store_->Query(rdbPredicates, columns);
    return resultSet;
}

int VoiceMailDataBase::BeginTransaction()
{
    if (store_ == nullptr) {
        HILOG_ERROR("VoiceMailDataBase BeginTransaction store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("VoiceMailDataBase BeginTransaction fail :%{public}d", ret);
    }
    return ret;
}

int VoiceMailDataBase::Commit()
{
    if (store_ == nullptr) {
        HILOG_ERROR(" VoiceMailDataBase Commit store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR(" VoiceMailDataBase Commit fail :%{public}d", ret);
    }
    return ret;
}

int VoiceMailDataBase::RollBack()
{
    if (store_ == nullptr) {
        HILOG_ERROR(" VoiceMailDataBase RollBack store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->RollBack();
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR(" VoiceMailDataBase RollBack fail :%{public}d", ret);
    }
    return ret;
}
} // namespace Contacts
} // namespace OHOS