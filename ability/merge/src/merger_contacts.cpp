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

#include "merger_contacts.h"

#include "common.h"
#include "contacts_columns.h"
#include "contacts_database.h"
#include "hilog_wrapper.h"
#include "match_candidate.h"
#include "merge_utils.h"

namespace OHOS {
namespace Contacts {
static int g_isNeedMergeNum[2] = {0, 1};
static int g_isNeedMergeTargetNum[2] = {0, 1};

MergerContacts::MergerContacts()
{
}

MergerContacts::~MergerContacts()
{
}

/**
 * @brief AutoMerge operation for candidate contacts
 *
 * @param store Conditions for split operation
 *
 * @return The result returned by the AutoMerge operation
 */
int MergerContacts::ContactMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore> store)
{
    if (store == nullptr) {
        HILOG_ERROR("store is empty");
        return RDB_OBJECT_EMPTY;
    }
    HILOG_INFO("MergerContacts::ContactMerge is starting");
    // query all ids with merge_mode = 2
    std::vector<std::set<int>> candidates = QueryMergeContacts(store, MERGE_MODE_AUTO);
    int ret = MergeCircle(store, candidates);
    return ret;
}

int MergerContacts::MergeCircle(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::vector<std::set<int>> candidates)
{
    if (candidates.empty()) {
        HILOG_ERROR("Merge candidate is empty");
        return RDB_EXECUTE_FAIL;
    }
    bool isModeHasError = false;
    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
        std::set<int> ids = *it;
        if (ids.size() == 1) {
            HILOG_ERROR("MergeCircle continue ");
            isModeHasError = true;
            continue;
        }
        auto minIdPosition = ids.begin();
        int minId = *minIdPosition;
        UpdateRawContacts(store, minId, ids);
        std::string sql = "UPDATE ";
        sql.append(ContactTableName::RAW_CONTACT)
            .append(" SET ")
            .append(RawContactColumns::IS_NEED_MERGE)
            .append(" = ")
            .append(std::to_string(g_isNeedMergeNum[1]))
            .append(", ")
            .append(RawContactColumns::IS_MERGE_TARGET)
            .append(" = ")
            .append(std::to_string(g_isNeedMergeTargetNum[0]))
            .append(" WHERE ")
            .append(ContactPublicColumns::ID)
            .append(" = ")
            .append(std::to_string(minId));
        int error = store->ExecuteSql(sql);
        if (error != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("MergeCircle ExecuteSql error : %{public}d ", error);
            return RDB_EXECUTE_FAIL;
        }
        MatchCandidate matchCandidate;
        matchCandidate.AddHasByRawId(store, minId);
        matchCandidate.FindMatchContact(store, minId);
    }
    if (isModeHasError) {
        HILOG_ERROR("MergerContacts::MergeCircle only one candidate has auto_merge_mode");
        return RDB_EXECUTE_FAIL;
    } else {
        return RDB_EXECUTE_OK;
    }
}

int MergerContacts::DeleteContacts(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int minId, std::set<int> handledIds)
{
    int ret = store->ExecuteSql("PRAGMA foreign_keys = OFF");
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteContacts foreign_keys close fail");
        return ret;
    }
    std::set<int> ids;
    std::string sql = "DELETE FROM ";
    sql.append(ContactTableName::CONTACT).append(" WHERE ");
    for (auto it = handledIds.begin(); it != handledIds.end(); ++it) {
        if (*it != minId) {
            ids.insert(*it);
        }
    }
    for (auto id = ids.begin(); id != ids.end(); ++id) {
        sql.append(ContactColumns::NAME_RAW_CONTACT_ID).append(" = ").append(std::to_string(*id));
        if (*id != *ids.rbegin()) {
            sql.append(" OR ");
        }
    }
    int code = store->ExecuteSql(sql);
    ret = store->ExecuteSql("PRAGMA foreign_keys = ON");
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("DeleteContacts foreign_keys open fail");
        return ret;
    }
    return code;
}

