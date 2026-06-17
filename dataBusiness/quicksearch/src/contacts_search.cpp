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

#include "contacts_search.h"

#include "rdb_errno.h"
#include "result_set.h"

#include "character_transliterate.h"
#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
ContactsSearch::ContactsSearch(void)
{
}

ContactsSearch::~ContactsSearch()
{
}

int64_t ContactsSearch::Insert(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int64_t contactId,
    int64_t rawContactId, OHOS::NativeRdb::ValuesBucket rawContactValues, int64_t &searchContactId)
{
    // searchContact insert data
    OHOS::NativeRdb::ValuesBucket searchContactValues = StructureSearchContactDataValueBucket(rawContactValues);
    // add raw_contact_id
    searchContactValues.PutInt(SearchContactColumns::RAW_CONTACT_ID, rawContactId);
    // add contact_id
    searchContactValues.PutInt(SearchContactColumns::CONTACT_ID, contactId);
    int rowSearchContactRet = rdbStore->Insert(searchContactId, ContactTableName::SEARCH_CONTACT, searchContactValues);
    return rowSearchContactRet;
}

/**
 * @brief Update contact data by quick_search_key
 *
 * @param rawContactId Parameters to be passed for update operation
 * @param type Parameters to be passed for update operation
 * @param rdbStore Conditions for update operation
 * @param searchContactValues Parameters to be passed for update operation
 *
 * @return The result returned by the update operation
 */
int ContactsSearch::UpdateSearchContact(int rawContactId, std::string type,
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, OHOS::NativeRdb::ValuesBucket searchContactValues)
{
    std::string upWhereClause;
    upWhereClause.append(SearchContactColumns::RAW_CONTACT_ID).append(" = ?");
    std::vector<std::string> upWhereArgs;
    upWhereArgs.push_back(std::to_string(rawContactId));
    int changedRows;
    // update search_contact table name
    int ret = rdbStore->Update(
        changedRows, ContactTableName::SEARCH_CONTACT, searchContactValues, upWhereClause, upWhereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("ContactsUpdateHelper UpdateDisplay  UpdateSearchContact fail:%{public}d", ret);
    }
    return ret;
}

/**
 * @brief Convert the rawcontact table insert parameter to the searchcontact table parameter
 *
 * @param rawContactValues Parameters to be passed for convert operation
 *
 * @return The result returned by the convert operation
 */
OHOS::NativeRdb::ValuesBucket ContactsSearch::StructureSearchContactDataValueBucket(
    OHOS::NativeRdb::ValuesBucket rawContactValues)
{
    // Will raw_ Convert contact table data to searchcontact table data
    OHOS::NativeRdb::ValuesBucket searchContactValueBucket;
    if (rawContactValues.HasColumn(RawContactColumns::ACCOUNT_ID)) {
        int accountId;
        OHOS::NativeRdb::ValueObject value;
        rawContactValues.GetObject(RawContactColumns::ACCOUNT_ID, value);
        value.GetInt(accountId);
        searchContactValueBucket.PutInt(SearchContactColumns::ACCOUNT_ID, accountId);
    }
    if (rawContactValues.HasColumn(RawContactColumns::DISPLAY_NAME)) {
        std::string disPlayName;
        OHOS::NativeRdb::ValueObject value;
        rawContactValues.GetObject(RawContactColumns::DISPLAY_NAME, value);
        value.GetString(disPlayName);
        searchContactValueBucket.PutString(SearchContactColumns::DISPLAY_NAME, disPlayName);
    }
    if (rawContactValues.HasColumn(RawContactColumns::PHONETIC_NAME)) {
        std::string phoneticName;
        OHOS::NativeRdb::ValueObject value;
        rawContactValues.GetObject(RawContactColumns::PHONETIC_NAME, value);
        value.GetString(phoneticName);
        searchContactValueBucket.PutString(SearchContactColumns::PHONETIC_NAME, phoneticName);
    }
    if (rawContactValues.HasColumn(RawContactColumns::PHOTO_FILE_ID)) {
        int photoFileId;
        OHOS::NativeRdb::ValueObject value;
        rawContactValues.GetObject(RawContactColumns::PHOTO_FILE_ID, value);
        value.GetInt(photoFileId);
        searchContactValueBucket.PutInt(SearchContactColumns::PHOTO_FILE_ID, photoFileId);
    }
    if (rawContactValues.HasColumn(RawContactColumns::PHOTO_ID)) {
        int photoId;
        OHOS::NativeRdb::ValueObject value;
        rawContactValues.GetObject(RawContactColumns::PHOTO_ID, value);
        value.GetInt(photoId);
        searchContactValueBucket.PutInt(SearchContactColumns::PHOTO_ID, photoId);
    }
    return searchContactValueBucket;
}
} // namespace Contacts
} // namespace OHOS