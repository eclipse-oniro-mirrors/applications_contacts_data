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

#include "raw_contacts.h"

#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
RawContacts::RawContacts(void)
{
}

RawContacts::~RawContacts()
{
}

/**
 * @brief Insert data to the raw_contact table
 *
 * @param rdbStore Insert operation based on rdbStore
 * @param rawContactValues Raw contact values to be inserted
 *
 * @return Insert database results code
 */
int RawContacts::InsertRawContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int64_t &outRawContactId,
    OHOS::NativeRdb::ValuesBucket rawContactValues)
{
    int rowRet = rdbStore->Insert(outRawContactId, ContactTableName::RAW_CONTACT, rawContactValues);
    if (rowRet != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("RawContacts InsertRawContact fail:%{public}d", rowRet);
    }
    return rowRet;
}

/**
 * @brief Update data in the raw_contact table
 *
 * @param rdbStore Update operation based on rdbStore
 * @param upRawContactValues Pass in parameter upRawContactValues
 * @param whereClause Clauses for update operation
 * @param whereArgs Conditions for update operation
 *
 * @return Update database results code
 */
int RawContacts::UpdateRawContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore,
    OHOS::NativeRdb::ValuesBucket upRawContactValues, std::string whereClause, std::vector<std::string> whereArgs)
{
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = rdbStore->Update(changedRows, ContactTableName::RAW_CONTACT, upRawContactValues, whereClause, whereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("RawContacts UpdateRawContact fail:%{public}d", ret);
    }
    return ret;
}

int RawContacts::UpdateRawContactById(int &rawContactId, std::string type,
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, OHOS::NativeRdb::ValuesBucket rawContactValues)
{
    std::string upWhereClause;
    upWhereClause.append(ContactPublicColumns::ID).append(" = ?");
    std::vector<std::string> upWhereArgs;
    upWhereArgs.push_back(std::to_string(rawContactId));
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret =
        rdbStore->Update(changedRows, ContactTableName::RAW_CONTACT, rawContactValues, upWhereClause, upWhereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("RawContacts  UpdateRawContactById fail:%{public}d", ret);
    }
    return ret;
}

int RawContacts::GetDeleteContactIdByAccountId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int accountId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("RawContacts GetDeleteContactIdByAccountId store_ is  nullptr or accountId illegal");
        return RDB_OBJECT_EMPTY;
    }
    if (accountId < ID_EMPTY) {
        HILOG_ERROR("RawContacts GetDeleteContactIdByAccountId accountId illegal");
        return OPERATION_ERROR;
    }
    std::vector<std::string> selectArgs;
    selectArgs.push_back(std::to_string(accountId));
    selectArgs.push_back(std::to_string(accountId));
    std::string sql = "";
    sql.append("SELECT ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(RawContactColumns::ACCOUNT_ID)
        .append(" = ? AND ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" NOT NULL AND ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" NOT IN (")
        .append(" SELECT ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(RawContactColumns::ACCOUNT_ID)
        .append(" != ?")
        .append(" AND ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" NOT NULL )");
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> rawResult = store_->QuerySql(sql, selectArgs);
    int resultSetNum = rawResult->GoToFirstRow();
    int currConcactIdValue = 0;
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        int currValueIndex = 0;
        rawResult->GetColumnIndex(RawContactColumns::CONTACT_ID, currValueIndex);
        rawResult->GetInt(currValueIndex, currConcactIdValue);
        if (currConcactIdValue > 0) {
            break;
        }
        resultSetNum = rawResult->GoToNextRow();
    }
    rawResult->Close();
    return currConcactIdValue;
}

int RawContacts::GetDeleteRawContactIdByAccountId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int accountId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("RawContacts GetDeleteRawContactIdByAccountId store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (accountId < ID_EMPTY) {
        HILOG_ERROR("RawContacts GetDeleteContactIdByAccountId accountId illegal");
        return OPERATION_ERROR;
    }
    std::vector<std::string> selectArgs;
    selectArgs.push_back(std::to_string(accountId));
    std::string sql = "";
    sql.append("SELECT ")
        .append(RawContactColumns::ID)
        .append(" FROM ")
        .append(ContactTableName::RAW_CONTACT)
        .append(" WHERE ")
        .append(RawContactColumns::ACCOUNT_ID)
        .append(" = ?")
        .append(" AND ")
        .append(RawContactColumns::ACCOUNT_ID)
        .append(" NOT NULL");
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> rawResult = store_->QuerySql(sql, selectArgs);
    int resultSetNum = rawResult->GoToFirstRow();
    int currConcactIdValue = 0;
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        rawResult->GetInt(0, currConcactIdValue);
        if (currConcactIdValue > 0) {
            break;
        }
        resultSetNum = rawResult->GoToNextRow();
    }
    rawResult->Close();
    return currConcactIdValue;
}

int RawContacts::DeleteRawcontactByRawId(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int needDeleteRawContactId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("RawContacts DeleteRawcontactByRawId store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (needDeleteRawContactId < ID_EMPTY) {
        HILOG_ERROR("RawContacts DeleteRawcontactByRawId needDeleteRawContactId illegal");
        return OPERATION_ERROR;
    }
    int rowId = OHOS::NativeRdb::E_OK;
    std::vector<std::string> whereArgs;
    whereArgs.push_back(std::to_string(needDeleteRawContactId));
    std::string whereCase;
    whereCase.append(RawContactColumns::ID).append(" = ?");
    int delRawContact = store_->Delete(rowId, ContactTableName::RAW_CONTACT, whereCase, whereArgs);
    HILOG_INFO("DeleteRawcontactByRawId : status is %{public}d", delRawContact);
    return delRawContact;
}
} // namespace Contacts
} // namespace OHOS