std::set<int> MergerContacts::HandleIds(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> ids)
{
    std::set<int> rawIds;
    for (auto id = ids.begin(); id != ids.end(); ++id) {
        if (*id != *ids.rbegin()) {
            std::string sqlBuilder = "SELECT ";
            sqlBuilder.append(ContactPublicColumns::ID)
                .append(" FROM ")
                .append(ContactTableName::RAW_CONTACT)
                .append(" WHERE ")
                .append(RawContactColumns::CONTACT_ID)
                .append(" = (SELECT ")
                .append(RawContactColumns::CONTACT_ID)
                .append(" FROM")
                .append(ContactTableName::RAW_CONTACT)
                .append(" WHERE ")
                .append(ContactPublicColumns::ID)
                .append(" = ")
                .append(std::to_string(*id))
                .append(")");
            std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sqlBuilder);
            int resultSetNum = resultSet->GoToFirstRow();
            while (resultSetNum == OHOS::NativeRdb::E_OK) {
                int rawId = 0;
                resultSet->GetInt(0, rawId);
                rawIds.insert(rawId);
                resultSetNum = resultSet->GoToNextRow();
            }
            resultSet->Close();
        } else {
            rawIds.insert(*id);
        }
    }
    return rawIds;
}

void MergerContacts::UpdateRawContacts(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int minRawId, std::set<int> Ids)
{
    std::string sqlBuilder = "SELECT ";
    sqlBuilder.append(RawContactColumns::CONTACT_ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ")
        .append(std::to_string(minRawId))
        .append(" AND ")
        .append(RawContactColumns::IS_DELETED)
        .append(" = 0");
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sqlBuilder);
    int minContactId = 0;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        resultSet->GetInt(0, minContactId);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    UpdateRawContactsExecute(store, minContactId, Ids);
    DeleteContacts(store, minRawId, Ids);
}

void MergerContacts::UpdateRawContactsExecute(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int minContactId, std::set<int> Ids)
{
    unsigned int size = Ids.size();
    if (size < 0) {
        return;
    }
    std::string updateRawContact = "UPDATE ";
    updateRawContact.append(ContactTableName::RAW_CONTACT)
        .append(" SET ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" = ")
        .append(std::to_string(minContactId))
        .append(", ")
        .append(RawContactColumns::MERGE_MODE)
        .append(" = ")
        .append(std::to_string(MERGE_MODE_DEFAULT))
        .append(", ")
        .append(RawContactColumns::IS_NEED_MERGE)
        .append(" = ")
        .append(std::to_string(g_isNeedMergeNum[0]))
        .append(" WHERE ( ");
    std::string updateSearchSql = "UPDATE ";
    updateSearchSql.append(ContactTableName::SEARCH_CONTACT)
        .append(" SET ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" = ")
        .append(std::to_string(minContactId))
        .append(" WHERE ");
    for (auto it = Ids.begin(); it != Ids.end(); ++it) {
        updateRawContact.append("id = ").append(std::to_string(*it));
        updateSearchSql.append("id = ").append(std::to_string(*it));
        if (*it != *Ids.rbegin()) {
            updateRawContact.append(" OR ");
            updateSearchSql.append(" OR ");
        }
    }
    updateRawContact.append(" ) AND is_deleted = 0");
    int ret = store->ExecuteSql(updateRawContact);
    int code = store->ExecuteSql(updateSearchSql);
    if (ret != RDB_EXECUTE_OK || code != RDB_EXECUTE_OK) {
        return;
    }
}

std::string MergerContacts::QueryCandidateName(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int id)
{
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
    std::string sqlBuilder = "SELECT ";
    sqlBuilder.append(ContactDataColumns::DETAIL_INFO)
        .append(" FROM ")
        .append(ContactTableName::CONTACT_DATA)
        .append(" WHERE ")
        .append(ContactDataColumns::RAW_CONTACT_ID)
        .append(" = ")
        .append(std::to_string(id))
        .append(" AND ")
        .append(ContactDataColumns::TYPE_ID)
        .append(" = ")
        .append(std::to_string(nameType));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sqlBuilder);
    std::string candidateName;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        resultSet->GetString(0, candidateName);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return candidateName;
}

std::set<int> MergerContacts::QueryTargetName(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, std::set<std::string> data)
{
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
    std::set<int> ids;
    for (auto it = data.begin(); it != data.end(); it++) {
        std::string query = "SELECT ";
        query.append(ContactDataColumns::RAW_CONTACT_ID)
            .append(" FROM ")
            .append(ViewName::VIEW_CONTACT_DATA)
            .append(" WHERE ")
            .append(ContactDataColumns::DETAIL_INFO)
            .append(" = '")
            .append(*it)
            .append("' AND ")
            .append(ContactDataColumns::TYPE_ID)
            .append(" = ")
            .append(std::to_string(nameType))
            .append(" AND is_deleted = 0");
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(query);
        int resultSetNum = resultSet->GoToFirstRow();
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            int value = 0;
            resultSet->GetInt(0, value);
            if (value != rawId) {
                ids.insert(value);
            }
            resultSetNum = resultSet->GoToNextRow();
        }
        resultSet->Close();
    }
    return ids;
}

