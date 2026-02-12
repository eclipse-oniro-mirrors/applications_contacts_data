/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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

#include "privacy_contacts_manager.h"

#include "os_account_manager.h"
#include "common.h"
#include "hilog_wrapper.h"
#include "system_ability_definition.h"
#include "contacts_columns.h"
#include "raw_data_parser.h"
#include "iservice_registry.h"
#include "rdb_utils.h"
#include "calllog_common.h"

namespace OHOS {
namespace Contacts {
std::shared_ptr<PrivacyContactsManager> instance_ = nullptr;
std::shared_ptr<PrivacyContactsManager> PrivacyContactsManager::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = std::make_shared<PrivacyContactsManager>();
    }
    if (instance_ != nullptr) {
        return instance_;
    }
    return nullptr;
}

bool PrivacyContactsManager::isMigratePrivacyCallLogEnbale()
{
    std::string datashareUri = SETTING_DB_URI;
    std::shared_ptr<DataShare::DataShareHelper> helper =
        RetryCreateDataShareHelper([&]() { return CreateDataShareHelper(datashareUri); });
    if (helper == nullptr) {
        HILOG_ERROR("helper is null");
        return false;
    }
    Uri uri(datashareUri + "&key=private_contact_switch");
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("KEYWORD", "private_contact_switch");
    predicates.EqualTo("VALUE", 1);
    auto resultSet = helper->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("helper: query error, result is null");
        helper->Release();
        return false;
    }
    auto opearateResult = resultSet->GoToFirstRow();
    if (opearateResult != DataShare::E_OK) {
        HILOG_ERROR("helper: query error, go to first row error");
        resultSet->Close();
        helper->Release();
        return false;
    }
    resultSet->Close();
    helper->Release();
    return true;
}

// LCOV_EXCL_START
int32_t PrivacyContactsManager::SyncPrivacyContactsBackup()
{
    HILOG_INFO("SyncPrivacyContactsBackup start");
    if (!IsPrivacySpace()) {
        HILOG_WARN("It is not in privacy space");
        return OPERATION_ERROR;
    }
    {
        std::lock_guard<std::mutex> lock(migrateContactMutex_);
        std::vector<DataShare::DataShareValuesBucket> valueBuckets;
        auto operateSuccess = QueryContactsFromPrivacyContactData(valueBuckets);
        if (!operateSuccess) {
            HILOG_ERROR("QueryContactsFromPrivacyContactData failed");
            return OPERATION_ERROR;
        }
        operateSuccess = DeleteAllContactsFromPrivacyBackup();
        if (!operateSuccess) {
            HILOG_ERROR("DeleteAllContactsFromPrivacyBackup failed");
            return OPERATION_ERROR;
        }
        operateSuccess = InsertContactDataToPrivacyBackup(valueBuckets);
        if (!operateSuccess) {
            HILOG_ERROR("InsertContactDataToPrivacyBackup failed");
            return OPERATION_ERROR;
        }
        HILOG_INFO("SyncPrivacyContactsBackup success, rowCount: %{public}zu", valueBuckets.size());
    }
    return OPERATION_OK;
}
// LCOV_EXCL_STOP

bool PrivacyContactsManager::InsertContactDataToPrivacyBackup(
    const std::vector<OHOS::NativeRdb::ValuesBucket> &contactDatas)
{
    std::vector<DataShare::DataShareValuesBucket> dataShareValues;
    for (auto &contactData : contactDatas) {
        OHOS::NativeRdb::ValuesBucket valueBucket;
        if (GetValuesBucketFromContactData(contactData, valueBucket)) {
            dataShareValues.emplace_back(ToDataShareValuesBucket(valueBucket));
        }
    }
    return InsertPrivacyContactsBackup(dataShareValues);
}

bool PrivacyContactsManager::InsertContactDataToPrivacyBackup(
    const std::vector<DataShare::DataShareValuesBucket> &contactDatas)
{
    std::vector<DataShare::DataShareValuesBucket> dataShareValues;
    for (auto &contactData : contactDatas) {
        OHOS::NativeRdb::ValuesBucket valueBucket;
        if (GetValuesBucketFromContactData(RdbDataShareAdapter::RdbUtils::ToValuesBucket(contactData), valueBucket)) {
            dataShareValues.emplace_back(ToDataShareValuesBucket(valueBucket));
        }
    }
    return InsertPrivacyContactsBackup(dataShareValues);
}

bool PrivacyContactsManager::InsertContactDataToPrivacyBackup(const OHOS::NativeRdb::ValuesBucket &contactData)
{
    std::vector<DataShare::DataShareValuesBucket> dataShareValues;
    OHOS::NativeRdb::ValuesBucket valueBucket;
    if (GetValuesBucketFromContactData(contactData, valueBucket)) {
        dataShareValues.emplace_back(ToDataShareValuesBucket(valueBucket));
    }
    return InsertPrivacyContactsBackup(dataShareValues);
}

bool PrivacyContactsManager::DeleteContactsFromPrivacyBackup(std::vector<int> rawContactIds)
{
    if (rawContactIds.size() == 0) {
        HILOG_WARN("DeleteContactsFromPrivacyBackup values is empty!");
        return true;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string uri;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CONTACT_URI, helper, uri)) {
        HILOG_ERROR("DeleteContactsFromPrivacyBackup: GetHelperAndUrl failed!");
        return false;
    }
