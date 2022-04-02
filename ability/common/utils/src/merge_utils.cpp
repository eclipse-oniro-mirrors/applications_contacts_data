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

#include "merge_utils.h"

#include "contacts_columns.h"
#include "contacts_database.h"

namespace OHOS {
namespace Contacts {
MergeUtils::MergeUtils(void)
{
}

MergeUtils::~MergeUtils()
{
}

std::set<std::string> MergeUtils::QueryRawContactByType(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, int typeId)
{
    std::string sql = "SELECT ";
    sql.append(ContactPublicColumns::ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" = (SELECT ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ?")
        .append(" AND is_deleted = 0 )");
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back(std::to_string(rawId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> rawIdsSet = store->QuerySql(sql, selectionArgs);
    std::set<int> rawIds;
    int resultSetNum = rawIdsSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int value = 0;
        rawIdsSet->GetInt(0, value);
        rawIds.insert(value);
        resultSetNum = rawIdsSet->GoToNextRow();
    }
    rawIdsSet->Close();
    return QueryDataExecute(store, rawIds, typeId);
}

std::set<std::string> MergeUtils::QueryDataExecute(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> rawIds, int typeId)
{
    std::set<std::string> result;
    unsigned int size = rawIds.size();
    if (size <= 0) {
        return result;
    }
    std::string query = "SELECT ";
    query.append(ContactDataColumns::DETAIL_INFO)
        .append(" FROM ")
        .append(ContactTableName::CONTACT_DATA)
        .append(" WHERE ");
    for (auto rawId = rawIds.begin(); rawId != rawIds.end(); ++rawId) {
        query.append(ContactDataColumns::RAW_CONTACT_ID)
            .append(" = ")
            .append(std::to_string(*rawId))
            .append(" AND ")
            .append(ContactDataColumns::TYPE_ID)
            .append(" = ? ");
        if (*rawId != *rawIds.rbegin()) {
            query.append(" OR ");
        }
    }
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back(std::to_string(typeId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(query, selectionArgs);
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        std::string value;
        resultSet->GetString(0, value);
        if (!value.empty()) {
            result.insert(value);
        }
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return result;
}

std::vector<int> MergeUtils::QueryByDataName(
    int rawId, std::set<std::string> data, std::shared_ptr<OHOS::NativeRdb::RdbStore> store)
{
    std::vector<int> ids;
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
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
                ids.push_back(value);
            }
            resultSetNum = resultSet->GoToNextRow();
        }
        resultSet->Close();
    }
    return ids;
}

bool MergeUtils::SetEqual(std::set<std::string> setLeft, std::set<std::string> setRight)
{
    if (setLeft.size() != setRight.size()) {
        return false;
    }
    std::map<std::set<std::string>, std::string> mTemp;
    mTemp[setLeft] = "1";
    if (mTemp.find(setRight) == mTemp.end()) {
        return false;
    }
    return true;
}

void MergeUtils::AddHasJudgeForRawId(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, OHOS::NativeRdb::ValuesBucket &values)
{
    std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
    int nameType = contactsDataBase->GetTypeId(ContentTypeData::NAME);
    int phoneType = contactsDataBase->GetTypeId(ContentTypeData::PHONE);
    int emailType = contactsDataBase->GetTypeId(ContentTypeData::EMAIL);
    int groupType = contactsDataBase->GetTypeId(ContentTypeData::GROUP_MEMBERSHIP);
    std::vector<int> types;
    types.push_back(nameType);
    types.push_back(phoneType);
    types.push_back(emailType);
    types.push_back(groupType);
    unsigned int size = types.size();
    for (unsigned int i = 0; i < size; i++) {
        std::string sql = "SELECT 1 FROM contact_data WHERE type_id = " + std::to_string(types[i]);
        sql.append(" AND raw_contact_id = " + std::to_string(rawId)).append(" LIMIT 1");
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store->QuerySql(sql);
        int resultSetNum = resultSet->GoToFirstRow();
        if (resultSetNum == 0) {
            switch (i) {
                case HAS_NAME:
                    values.Delete(ContactColumns::HAS_DISPLAY_NAME);
                    values.PutInt(ContactColumns::HAS_DISPLAY_NAME, 1);
                    break;
                case HAS_PHONE:
                    values.Delete(ContactColumns::HAS_PHONE_NUMBER);
                    values.PutInt(ContactColumns::HAS_PHONE_NUMBER, 1);
                    break;
                case HAS_EMAIL:
                    values.Delete(ContactColumns::HAS_EMAIL);
                    values.PutInt(ContactColumns::HAS_EMAIL, 1);
                    break;
                case HAS_GROUP:
                    values.Delete(ContactColumns::HAS_GROUP);
                    values.PutInt(ContactColumns::HAS_GROUP, 1);
                    break;
                default:
                    HILOG_ERROR("AddHasJudge switch code error");
                    break;
            }
        }
    }
}

void MergeUtils::GetRawIdsByRawId(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, std::set<int> &rawIds)
{
    std::string queryMergeId = "SELECT ";
    queryMergeId.append(ContactPublicColumns::ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" = ")
        .append(" (SELECT ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ? ")
        .append(" ) AND is_deleted = 0");
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back(std::to_string(rawId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> rawIdsSet = store->QuerySql(queryMergeId, selectionArgs);
    int resultSetNum = rawIdsSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int value = 0;
        rawIdsSet->GetInt(0, value);
        if (value != rawId) {
            rawIds.insert(value);
        }
        resultSetNum = rawIdsSet->GoToNextRow();
    }
    rawIdsSet->Close();
}
} // namespace Contacts
} // namespace OHOS