std::vector<std::set<int>> MergerContacts::QueryMergeContacts(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int mode)
{
    std::string sqlBuilder = "SELECT ";
    sqlBuilder.append(ContactPublicColumns::ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(RawContactColumns::IS_MERGE_TARGET)
        .append(" = ")
        .append(std::to_string(g_isNeedMergeTargetNum[1]))
        .append(" AND ")
        .append(RawContactColumns::IS_DELETED)
        .append(" = 0")
        .append(" AND ")
        .append(RawContactColumns::MERGE_MODE)
        .append(" = ")
        .append(std::to_string(mode));
    std::vector<int> currentIds;
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sqlBuilder);
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int currentId = 0;
        resultSet->GetInt(0, currentId);
        currentIds.push_back(currentId);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    HILOG_INFO("QueryMergeContacts currentIds' size = %{public}zu", currentIds.size());
    std::vector<std::set<int>> candidates;
    UpdateCandidate(store, currentIds, candidates);
    return candidates;
}

void MergerContacts::UpdateCandidate(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::vector<int> currentIds,
    std::vector<std::set<int>> &candidates)
{
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
    int phoneType = contactsDataBase->GetTypeId(ContentTypeData::PHONE);
    MergeUtils mergeUtils;
    for (auto rawId : currentIds) {
        std::set<std::string> names = mergeUtils.QueryRawContactByType(store, rawId, nameType);
        std::set<std::string> phones = mergeUtils.QueryRawContactByType(store, rawId, phoneType);
        if (names.empty()) {
            HILOG_ERROR("MergerContacts::UpdateCandidate target names is empty");
            break;
        }
        std::set<int> nameIds = QueryTargetName(store, rawId, names);
        std::set<int> rawIds;
        rawIds.insert(rawId);
        for (auto nameId = nameIds.begin(); nameId != nameIds.end(); nameId++) {
            std::set<std::string> otherPhones = mergeUtils.QueryRawContactByType(store, *nameId, phoneType);
            bool isMatch = mergeUtils.SetEqual(phones, otherPhones);
            if (isMatch) {
                rawIds.insert(*nameId);
            }
        }
        HILOG_INFO("rawIds' size = %{public}zu", rawIds.size());
        candidates.push_back(rawIds);
    }
    HILOG_INFO("candidates' size = %{public}zu", candidates.size());
}