// LCOV_EXCL_START
    Uri uriObj(uri);
    int result = Contacts::OPERATION_OK;
    std::unordered_set<int> rawContactIdSet(rawContactIds.begin(), rawContactIds.end());
    std::vector<std::string> rawContactIdsStrList;
    std::for_each(rawContactIdSet.begin(), rawContactIdSet.end(), [&](int x) {
        rawContactIdsStrList.emplace_back(std::to_string(x));
    });
    DataShare::DataSharePredicates predicates;
    predicates.In(PrivacyContactsBackupColumns::RAW_CONTACT_ID, rawContactIdsStrList);
    result = helper->Delete(uriObj, predicates);
    helper->Release();
    if (result == Contacts::OPERATION_OK) {
        HILOG_INFO("DeleteContactsFromPrivacyBackup finish, result: %{public}d", result);
        return true;
    }
    HILOG_ERROR("DeleteContactsFromPrivacyBackup failed, result: %{public}d", result);
    return false;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::ProcessDataAfterPrivacySpaceDeleted()
{
    int32_t userId;
    if (GetSpecialTypeUserId(OHOS::AccountSA::OsAccountType::PRIVATE, userId) == OPERATION_OK) {
        HILOG_WARN("Non-Privacy space deleted");
        return false;
    }
    bool result = false;
    result = DeleteAllContactsFromPrivacyBackup();
    if (!result) {
        HILOG_ERROR("DeleteAllContactsFromPrivacyBackup failed!");
    }
    result = DeleteAllCalllogsWithPrivacyTag();
    if (!result) {
        HILOG_ERROR("DeleteAllCalllogsWithPrivacyTag failed!");
    }
    result = DeletePrivacyTagOnUnreadCalllog();
    if (!result) {
        HILOG_ERROR("DeletePrivacyTagOnUnreadCalllog failed!");
    }
    HILOG_INFO("Privacy space deleted, clear privacy info");
    return result;
}

bool PrivacyContactsManager::DeleteAllContactsFromPrivacyBackup()
{
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string uri;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CONTACT_URI, helper, uri)) {
        HILOG_ERROR("DeleteAllContactsFromPrivacyBackup: GetHelperAndUrl failed!");
        return false;
    }
// LCOV_EXCL_START
    Uri uriObj(uri);
    DataShare::DataSharePredicates predicates;
    int result = helper->Delete(uriObj, predicates);
    helper->Release();
    if (result == Contacts::OPERATION_OK) {
        HILOG_INFO("DeleteAllContactsFromPrivacyBackup finish, result: %{public}d", result);
        return true;
    }
    HILOG_ERROR("DeleteAllContactsFromPrivacyBackup failed, result: %{public}d", result);
    return false;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::DeleteAllCalllogsWithPrivacyTag()
{
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string uri;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CALL_LOG_URI, helper, uri)) {
        HILOG_ERROR("DeleteAllCalllogsWithPrivacyTag: GetHelperAndUrl failed!");
        return false;
    }
// LCOV_EXCL_START
    Uri uriObj(uri);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(CallLogColumns::PRIVACY_TAG, static_cast<int32_t>(PrivacyTag::CALL_LOG_PRIVACY_TAG));
    int result = helper->Delete(uriObj, predicates);
    helper->Release();
    if (result == Contacts::OPERATION_OK) {
        HILOG_INFO("DeleteAllCalllogsWithPrivacyTag finish, result: %{public}d", result);
        return true;
    }
    HILOG_ERROR("DeleteAllCalllogsWithPrivacyTag failed, result: %{public}d", result);
    return false;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::DeletePrivacyTagOnUnreadCalllog()
{
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string uri;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CALL_LOG_URI, helper, uri)) {
        HILOG_ERROR("DeletePrivacyTagOnUnreadCalllog: GetHelperAndUrl failed!");
        return false;
    }
// LCOV_EXCL_START
    Uri uriObj(uri);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(
        CallLogColumns::PRIVACY_TAG, static_cast<int32_t>(PrivacyTag::CALL_LOG_UNREAD_MISSED_PRIVACY_TAG));
    DataShare::DataShareValuesBucket valueBucket;
    valueBucket.Put(CallLogColumns::PRIVACY_TAG, static_cast<int32_t>(PrivacyTag::CALL_LOG_NON_PRIVACY_TAG));
    int result = helper->Update(uriObj, predicates, valueBucket);
    helper->Release();
    if (result == Contacts::OPERATION_OK) {
        HILOG_INFO("DeletePrivacyTagOnUnreadCalllog finish, result: %{public}d", result);
        return true;
    }
    HILOG_ERROR("DeletePrivacyTagOnUnreadCalllog failed, result: %{public}d", result);
    return false;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::restorePrivacContacts(const std::vector<int32_t> &rawIds)
{
    HILOG_INFO("restorePrivacContacts start, rawIds count: %{public}zu", rawIds.size());
    std::vector<DataShare::DataShareValuesBucket> contactDatas;
    QueryPrivacyContactsByRawContactId(rawIds, contactDatas);
    return InsertContactDataToPrivacyBackup(contactDatas);
}

