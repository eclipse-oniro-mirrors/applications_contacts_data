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

#include "contacts_account.h"

#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
std::shared_ptr<ContactsAccount> ContactsAccount::instance_ = nullptr;

std::shared_ptr<ContactsAccount> ContactsAccount::GetInstance()
{
    if (instance_ == nullptr) {
        instance_.reset(new ContactsAccount());
    }
    return instance_;
}

ContactsAccount::ContactsAccount(void)
{
}

ContactsAccount::~ContactsAccount()
{
}

/**
 * @brief Insert ContactsAccount into rdbStore
 *
 * @param rdbStore Insert operation based on rdbStore
 * @param accountName Pass in parameter account name
 * @param accountType Pass in parameter account type
 *
 * @return Insert database results code
 */
int64_t ContactsAccount::Insert(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, std::string accountName, std::string accountType)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    OHOS::NativeRdb::ValuesBucket values;
    values.PutString(AccountColumns::ACCOUNT_NAME, accountName);
    values.PutString(AccountColumns::ACCOUNT_TYPE, accountType);
    int64_t outRowId = OHOS::NativeRdb::E_OK;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsAccount insert store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int ret = store_->Insert(outRowId, ContactTableName::ACCOUNT, values);
    HILOG_INFO(" ContactsAccount insert ret :%{public}d", ret);
    return outRowId;
};

void ContactsAccount::PrepopulateCommonAccountTypes(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore)
{
    int typeResult = LookupAccountTypeId(rdbStore, AccountData::ACCOUNT_NAME, AccountData::ACCOUNT_TYPE);
    if (typeResult == RDB_EXECUTE_FAIL) {
        Insert(rdbStore, AccountData::ACCOUNT_NAME, AccountData::ACCOUNT_TYPE);
    } else {
        HILOG_INFO("ContactsAccount account is exist");
    }
}

int ContactsAccount::LookupAccountTypeId(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, std::string accountName, std::string accountType)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    std::string sqlBuilder = "SELECT * FROM ";
    sqlBuilder.append(ContactTableName::ACCOUNT)
        .append(" WHERE ")
        .append(AccountColumns::ACCOUNT_NAME)
        .append(" = ")
        .append("'")
        .append(accountName)
        .append("'")
        .append(" AND ")
        .append(AccountColumns::ACCOUNT_TYPE)
        .append(" = ")
        .append("'")
        .append(accountType)
        .append("'");
    std::vector<std::string> selectionArgs;
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store_->QuerySql(sqlBuilder, selectionArgs);
    int ret = resultSet->GoToFirstRow();
    if (ret != OHOS::NativeRdb::E_OK) {
        resultSet->Close();
        return RDB_EXECUTE_FAIL;
    }
    int columnIndex = 0;
    resultSet->GetColumnIndex(ContactPublicColumns::ID, columnIndex);
    int accountId = 0;
    resultSet->GetInt(columnIndex, accountId);
    resultSet->Close();
    return accountId;
}

std::vector<AccountDataCollection> ContactsAccount::GetAccountFromLocal(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    std::vector<AccountDataCollection> values;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsAccount GetAccountFromLocal store_ is nullptr");
        return values;
    }
    std::string buildQuery = "";
    buildQuery.append("select ")
        .append(AccountColumns::ACCOUNT_NAME)
        .append(",")
        .append(AccountColumns::ACCOUNT_TYPE)
        .append(",")
        .append(AccountColumns::DATA_INFO)
        .append(" from ")
        .append(ContactTableName::ACCOUNT);
    std::vector<std::string> selectArgs;
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> result = store_->QuerySql(buildQuery, selectArgs);
    int resultSetNum = result->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        std::string accountName;
        std::string accountType;
        std::string accountCollection;
        int accountNameIndex = 0;
        int accountTypeIndex = 0;
        int accountCollectionIndex = 0;
        result->GetColumnIndex(AccountColumns::ACCOUNT_NAME, accountNameIndex);
        result->GetColumnIndex(AccountColumns::ACCOUNT_TYPE, accountTypeIndex);
        result->GetColumnIndex(AccountColumns::DATA_INFO, accountCollectionIndex);
        result->GetString(accountNameIndex, accountName);
        result->GetString(accountTypeIndex, accountType);
        result->GetString(accountCollectionIndex, accountCollection);
        AccountDataCollection collection = AccountDataCollection(accountName, accountType, accountCollection);
        values.push_back(collection);
        resultSetNum = result->GoToNextRow();
    }
    result->Close();
    return values;
}

