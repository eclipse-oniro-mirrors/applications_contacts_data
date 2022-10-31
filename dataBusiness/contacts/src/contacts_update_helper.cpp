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

#include "contacts_update_helper.h"

#include "calllog_database.h"
#include "character_transliterate.h"
#include "common.h"
#include "construction_name.h"
#include "contacts.h"
#include "contacts_columns.h"
#include "contacts_database.h"
#include "contacts_search.h"
#include "contacts_type.h"
#include "hilog_wrapper.h"
#include "raw_contacts.h"
#include "voicemail_database.h"

namespace OHOS {
namespace Contacts {
ContactsUpdateHelper::ContactsUpdateHelper(void)
{
}

ContactsUpdateHelper::~ContactsUpdateHelper()
{
}

int ContactsUpdateHelper::UpdateDisplay(std::vector<int> rawContactIdVector, std::vector<std::string> types,
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, OHOS::NativeRdb::ValuesBucket contactDataDataValues,
    bool isDelete)
{
    int ret = RDB_EXECUTE_OK;
    unsigned int count = rawContactIdVector.size();
    unsigned int countType = types.size();
    if (countType != count) {
        HILOG_ERROR("ContactsUpdateHelper UpdateDisplay Illegal rawContactId size type:%{public}d", count);
        HILOG_ERROR("ContactsUpdateHelper UpdateDisplay Illegal type size :%{public}d", countType);
        return ret;
    }
    ContactsSearch contactsSearch;
    RawContacts rawContacts;
    Contacts contactsContact;
    for (unsigned int i = 0; i < count; i++) {
        int rawContactId = rawContactIdVector[i];
        std::string type = types[i];
        if (strcmp(type.c_str(), ContentTypeData::ORGANIZATION) == 0) {
            OHOS::NativeRdb::ValuesBucket rawContactValues =
                GetUpdateCompanyValuesBucket(contactDataDataValues, isDelete);
            if (rawContactValues.Size() <= 0) {
                return ret;
            }
            ret = rawContacts.UpdateRawContactById(rawContactId, type, rdbStore, rawContactValues);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateRawContact failed type:%{public}s", type.c_str());
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateRawContact failed:%{public}d", ret);
                return ret;
            }
            OHOS::NativeRdb::ValuesBucket contactValues = GetUpdateCompanyValuesBucket(contactDataDataValues, isDelete);
            if (contactValues.Size() <= 0) {
                return ret;
            }
            ret = contactsContact.UpdateContact(rawContactId, rdbStore, contactValues);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateContact failed type:%{public}s", type.c_str());
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateContact failed:%{public}d", ret);
                return ret;
            }
        } else if (strcmp(type.c_str(), ContentTypeData::NAME) == 0) {
            ret = UpdateName(contactDataDataValues, isDelete, rawContactId, type, rdbStore);
            if (ret != OHOS::NativeRdb::E_OK) {
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateRawContact failed type:%{public}s", type.c_str());
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateRawContact failed:%{public}d", ret);
                return ret;
            }
        } else {
            ret = RDB_EXECUTE_OK;
        }
    }
    return ret;
}