bool PrivacyContactsManager::InsertPrivacContactsByContactDatas(
    const std::vector<OHOS::NativeRdb::ValuesBucket> &contactDatas)
{
    HILOG_INFO("InsertPrivacContactsByContactDatas, count: %{public}zu", contactDatas.size());
    return InsertContactDataToPrivacyBackup(contactDatas);
}

DataShare::DataShareValuesBucket PrivacyContactsManager::ToDataShareValuesBucket(
    OHOS::NativeRdb::ValuesBucket valuesBucket)
{
    std::map<std::string, DataShare::DataShareValueObject::Type> values;
    for (auto &[key, value] : valuesBucket.values_) {
        DataShare::DataShareValueObject::Type dsValue;
        OHOS::NativeRdb::RawDataParser::Convert(std::move(value.value), dsValue);
        values.emplace(key, std::move(dsValue));
    }
    return DataShare::DataShareValuesBucket(std::move(values));
}

bool PrivacyContactsManager::GetValuesBucketFromContactData(
    const OHOS::NativeRdb::ValuesBucket &contactDataValuesBucket, OHOS::NativeRdb::ValuesBucket &outValuesBucket)
{
    OHOS::NativeRdb::ValueObject typeIdObj;
    contactDataValuesBucket.GetObject(ContactDataColumns::TYPE_ID, typeIdObj);
    std::string typeIdStr;
    typeIdObj.GetString(typeIdStr);
    int32_t typeId = std::atoi(typeIdStr.c_str());
    HILOG_INFO("GetValuesBucketFromContactData, typeId: %{public}d", typeId);
    if (typeId == static_cast<int32_t>(ContentTypeData::PHONE_INT_VALUE)) {
        OHOS::NativeRdb::ValueObject phoneNum;
        contactDataValuesBucket.GetObject(ContactDataColumns::DETAIL_INFO, phoneNum);
        OHOS::NativeRdb::ValueObject contactDataId;
        contactDataValuesBucket.GetObject(ContactDataColumns::ID, contactDataId);
        OHOS::NativeRdb::ValueObject rawContactId;
        contactDataValuesBucket.GetObject(ContactDataColumns::RAW_CONTACT_ID, rawContactId);
        OHOS::NativeRdb::ValueObject formatPhoneNum;
        contactDataValuesBucket.GetObject(ContactDataColumns::FORMAT_PHONE_NUMBER, formatPhoneNum);
        outValuesBucket.Put(PrivacyContactsBackupColumns::CONTACT_DATA_ID, contactDataId);
        outValuesBucket.Put(PrivacyContactsBackupColumns::RAW_CONTACT_ID, rawContactId);
        outValuesBucket.Put(PrivacyContactsBackupColumns::PHONE_NUMBER, phoneNum);
        outValuesBucket.Put(PrivacyContactsBackupColumns::FORMAT_PHONE_NUMBER, formatPhoneNum);
        return true;
    }
    return false;
}

bool PrivacyContactsManager::IsMainSpace()
{
    // ADMIN 为主空间
    return IsSpecialSpace(OHOS::AccountSA::OsAccountType::ADMIN);
}

bool PrivacyContactsManager::IsPrivacySpace()
{
    // PRIVATE 为隐私空间
    return IsSpecialSpace(OHOS::AccountSA::OsAccountType::PRIVATE);
}

bool PrivacyContactsManager::TryToMigratePrivateCallLog(const DataShare::DataShareValuesBucket &value)
{
    if (!isMigratePrivacyCallLogEnbale()) {
        HILOG_INFO("Migrate privacy call log disable!");
        return false;
    }
    if (!IsMainSpace()) {
        HILOG_INFO("It is not in main space");
        return false;
    }
    bool isPrivacySpacePhoneNumber = false;
    bool isValid = false;
    auto phoneNum = static_cast<std::string>(value.Get(CallLogColumns::PHONE_NUMBER, isValid));
    QueryIsPrivacySpacePhoneNumber(phoneNum, isPrivacySpacePhoneNumber);
    if (!isPrivacySpacePhoneNumber) {
        HILOG_INFO("%{private}s is not a private space number", phoneNum.c_str());
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(migrateCallLogMutex_);
        std::vector<DataShare::DataShareValuesBucket> valuesBuckets;
        isValid = QueryCallLogWithPhoneNumber(phoneNum, valuesBuckets);
        HILOG_INFO("QueryCallLogWithPhoneNumber end, valuesBuckets: %{public}zu", valuesBuckets.size());
        valuesBuckets.emplace_back(value);
        UpdatePrivacyTag(valuesBuckets);
    }
    return true;
}

bool PrivacyContactsManager::QueryCallLogWithPhoneNumber(
    const std::string &phoneNum, std::vector<DataShare::DataShareValuesBucket> &values)
{
    std::shared_ptr<DataShare::DataShareHelper> helper = nullptr;
    std::string url;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CALL_LOG_URI, helper, url)) {
        return false;
    }