int ContactsAccount::GetNotExistAccount(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, AccountDataCollection collection)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsAccount GetNotExistAccount store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (collection.GetcAccountName() != "" || collection.GetcAccountType() != "") {
        std::vector<std::string> selectArgs;
        std::string buildSql = "";
        buildSql.append("select ")
            .append(AccountColumns::ID)
            .append(" from ")
            .append(ContactTableName::ACCOUNT)
            .append(" where ")
            .append(AccountColumns::ACCOUNT_NAME)
            .append(" = ? and ")
            .append(AccountColumns::ACCOUNT_TYPE)
            .append(" = ? and (")
            .append(AccountColumns::DATA_INFO)
            .append(" IS NULL or ")
            .append(AccountColumns::DATA_INFO)
            .append(" = ?)");
        selectArgs.push_back(collection.GetcAccountName());
        selectArgs.push_back(collection.GetcAccountType());
        selectArgs.push_back(collection.GetcDataCollection());
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> result = store_->QuerySql(buildSql, selectArgs);
        int resultSetNum = result->GoToFirstRow();
        int reValue = RDB_EXECUTE_FAIL;
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            int reValueIndex = 0;
            result->GetColumnIndex(AccountColumns::ID, reValueIndex);
            if (reValueIndex < 0) {
                break;
            }
            result->GetInt(reValueIndex, reValue);
            result->GoToNextRow();
            break;
        }
        result->Close();
        return reValue;
    }
    return OPERATION_ERROR;
}

int ContactsAccount::DeleteAccountByAccountId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int accountId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsAccount DeleteAccountByAccountId store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (accountId < ID_EMPTY) {
        return OPERATION_ERROR;
    }
    int rowId = 0;
    std::vector<std::string> whereArgs;
    whereArgs.push_back(std::to_string(accountId));
    std::string whereCase;
    whereCase.append(AccountColumns::ID).append(" = ?");
    int delAccount = store_->Delete(rowId, ContactTableName::ACCOUNT, whereCase, whereArgs);
    return delAccount;
}

int ContactsAccount::DeleteDataByRawId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int needDeleteRawContactId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsAccount DeleteDataByRawId store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (needDeleteRawContactId < ID_EMPTY) {
        return OPERATION_ERROR;
    }
    int rowId = 0;
    std::vector<std::string> whereArgs;
    whereArgs.push_back(std::to_string(needDeleteRawContactId));
    std::string whereCase;
    whereCase.append(ContactDataColumns::RAW_CONTACT_ID).append(" = ?");
    int delData = store_->Delete(rowId, ContactTableName::CONTACT_DATA, whereCase, whereArgs);
    return delData;
}

int ContactsAccount::DeleteGroupsByAccountId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int accountId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsAccount DeleteGroupsByAccountId store_ is  nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (accountId < ID_EMPTY) {
        return OPERATION_ERROR;
    }
    int rowId = 0;
    std::vector<std::string> whereArgs;
    whereArgs.push_back(std::to_string(accountId));
    std::string whereCase;
    whereCase.append(GroupsColumns::ACCOUNT_ID).append(" = ? ");
    int delGroup = store_->Delete(rowId, ContactTableName::GROUPS, whereCase, whereArgs);
    return delGroup;
}

int ContactsAccount::StopForegin(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsAccount StopForegin store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    return store_->ExecuteSql("PRAGMA foreign_keys = OFF");
}

int ContactsAccount::OpenForegin(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsAccount OpenForegin store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    return store_->ExecuteSql("PRAGMA foreign_keys = ON");
}
} // namespace Contacts
} // namespace OHOS