int ContactsUpdateHelper::UpdateName(OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete, int rawContactId,
    std::string type, std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore)
{
    OHOS::NativeRdb::ValuesBucket rawContactValues = GetUpdateDisPlayNameValuesBucket(linkDataDataValues, isDelete);
    OHOS::NativeRdb::ValuesBucket searchContactValues = GetUpdateSearchNameValuesBucket(linkDataDataValues, isDelete);
    std::string disPlayName;
    OHOS::NativeRdb::ValueObject typeValue;
    rawContactValues.GetObject(RawContactColumns::DISPLAY_NAME, typeValue);
    typeValue.GetString(disPlayName);
    if (!disPlayName.empty()) {
        ConstructionName name;
        name.GetConstructionName(disPlayName, name);
        std::string searchName;
        searchName.append(disPlayName);
        if (!name.nameFullFight_.empty()) {
            searchName.append("||");
            searchName.append(name.nameFullFight_);
        }
        if (!name.initials_.empty()) {
            searchName.append("||");
            searchName.append(name.initials_);
        }
        if (!searchName.empty()) {
            searchContactValues.PutString(SearchContactColumns::SEARCH_NAME, searchName);
        }
        rawContactValues.PutString(RawContactColumns::PHOTO_FIRST_NAME, name.photoFirstName_);
        if (!name.sortFirstLetter_.empty()) {
            // add sort and sort_first_letter
            rawContactValues.PutString(RawContactColumns::SORT_FIRST_LETTER, name.sortFirstLetter_);
            rawContactValues.PutString(RawContactColumns::SORT, std::to_string(name.sortFirstLetterCode_));
        }
    }
    int ret = RDB_EXECUTE_OK;
    if (rawContactValues.Size() > 0) {
        RawContacts rawContacts;
        rawContacts.UpdateRawContactById(rawContactId, type, rdbStore, rawContactValues);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateRawContact failed type:%{public}s", type.c_str());
            HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateRawContact failed:%{public}d", ret);
            return ret;
        }
    }
    if (searchContactValues.Size() > 0) {
        // update SearchContact name
        ContactsSearch contactsSearch;
        ret = contactsSearch.UpdateSearchContact(rawContactId, type, rdbStore, searchContactValues);
        if (ret != OHOS::NativeRdb::E_OK) {
            HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateSearchContact failed type:%{public}s", type.c_str());
            HILOG_ERROR("ContactsUpdateHelper UpdateDisplay UpdateSearchContact failed:%{public}d", ret);
            return ret;
        }
    }
    return ret;
}

/**
 * @brief ContactsUpdateHelper update table calllog
 *
 * @param rawContactIdVector Collection of IDs to update
 * @param rdbStore Database that stores the table calllog
 * @param isDelete Contacts field value to update
 *
 * @return Update calllog results code
 */
void ContactsUpdateHelper::UpdateCallLogByPhoneNum(
    std::vector<int> &rawContactIdVector, std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore, bool isDelete)
{
    std::shared_ptr<ContactsDataBase> contactDataBase_ = ContactsDataBase::GetInstance();
    if (rdbStore != contactDataBase_->contactStore_) {
        HILOG_ERROR("UpdateCallLogByPhoneNum rdbStore is profile, not need update");
        return;
    }
    unsigned int count = rawContactIdVector.size();
    for (unsigned int i = 0; i < count; i++) {
        std::string queryContactIdSql;
        queryContactIdSql.append(" SELECT ")
            .append(RawContactColumns::CONTACT_ID)
            .append(" FROM ")
            .append(ViewName::VIEW_RAW_CONTACT)
            .append(" WHERE ")
            .append(ContactPublicColumns::ID)
            .append(" = ")
            .append(std::to_string(rawContactIdVector[i]));
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> rawContactResultSet =
            rdbStore->QuerySql(queryContactIdSql);
        int contactId = 0;
        if (rawContactResultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
            std::string columnName = RawContactColumns::CONTACT_ID;
            int columnIndex = 0;
            rawContactResultSet->GetColumnIndex(columnName, columnIndex);
            rawContactResultSet->GetInt(columnIndex, contactId);
            rawContactResultSet->GoToNextRow();
        }
        rawContactResultSet->Close();
        if (contactId <= 0) {
            return;
        }
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = QueryDataForCallLog(rdbStore, contactId);
        DataToUpdateCallLog(isDelete, contactId, resultSet);
    }
}

void ContactsUpdateHelper::DataToUpdateCallLog(
    bool isDelete, int contactId, std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet)
{
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        std::string phoneNumber;
        std::string name;
        std::string quickSearch;
        // not delete contact
        if (!isDelete) {
            resultSet->GetString(0, name);
        }
        resultSet->GetString(RESULT_GET_ONE, phoneNumber);
        resultSet->GetString(RESULT_GET_TWO, quickSearch);
        UpdateCallLog(phoneNumber, name, quickSearch, isDelete, true);
        UpdateCallLog(phoneNumber, name, quickSearch, isDelete, false);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    if (rowCount == 0) {
        // contact data update callLog not found
        std::string quickSearch = std::to_string(contactId);
        std::string name;
        UpdateCallLogNameNull(name, quickSearch, true);
        UpdateCallLogNameNull(name, quickSearch, false);
    }
}