// LCOV_EXCL_START
    DataShare::DataSharePredicates predicates = GetPrivacyCallLogPredicates(phoneNum);
    Uri uri(url);
    std::vector<std::string> columns;
    auto resultSet = helper->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("resultSet is nullptr!");
        helper->Release();
        return false;
    }
    ConvertResultSetToValuesBucket(resultSet, values);
    resultSet->Close();
    helper->Release();
    return true;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::ProcessPrivacyCallLog(
    std::vector<OHOS::NativeRdb::ValuesBucket> &values, std::vector<OHOS::NativeRdb::ValuesBucket> &unprocessedValues)
{
    if (!isMigratePrivacyCallLogEnbale()) {
        HILOG_INFO("Migrate privacy call log disable!");
        unprocessedValues = values;
        return false;
    }
    if (!IsMainSpace()) {
        HILOG_INFO("ProcessPrivacyCallLog: It is not in main space");
        unprocessedValues = values;
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(processCallLogMutex_);
        std::vector<DataShare::DataShareValuesBucket> privacyValues;
        QueryIsPrivacySpacePhoneNumber(values, privacyValues, unprocessedValues);
        return UpdatePrivacyTag(privacyValues);
    }
}

bool PrivacyContactsManager::ProcessPrivacyCallLog(std::vector<DataShare::DataShareValuesBucket> &values,
    std::vector<DataShare::DataShareValuesBucket> &unprocessedValues)
{
    if (!isMigratePrivacyCallLogEnbale()) {
        HILOG_INFO("Migrate privacy call log disable!");
        unprocessedValues = values;
        return false;
    }
    if (!IsMainSpace()) {
        HILOG_INFO("ProcessPrivacyCallLog: It is not in main space");
        unprocessedValues = values;
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(processCallLogMutex_);
        std::vector<DataShare::DataShareValuesBucket> privacyValues;
        QueryIsPrivacySpacePhoneNumber(values, privacyValues, unprocessedValues);
        return UpdatePrivacyTag(privacyValues);
    }
}

bool PrivacyContactsManager::QueryIsPrivacySpacePhoneNumber(const std::string &phoneNum, bool &result)
{
    result = false;
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string url;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CONTACT_URI, helper, url)) {
        return false;
    }
// LCOV_EXCL_START
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(PrivacyContactsBackupColumns::PHONE_NUMBER, phoneNum);
    predicates.Or();
    predicates.EqualTo(PrivacyContactsBackupColumns::FORMAT_PHONE_NUMBER, phoneNum);
    std::vector<std::string> columns = {PrivacyContactsBackupColumns::PHONE_NUMBER};
    auto uriObj = Uri(url);
    auto resultSet = helper->Query(uriObj, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("query error, result is null");
        helper->Release();
        return false;
    }
    if (resultSet->GoToFirstRow() != DataShare::E_OK) {
        HILOG_WARN("%{private}s: it is not private space number", phoneNum.c_str());
        resultSet->Close();
        helper->Release();
        return true;
    }
    resultSet->Close();
    helper->Release();
    result = true;
    return true;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::QueryIsPrivacySpacePhoneNumber(
    const std::vector<DataShare::DataShareValuesBucket> &phoneNums,
    std::vector<DataShare::DataShareValuesBucket> &privacyNums,
    std::vector<DataShare::DataShareValuesBucket> &normalNums)
{
    std::unordered_set<std::string> allPrivacyNums;
    GetPrivacySpacePhoneNumbers(allPrivacyNums);
    for (const auto &value : phoneNums) {
        bool isValid = false;
        std::string phoneNumber = value.Get(CallLogColumns::PHONE_NUMBER, isValid);
        if (!isValid) {
            HILOG_ERROR("QueryIsPrivacySpacePhoneNumber get phoneNum failed");
            normalNums.emplace_back(value);
            continue;
        }
        if (allPrivacyNums.find(phoneNumber) != allPrivacyNums.end()) {
            privacyNums.emplace_back(value);
            continue;
        }
        normalNums.emplace_back(value);
    }
    return true;
}

bool PrivacyContactsManager::QueryIsPrivacySpacePhoneNumber(const std::vector<OHOS::NativeRdb::ValuesBucket> &phoneNums,
    std::vector<DataShare::DataShareValuesBucket> &privacyNums, std::vector<OHOS::NativeRdb::ValuesBucket> &normalNums)
{
    std::unordered_set<std::string> allPrivacyNums;
    GetPrivacySpacePhoneNumbers(allPrivacyNums);
    for (const auto &value : phoneNums) {
        OHOS::NativeRdb::ValueObject phoneNumObj;
        value.GetObject(CallLogColumns::PHONE_NUMBER, phoneNumObj);
        std::string phoneNumber;
        phoneNumObj.GetString(phoneNumber);
        if (allPrivacyNums.find(phoneNumber) != allPrivacyNums.end()) {
            privacyNums.emplace_back(ToDataShareValuesBucket(value));
            continue;
        }
        normalNums.emplace_back(value);
    }
    return true;
}

