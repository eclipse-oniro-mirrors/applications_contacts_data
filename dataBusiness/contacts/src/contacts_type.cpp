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

#include "contacts_type.h"

#include "common.h"
#include "contacts_columns.h"
#include "contacts_database.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
ContactsType::ContactsType(void)
{
}

ContactsType::~ContactsType()
{
}

int64_t ContactsType::Insert(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, std::string typeValue, int typeId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    OHOS::NativeRdb::ValuesBucket values;
    values.PutString(ContentTypeColumns::CONTENT_TYPE, typeValue);
    if (typeId != RDB_OBJECT_EMPTY) {
        values.PutInt(ContentTypeColumns::ID, typeId);
    }
    if (store_ == nullptr) {
        HILOG_ERROR("ContactsType insert store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int64_t outRowId = RDB_EXECUTE_FAIL;
    int ret = store_->Insert(outRowId, ContactTableName::CONTACT_TYPE, values);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsType insert ret :%{public}d", ret);
    }
    return outRowId;
}

void ContactsType::PrepopulateCommonTypes(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore)
{
    std::vector<std::string> vectorType;
    vectorType.push_back(ContentTypeData::EMAIL);
    vectorType.push_back(ContentTypeData::IM);
    vectorType.push_back(ContentTypeData::NICKNAME);
    vectorType.push_back(ContentTypeData::ORGANIZATION);
    vectorType.push_back(ContentTypeData::PHONE);
    vectorType.push_back(ContentTypeData::NAME);
    vectorType.push_back(ContentTypeData::ADDRESS);
    vectorType.push_back(ContentTypeData::PHOTO);
    vectorType.push_back(ContentTypeData::GROUP_MEMBERSHIP);
    vectorType.push_back(ContentTypeData::NOTE);
    vectorType.push_back(ContentTypeData::CONTACT_EVENT);
    vectorType.push_back(ContentTypeData::WEBSITE);
    vectorType.push_back(ContentTypeData::RELATION);
    vectorType.push_back(ContentTypeData::CONTACT_MISC);
    vectorType.push_back(ContentTypeData::HICALL_DEVICE);
    vectorType.push_back(ContentTypeData::CAMCARD);
    vectorType.push_back(ContentTypeData::SIP_ADDRESS);
    int size = (int)vectorType.size();
    for (int i = 0; i < size; i++) {
        std::string typeValue = vectorType[i];
        int error = LookupTypeId(rdbStore, typeValue);
        if (error == RDB_EXECUTE_FAIL) {
            int typeId = i + 1;
            Insert(rdbStore, typeValue, typeId);
        }
    }
}

int ContactsType::LookupTypeId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, std::string typeValue)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    std::string sqlBuilder = "SELECT * FROM ";
    sqlBuilder.append(ContactTableName::CONTACT_TYPE)
        .append(" WHERE ")
        .append(ContentTypeColumns::CONTENT_TYPE)
        .append(" = ? ");
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back(typeValue);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store_->QuerySql(sqlBuilder, selectionArgs);
    int ret = resultSet->GoToFirstRow();
    if (ret != OHOS::NativeRdb::E_OK) {
        return RDB_EXECUTE_FAIL;
    }
    int columnIndex = RDB_EXECUTE_FAIL;
    resultSet->GetColumnIndex(ContactPublicColumns::ID, columnIndex);
    int typeId = RDB_EXECUTE_FAIL;
    resultSet->GetInt(columnIndex, typeId);
    resultSet->Close();
    return typeId;
}

std::string ContactsType::GetTypeText(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int typeId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_ = rdbStore;
    std::string sqlBuilder = "SELECT * FROM ";
    sqlBuilder.append(ContactTableName::CONTACT_TYPE)
        .append(" WHERE ")
        .append(ContactPublicColumns::ID)
        .append(" = ?");
    std::vector<std::string> selectionArgs;
    selectionArgs.push_back(std::to_string(typeId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = store_->QuerySql(sqlBuilder, selectionArgs);
    int ret = resultSet->GoToFirstRow();
    if (ret != OHOS::NativeRdb::E_OK) {
        resultSet->Close();
        HILOG_ERROR("ContactsType lookupTypeText ret :%{public}d", ret);
    }
    std::string TypeText;
    int columnIndex = RDB_EXECUTE_FAIL;
    resultSet->GetColumnIndex(ContentTypeColumns::CONTENT_TYPE, columnIndex);
    resultSet->GetString(columnIndex, TypeText);
    resultSet->Close();
    return TypeText;
}
} // namespace Contacts
} // namespace OHOS