std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactsUpdateHelper::QueryDataForCallLog(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore, int contactId)
{
    ContactsType contactsType;
    int typeNameId = contactsType.LookupTypeId(rdbStore, ContentTypeData::PHONE);
    std::string sql;
    sql.append("SELECT ")
        .append(RawContactColumns::DISPLAY_NAME)
        .append(",")
        .append(ContactDataColumns::DETAIL_INFO)
        .append(",")
        .append(RawContactColumns::CONTACT_ID)
        .append(" FROM ")
        .append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE ")
        .append(ContactDataColumns::RAW_CONTACT_ID)
        .append(" = (SELECT min(")
        .append(ContactDataColumns::RAW_CONTACT_ID)
        .append(") FROM ")
        .append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE ")
        .append(RawContactColumns::CONTACT_ID)
        .append(" = ")
        .append(std::to_string(contactId))
        .append(" AND ")
        .append(ContentTypeColumns::CONTENT_TYPE)
        .append(" = '")
        .append(ContentTypeData::PHONE)
        .append("') AND ")
        .append(ContactDataColumns::TYPE_ID)
        .append(" = ")
        .append(std::to_string(typeNameId));
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = rdbStore->QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("ContactsUpdateHelper QueryDataForCallLog resultSet is nullptr ");
    }
    return resultSet;
}