bool PrivacyContactsManager::QueryPrivacyContactsByRawContactId(
    const std::vector<int32_t> &rawIds, std::vector<DataShare::DataShareValuesBucket> &contactDatas)
{
    if (rawIds.size() == 0) {
        HILOG_WARN("QueryPrivacyContactsByRawContactId rawIds is empty!");
        return true;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = nullptr;
    std::string url;
    bool result = GetHelperAndUrl(OHOS::AccountSA::OsAccountType::PRIVATE, CONTACT_URI, helper, url);
    if (!result) {
        HILOG_ERROR("QueryPrivacyContactsByRawContactId, helper is nullptr");
        return false;
    }
// LCOV_EXCL_START
    Uri uri(url);
    std::vector<std::string> columns;
    std::unordered_set<int32_t> rawIdSet(rawIds.begin(), rawIds.end());
    std::vector<std::string> rawIdsStrList;
    std::for_each(rawIdSet.begin(), rawIdSet.end(), [&](int x) {
        rawIdsStrList.emplace_back(std::to_string(x));
    });
    DataShare::DataSharePredicates predicates;
    predicates.In(ContactDataColumns::RAW_CONTACT_ID, rawIdsStrList);
    predicates.EqualTo(ContactDataColumns::TYPE_ID, ContentTypeData::PHONE_INT_VALUE);
    predicates.EqualTo(RawContactColumns::IS_DELETED, Contacts::DELETE_MARK);
    auto resultSet = helper->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryPrivacyContactsByRawContactId, resultSet is null");
        helper->Release();
        return false;
    }
    ConvertResultSetToValuesBucket(resultSet, contactDatas);
    resultSet->Close();
    helper->Release();
    HILOG_INFO("QueryPrivateContactsByRawContactId finish, contactDatas: %{public}zu", contactDatas.size());
    return true;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::UpdatePrivacyTag(std::vector<DataShare::DataShareValuesBucket> &values)
{
    if (!isMigratePrivacyCallLogEnbale()) {
        HILOG_INFO("Migrate privacy call log disable!");
        return false;
    }
    if (values.empty()) {
        HILOG_WARN("UpdatePrivacyTag, values is empty");
        return false;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = nullptr;
    std::string url;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CALL_LOG_URI, helper, url)) {
        HILOG_ERROR("UpdatePrivacyTag, helper is nullptr");
        return false;
    }
// LCOV_EXCL_START
    url += "&isPrivacy=true&isFromBatch=true";
    Uri uri(url);
    std::vector<DataShare::DataShareValuesBucket> insertValues;
    auto result = OPERATION_ERROR;
    for (auto &value : values) {
        bool isValid = UpdatePrivacyTagOfValueBucket(value);
        auto id = static_cast<int32_t>(value.Get(CallLogColumns::ID, isValid));
        if (!isValid) {
            insertValues.emplace_back(value);
            continue;
        }
        DataShare::DataSharePredicates predicates;
        predicates.EqualTo(CallLogColumns::ID, id);
        result = helper->Update(uri, predicates, value);
        if (result == OPERATION_ERROR) {
            HILOG_ERROR("UpdatePrivacyTag Update error, result: %{public}d", result);
            continue;
        }
    }
    if (!insertValues.empty()) {
        result = helper->BatchInsert(uri, insertValues);
        if (result == OPERATION_ERROR) {
            HILOG_ERROR("UpdatePrivacyTag BatchInsert error, result: %{public}d", result);
            helper->Release();
            return false;
        }
    }
    helper->Release();
    HILOG_INFO("UpdatePrivacyTag finish, rowCount: %{public}lu", (unsigned long) values.size());
    return true;
// LCOV_EXCL_STOP
}

// LCOV_EXCL_START
bool PrivacyContactsManager::MigratePrivacyCallLog()
{
    if (!isMigratePrivacyCallLogEnbale()) {
        HILOG_INFO("Migrate privacy call log disable!");
        return false;
    }
    if (!IsPrivacySpace()) {
        HILOG_INFO("It is not in privacy space!");
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(migrateCallLogMutex_);
        std::vector<DataShare::DataShareValuesBucket> values;
        std::vector<std::string> deleteIds;
        auto result = QueryCallLogWithPrivacyTag(values, deleteIds);
        if (!result) {
            HILOG_ERROR("MigratePrivateCallLog: QueryCallLogWithPrivacyTag failed!");
            return false;
        }
        if (values.empty()) {
            HILOG_WARN("MigratePrivateCallLog: values is empty!");
            return false;
        }
        result = InsertPrivacyCallLog(values);
        if (result) {
            HILOG_INFO("MigratePrivateCallLog DeleteCallLogAfterMigrate");
            result = DeleteCallLogAfterMigrate(deleteIds);
        }
        HILOG_INFO("MigratePrivateCallLog end, result: %{public}d", result);
        return result;
    }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
bool PrivacyContactsManager::QueryCallLogWithPrivacyTag(
    std::vector<DataShare::DataShareValuesBucket> &values, std::vector<std::string> &deleteIds)
{
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string url;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CALL_LOG_URI, helper, url)) {
        HILOG_ERROR("QueryCallLogWithPrivacyTag: GetHelperAndUrl failed!");
        return false;
    }
    Uri uriObj(url);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(CallLogColumns::PRIVACY_TAG, static_cast<int32_t>(Contacts::PrivacyTag::CALL_LOG_PRIVACY_TAG));
    std::vector<std::string> columns;
    auto resultSet = helper->Query(uriObj, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryCallLogWithPrivacyTag: resultSet is null");
        helper->Release();
        return false;
    }
    resultSet->GetAllColumnNames(columns);
    auto operationResult = resultSet->GoToFirstRow();
    while (operationResult == DataShare::E_OK) {
        DataShare::DataShareValuesBucket valuesBucket;
        for (const auto &column : columns) {
            std::string value;
            int32_t columnIndex = 0;
            resultSet->GetColumnIndex(column, columnIndex);
            resultSet->GetString(columnIndex, value);
            if (column == CallLogColumns::ID) {
                deleteIds.emplace_back(value);
                continue;
            } else if (column == CallLogColumns::PRIVACY_TAG) {
                value = std::to_string(static_cast<int32_t>(PrivacyTag::CALL_LOG_MIGRATED_PRIVACY_TAG));
            }
            valuesBucket.Put(column, value);
        }
        values.emplace_back(valuesBucket);
        operationResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
    helper->Release();
    return true;
}
// LCOV_EXCL_STOP

