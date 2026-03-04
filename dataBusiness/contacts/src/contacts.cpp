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

#include "contacts.h"

#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
Contacts::Contacts(void)
{
}

Contacts::~Contacts()
{
}

/**
 * @brief Insert the raw_contact table at the same time as the contact table
 *
 * @param rdbStore Database operation object
 * @param outRawContactId raw_contact table id
 * @param rawContactValues inserted raw_contact values
 * @param contactId contact_id address
 *
 * @return insert contact error or success code
 */
int Contacts::InsertContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, const int64_t &outRawContactId,
    OHOS::NativeRdb::ValuesBucket rawContactValues, int64_t &contactId)
{
    OHOS::NativeRdb::ValuesBucket contactValues = StructureContactDataValueBucket(rawContactValues);
    contactValues.PutInt(ContactColumns::NAME_RAW_CONTACT_ID, outRawContactId);
    int rowContactRet = rdbStore->Insert(contactId, ContactTableName::CONTACT, contactValues);
    return rowContactRet;
}

int Contacts::UpdateContact(const int &rawContactId, std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore,
    OHOS::NativeRdb::ValuesBucket linkDataDataValues)
{
    std::string upWhereClause;
    upWhereClause.append(ContactColumns::NAME_RAW_CONTACT_ID).append(" = ?");
    std::vector<std::string> upWhereArgs;
    upWhereArgs.push_back(std::to_string(rawContactId));
    int changedRows = OHOS::NativeRdb::E_OK;
    int ret = rdbStore->Update(changedRows, ContactTableName::CONTACT, linkDataDataValues, upWhereClause, upWhereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsUpdateHelper UpdateDisplay  UpdateSearchContact fail:%{public}d", ret);
    }
    return ret;
}

/**
 * @brief Build conatct data
 *
 * @param rawContactValues inserted contact values
 *
 * @return contact table values
 */
OHOS::NativeRdb::ValuesBucket Contacts::StructureContactDataValueBucket(OHOS::NativeRdb::ValuesBucket rawContactValues)
{
    OHOS::NativeRdb::ValuesBucket contactValueBucket;
    if (rawContactValues.HasColumn(RawContactColumns::PHOTO_FILE_ID)) {
        int result = ContactValueBucketGetInt(rawContactValues, RawContactColumns::PHOTO_FILE_ID);
        contactValueBucket.PutInt(ContactColumns::PHOTO_FILE_ID, result);
    }
    if (rawContactValues.HasColumn(RawContactColumns::READ_ONLY)) {
        int result = ContactValueBucketGetInt(rawContactValues, RawContactColumns::READ_ONLY);
        contactValueBucket.PutInt(ContactColumns::READ_ONLY, result);
    }
    if (rawContactValues.HasColumn(RawContactColumns::PHOTO_ID)) {
        int result = ContactValueBucketGetInt(rawContactValues, RawContactColumns::PHOTO_ID);
        contactValueBucket.PutInt(ContactColumns::PHOTO_ID, result);
    }
    if (rawContactValues.HasColumn(RawContactColumns::IS_TRANSFER_VOICEMAIL)) {
        int result = ContactValueBucketGetInt(rawContactValues, RawContactColumns::IS_TRANSFER_VOICEMAIL);
        contactValueBucket.PutInt(ContactColumns::IS_TRANSFER_VOICEMAIL, result);
    }
    if (rawContactValues.HasColumn(RawContactColumns::COMPANY)) {
        std::string result = ContactValueBucketGetString(rawContactValues, RawContactColumns::COMPANY);
        contactValueBucket.PutString(ContactColumns::COMPANY, result);
    }
    if (rawContactValues.HasColumn(RawContactColumns::POSITION)) {
        std::string result = ContactValueBucketGetString(rawContactValues, RawContactColumns::POSITION);
        contactValueBucket.PutString(ContactColumns::POSITION, result);
    }
    return contactValueBucket;
}

int Contacts::ContactValueBucketGetInt(OHOS::NativeRdb::ValuesBucket rawContactValues, std::string columnName)
{
    int result = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::ValueObject value;
    rawContactValues.GetObject(columnName, value);
    value.GetInt(result);
    return result;
}

std::string Contacts::ContactValueBucketGetString(
    OHOS::NativeRdb::ValuesBucket rawContactValues, std::string columnName)
{
    std::string result;
    OHOS::NativeRdb::ValueObject value;
    rawContactValues.GetObject(columnName, value);
    value.GetString(result);
    return result;
}

int Contacts::DeleteContactById(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int needDeleteContactId)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store = rdbStore;
    if (store == nullptr) {
        HILOG_ERROR("ContactsAccount DeletecontactById store is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    if (needDeleteContactId < ID_EMPTY) {
        HILOG_ERROR("ContactsAccount DeleteDataByRawId needDeleteContactId illegal");
        return OPERATION_ERROR;
    }
    int rowId = OHOS::NativeRdb::E_OK;
    std::vector<std::string> whereArgs;
    whereArgs.push_back(std::to_string(needDeleteContactId));
    std::string whereCase;
    whereCase.append(ContactColumns::ID).append(" = ?");
    int delContact = store->Delete(rowId, ContactTableName::CONTACT, whereCase, whereArgs);
    return delContact;
}
} // namespace Contacts
} // namespace OHOS