int ContactsUpdateHelper::UpdateCallLogNameNull(std::string &name, std::string &quickSearch, bool isCallLog)
{
    std::shared_ptr<CallLogDataBase> callLogDataBase = CallLogDataBase::GetInstance();
    OHOS::NativeRdb::ValuesBucket updateCallLogValues;
    updateCallLogValues.PutNull(CallLogColumns::DISPLAY_NAME);
    updateCallLogValues.PutNull(CallLogColumns::QUICK_SEARCH_KEY);
    std::string tabName = CallsTableName::VOICEMAIL;
    if (isCallLog) {
        tabName = CallsTableName::CALLLOG;
    }
    auto predicates = OHOS::NativeRdb::RdbPredicates(tabName);
    std::string updateWheres;
    updateWheres.append(CallLogColumns::QUICK_SEARCH_KEY).append(" = ? ");
    std::vector<std::string> updateArgs;
    updateArgs.push_back(quickSearch);
    predicates.SetWhereClause(updateWheres);
    predicates.SetWhereArgs(updateArgs);
    int ret = RDB_EXECUTE_FAIL;
    if (isCallLog) {
        ret = callLogDataBase->UpdateCallLog(updateCallLogValues, predicates);
    } else {
        ret = VoiceMailDataBase::GetInstance()->UpdateVoiceMail(updateCallLogValues, predicates);
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateCallLogPhone name error, phone number is %{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

/**
 * @brief Update table calllog
 *
 * @param phoneNumber Contacts's phone number
 * @param name CallLog name to update
 * @param quickSearch Contacts's quick search key
 * @param isDelete Contacts field value to update
 * @param isCallLog Contacts field value to update
 *
 * @return Update calllog results code
 */
int ContactsUpdateHelper::UpdateCallLog(
    std::string &phoneNumber, std::string &name, std::string &quickSearch, bool isDelete, bool isCallLog)
{
    if (phoneNumber.empty()) {
        return RDB_EXECUTE_OK;
    }
    std::shared_ptr<CallLogDataBase> callLogDataBase = CallLogDataBase::GetInstance();
    OHOS::NativeRdb::ValuesBucket updateCallLogValues;
    if (isDelete) {
        updateCallLogValues.PutNull(CallLogColumns::DISPLAY_NAME);
        updateCallLogValues.PutNull(CallLogColumns::QUICK_SEARCH_KEY);
    } else {
        updateCallLogValues.PutString(CallLogColumns::DISPLAY_NAME, name);
        updateCallLogValues.PutString(CallLogColumns::QUICK_SEARCH_KEY, quickSearch);
    }
    updateCallLogValues.PutString(CallLogColumns::PHONE_NUMBER, phoneNumber);
    std::string tabName = CallsTableName::VOICEMAIL;
    if (isCallLog) {
        tabName = CallsTableName::CALLLOG;
    }
    auto predicates = OHOS::NativeRdb::RdbPredicates(tabName);
    std::string updateWheres;
    updateWheres.append(CallLogColumns::PHONE_NUMBER).append(" = ? ");
    std::vector<std::string> updateArgs;
    updateArgs.push_back(phoneNumber);
    predicates.SetWhereClause(updateWheres);
    predicates.SetWhereArgs(updateArgs);
    int ret = RDB_EXECUTE_FAIL;
    if (isCallLog) {
        ret = callLogDataBase->UpdateCallLog(updateCallLogValues, predicates);
    } else {
        ret = VoiceMailDataBase::GetInstance()->UpdateVoiceMail(updateCallLogValues, predicates);
    }
    if (ret != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("UpdateCallLogPhone name error, phone number is %{public}d", ret);
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

OHOS::NativeRdb::ValuesBucket ContactsUpdateHelper::GetUpdateDisPlayNameValuesBucket(
    OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    if (isDelete) {
        valuesBucket.PutNull(RawContactColumns::DISPLAY_NAME);
        valuesBucket.PutNull(RawContactColumns::SORT_FIRST_LETTER);
        valuesBucket.PutNull(RawContactColumns::SORT);
        return valuesBucket;
    }
    std::string displayName;
    if (linkDataDataValues.HasColumn(ContactDataColumns::DETAIL_INFO)) {
        OHOS::NativeRdb::ValueObject typeValue;
        linkDataDataValues.GetObject(ContactDataColumns::DETAIL_INFO, typeValue);
        typeValue.GetString(displayName);
    }
    valuesBucket.PutString(RawContactColumns::DISPLAY_NAME, displayName);
    std::string phoneticName;
    if (linkDataDataValues.HasColumn(RawContactColumns::PHONETIC_NAME)) {
        OHOS::NativeRdb::ValueObject typeValue;
        linkDataDataValues.GetObject(ContactDataColumns::PHONETIC_NAME, typeValue);
        typeValue.GetString(phoneticName);
    }
    valuesBucket.PutString(RawContactColumns::PHONETIC_NAME, phoneticName);
    return valuesBucket;
}

OHOS::NativeRdb::ValuesBucket ContactsUpdateHelper::GetUpdateSearchNameValuesBucket(
    OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    if (isDelete) {
        valuesBucket.PutNull(SearchContactColumns::SEARCH_NAME);
        return valuesBucket;
    }
    std::string displayName;
    if (linkDataDataValues.HasColumn(ContactDataColumns::DETAIL_INFO)) {
        OHOS::NativeRdb::ValueObject typeValue;
        linkDataDataValues.GetObject(ContactDataColumns::DETAIL_INFO, typeValue);
        typeValue.GetString(displayName);
    }
    valuesBucket.PutString(RawContactColumns::DISPLAY_NAME, displayName);
    return valuesBucket;
}

OHOS::NativeRdb::ValuesBucket ContactsUpdateHelper::GetUpdateCompanyValuesBucket(
    OHOS::NativeRdb::ValuesBucket linkDataDataValues, bool isDelete)
{
    OHOS::NativeRdb::ValuesBucket valuesBucket;
    if (isDelete) {
        valuesBucket.PutNull(RawContactColumns::COMPANY);
        valuesBucket.PutNull(RawContactColumns::POSITION);
        return valuesBucket;
    }
    if (linkDataDataValues.HasColumn(ContactDataColumns::DETAIL_INFO)) {
        std::string company;
        OHOS::NativeRdb::ValueObject typeValue;
        linkDataDataValues.GetObject(ContactDataColumns::DETAIL_INFO, typeValue);
        typeValue.GetString(company);
        valuesBucket.PutString(RawContactColumns::COMPANY, company);
    }
    if (linkDataDataValues.HasColumn(ContactDataColumns::POSITION)) {
        std::string position;
        OHOS::NativeRdb::ValueObject typeValue;
        linkDataDataValues.GetObject(ContactDataColumns::POSITION, typeValue);
        typeValue.GetString(position);
        valuesBucket.PutString(RawContactColumns::POSITION, position);
    }
    return valuesBucket;
}
} // namespace Contacts
} // namespace OHOS