bool PrivacyContactsManager::InsertPrivacyCallLog(const std::vector<DataShare::DataShareValuesBucket> &values)
{
    if (!isMigratePrivacyCallLogEnbale()) {
        HILOG_INFO("Migrate privacy call log disable!");
        return false;
    }
    if (values.empty()) {
        HILOG_WARN("InsertPrivateCallLog failed, values is empty!");
        return false;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = nullptr;
    std::string url;
    bool result = GetHelperAndUrl(OHOS::AccountSA::OsAccountType::PRIVATE, CALL_LOG_URI, helper, url);
    if (!result) {
        HILOG_ERROR("GetPrivateSpaceHelperAndUrl fail!");
        return false;
    }
// LCOV_EXCL_START
    url += "&isPrivacy=true";
    Uri uri(url);
    auto insertResult = helper->BatchInsert(uri, values);
    helper->Release();
    if (insertResult == OPERATION_ERROR) {
        HILOG_ERROR("InsertPrivateCallLog failed, insertResult: %{public}d", insertResult);
        return false;
    }
    HILOG_INFO("InsertPrivateCallLog finish, rowCount: %{public}zu", values.size());
    return true;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::DeleteCallLogAfterMigrate(const std::vector<std::string> &deleteIds)
{
    if (!isMigratePrivacyCallLogEnbale()) {
        HILOG_INFO("Migrate privacy call log disable!");
        return false;
    }
    if (deleteIds.empty()) {
        HILOG_WARN("DeleteCallLogAfterMigrate failed, values is empty!");
        return false;
    }
    HILOG_INFO("DeleteCallLogAfterMigrate start, values: %{public}zu", deleteIds.size());
    std::shared_ptr<DataShare::DataShareHelper> helper = nullptr;
    std::string url;
    bool result = GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CALL_LOG_URI, helper, url);
    if (!result) {
        HILOG_ERROR("DeleteCallLogAfterMigrate GetHelperAndUrl fail!");
        return false;
    }
// LCOV_EXCL_START
    Uri uri(url);
    DataShare::DataSharePredicates predicates;
    predicates.In(CallLogColumns::ID, deleteIds);
    auto res = helper->Delete(uri, predicates);
    helper->Release();
    if (res != OPERATION_OK) {
        HILOG_ERROR("DeleteCallLogAfterMigrate error, result: %{public}d", res);
        return false;
    }
    HILOG_INFO("DeleteCallLogAfterMigrate finish, deleteCount: %{public}lu", (unsigned long) deleteIds.size());
    return true;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::ConvertResultSetToValuesBucket(
    const std::shared_ptr<DataShare::DataShareResultSet> &resultSet,
    std::vector<DataShare::DataShareValuesBucket> &valuesBuckets)
{
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    int32_t operationResult = resultSet->GoToFirstRow();
    while (operationResult == DataShare::E_OK) {
// LCOV_EXCL_START
        DataShare::DataShareValuesBucket valueBucket;
        for (size_t columnIndex = 0; columnIndex < columnNames.size(); columnIndex++) {
            DataShare::DataType dataType;
            resultSet->GetDataType(columnIndex, dataType);
            if (dataType == DataShare::DataType::TYPE_INTEGER) {
                int64_t value = 0;
                resultSet->GetLong(columnIndex, value);
                valueBucket.Put(columnNames[columnIndex], value);
            } else {
                std::string value;
                resultSet->GetString(columnIndex, value);
                valueBucket.Put(columnNames[columnIndex], value);
            }
        }
        valuesBuckets.emplace_back(valueBucket);
        operationResult = resultSet->GoToNextRow();
    }
    return true;
// LCOV_EXCL_STOP
}

bool PrivacyContactsManager::IsSpecialSpace(OHOS::AccountSA::OsAccountType specialType)
{
    OHOS::AccountSA::OsAccountType type;
    OHOS::AccountSA::OsAccountManager::GetOsAccountTypeFromProcess(type);
    HILOG_WARN("IsSpecialSpace, type: %{public}d, specialType: %{public}d", type, specialType);
    return type == specialType;
}

bool PrivacyContactsManager::InsertPrivacyContactsBackup(std::vector<DataShare::DataShareValuesBucket> &values)
{
    if (values.size() == 0) {
        HILOG_WARN("InsertPrivacyContactsBackup values is empty!");
        return true;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string uri;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CONTACT_URI, helper, uri)) {
        HILOG_ERROR("InsertPrivacyContactsBackup: GetHelperAndUrl failed!");
        return false;
    }
    Uri uriObj(uri);
    auto result = helper->BatchInsert(uriObj, values);
    helper->Release();
    if (result == Contacts::OPERATION_OK) {
        HILOG_INFO("InsertPrivacyContactsBackup finish, insertResult: %{public}d", result);
        return true;
    }
    HILOG_ERROR("InsertPrivacyContactsBackup failed, insertResult: %{public}d", result);
    return false;
}

// LCOV_EXCL_START
bool PrivacyContactsManager::QueryContactsFromPrivacyContactData(std::vector<DataShare::DataShareValuesBucket> &values)
{
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string uri;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::PRIVATE, CONTACT_URI, helper, uri)) {
        HILOG_ERROR("GetHelperAndUrl failed!");
        return false;
    }
    Uri uriObj(uri);
    OHOS::DataShare::DataSharePredicates predicates;
    predicates.EqualTo(ContactDataColumns::TYPE_ID, static_cast<int32_t>(ContentTypeData::PHONE_INT_VALUE));
    std::vector<std::string> columns = {ContactDataColumns::ID,
        ContactDataColumns::RAW_CONTACT_ID,
        ContactDataColumns::TYPE_ID,
        ContactDataColumns::DETAIL_INFO,
        ContactDataColumns::FORMAT_PHONE_NUMBER};
    auto resultSet = helper->Query(uriObj, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("resultSet is null!");
        helper->Release();
        return false;
    }
    ConvertResultSetToValuesBucket(resultSet, values);
    resultSet->Close();
    helper->Release();
    HILOG_INFO("resultSet size: %{public}zu", values.size());
    return true;
}
// LCOV_EXCL_STOP