std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> MergerContacts::SelectCandidate(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store)
{
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet;
    if (store == nullptr) {
        HILOG_ERROR("MergerContacts SelectCandidate store is nullptr");
        return resultSet;
    }
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
    int phoneType = contactsDataBase->GetTypeId(ContentTypeData::PHONE);
    std::string sql = "SELECT ";
    sql.append(ContactDataColumns::RAW_CONTACT_ID)
        .append(", ")
        .append(RawContactColumns::CONTACT_ID)
        .append(", ")
        .append(ContentTypeColumns::CONTENT_TYPE)
        .append(", ")
        .append(ContactDataColumns::TYPE_ID)
        .append(", ")
        .append(RawContactColumns::DISPLAY_NAME)
        .append(", ")
        .append(RawContactColumns::PHOTO_FIRST_NAME)
        .append(", ")
        .append(ContactDataColumns::DETAIL_INFO)
        .append(" FROM ")
        .append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE ")
        .append(ContactDataColumns::RAW_CONTACT_ID)
        .append(" IN(SELECT ")
        .append(RawContactColumns::ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(RawContactColumns::IS_DELETED)
        .append(" = 0")
        .append(" AND ")
        .append(RawContactColumns::MERGE_MODE)
        .append(" = ")
        .append(std::to_string(MERGE_MODE_MANUAL))
        .append(") AND ( ")
        .append(ContactDataColumns::TYPE_ID)
        .append(" = ")
        .append(std::to_string(phoneType))
        .append(" OR ")
        .append(ContactDataColumns::TYPE_ID)
        .append(" = ")
        .append(std::to_string(nameType))
        .append(" ) GROUP BY ")
        .append(RawContactColumns::CONTACT_ID);
    resultSet = store->QuerySql(sql);
    return resultSet;
}

/**
 * @brief ManualMerge operation for candidate contacts
 *
 * @param store Conditions for split operation
 * @param predicates Conditions for update operation
 *
 * @return The result returned by the ManualMerge operation
 */
int MergerContacts::ReContactMerge(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, const DataShare::DataSharePredicates &predicates)
{
    HILOG_INFO("MergerContacts::ReContactMerge is starting");
    std::vector<std::string> whereArgs = predicates.GetWhereArgs();
    std::set<int> ids;
    for (auto value : whereArgs) {
        ids.insert(atoi(value.c_str()));
    }
    if (ids.size() == 0) {
        return RDB_EXECUTE_FAIL;
    }
    std::vector<std::set<int>> selectedIds = SelectIdsByName(store, ids);
    int minId = 0;
    bool isModeHasError = false;
    for (auto it = selectedIds.begin(); it != selectedIds.end(); ++it) {
        std::set<int> currentIds = *it;
        std::set<int> handledIds = HandleIds(store, currentIds);
        HILOG_INFO("ReContactMerge size = :%{public}zu", handledIds.size());
        if (handledIds.size() == 1) {
            HILOG_ERROR("MergeCircle continue ");
            isModeHasError = true;
            continue;
        }
        auto minIdPosition = handledIds.begin();
        minId = *minIdPosition;
        int ret = ManualMergeOperation(store, minId, handledIds);
        if (ret != RDB_EXECUTE_OK) {
            return RDB_EXECUTE_FAIL;
        }
        MatchCandidate matchCandidate;
        matchCandidate.FindMatchContact(store, minId);
    }
    if (isModeHasError) {
        HILOG_ERROR("MergerContacts::MergeCircle only one candidate has manual_merge_mode");
        return RDB_EXECUTE_FAIL;
    } else {
        return RDB_EXECUTE_OK;
    }
}

int MergerContacts::ManualMergeOperation(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int minId, std::set<int> handledIds)
{
    UpdateRawContacts(store, minId, handledIds);
    DeleteContacts(store, minId, handledIds);
    std::string sql = "UPDATE ";
    sql.append(ContactTableName::RAW_CONTACT)
        .append(" SET ")
        .append(RawContactColumns::IS_NEED_MERGE)
        .append(" = ")
        .append(std::to_string(g_isNeedMergeNum[1]))
        .append(", ")
        .append(RawContactColumns::IS_MERGE_TARGET)
        .append(" = ")
        .append(std::to_string(g_isNeedMergeTargetNum[0]))
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ")
        .append(std::to_string(minId));
    int error = store->ExecuteSql(sql);
    if (error != RDB_EXECUTE_OK) {
        HILOG_ERROR("MergerContacts::ReContactMerge errorcode :%{public}d", error);
        return error;
    }
    return RDB_EXECUTE_OK;
}

std::vector<std::set<int>> MergerContacts::SelectIdsByName(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> ids)
{
    HILOG_INFO("MergerContacts::SelectIdsByName is starting");
    std::vector<std::set<int>> selectedIds;
    std::set<std::string> names;
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
    for (auto id : ids) {
        std::string sql = "SELECT ";
        sql.append(ContactDataColumns::DETAIL_INFO)
            .append(" FROM ")
            .append(ContactTableName::CONTACT_DATA)
            .append(" WHERE ")
            .append(ContactDataColumns::RAW_CONTACT_ID)
            .append(" = ")
            .append(std::to_string(id))
            .append(" AND ")
            .append(ContactDataColumns::TYPE_ID)
            .append(" = ")
            .append(std::to_string(nameType));
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sql);
        int resultSetNum = resultSet->GoToFirstRow();
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            std::string name;
            resultSet->GetString(0, name);
            names.insert(name);
            resultSetNum = resultSet->GoToNextRow();
        }
        resultSet->Close();
    }
    UpdateIdsByName(store, names, ids, selectedIds);
    return selectedIds;
}

