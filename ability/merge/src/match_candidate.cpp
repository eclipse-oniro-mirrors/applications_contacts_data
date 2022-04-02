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

#include "match_candidate.h"

#include "candidate.h"
#include "common.h"
#include "contacts.h"
#include "contacts_columns.h"
#include "contacts_database.h"
#include "hilog_wrapper.h"
#include "merge_utils.h"

namespace OHOS {
namespace Contacts {
MatchCandidate::MatchCandidate()
{
}

MatchCandidate::~MatchCandidate()
{
}

/**
 * @brief Query operation for update candidate's merge_mode
 *
 * @param store Conditions for query operation
 * @param rawId Contacts's raw_contact_id to query
 *
 * @return The result returned by the update operation
 */
int MatchCandidate::FindMatchContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId)
{
    CandidateStatus candidateStatus;
    // Query candidates
    Candidate candidate = candidateStatus.QueryAllForMerge(store, rawId);
    if (candidate.autoIds_.size() > 1) {
        int minAutoid = *candidate.autoIds_.begin();
        if (minAutoid != -1) {
            std::string updateMergeTarget = "UPDATE ";
            updateMergeTarget.append(ContactTableName::RAW_CONTACT)
                .append(" SET ")
                .append(RawContactColumns::IS_MERGE_TARGET)
                .append(" = 1 ")
                .append(" WHERE ")
                .append(ContactPublicColumns::ID)
                .append(" = ")
                .append(std::to_string(minAutoid));
            int code = store->ExecuteSql(updateMergeTarget);
            if (code != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("update is_merge_target error code is : %{public}d ", code);
            }
        }
    }
    // update merge_mode
    int retCode = UpdateMergeMode(store, candidate);
    HILOG_INFO("MatchCandidate::FindMatchContact retCode : %{public}d ", retCode);
    return retCode;
}

/**
 * @brief Split operation for already merged contacts
 *
 * @param store Conditions for split operation
 * @param rawId Contacts's raw_contact_id to split
 *
 * @return The result returned by the split operation
 */
int MatchCandidate::Split(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId)
{
    if (store == nullptr) {
        HILOG_ERROR("MatchCandidate::Split store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    std::set<int> rawIds;
    MergeUtils mergeUtils;
    mergeUtils.GetRawIdsByRawId(store, rawId, rawIds);
    if (rawIds.size() < 1) {
        HILOG_ERROR("MatchCandidate::Split is error");
        return OPERATION_ERROR;
    }
    int code = RDB_EXECUTE_FAIL;
    for (auto newRawId = rawIds.begin(); newRawId != rawIds.end(); ++newRawId) {
        int contactsId = AddNewContact(store, *newRawId);
        if (contactsId <= 0) {
            continue;
        }
        code = UpdateRawContact(store, *newRawId, contactsId);
        UpdateSearch(store, *newRawId, contactsId);
        FindMatchContact(store, *newRawId);
    }
    AddHasByRawId(store, rawId);
    return code;
}

void MatchCandidate::AddHasByRawId(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId)
{
    MergeUtils mergeUtils;
    OHOS::NativeRdb::ValuesBucket values;
    values.PutInt(ContactColumns::HAS_DISPLAY_NAME, 0);
    values.PutInt(ContactColumns::HAS_PHONE_NUMBER, 0);
    values.PutInt(ContactColumns::HAS_EMAIL, 0);
    values.PutInt(ContactColumns::HAS_GROUP, 0);
    mergeUtils.AddHasJudgeForRawId(store, rawId, values);
    Contacts contacts;
    int contactsRet = contacts.UpdateContact(rawId, store, values);
    HILOG_INFO("MatchCandidate::AddHasByRawId  UpdateContact ret is : %{public}d", contactsRet);
}

int MatchCandidate::UpdateSearch(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, int contactsId)
{
    std::string sql = "UPDATE ";
    sql.append(ContactTableName::SEARCH_CONTACT)
        .append(" SET ")
        .append(SearchContactColumns::CONTACT_ID)
        .append(" = ")
        .append(std::to_string(contactsId))
        .append(" WHERE ")
        .append(SearchContactColumns::RAW_CONTACT_ID)
        .append(" = ")
        .append(std::to_string(rawId));
    int ret = store->ExecuteSql(sql);
    return ret;
}

int64_t MatchCandidate::AddNewContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId)
{
    MergeUtils mergeUtils;
    std::string sql = "SELECT ";
    sql.append(ContactColumns::PHOTO_ID)
        .append(", ")
        .append(ContactColumns::PHOTO_FILE_ID)
        .append(", ")
        .append(ContactColumns::PERSONAL_RINGTONE)
        .append(", ")
        .append(ContactColumns::COMPANY)
        .append(", ")
        .append(ContactColumns::POSITION)
        .append(", ")
        .append(ContactColumns::READ_ONLY)
        .append(", ")
        .append(ContactColumns::PERSONAL_NOTIFICATION_RINGTONE)
        .append(", ")
        .append(ContactColumns::IS_TRANSFER_VOICEMAIL)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ")
        .append(std::to_string(rawId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sql);
    std::vector<OHOS::NativeRdb::ValuesBucket> value = ResultSetToValuesBucket(resultSet);
    value[0].PutInt(ContactColumns::NAME_RAW_CONTACT_ID, rawId);
    mergeUtils.AddHasJudgeForRawId(store, rawId, value[0]);
    int64_t rawIdTemp = rawId;
    int64_t contactsId = 0;
    Contacts contacts;
    int contactsRet = contacts.InsertContact(store, rawIdTemp, value[0], contactsId);
    if (contactsRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("MatchCandidate AddNewContact is error %{public}d", contactsRet);
    }
    return contactsId;
}

int MatchCandidate::ExecuteUpdateMode(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> Ids, int mode)
{
    int ret = RDB_EXECUTE_FAIL;
    if (Ids.size() < 1) {
        HILOG_ERROR("MatchCandidate::ExecuteUpdateMode Ids.size() < 1");
        return ret;
    }
    std::string sql = "UPDATE ";
    sql.append(ContactTableName::RAW_CONTACT)
        .append(" SET ")
        .append(RawContactColumns::MERGE_MODE)
        .append(" = ")
        .append(std::to_string(mode))
        .append(" WHERE ");
    for (auto it = Ids.begin(); it != Ids.end(); it++) {
        sql.append("id = ").append(std::to_string(*it));
        if (*it != *Ids.rbegin()) {
            sql.append(" OR ");
        }
    }
    ret = store->ExecuteSql(sql);
    return ret;
}

int MatchCandidate::UpdateMergeMode(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, Candidate candidate)
{
    HILOG_INFO("MatchCandidate::UpdateMergeMode is start mode :%{public}d ", candidate.mergeMode_);
    int error = RDB_EXECUTE_FAIL;
    switch (candidate.mergeMode_) {
        case MERGE_MODE_DEFAULT:
            error = ExecuteUpdateMode(store, candidate.manualIds_, MERGE_MODE_DEFAULT);
            break;
        case MERGE_MODE_MANUAL:
            error = ExecuteUpdateMode(store, candidate.manualIds_, MERGE_MODE_MANUAL);
            break;
        case MERGE_MODE_AUTO:
            error = ExecuteUpdateMode(store, candidate.autoIds_, MERGE_MODE_AUTO);
            break;
        default:
            HILOG_ERROR("UpdateMergeMode mode error");
            break;
    }
    return error;
}

int MatchCandidate::UpdateRawContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, int64_t contactsId)
{
    std::string sql = "UPDATE ";
    sql.append(ContactTableName::RAW_CONTACT)
        .append(" SET ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" = ")
        .append(std::to_string(contactsId))
        .append(", ")
        .append(RawContactColumns::MERGE_MODE)
        .append(" = 0, ")
        .append(RawContactColumns::IS_NEED_MERGE)
        .append(" = 1")
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ")
        .append(std::to_string(rawId));
    int code = store->ExecuteSql(sql);
    return code;
}

std::vector<OHOS::NativeRdb::ValuesBucket> MatchCandidate::ResultSetToValuesBucket(
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet)
{
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    std::vector<OHOS::NativeRdb::ValuesBucket> vectorQueryData;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        OHOS::NativeRdb::ValuesBucket valuesBucketElement;
        unsigned int size = columnNames.size();
        for (unsigned int i = 0; i < size; i++) {
            std::string typeValue = columnNames[i];
            int columnIndex = 0;
            resultSet->GetColumnIndex(typeValue, columnIndex);
            OHOS::NativeRdb::ColumnType columnType;
            resultSet->GetColumnType(columnIndex, columnType);
            if (columnType == OHOS::NativeRdb::ColumnType::TYPE_INTEGER) {
                int intValue = 0;
                resultSet->GetInt(columnIndex, intValue);
                valuesBucketElement.PutInt(typeValue, intValue);
            } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_FLOAT) {
                double doubleValue = 0;
                resultSet->GetDouble(columnIndex, doubleValue);
                valuesBucketElement.PutDouble(typeValue, doubleValue);
            } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_STRING) {
                std::string stringValue;
                resultSet->GetString(columnIndex, stringValue);
                valuesBucketElement.PutString(typeValue, stringValue);
            }
        }
        vectorQueryData.push_back(valuesBucketElement);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return vectorQueryData;
}
} // namespace Contacts
} // namespace OHOS