DataShare::DataSharePredicates PrivacyContactsManager::GetPrivacyCallLogPredicates(const std::string &phoneNum)
{
    DataShare::DataSharePredicates predicates;
    predicates.BeginWrap();
    // 已读的未接来电或接通的来电
    predicates.BeginWrap();
    predicates.EqualTo(CallLogColumns::IS_READ, static_cast<int32_t>(CALL_IS_READ));
    predicates.And();
    predicates.EqualTo(CallLogColumns::CALL_DIRECTION, static_cast<int32_t>(CALL_DIRECTION_IN));
    predicates.EndWrap();
    predicates.Or();
    // 主叫
    predicates.EqualTo(CallLogColumns::CALL_DIRECTION, static_cast<int32_t>(CALL_DIRECTION_OUT));
    predicates.EndWrap();
    predicates.And();
    predicates.EqualTo(CallLogColumns::PHONE_NUMBER, phoneNum);
    predicates.NotEqualTo(
        CallLogColumns::PRIVACY_TAG, static_cast<int32_t>(PrivacyTag::CALL_LOG_UNREAD_MISSED_PRIVACY_TAG));
    predicates.NotEqualTo(CallLogColumns::PRIVACY_TAG, static_cast<int32_t>(PrivacyTag::CALL_LOG_PRIVACY_TAG));
    return predicates;
}

bool PrivacyContactsManager::UpdatePrivacyTagOfValueBucket(DataShare::DataShareValuesBucket &value)
{
    bool isValid = false;
    auto callIsRead = GetIntFromDataShareValueObject(value.Get(CallLogColumns::IS_READ, isValid));
    if (!isValid) {
        HILOG_WARN("UpdatePrivacyTagOfValueBucket, get callIsRead failed");
        callIsRead = static_cast<int64_t>(CALL_IS_READ);
    }
    auto callDirection = GetIntFromDataShareValueObject(value.Get(CallLogColumns::CALL_DIRECTION, isValid));
    if (!isValid) {
        HILOG_WARN("UpdatePrivacyTagOfValueBucket, get callDirection failed");
        callDirection = static_cast<int64_t>(CALL_DIRECTION_OUT);
    }
    auto callAnswerState = GetIntFromDataShareValueObject(value.Get(CallLogColumns::ANSWER_STATE, isValid));
    if (!isValid) {
        HILOG_WARN("UpdatePrivacyTagOfValueBucket, get callAnswerState failed");
        callAnswerState = static_cast<int64_t>(CALL_ANSWER_ACTIVED);
    }
    HILOG_INFO("UpdatePrivacyTagOfValueBucket, callIsRead: %{public}lld, callDirection: %{public}lld, callAnswerState: "
               "%{public}lld", (long long) callIsRead, (long long) callDirection, (long long) callAnswerState);
    if (callIsRead == static_cast<int64_t>(CALL_IS_UNREAD) &&
        callDirection == static_cast<int64_t>(CALL_DIRECTION_IN) &&
        callAnswerState == static_cast<int64_t>(CALL_ANSWER_MISSED)) {
        HILOG_INFO("unread missed call log update privacy tag");
        value.valuesMap[CallLogColumns::PRIVACY_TAG] =
            static_cast<int64_t>(PrivacyTag::CALL_LOG_UNREAD_MISSED_PRIVACY_TAG);
    } else {
        HILOG_INFO("call log update privacy tag");
        value.valuesMap[CallLogColumns::PRIVACY_TAG] = static_cast<int64_t>(PrivacyTag::CALL_LOG_PRIVACY_TAG);
    }
    return isValid;
}