void MergerContacts::UpdateIdsByName(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<std::string> names,
    std::set<int> ids, std::vector<std::set<int>> &selectedIds)
{
    for (auto name : names) {
        std::string sqlBuilder = "SELECT ";
        sqlBuilder.append(ContactDataColumns::RAW_CONTACT_ID)
            .append(" FROM ")
            .append(ContactTableName::CONTACT_DATA)
            .append(" WHERE ")
            .append(ContactDataColumns::DETAIL_INFO)
            .append(" = '")
            .append(name)
            .append("' AND ")
            .append(ContactDataColumns::RAW_CONTACT_ID)
            .append(" IN(");
        for (auto id = ids.begin(); id != ids.end(); ++id) {
            if (*id == *ids.rbegin()) {
                sqlBuilder.append(std::to_string(*id)).append(")");
            } else {
                sqlBuilder.append(std::to_string(*id)).append(", ");
            }
        }
        std::set<int> currentIds;
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sqlBuilder);
        int resultSetNum = resultSet->GoToFirstRow();
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            int id = 0;
            resultSet->GetInt(0, id);
            currentIds.insert(id);
            resultSetNum = resultSet->GoToNextRow();
        }
        resultSet->Close();
        selectedIds.push_back(currentIds);
    }
}

bool MergerContacts::isNameMatch(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> ids)
{
    std::set<std::string> names;
    for (auto it : ids) {
        std::string name = QueryCandidateName(store, it);
        names.insert(name);
    }
    if (names.size() == 1) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Force contact merge for candidate contacts
 *
 * @param store Conditions for split operation
 * @param ids Conditions for update operation
 *
 * @return The result returned by the ForceContactMerge operation
 */
int MergerContacts::ForceContactMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> ids)
{
    if (store == nullptr) {
        HILOG_ERROR("store is empty");
        return RDB_OBJECT_EMPTY;
    }
    int minId = 0;
    if (isNameMatch(store, ids)) {
        std::set<int> handledIds = HandleIds(store, ids);
        HILOG_INFO("ForceContactMerge size = %{public}zu", handledIds.size());
        auto minIdPosition = handledIds.begin();
        minId = *minIdPosition;
        UpdateRawContacts(store, minId, handledIds);
        DeleteContacts(store, minId, handledIds);
    } else {
        HILOG_ERROR("Invalid parameter passed");
        return RDB_EXECUTE_FAIL;
    }
    std::string sql = "UPDATE ";
    sql.append(ContactTableName::RAW_CONTACT)
        .append(" SET ")
        .append(RawContactColumns::IS_NEED_MERGE)
        .append(" = ")
        .append(std::to_string(g_isNeedMergeNum[1]))
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ")
        .append(std::to_string(minId));
    int error = store->ExecuteSql(sql);
    if (error != RDB_EXECUTE_OK) {
        HILOG_ERROR("MergerContacts::ForceMerge errorcode :%{public}d", error);
        return error;
    }
    return RDB_EXECUTE_OK;
}
} // namespace Contacts
} // namespace OHOS
