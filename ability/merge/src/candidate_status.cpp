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

#include "candidate_status.h"

#include "common.h"
#include "contacts_columns.h"
#include "contacts_database.h"
#include "hilog_wrapper.h"
#include "merge_utils.h"

namespace OHOS {
namespace Contacts {
CandidateStatus::CandidateStatus()
{
}

CandidateStatus::~CandidateStatus()
{
}

/**
 * @brief Query operation for merging candidates
 *
 * @param store Conditions for query operation
 * @param rawId Contacts's raw_contact_id to query
 *
 * @return Collection of canddidate contacts with the same raw_contact_id
 */
Candidate CandidateStatus::QueryAllForMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId)
{
    Candidate candidate;
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
    int phoneType = contactsDataBase->GetTypeId(ContentTypeData::PHONE);
    MergeUtils mergeUtils;
    std::set<std::string> names = mergeUtils.QueryRawContactByType(store, rawId, nameType);
    // query other name with the raw_contact_id
    std::vector<int> nameIds = mergeUtils.QueryByDataName(rawId, names, store);
    // query current phone
    std::set<std::string> phones = mergeUtils.QueryRawContactByType(store, rawId, phoneType);
    std::set<int> autoIds;
    std::set<int> manualIds;
    unsigned int size = nameIds.size();
    for (unsigned int i = 0; i < size; i++) {
        if (!IsNeedMerge(store, nameIds[i]) || !IsMergeStatus(store, nameIds[i])) {
            continue;
        }
        std::set<std::string> otherPhones = mergeUtils.QueryRawContactByType(store, nameIds[i], phoneType);
        if (phones.empty() && otherPhones.empty()) {
            autoIds.insert(nameIds[i]);
        }
        if (phones.empty() || otherPhones.empty()) {
            manualIds.insert(nameIds[i]);
        }
        if (mergeUtils.SetEqual(phones, otherPhones)) {
            autoIds.insert(nameIds[i]);
        } else {
            manualIds.insert(nameIds[i]);
        }
    }
    candidate.autoIds_ = autoIds;
    candidate.manualIds_ = manualIds;
    AddMergedStatus(candidate);
    HILOG_INFO("QueryAllForMerge mode is : %{public}d ", candidate.mergeMode_);
    candidate.autoIds_.insert(rawId);
    candidate.manualIds_.insert(rawId);
    HILOG_INFO("QueryAllForMerge size is : %{public}zu ", candidate.autoIds_.size());
    HILOG_INFO("QueryAllForMerge candidate.manualIds_ is : %{public}zu ", candidate.manualIds_.size());
    return candidate;
}

void CandidateStatus::AddMergedStatus(Candidate &candidate)
{
    if (!candidate.autoIds_.empty()) {
        candidate.mergeMode_ = MERGE_MODE_AUTO;
    } else if (!candidate.manualIds_.empty()) {
        candidate.mergeMode_ = MERGE_MODE_MANUAL;
    } else {
        candidate.mergeMode_ = MERGE_MODE_DEFAULT;
    }
}

/**
 * @brief Query operation for checking if candidate marging is needed
 *
 * @param store Conditions for query operation
 * @param rawId Contacts's raw_contact_id to query
 *
 * @return The result returned by the query operation
 */
bool CandidateStatus::IsNeedMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId)
{
    std::string isNeedMergeSql = "SELECT ";
    isNeedMergeSql.append(RawContactColumns::IS_NEED_MERGE)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ")
        .append(std::to_string(rawId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> isNeedMergeSet = store->QuerySql(isNeedMergeSql);
    bool isNeedMerge = false;
    int resultSetNum = isNeedMergeSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int value = 0;
        isNeedMergeSet->GetInt(0, value);
        if (value == 1) {
            isNeedMerge = true;
        } else {
            isNeedMerge = false;
        }
        resultSetNum = isNeedMergeSet->GoToNextRow();
    }
    isNeedMergeSet->Close();
    return isNeedMerge;
}

/**
 * @brief Check if the candidate has true merge_status
 *
 * @param store Conditions for query operation
 * @param rawId Contacts's raw_contact_id to query
 *
 * @return True if the candidate is in the merged status; flase otherwise
 */
bool CandidateStatus::IsMergeStatus(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId)
{
    std::string isMergeSwitchSql = "SELECT ";
    isMergeSwitchSql.append(RawContactColumns::MERGE_STATUS)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ")
        .append(std::to_string(rawId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> MergeSwitchSet = store->QuerySql(isMergeSwitchSql);
    bool isMergeSwitch = false;
    int resultSetNum = MergeSwitchSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int value = 0;
        MergeSwitchSet->GetInt(0, value);
        if (value == 1) {
            isMergeSwitch = true;
        } else {
            isMergeSwitch = false;
        }
        resultSetNum = MergeSwitchSet->GoToNextRow();
    }
    MergeSwitchSet->Close();
    return isMergeSwitch;
}

/**
 * @brief Check if the candidate has been merged
 *
 * @param store Conditions for query operation
 * @param rawId Contacts's raw_contact_id to query
 *
 * @return True if the candidate has been merged; false otherwise
 */
bool CandidateStatus::IsMerged(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId)
{
    std::string isNeedMergeSql = "SELECT ";
    isNeedMergeSql.append(RawContactColumns::MERGE_MODE)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ")
        .append(std::to_string(rawId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> isNeedMergeSet = store->QuerySql(isNeedMergeSql);
    bool mergeMode = false;
    int resultSetNum = isNeedMergeSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int value = 0;
        isNeedMergeSet->GetInt(0, value);
        if (value == 0) {
            mergeMode = true;
        } else {
            mergeMode = false;
        }
        resultSetNum = isNeedMergeSet->GoToNextRow();
    }
    isNeedMergeSet->Close();
    bool isMerged = mergeMode && !IsNeedMerge(store, rawId);
    return isMerged;
}

bool CandidateStatus::JudgeDataDifferent(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::vector<int> ids, int rawId)
{
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    MergeUtils mergeUtils;
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
    int phoneType = contactsDataBase->GetTypeId(ContentTypeData::PHONE);
    std::set<std::string> names = mergeUtils.QueryRawContactByType(store, rawId, nameType);
    std::set<std::string> phones = mergeUtils.QueryRawContactByType(store, rawId, phoneType);
    unsigned int size = ids.size();
    for (unsigned int i = 0; i < size; i++) {
        MergeUtils mergeIdUtils;
        std::set<std::string> idNames = mergeIdUtils.QueryRawContactByType(store, ids[i], nameType);
        std::set<std::string> idPhones = mergeIdUtils.QueryRawContactByType(store, ids[i], phoneType);
        if (names != idNames || phones != idPhones) {
            HILOG_INFO("CandidateStatus::JudgeDataDifferent is true");
            return true;
        }
    }
    return false;
}
} // namespace Contacts
} // namespace OHOS