bool PrivacyContactsManager::GetHelperAndUrl(const OHOS::AccountSA::OsAccountType &type,
    const std::string &datashareUri, std::shared_ptr<DataShare::DataShareHelper> &helper, std::string &url)
{
    int userId = -1;
    auto result = GetSpecialTypeUserId(type, userId);
    if (result != OPERATION_OK) {
        HILOG_ERROR("get userId faield!");
        return false;
    }
// LCOV_EXCL_START
    std::string tableName;
    if (datashareUri == CALL_LOG_URI) {
        tableName = "/calls/calllog";
    } else {
        if (type == OHOS::AccountSA::OsAccountType::ADMIN) {
            tableName = "/contacts/privacy_contacts_backup";
        } else if (type == OHOS::AccountSA::OsAccountType::PRIVATE) {
            tableName = "/contacts/contact_data";
        } else {
            HILOG_ERROR("type error!");
            return false;
        }
    }
    url = datashareUri + tableName + "?user=" + std::to_string(userId);
    helper = RetryCreateDataShareHelper([&]() { return CreateDataShareHelper(url); });

    HILOG_INFO("userId: %{public}d, uri: %{public}s", userId, url.c_str());
    if (helper == nullptr) {
        HILOG_ERROR("helper is nullptr!");
        return false;
    }
    return true;
// LCOV_EXCL_STOP
}

int32_t PrivacyContactsManager::GetSpecialTypeUserId(OHOS::AccountSA::OsAccountType type, int32_t &userId)
{
    // 判断是否是指定类型的账号
    std::vector<OHOS::AccountSA::OsAccountInfo> osAccountInfos;
    OHOS::AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    for (const auto &account : osAccountInfos) {
        if (type == account.GetType()) {
            userId = account.GetLocalId();
            return OPERATION_OK;
        }
    }
    // 没有找到指定空间的userId
    HILOG_ERROR("GetPrivateSpaceUserId failed: special type user not exist!: %{public}d", type);
    return OPERATION_ERROR;
}

std::shared_ptr<DataShare::DataShareHelper> PrivacyContactsManager::CreateDataShareHelper(std::string uri)
{
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HILOG_ERROR("Get system ability mgr failed.");
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("GetSystemAbility Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, uri, "");
}

int64_t PrivacyContactsManager::GetIntFromDataShareValueObject(const DataShare::DataShareValueObject &object)
{
    int64_t result = 0;
    if (std::get_if<int64_t>(&object.value) != nullptr) {
        result = static_cast<int64_t>(object);
    } else if (std::get_if<bool>(&object.value) != nullptr) {
        result = static_cast<int64_t>(static_cast<int64_t>(object));
    } else if (std::get_if<double>(&object.value) != nullptr) {
        result = static_cast<int64_t>(static_cast<double>(object));
    } else if (std::get_if<std::string>(&object.value) != nullptr) {
        result = std::atoi(static_cast<std::string>(object).c_str());
    }
    return result;
}

bool PrivacyContactsManager::GetPrivacySpacePhoneNumbers(std::unordered_set<std::string> &allPrivacyNums)
{
    std::shared_ptr<DataShare::DataShareHelper> helper;
    std::string url;
    if (!GetHelperAndUrl(OHOS::AccountSA::OsAccountType::ADMIN, CONTACT_URI, helper, url)) {
        HILOG_ERROR("GetPrivacySpacePhoneNumbers, helper is nullptr");
        return false;
    }
// LCOV_EXCL_START
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns{
        PrivacyContactsBackupColumns::PHONE_NUMBER, PrivacyContactsBackupColumns::FORMAT_PHONE_NUMBER};
    auto uriObj = Uri(url);
    auto resultSet = helper->Query(uriObj, predicates, columns);
    if (resultSet == nullptr) {
        HILOG_ERROR("query error, result is null");
        helper->Release();
        return false;
    }
    int32_t operationResult = resultSet->GoToFirstRow();
    while (operationResult == DataShare::E_OK) {
        for (size_t i = 0; i < columns.size(); i++) {
            std::string phoneNum;
            resultSet->GetString(i, phoneNum);
            if (!phoneNum.empty()) {
                allPrivacyNums.emplace(phoneNum);
            }
        }
        operationResult = resultSet->GoToNextRow();
    }
    resultSet->Close();
    helper->Release();
    return true;
// LCOV_EXCL_STOP
}

template <typename Func>
std::shared_ptr<DataShare::DataShareHelper> PrivacyContactsManager::RetryCreateDataShareHelper(Func &&func)
{
    std::shared_ptr<DataShare::DataShareHelper> helper = std::forward<decltype(func)>(func)();
    auto retryTimes = 0;
    while (helper == nullptr) {
        if (retryTimes++ < PRIVACY_CONTACTS_MANAGER_RETRY_CREATE_DATASHARE_HELPER_RETRY_TIMES) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(PRIVACY_CONTACTS_MANAGER_RETRY_CREATE_DATASHARE_HELPER_SLEEP_TIME));
            HILOG_INFO("Retry CreateDataShareHelper times: %{public}d", retryTimes);
            helper = std::forward<decltype(func)>(func)();
        } else {
            break;
        }
    }
    return helper;
}
}  // namespace Contacts
}  // namespace OHOS