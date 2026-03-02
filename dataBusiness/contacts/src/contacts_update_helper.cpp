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
#include "contacts_type.h"
#include "hilog_wrapper.h"
#include "raw_contacts.h"
#include "voicemail_database.h"
#include "contacts_common_event.h"
#include "rdb_utils.h"
#include "dataobs_mgr_client.h"

namespace OHOS {
namespace Contacts {
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;

ContactsUpdateHelper::ContactsUpdateHelper(void)
{}

ContactsUpdateHelper::~ContactsUpdateHelper()
{}

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
    ret = UpdateDisplayResult(rawContactIdVector, types, rdbStore, contactDataDataValues, isDelete, count);
    return ret;
}

int ContactsUpdateHelper::UpdateDisplayResult(std::vector<int> rawContactIdVector, std::vector<std::string> types,
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, OHOS::NativeRdb::ValuesBucket contactDataDataValues,
    bool isDelete, unsigned int count)
{
    int ret = RDB_EXECUTE_OK;
    RawContacts rawContacts;
    Contacts contactsContact;
    for (unsigned int i = 0; i < count; i++) {
        int rawContactId = rawContactIdVector[i];
        std::string type = types[i];
        if (strcmp(type.c_str(), ContentTypeData::ORGANIZATION) == 0) {
            OHOS::NativeRdb::ValuesBucket rawContactValues =
                GetUpdateCompanyValuesBucket(contactDataDataValues, isDelete);
            if (rawContactValues.Size() <= 0) {
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay Illegal rawContactValues.Size() :%{public}d",
                    rawContactValues.Size());
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay failed type:%{public}s", type.c_str());
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
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay Illegal contactValues.Size() :%{public}d",
                    contactValues.Size());
                HILOG_ERROR("ContactsUpdateHelper UpdateDisplay failed type:%{public}s", type.c_str());
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
                HILOG_ERROR("ContactsUpdateHelper UpdateRawContact failed type:%{public}s", type.c_str());
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
    std::string disPlayName;
    OHOS::NativeRdb::ValueObject typeValue;
    rawContactValues.GetObject(RawContactColumns::DISPLAY_NAME, typeValue);
    typeValue.GetString(disPlayName);
    if (!disPlayName.empty()) {
        ConstructionName name;
        name.GetConstructionName(disPlayName, name);
        rawContactValues.PutString(RawContactColumns::PHOTO_FIRST_NAME, name.photoFirstName_);
        if (!name.sortFirstLetter_.empty()) {
            // add sort and sort_first_letter
            rawContactValues.PutString(RawContactColumns::SORT_FIRST_LETTER, name.sortFirstLetter_);
            rawContactValues.PutString(RawContactColumns::SORT, std::to_string(name.sortFirstLetterCode_));
        }
        // 如果名称disPlayName不为空，才会生成排序关键字
        if (!name.sortKey.empty()) {
            rawContactValues.PutString(RawContactColumns::SORT_KEY, name.sortKey);
        }
    }
    int ret = UpdateNameResult(rawContactValues, rawContactId, type, rdbStore);
    return ret;
}

int ContactsUpdateHelper::UpdateNameResult(OHOS::NativeRdb::ValuesBucket rawContactValues, int rawContactId,
    std::string type, std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore)
{
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
    return ret;
}

/**
 * @brief ContactsUpdateHelper update table calllog
 *
 * @param rawContactIdVector Collection of IDs to update
 * @param rdbStore Database that stores the table calllog
 *
 * @return Update calllog results code
 */
void ContactsUpdateHelper::UpdateCallLogWhenBatchDelContact(
    std::vector<std::string> &contactIdVector, std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
{
    if (contactIdVector.empty()) {
        HILOG_ERROR("ContactsUpdateHelper UpdateCallLogWhenBatchDelContact contactIdVector is empty");
        return;
    }
    std::set<std::string> detailInfoSet;
    std::vector<std::string> detailInfos = QueryDeletedPhoneData(contactIdVector, detailInfoSet, rdbStore);
    if (detailInfos.empty()) {
        HILOG_WARN("ContactsUpdateHelper UpdateCallLogWhenBatchDelContact detailInfos is empty");
        return;
    }
    std::map<std::string, std::pair<std::string, int>> phoneToNewContactId =
        CheckPhoneInOtherContact(contactIdVector, detailInfos, detailInfoSet, rdbStore);
    std::shared_ptr<CallLogDataBase> callLogDataBase = CallLogDataBase::GetInstance();
    HILOG_WARN("ContactsUpdateHelper UpdateCallLogWhenBatchDelContact detailInfoSet size:%{public}ld,"
               "phoneToNewContactId size:%{public}ld",
        (long) detailInfoSet.size(),
        (long) phoneToNewContactId.size());
    int ret = 0;
    if (!detailInfoSet.empty()) {
        std::vector<std::string> deletePhoneVector;
        deletePhoneVector.assign(detailInfoSet.begin(), detailInfoSet.end());
        OHOS::NativeRdb::ValuesBucket updateCallLogValues;
        // 该联系人被删了，那就所有关联通话记录相关字段都置空
        updateCallLogValues.PutNull(CallLogColumns::DISPLAY_NAME);
        updateCallLogValues.PutNull(CallLogColumns::QUICK_SEARCH_KEY);
        updateCallLogValues.PutNull(CallLogColumns::EXTRA1);
        std::string tabName = CallsTableName::CALLLOG;
        // 根据quicksearch 与号码字段，取消通话记录和联系人的关联
        auto predicates = OHOS::NativeRdb::RdbPredicates(tabName);
        predicates.In(CallLogColumns::QUICK_SEARCH_KEY, contactIdVector);
        predicates.And();
        predicates.BeginWrap();
        predicates.In(CallLogColumns::FORMAT_PHONE_NUMBER, deletePhoneVector);
        predicates.Or();
        predicates.In(CallLogColumns::PHONE_NUMBER, deletePhoneVector);
        predicates.EndWrap();
        ret = callLogDataBase->UpdateCallLog(updateCallLogValues, predicates);
        if (ret < RDB_EXECUTE_OK) {
            HILOG_ERROR("ContactsUpdateHelper UpdateCallLogWhenBatchDelContact error,ret:%{public}d,", ret);
            return;
        }
    }
    if (!phoneToNewContactId.empty()) {
        ret += UpdateCallLogToOtherContact(phoneToNewContactId, callLogDataBase);
    }
    HILOG_INFO("ContactsUpdateHelper UpdateCallLogWhenBatchDelContact,ret = %{public}d", ret);
    UpdateCallLogByPhoneNumNotifyChange(ret);
}

std::vector<std::string> ContactsUpdateHelper::QueryDeletedPhoneData(std::vector<std::string> &contactIdArr,
    std::set<std::string> &detailInfoSet, std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_)
{
    auto predicates = OHOS::NativeRdb::RdbPredicates(ViewName::VIEW_CONTACT_DATA);
    predicates.In(RawContactColumns::CONTACT_ID, contactIdArr);
    predicates.And();
    predicates.EqualTo(ContactDataColumns::TYPE_ID, ContentTypeData::PHONE_INT_VALUE);

    std::vector<std::string> columns = {ContactDataColumns::DETAIL_INFO, ContactDataColumns::FORMAT_PHONE_NUMBER};
    auto resultSet = store_->Query(predicates, columns);
    int getRowResult = resultSet->GoToFirstRow();
    std::vector<std::string> detailInfos;
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        std::string detailInfo;
        resultSet->GetString(INDEX_ZERO, detailInfo);
        std::string formatPhoneNumber;
        resultSet->GetString(INDEX_ONE, formatPhoneNumber);
        if (!formatPhoneNumber.empty()) {
            detailInfos.push_back(formatPhoneNumber);
            detailInfoSet.insert(formatPhoneNumber);
        } else {
            detailInfos.push_back(detailInfo);
            detailInfoSet.insert(detailInfo);
        }
        getRowResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return detailInfos;
}

std::map<std::string, std::pair<std::string, int>> ContactsUpdateHelper::CheckPhoneInOtherContact(
    std::vector<std::string> &contactIdArr, std::vector<std::string> &detailInfos, std::set<std::string> &detailInfoSet,
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_)
{
    std::map<std::string, std::pair<std::string, int>> phoneToNewContactId;
    auto predicates = OHOS::NativeRdb::RdbPredicates(ViewName::VIEW_CONTACT_DATA);
    predicates.NotIn(RawContactColumns::CONTACT_ID, contactIdArr);
    predicates.And();
    predicates.BeginWrap();
    predicates.In(ContactDataColumns::FORMAT_PHONE_NUMBER, detailInfos);
    predicates.Or();
    predicates.In(ContactDataColumns::DETAIL_INFO, detailInfos);
    predicates.EndWrap();
    predicates.And();
    predicates.EqualTo(RawContactColumns::IS_DELETED, 0);
    predicates.And();
    predicates.EqualTo(ContactDataColumns::TYPE_ID, ContentTypeData::PHONE_INT_VALUE);

    std::vector<std::string> columns = {ContactDataColumns::DETAIL_INFO,
        ContactDataColumns::FORMAT_PHONE_NUMBER,
        RawContactColumns::CONTACT_ID,
        RawContactColumns::DISPLAY_NAME};
    auto queryResultSet = store_->Query(predicates, columns);
    int getRowResult = queryResultSet->GoToFirstRow();
    while (getRowResult == OHOS::NativeRdb::E_OK) {
        std::string formatPhoneNumber;
        queryResultSet->GetString(INDEX_ONE, formatPhoneNumber);
        int contactId;
        queryResultSet->GetInt(INDEX_TWO, contactId);
        std::string displayName;
        queryResultSet->GetString(INDEX_THREE, displayName);
        if (!formatPhoneNumber.empty()) {
            detailInfoSet.erase(formatPhoneNumber);
            phoneToNewContactId.insert(std::pair<std::string, std::pair<std::string, int>>(
                formatPhoneNumber, std::pair<std::string, int>(displayName, contactId)));
        } else {
            std::string detailInfo;
            queryResultSet->GetString(INDEX_ZERO, detailInfo);
            detailInfoSet.erase(detailInfo);
            phoneToNewContactId.insert(std::pair<std::string, std::pair<std::string, int>>(
                detailInfo, std::pair<std::string, int>(displayName, contactId)));
        }
        getRowResult = queryResultSet->GoToNextRow();
    }
    queryResultSet->Close();
    return phoneToNewContactId;
}

int ContactsUpdateHelper::UpdateCallLogToOtherContact(
    std::map<std::string, std::pair<std::string, int>> &phoneToNewContactId,
    std::shared_ptr<CallLogDataBase> &callLogDataBase)
{
    int updateNum = 0;
    for (const auto &pair : phoneToNewContactId) {
        OHOS::NativeRdb::ValuesBucket updateCallLogValues;
        updateCallLogValues.PutString(CallLogColumns::DISPLAY_NAME, pair.second.first);
        updateCallLogValues.PutString(CallLogColumns::QUICK_SEARCH_KEY, std::to_string(pair.second.second));
        updateCallLogValues.PutString(CallLogColumns::EXTRA1, std::to_string(pair.second.second));
        auto predicates = OHOS::NativeRdb::RdbPredicates(CallsTableName::CALLLOG);
        predicates.EqualTo(CallLogColumns::FORMAT_PHONE_NUMBER, pair.first);
        predicates.Or();
        predicates.EqualTo(CallLogColumns::PHONE_NUMBER, pair.first);

        int ret = CallLogDataBase::GetInstance()->UpdateCallLog(updateCallLogValues, predicates);
        if (ret < RDB_EXECUTE_OK) {
            HILOG_ERROR("ContactsUpdateHelper UpdateCallLogToOtherContact error,ret:%{public}d", ret);
            return RDB_EXECUTE_FAIL;
        } else {
            updateNum += ret;
        }
    }
    return updateNum;
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
    int totalUpdateRow = 0;
    for (unsigned int i = 0; i < count; i++) {
        // 查询contactId
        auto rawPredicates = OHOS::NativeRdb::RdbPredicates(ContactTableName::RAW_CONTACT);
        rawPredicates.EqualTo(ContactPublicColumns::ID, rawContactIdVector[i]);
        rawPredicates.And();
        rawPredicates.NotEqualTo(RawContactColumns::PRIMARY_CONTACT, 1);
        std::vector<std::string> columns;
        columns.push_back(RawContactColumns::CONTACT_ID);
        columns.push_back(RawContactColumns::IS_DELETED);
        auto rawResultSet = rdbStore->Query(rawPredicates, columns);
        int contactId = 0;
        int isDeleteDb = 0;
        if (rawResultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
            rawResultSet->GetInt(0, contactId);
            rawResultSet->GetInt(1, isDeleteDb);
        }
        rawResultSet->Close();

        if (contactId <= 0) {
            HILOG_ERROR("queryContactId error,contactId:%{public}d,rawId:%{public}d", contactId, rawContactIdVector[i]);
            continue;
        }
        // 如果联系人已经被删了，而任务处理的是没有删除的标记，说明时序有问题，不需要再按联系人更新处理了
        if (isDeleteDb == 1 && !isDelete) {
            HILOG_ERROR("rawId isDelete is 1, task deleteFlag is false:%{public}d,rawId:%{public}d",
                contactId, rawContactIdVector[i]);
            continue;
        }
        // 通过contactId处理，查询名称，电话号码等信息信息
        auto resultSet = QueryDataForCallLog(rdbStore, contactId);
        // 更新通话记录
        int updateRow = DataToUpdateCallLog(isDelete, contactId, resultSet);
        if (updateRow > 0) {
            totalUpdateRow = totalUpdateRow + updateRow;
        }
    }
    time_t ts = time(NULL);
    HILOG_INFO("ContactsUpdateHelper::UpdateCallLogByPhoneNum,totalUpdateRow = %{public}d,ts = %{public}lld",
        totalUpdateRow,
        (long long) ts);
    UpdateCallLogByPhoneNumNotifyChange(totalUpdateRow);
}

void ContactsUpdateHelper::UpdateCallLogPhoto(
    std::vector<int> &rawContactIdVector, std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
{
    std::shared_ptr<ContactsDataBase> contactDataBase_ = ContactsDataBase::GetInstance();
    if (rdbStore != contactDataBase_->contactStore_) {
        HILOG_ERROR("UpdateCallLogPhoto rdbStore is profile, not need update");
        return;
    }
    unsigned int count = rawContactIdVector.size();
    int totalUpdateRow = 0;
    int photoContactId = 0;
    std::string photoExtra3;
    for (unsigned int i = 0; i < count; i++) {
        int rawId = rawContactIdVector[i];
        int contactId = 0;
        auto rawPredicates = OHOS::NativeRdb::RdbPredicates(ContactTableName::RAW_CONTACT);
        rawPredicates.EqualTo(ContactPublicColumns::ID, rawId);
        std::vector<std::string> columns;
        columns.push_back(RawContactColumns::EXTRA3);
        columns.push_back(RawContactColumns::CONTACT_ID);
        auto rawResultSet = rdbStore->Query(rawPredicates, columns);
        std::string extra3;
        if (rawResultSet->GoToFirstRow() == OHOS::NativeRdb::E_OK) {
            rawResultSet->GetString(0, extra3);  // 畅连头像字段
            rawResultSet->GetInt(1, contactId);
            rawResultSet->Close();
        } else {
            HILOG_ERROR("ContactsUpdateHelper UpdateCallLogPhoto resultSet is empty,rawId = %{public}d", rawId);
            rawResultSet->Close();
            continue;
        }
        HILOG_WARN("ContactsUpdateHelper UpdateCallLogPhoto contactId = %{public}d", contactId);
        int updateRow = 0;
        OHOS::NativeRdb::ValuesBucket updateCallLogValues;
        updateCallLogValues.PutString(CallLogColumns::EXTRA4, extra3);

        auto predicates = OHOS::NativeRdb::RdbPredicates(CallsTableName::CALLLOG);
        predicates.EqualTo(CallLogColumns::QUICK_SEARCH_KEY, contactId);

        updateRow = CallLogDataBase::GetInstance()->UpdateCallLog(updateCallLogValues, predicates);
        if (updateRow > 0) {
            totalUpdateRow = totalUpdateRow + updateRow;
        }
        photoContactId = contactId;
        photoExtra3 = extra3;
    }
    HILOG_INFO("ContactsUpdateHelper::UpdateCallLogPhoto,photoContactId = %{public}d, totalUpdateRow = %{public}d,"
               "photoExtra3 = %{public}s,ts = %{public}lld",
        photoContactId, totalUpdateRow, getExtra3Print(photoExtra3).c_str(), (long long) time(NULL));
    UpdateCallLogByPhoneNumNotifyChange(totalUpdateRow);
}

std::string ContactsUpdateHelper::getExtra3Print(std::string extra3)
{
    std::size_t index = extra3.rfind("/");
    std::string extra3Print = extra3;
    if (index >= 0 && index < extra3.length()) {
        extra3Print = extra3.substr(index);
    } else {
        extra3Print = extra3;
    }
    return extra3Print;
}

void ContactsUpdateHelper::UpdateCallLogByPhoneNumNotifyChange(int totalUpdateRow)
{
    time_t ts = time(NULL);
    if (totalUpdateRow > 0) {
        HILOG_INFO("UpdateCallLogByPhoneNum Calllog SendCallLogChange start,ts = %{public}lld", (long long) ts);
        ContactsCommonEvent::SendCallLogChange(CONTACT_UPDATE);

        auto obsMgrClient = DataObsMgrClient::GetInstance();
        if (obsMgrClient == nullptr) {
            HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
            return;
        }

        HILOG_INFO("UpdateCallLogByPhoneNum Calllog obsMgrClient->NotifyChange start,ts = %{public}lld", (long long) ts);
        Uri uri = Uri("datashare:///com.ohos.calllogability/calls/calllog");
        ErrCode ret = obsMgrClient->NotifyChange(uri);
        if (ret != ERR_OK) {
            HILOG_ERROR("%{public}s obsMgrClient->NotifyChange error return %{public}d", __func__, ret);
            return;
        } else {
            HILOG_INFO("%{public}s obsMgrClient->NotifyChange succeed return %{public}d, ts = %{public}lld",
                __func__,
                ret,
                (long long) time(NULL));
        }
        std::shared_ptr<CallLogDataBase> callLogDataBase = CallLogDataBase::GetInstance();
        callLogDataBase->NotifyCallLogChange();
    }
}

int ContactsUpdateHelper::DataToUpdateCallLog(
    bool isDelete, int contactId, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
{
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    int updateRow = 0;
    int resultSetNum = resultSet->GoToFirstRow();
    // 如果该联系人有号码
    if (resultSetNum == OHOS::NativeRdb::E_OK) {
        std::vector<std::string> phoneNumberArr;        // 联系人非法号码集
        std::vector<std::string> formatPhoneNumberArr;  // 联系人合法号码集
        std::string name;
        std::string quickSearch;
        std::string extra3;
        // not delete contact
        if (!isDelete) {
            resultSet->GetString(0, name);
        }
        resultSet->GetString(RESULT_GET_TWO, quickSearch);
        resultSet->GetString(RESULT_GET_THERR, extra3);  // 畅连头像字段
        // 获取联系人的电话号码，放入集合
        while (resultSetNum == OHOS::NativeRdb::E_OK) {
            std::string formatPhoneNumber;
            resultSet->GetString(RESULT_GET_FOUR, formatPhoneNumber);
            // 如果存在formatPhoneNumber，使用formatPhoneNumber匹配处理；否则使用phoneNumber匹配处理
            if (!formatPhoneNumber.empty()) {
                formatPhoneNumberArr.push_back(formatPhoneNumber);
                resultSetNum = resultSet->GoToNextRow();
            } else {
                std::string phoneNumber;
                resultSet->GetString(RESULT_GET_ONE, phoneNumber);
                phoneNumberArr.push_back(phoneNumber);
                resultSetNum = resultSet->GoToNextRow();
            }
        }
        resultSet->Close();
        // 更新通话记录
        updateRow = UpdateCallLog(phoneNumberArr, formatPhoneNumberArr, name, quickSearch, extra3, isDelete);
        HILOG_INFO("ContactsUpdateHelper::UpdateCallLog,isDelete:%{public}s,contactId:%{public}d,"
            "extra3:%{public}s,nameLength:%{public}ld,"
            "updateRow:%{public}d", isDelete ? "Y" : "N", contactId,
            getExtra3Print(extra3).c_str(), (long) name.size(), updateRow);
    }
    // 如果该联系人没有号码
    if (rowCount == 0) {
        // contact data update callLog not found
        std::string quickSearch = std::to_string(contactId);
        std::string name;
        int isCallLogUpdateRow = UpdateCallLogNameNull(name, quickSearch);
        if (isCallLogUpdateRow > 0) {
            updateRow = updateRow + isCallLogUpdateRow;
        }
        HILOG_INFO("ContactsUpdateHelper::UpdateCallLogNameNull,contactId:%{public}d,"
                   "nameLength:%{public}ld,isCallLogUpdateRow:%{public}d,"
                   "updateRow:%{public}d",
            contactId, (long) name.size(), isCallLogUpdateRow, updateRow);
    }
    return updateRow;
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> ContactsUpdateHelper::QueryDataForCallLog(
    std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore, int contactId)
{
    ContactsType contactsType;
    int typeNameId = contactsType.LookupTypeId(rdbStore, ContentTypeData::PHONE);
    std::string sql;
    /**
    通过联系人 id 查出该联系人所有手机
         SELECT
             RawContactColumns.DISPLAY_NAME,
             ContactDataColumns.DETAIL_INFO,
             RawContactColumns.CONTACT_ID,
             RawContactColumns.EXTRA3
         FROM
             ViewName.VIEW_CONTACT_DATA
         WHERE
             ContactDataColumns.RAW_CONTACT_ID = (
                 SELECT
                     MIN(ContactDataColumns.RAW_CONTACT_ID)
                 FROM
                     ViewName.VIEW_CONTACT_DATA
                 WHERE
                     RawContactColumns.CONTACT_ID = contactId
                     AND ContentTypeColumns.CONTENT_TYPE = 'PHONE'
             )
             AND ContactDataColumns.TYPE_ID = typeNameId;
    */
    sql.append("SELECT ")
        .append(RawContactColumns::DISPLAY_NAME)
        .append(",")
        .append(ContactDataColumns::DETAIL_INFO)
        .append(",")
        .append(RawContactColumns::CONTACT_ID)
        .append(",")
        .append(RawContactColumns::EXTRA3)
        .append(",")
        .append(ContactDataColumns::FORMAT_PHONE_NUMBER)  // 增加格式化号码查询
        .append(" FROM ")
        .append(ViewName::VIEW_CONTACT_DATA)
        .append(" WHERE ")
        .append(ContactDataColumns::RAW_CONTACT_ID)
        .append(" IN (SELECT ")
        .append(ContactDataColumns::RAW_CONTACT_ID)
        .append(" FROM ")
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
    auto resultSet = rdbStore->QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("ContactsUpdateHelper QueryDataForCallLog resultSet is nullptr ");
    }
    return resultSet;
}

int ContactsUpdateHelper::UpdateCallLogNameNull(std::string &name, std::string &quickSearch)
{
    std::shared_ptr<CallLogDataBase> callLogDataBase = CallLogDataBase::GetInstance();
    OHOS::NativeRdb::ValuesBucket updateCallLogValues;
    updateCallLogValues.PutNull(CallLogColumns::DISPLAY_NAME);
    updateCallLogValues.PutNull(CallLogColumns::QUICK_SEARCH_KEY);
    std::string tabName = CallsTableName::CALLLOG;
    updateCallLogValues.PutNull(CallLogColumns::EXTRA1);
    auto predicates = OHOS::NativeRdb::RdbPredicates(tabName);
    std::string updateWheres;
    updateWheres.append(CallLogColumns::QUICK_SEARCH_KEY).append(" = ? ");
    std::vector<std::string> updateArgs;
    updateArgs.push_back(quickSearch);
    predicates.SetWhereClause(updateWheres);
    predicates.SetWhereArgs(updateArgs);
    int ret = callLogDataBase->UpdateCallLog(updateCallLogValues, predicates);
    if (ret < RDB_EXECUTE_OK) {
        HILOG_ERROR("UpdateCallLogNameNull error,ret:%{public}d,nameSize:%{public}ld,quickSearch:%{public}s,",
            ret,
            (long) name.size(),
            quickSearch.c_str());
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
 *
 * @return Update calllog results code
 */
int ContactsUpdateHelper::UpdateCallLog(std::vector<std::string> &phoneNumberArr,
    std::vector<std::string> &formatPhoneNumberArr, std::string &name, std::string &quickSearch, std::string &extra3,
    bool isDelete)
{
    int updateRow = 0;
    // 如果是删除联系人
    if (isDelete) {
        int isCallLogUpdateRow = UpdateCallLogWhenDeleteContact(phoneNumberArr, formatPhoneNumberArr, quickSearch);
        if (isCallLogUpdateRow > 0) {
            updateRow = updateRow + isCallLogUpdateRow;
        }
    } else {
        int isCallLogUpdateRow =
            UpdateCallLogWhenContactChange(phoneNumberArr, formatPhoneNumberArr, name, quickSearch, extra3);
        if (isCallLogUpdateRow > 0) {
            updateRow = updateRow + isCallLogUpdateRow;
        }
    }
    return updateRow;
}

// 删除联系人，更新通话记录
int ContactsUpdateHelper::UpdateCallLogWhenDeleteContact(
    std::vector<std::string> &phoneNumberArr, std::vector<std::string> &formatPhoneNumberArr, std::string &quickSearch)
{
    if (phoneNumberArr.empty() && formatPhoneNumberArr.empty()) {
        HILOG_ERROR("ContactsUpdateHelper UpdateCallLogWhenDeleteContact phoneNumberArr is empty");
        return RDB_EXECUTE_OK;
    }
    std::shared_ptr<CallLogDataBase> callLogDataBase = CallLogDataBase::GetInstance();
    OHOS::NativeRdb::ValuesBucket updateCallLogValues;
    // 该联系人被删了，那就所有关联通话记录相关字段都置空
    updateCallLogValues.PutNull(CallLogColumns::DISPLAY_NAME);
    updateCallLogValues.PutNull(CallLogColumns::QUICK_SEARCH_KEY);
    updateCallLogValues.PutNull(CallLogColumns::EXTRA1);
    std::string tabName = CallsTableName::CALLLOG;
    // 根据quicksearch字段，取消通话记录和联系人的关联
    auto rdbPredicates = OHOS::NativeRdb::RdbPredicates(tabName);
    rdbPredicates.EqualTo(CallLogColumns::QUICK_SEARCH_KEY, quickSearch);
    int totalRet = 0;
    int ret = callLogDataBase->UpdateCallLog(updateCallLogValues, rdbPredicates);
    if (ret < RDB_EXECUTE_OK) {
        HILOG_ERROR("ContactsUpdateHelper UpdateCallLogWhenDeleteContact error,ret:%{public}d,"
                    "quickSearch:%{public}s",
            ret,
            quickSearch.c_str());
        return RDB_EXECUTE_FAIL;
    }
    totalRet = totalRet + ret;
    HILOG_INFO("ContactsUpdateHelper DeleteContact,totalRet = %{public}d", totalRet);
    return totalRet;
}

int ContactsUpdateHelper::UpdateCallLogWhenContactChange(std::vector<std::string> &phoneNumberArr,
    std::vector<std::string> &formatPhoneNumberArr, std::string &name, std::string &quickSearch, std::string &extra3)
{
    if (phoneNumberArr.empty() && formatPhoneNumberArr.empty()) {
        HILOG_ERROR("ContactsUpdateHelper UpdateCallLogWhenContactChange phoneNumberArr is empty");
        return RDB_EXECUTE_OK;
    }
    int totalRet = 0;
    int retAdd = RDB_EXECUTE_FAIL;
    int retDel = RDB_EXECUTE_FAIL;
    if (!formatPhoneNumberArr.empty()) {
        // 更新字段：增加联系人，增加号码，修改联系人信息
        retAdd = UpdateCallLogAddPhoneNumber(formatPhoneNumberArr, name, quickSearch, extra3, true);
        if (retAdd < RDB_EXECUTE_OK) {
            return RDB_EXECUTE_FAIL;
        }
        totalRet = totalRet + retAdd;
        // 删除字段：删除了联系人的号码
        // 电话号码与当前联系人不符，但是quickSearchKey关联到了当前联系人，将通话记录的联系人信息置空
        retDel = UpdateCallLogDelPhoneNumber(formatPhoneNumberArr, phoneNumberArr, name, quickSearch);
        if (retDel < RDB_EXECUTE_OK) {
            return RDB_EXECUTE_FAIL;
        }
        totalRet = totalRet + retDel;
    }
    if (phoneNumberArr.empty()) {
        return totalRet;
    }
    // 更新字段：增加联系人，增加号码，修改联系人信息
    retAdd = UpdateCallLogAddPhoneNumber(phoneNumberArr, name, quickSearch, extra3, false);
    if (retAdd < RDB_EXECUTE_OK) {
        return RDB_EXECUTE_FAIL;
    }
    totalRet = totalRet + retAdd;
    // 删除字段：删除了联系人的号码
    retDel = UpdateCallLogDelPhoneNumber(formatPhoneNumberArr, phoneNumberArr, name, quickSearch);
    if (retDel < RDB_EXECUTE_OK) {
        return RDB_EXECUTE_FAIL;
    }
    totalRet = totalRet + retDel;
    return totalRet;
}

int ContactsUpdateHelper::UpdateCallLogAddPhoneNumber(std::vector<std::string> &phoneNumberArr, std::string &name,
    std::string &quickSearch, std::string &extra3, bool isFormatNumber)
{
    // 更新字段：增加联系人，增加号码，修改联系人信息
    OHOS::NativeRdb::ValuesBucket updateCallLogValues;
    updateCallLogValues.PutString(CallLogColumns::DISPLAY_NAME, name);
    updateCallLogValues.PutString(CallLogColumns::QUICK_SEARCH_KEY, quickSearch);
    updateCallLogValues.PutString(CallLogColumns::EXTRA1, quickSearch);
    updateCallLogValues.PutString(CallLogColumns::EXTRA4, extra3);
    updateCallLogValues.PutInt(CallLogColumns::IS_CNAP, 0);
    auto predicates = OHOS::NativeRdb::RdbPredicates(CallsTableName::CALLLOG);
    if (isFormatNumber) {
        predicates.In(CallLogColumns::FORMAT_PHONE_NUMBER, phoneNumberArr);
    } else {
        predicates.In(CallLogColumns::PHONE_NUMBER, phoneNumberArr);
    }
    predicates.And();
    // quickSearchKey为空，或值就是这个联系人，就做更新操作
    predicates.BeginWrap();
    predicates.EqualTo(CallLogColumns::QUICK_SEARCH_KEY, "");
    predicates.Or();
    predicates.IsNull(CallLogColumns::QUICK_SEARCH_KEY);
    predicates.Or();
    predicates.EqualTo(CallLogColumns::QUICK_SEARCH_KEY, quickSearch);
    predicates.EndWrap();

    int ret = CallLogDataBase::GetInstance()->UpdateCallLog(updateCallLogValues, predicates);
    if (ret < RDB_EXECUTE_OK) {
        HILOG_ERROR("ContactsUpdateHelper UpdateCallLogWhenContactChange error,ret:%{public}d,nameSize:%{public}ld,"
                    "quickSearch:%{public}s",
            ret,
            (long) name.size(),
            quickSearch.c_str());
        return RDB_EXECUTE_FAIL;
    }
    return ret;
}

// 电话号码与当前联系人不符，但是quickSearchKey关联到了当前联系人，将通话记录的联系人信息置空
int ContactsUpdateHelper::UpdateCallLogDelPhoneNumber(std::vector<std::string> &formatPhoneNumberArr,
    std::vector<std::string> &phoneNumberArr, std::string &name, std::string &quickSearch)
{
    // 删除字段：删除了联系人的号码
    OHOS::NativeRdb::ValuesBucket updateCallLogValues;
    updateCallLogValues.PutNull(CallLogColumns::DISPLAY_NAME);
    updateCallLogValues.PutNull(CallLogColumns::QUICK_SEARCH_KEY);
    updateCallLogValues.PutNull(CallLogColumns::EXTRA1);
    auto predicates = OHOS::NativeRdb::RdbPredicates(CallsTableName::CALLLOG);
    predicates.NotIn(CallLogColumns::FORMAT_PHONE_NUMBER, formatPhoneNumberArr);
    predicates.And();
    predicates.NotIn(CallLogColumns::PHONE_NUMBER, phoneNumberArr);
    predicates.And();
    predicates.EqualTo(CallLogColumns::QUICK_SEARCH_KEY, quickSearch);
    int ret = CallLogDataBase::GetInstance()->UpdateCallLog(updateCallLogValues, predicates);
    if (ret < RDB_EXECUTE_OK) {
        HILOG_ERROR("ContactsUpdateHelper UpdateCallLogWhenContactChange error,ret:%{public}d,nameSize:%{public}ld,"
                    "quickSearch:%{public}s",
            ret,
            (long) name.size(),
            quickSearch.c_str());
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
        valuesBucket.PutString(RawContactColumns::SORT_FIRST_LETTER, ANONYMOUS_SORT_FIRST_LETTER);
        valuesBucket.PutString(RawContactColumns::SORT, ANONYMOUS_SORT);
        valuesBucket.PutNull(RawContactColumns::SORT_KEY);
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
}  // namespace Contacts
}  // namespace OHOS