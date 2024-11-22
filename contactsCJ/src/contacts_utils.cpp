/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "securec.h"
#include "contacts_napi_common.h" // from contacts_data/contacts
#include "contacts_utils.h"

using namespace OHOS;
using namespace OHOS::ContactsFfi;
using namespace OHOS::DataShare;

namespace OHOS {
namespace ContactsFfi {

// return true if succeeded; after it b is fully allocated or completely empty with errCode set
bool allocBucket(ValuesBucket* b, int total, int32_t *errCode)
{
    if (*errCode != ContactsApi::SUCCESS) {
        return false;
    }
    if (total > 0) {
        b->size = total;
        b->key = (char**) malloc(total * sizeof(char*));
        if (b->key == nullptr) {
            *errCode = ContactsApi::ERROR;
            b->freeContent(); // actually. there is nothing to free, just set size to 0
            return false;
        }
        b->value = (struct CValueType*) malloc(total * sizeof(struct CValueType));
        if (b->value == nullptr) {
            *errCode = ContactsApi::ERROR;
            b->freeContent();
            return false;
        }
    }
    return true;
}

DataShareValuesBucket convertToDataShareVB(ValuesBucket vb)
{
    int64_t size = vb.size;
    DataShareValuesBucket dsvb = DataShareValuesBucket();
    for (int64_t i = 0; i < size; i++) {
        switch (vb.value[i].tag) {
            case DataShareValueObjectType::TYPE_STRING: {
                DataShareValueObject valueObject = DataShareValueObject(std::string(vb.value[i].string));
                std::string keyStr = vb.key[i];
                dsvb.Put(keyStr, valueObject);
                break;
            }
            case DataShareValueObjectType::TYPE_INT: {
                DataShareValueObject valueObject = DataShareValueObject(vb.value[i].integer);
                std::string keyStr = vb.key[i];
                dsvb.Put(keyStr, valueObject);
                break;
            }
            default: { // should not reach here
                std::string keyStr = vb.key[i];
                HILOG_ERROR("VB unsupported value type is %{public}d for key = %{public}s",
                    vb.value[i].tag, keyStr.c_str());
                dsvb.Put(vb.key[i], DataShareValueObject());
                break;
            }
        }
    }
    return dsvb;
}

void PutQuickSearchKey(std::shared_ptr<DataShareResultSet> &resultSet,
    std::map<int, std::string> &quickSearchMap, int contactIdValue)
{
    std::string quickSearchValue = "";
    std::string quickSearchKey = "quick_search_key";
    int columnIndex = 0;
    resultSet->GetColumnIndex(quickSearchKey, columnIndex);
    resultSet->GetString(columnIndex, quickSearchValue);
    if (quickSearchMap.count(contactIdValue) <= 0) {
        quickSearchMap.insert(std::pair<int, std::string>(contactIdValue, quickSearchValue));
    }
}

std::vector<ValuesBucket> GetResultMapValue(std::map<int, std::vector<ValuesBucket>> &resultSetMap, int contactId)
{
    std::vector<ValuesBucket> contactData;
    if (resultSetMap.count(contactId) > 0) {
        std::map<int, std::vector<ValuesBucket>>::iterator it = resultSetMap.find(contactId);
        contactData = it->second;
    } else {
        contactData = std::vector<ValuesBucket>();
        resultSetMap.insert(std::pair<int, std::vector<ValuesBucket>>(contactId, contactData));
        std::map<int, std::vector<ValuesBucket>>::iterator it = resultSetMap.find(contactId);
        contactData = it->second;
    }
    return contactData;
}

void copyBucket(ValuesBucket* dst, int dstIdx, ValuesBucket &src)
{
    dst[dstIdx].key = src.key;
    dst[dstIdx].value = src.value;
    dst[dstIdx].size = src.size;
}

char* TransformFromString(std::string &str, int32_t* errCode)
{
    int64_t len = str.size() + 1;
    char* retValue = static_cast<char *>(malloc(len));
    if (retValue == nullptr) {
        *errCode = ContactsApi::ERROR;
        return nullptr;
    }
    *errCode = memcpy_s(retValue, len, str.c_str(), len);
    if (*errCode != ContactsApi::SUCCESS) {
        free(retValue);
        retValue = nullptr;
    }
    return retValue;
}

void PutSingleString(ValuesBucket &bucket, int idx, std::string key, std::string val, int32_t *errCode)
{
    bucket.key[idx] = TransformFromString(key, errCode);
    if (*errCode != ContactsApi::SUCCESS) {
        return;
    }
    bucket.value[idx].tag = static_cast<int>(DataType::TYPE_STRING);
    bucket.value[idx].string = TransformFromString(val, errCode);
}

ValuesBucket singleStringAsValueBucket(std::string contentType, std::string value, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_2, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", contentType, errCode);
    PutSingleString(b, BUCKET_IDX_1, "detail_info", value, errCode);
    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

void PutResultValue(ValuesBucket &bucket, int idx, std::string contentStoreKey,
                    std::shared_ptr<DataShareResultSet> &resultSet, std::string contentLoadKey, int32_t *errCode)
{
    int columnIndex = 0;
    resultSet->GetColumnIndex(contentLoadKey, columnIndex);
    DataType columnType;
    resultSet->GetDataType(columnIndex, columnType);

    bucket.key[idx] = TransformFromString(contentStoreKey, errCode);
    if (*errCode != ContactsApi::SUCCESS) {
        return;
    }

    bucket.value[idx].tag = static_cast<int>(columnType);

    // NULL and BLOB are ignored here
    if (columnType == DataType::TYPE_STRING) {
        std::string stringValue;
        resultSet->GetString(columnIndex, stringValue);
        bucket.value[idx].string = TransformFromString(stringValue, errCode);
        if (*errCode != ContactsApi::SUCCESS) {
            return;
        }
    } else if (columnType == DataType::TYPE_INTEGER) {
        int intValue = 0;
        resultSet->GetInt(columnIndex, intValue);
        bucket.value[idx].integer = intValue;
    } else if (columnType == DataType::TYPE_FLOAT) {
        double doubleValue = 0;
        resultSet->GetDouble(columnIndex, doubleValue);
        bucket.value[idx].dou = doubleValue;
    }
}

void PutResultValue(ValuesBucket &bucket, int idx,
                    std::shared_ptr<DataShareResultSet> &resultSet, std::string contentKey, int32_t *errCode)
{
    PutResultValue(bucket, idx, contentKey, resultSet, contentKey, errCode);
}

/**
 * @brief Converting resultset of a query to Email data ValuesBucket
 */
ValuesBucket resultSetAsEmail(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_5, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "email", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "alias_detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_3, resultSet, "custom_data", errCode);
    PutResultValue(b, BUCKET_IDX_4, resultSet, "extend7", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Name data ValuesBucket
 */
ValuesBucket resultSetAsName(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_10, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "name", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "alpha_name", errCode);
    PutResultValue(b, BUCKET_IDX_3, resultSet, "other_lan_last_name", errCode);
    PutResultValue(b, BUCKET_IDX_4, resultSet, "other_lan_first_name", errCode);
    PutResultValue(b, BUCKET_IDX_5, resultSet, "family_name", errCode);
    PutResultValue(b, BUCKET_IDX_6, resultSet, "middle_name_phonetic", errCode);
    PutResultValue(b, BUCKET_IDX_7, resultSet, "given_name", errCode);
    PutResultValue(b, BUCKET_IDX_8, resultSet, "given_name_phonetic", errCode);
    PutResultValue(b, BUCKET_IDX_9, resultSet, "phonetic_name", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Portrait data ValuesBucket
 */
ValuesBucket resultSetAsPortrait(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_2, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "photo", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Event data ValuesBucket
 */
ValuesBucket resultSetAsEvent(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_4, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "contact_event", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "custom_data", errCode);
    PutResultValue(b, BUCKET_IDX_3, resultSet, "extend7", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Group data ValuesBucket
 */
ValuesBucket resultSetAsGroup(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_3, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "group_membership", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "group_name", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to ImAddress data ValuesBucket
 */
ValuesBucket resultSetAsImAddress(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_4, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "im", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "custom_data", errCode);
    PutResultValue(b, BUCKET_IDX_3, resultSet, "extend7", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to PhoneNumber data ValuesBucket
 */
ValuesBucket resultSetAsPhone(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_4, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "phone", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "custom_data", errCode);
    PutResultValue(b, BUCKET_IDX_3, resultSet, "extend7", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to PostalAddress data ValuesBucket
 */
ValuesBucket resultSetAsPostAddress(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_11, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "postal_address", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "neighborhood", errCode);
    PutResultValue(b, BUCKET_IDX_3, resultSet, "pobox", errCode);
    PutResultValue(b, BUCKET_IDX_4, resultSet, "postcode", errCode);
    PutResultValue(b, BUCKET_IDX_5, resultSet, "region", errCode);
    PutResultValue(b, BUCKET_IDX_6, resultSet, "street", errCode);
    PutResultValue(b, BUCKET_IDX_7, resultSet, "city", errCode);
    PutResultValue(b, BUCKET_IDX_8, resultSet, "country", errCode);
    PutResultValue(b, BUCKET_IDX_9, resultSet, "custom_data", errCode);
    PutResultValue(b, BUCKET_IDX_10, resultSet, "extend7", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Relation data ValuesBucket
 */
ValuesBucket resultSetAsRelation(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_4, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "relation", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "custom_data", errCode);
    PutResultValue(b, BUCKET_IDX_3, resultSet, "extend7", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to SipAddress data ValuesBucket
 */
ValuesBucket resultSetAsSipAddress(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_4, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "sip_address", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "custom_data", errCode);
    PutResultValue(b, BUCKET_IDX_3, resultSet, "extend7", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Website data ValuesBucket
 */
ValuesBucket resultSetAsWebsite(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_2, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "website", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Nickname data ValuesBucket
 */
ValuesBucket resultSetAsNickname(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_2, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "nickname", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Note data ValuesBucket
 */
ValuesBucket resultSetAsNote(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_2, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "note", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to Organization data ValuesBucket
 */
ValuesBucket resultSetAsOrganization(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    struct ValuesBucket b;
    if (!allocBucket(&b, BUCKET_COUNT_3, errCode)) {
        return b;
    }

    PutSingleString(b, BUCKET_IDX_0, "content_type", "organization", errCode);
    PutResultValue(b, BUCKET_IDX_1, resultSet, "detail_info", errCode);
    PutResultValue(b, BUCKET_IDX_2, resultSet, "position", errCode);

    if (*errCode != ContactsApi::SUCCESS) {
        b.freeContent();
        return b;
    }

    return b;
}

/**
 * @brief Converting resultset of a query to one of contact's ValuesBucket
 */
void addResultSetAsValuesBucket(std::vector<ValuesBucket> contactData,
                                std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    int typeIdValue = 0;
    std::string typeId = "type_id";
    int columnIndexType = 0;
    resultSet->GetColumnIndex(typeId, columnIndexType);
    resultSet->GetInt(columnIndexType, typeIdValue);
    switch (typeIdValue) {
        case ContactsApi::EMAIL:            contactData.push_back(resultSetAsEmail(resultSet, errCode)); return;
        case ContactsApi::NAME:             contactData.push_back(resultSetAsName(resultSet, errCode)); return;
        case ContactsApi::PHOTO:            contactData.push_back(resultSetAsPortrait(resultSet, errCode)); return;
        case ContactsApi::CONTACT_EVENT:    contactData.push_back(resultSetAsEvent(resultSet, errCode)); return;
        case ContactsApi::GROUP_MEMBERSHIP: contactData.push_back(resultSetAsGroup(resultSet, errCode)); return;
        case ContactsApi::IM:               contactData.push_back(resultSetAsImAddress(resultSet, errCode)); return;
        case ContactsApi::PHONE:            contactData.push_back(resultSetAsPhone(resultSet, errCode)); return;
        case ContactsApi::POSTAL_ADDRESS:   contactData.push_back(resultSetAsPostAddress(resultSet, errCode)); return;
        case ContactsApi::RELATION:         contactData.push_back(resultSetAsRelation(resultSet, errCode)); return;
        case ContactsApi::SIP_ADDRESS:      contactData.push_back(resultSetAsSipAddress(resultSet, errCode)); return;
        case ContactsApi::WEBSITE:          contactData.push_back(resultSetAsWebsite(resultSet, errCode)); return;
        case ContactsApi::NICKNAME:         contactData.push_back(resultSetAsNickname(resultSet, errCode)); return;
        case ContactsApi::NOTE:             contactData.push_back(resultSetAsNote(resultSet, errCode)); return;
        case ContactsApi::ORGANIZATION:     contactData.push_back(resultSetAsOrganization(resultSet, errCode)); return;
        default:                            return;
    }
}

// returns false when mem allocation failed
bool allocateDataForContact(ContactsData* allContacts, int contactIndex, int contactId, std::string searchKey,
                            std::vector<ValuesBucket> contactDataVector, int32_t *errCode)
{
    int bucketIndex = 0;
    // consider passing contactId as integer value
    ValuesBucket idBucket = singleStringAsValueBucket("id", std::to_string(contactId), errCode);
    if (*errCode != ContactsApi::SUCCESS) {
        free(allContacts->contactsData[contactIndex].data);
        allContacts->contactsData[contactIndex].bucketCount = 0;
        allContacts->contactsData[contactIndex].data = nullptr;
        return false;
    }
    ValuesBucket searchKeyBucket = singleStringAsValueBucket("key", searchKey, errCode);
    if (*errCode != ContactsApi::SUCCESS) {
        idBucket.freeContent();
        free(allContacts->contactsData[contactIndex].data);
        allContacts->contactsData[contactIndex].bucketCount = 0;
        allContacts->contactsData[contactIndex].data = nullptr;
        return false;
    }
    copyBucket(allContacts->contactsData[contactIndex].data, bucketIndex, idBucket);
    bucketIndex++;
    copyBucket(allContacts->contactsData[contactIndex].data, bucketIndex, searchKeyBucket);
    bucketIndex++;

    // copy bucket pointers from vector
    for (std::vector<ValuesBucket>::size_type i = 0; i < contactDataVector.size(); i++, bucketIndex++) {
        copyBucket(allContacts->contactsData[contactIndex].data, bucketIndex, contactDataVector[i]);
    }

    return true;
}

void releaseAllContacts(ContactsData* allContacts)
{
    for (int i = 0; i < allContacts->contactsCount; i++) {
        if (allContacts->contactsData[i].data != nullptr) {
            for (int b = 0; b < allContacts->contactsData[i].bucketCount; b++) {
                ValuesBucket bucket = allContacts->contactsData[i].data[b];
                bucket.freeContent();
            }
            free(allContacts->contactsData[i].data);
            allContacts->contactsData[i].data = nullptr;
            allContacts->contactsData[i].bucketCount = 0;
        }
    }
    free(allContacts->contactsData);
    allContacts->contactsData = nullptr;
    allContacts->contactsCount = 0;
}

void releaseRresultSetMapBuckets(std::map<int, std::vector<ValuesBucket>> resultSetMap)
{
    std::map<int, std::vector<ValuesBucket>>::iterator it;
    for (it = resultSetMap.begin(); it != resultSetMap.end(); it++) {
        std::vector<ValuesBucket> contactDataVector = it->second;
        for (std::vector<ValuesBucket>::size_type i = 0; i < contactDataVector.size(); i++) {
            ValuesBucket bucket = contactDataVector[i];
            bucket.freeContent();
        }
    }
}

ContactsData* allocCollectedContacts(std::map<int, std::vector<ValuesBucket>> resultSetMap,
                                     std::map<int, std::string> quickSearchMap, int32_t *errCode)
{
    ContactsData* allContacts = (struct ContactsData*) malloc(sizeof(struct ContactsData));
    if (allContacts == nullptr) {
        HILOG_ERROR("ContactUtils::allocCollectedContacts fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return nullptr;
    }

    int totalContacts = resultSetMap.size();
    if (totalContacts == 0) {
        free(allContacts);
        return nullptr;
    }

    allContacts->contactsData = (ContactData*) malloc(totalContacts * sizeof(ContactData));
    if (allContacts->contactsData == nullptr) {
        free(allContacts);
        HILOG_ERROR("ContactUtils::allocCollectedContacts fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return nullptr;
    }
    allContacts->contactsCount = totalContacts;

    int contactIndex = 0;
    std::map<int, std::vector<ValuesBucket>>::iterator it;
    for (it = resultSetMap.begin(); it != resultSetMap.end(); it++, contactIndex++) {
        int contactId = it->first;
        std::vector<ValuesBucket> contactDataVector = it->second;
        std::string searchKey = quickSearchMap.find(contactId)->second;

        if (*errCode != ContactsApi::SUCCESS) {
            allContacts->contactsData[contactIndex].bucketCount = 0;
            allContacts->contactsData[contactIndex].data = nullptr;
            continue;
        }
        int totalBuckets = 2 + contactDataVector.size(); // 2 more for contactId and searchKey buckets
        allContacts->contactsData[contactIndex].bucketCount = totalBuckets;
        allContacts->contactsData[contactIndex].data =
            (struct ValuesBucket*) malloc(totalBuckets * sizeof(struct ValuesBucket));
        if (allContacts->contactsData[contactIndex].data == nullptr) {
            *errCode = ContactsApi::ERROR;
            continue;
        }

        allocateDataForContact(allContacts, contactIndex, contactId, searchKey, contactDataVector, errCode);
    }

    if (*errCode != ContactsApi::SUCCESS) {
        releaseAllContacts(allContacts);
        releaseRresultSetMapBuckets(resultSetMap);
        return nullptr;
    }

    return allContacts;
}

// it closes resultSet after parse
ContactsData* parseResultSetForContacts(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    if (resultSet == nullptr) {
        HILOG_ERROR("ContactUtils::parseResultSetForContacts resultSet is nullptr");
        return nullptr;
    }
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    HILOG_INFO("parseResultSetForContacts GetRowCount is %{public}d", rowCount);
    if (rowCount == 0) {
        resultSet->Close();
        return nullptr;
    }
    std::map<int, std::vector<ValuesBucket>> resultSetMap; // factored by contactId
    std::map<int, std::string> quickSearchMap;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        int contactIdValue = 0;
        std::string contactIdKey = "contact_id";
        int contactIndex = 0;
        resultSet->GetColumnIndex(contactIdKey, contactIndex);
        resultSet->GetInt(contactIndex, contactIdValue);
        std::vector<ValuesBucket> contactData = GetResultMapValue(resultSetMap, contactIdValue);
        PutQuickSearchKey(resultSet, quickSearchMap, contactIdValue);
        addResultSetAsValuesBucket(contactData, resultSet, errCode);
        if (*errCode != ContactsApi::SUCCESS) {
            HILOG_ERROR("ContactUtils::parseResultSetForContacts fail to mem alloc");
            break;
        }
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();

    if (*errCode != ContactsApi::SUCCESS) {
        std::map<int, std::vector<ValuesBucket>>::iterator it;
        for (it = resultSetMap.begin(); it != resultSetMap.end(); it++) {
            std::vector<ValuesBucket> contactDataVector = it->second;
            for (std::vector<ValuesBucket>::size_type i = 0; i < contactDataVector.size(); i++) {
                ValuesBucket bucket = contactDataVector[i];
                bucket.freeContent();
            }
        }
        return nullptr;
    }
    return allocCollectedContacts(resultSetMap, quickSearchMap, errCode);
}

/**
 * @brief Converting resultset of a query to Group data ValuesBucket
 */
void resultSetAsGroup(ValuesBucket* groups, int idx, std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    int total = 2;
    groups[idx].key = (char**) malloc(total * sizeof(char*));
    if (groups[idx].key == nullptr) {
        HILOG_ERROR("ContactUtils::resultSetAsGroup fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return;
    }

    groups[idx].value = (struct CValueType*) malloc(total * sizeof(struct CValueType));
    if (groups[idx].value == nullptr) {
        free(groups[idx].key);
        HILOG_ERROR("ContactUtils::resultSetAsGroup fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return;
    }
    groups[idx].size = total;

    // content_type for group is redundant
    PutResultValue(groups[idx], BUCKET_IDX_0, "detail_info", resultSet, "id", errCode);
    if (*errCode != ContactsApi::SUCCESS) {
        free(groups[idx].key);
        free(groups[idx].value);
        HILOG_ERROR("ContactUtils::resultSetAsGroup fail to mem alloc");
        return;
    }

    PutResultValue(groups[idx], BUCKET_IDX_1, "group_name", resultSet, "group_name", errCode);
    if (*errCode != ContactsApi::SUCCESS) {
        free(groups[idx].key);
        free(groups[idx].value);
        HILOG_ERROR("ContactUtils::resultSetAsGroup fail to mem alloc");
        return;
    }
}

/**
 * @brief Converting resultset of a query to Holder data ValuesBucket
 */
void resultSetAsHolder(ValuesBucket* holders, int idx, std::shared_ptr<DataShareResultSet> &resultSet,
                       int32_t *errCode)
{
    int total = 3;
    holders[idx].key = (char**) malloc(total * sizeof(char*));
    if (holders[idx].key == nullptr) {
        HILOG_ERROR("ContactUtils::resultSetAsHolder fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return;
    }

    holders[idx].value = (struct CValueType*) malloc(total * sizeof(struct CValueType));
    if (holders[idx].value == nullptr) {
        free(holders[idx].key);
        HILOG_ERROR("ContactUtils::resultSetAsHolder fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return;
    }

    holders[idx].size = total;

    // content_type for holder is redundant
    PutResultValue(holders[idx], BUCKET_IDX_0, "detail_info", resultSet, "account_name", errCode);
    PutResultValue(holders[idx], BUCKET_IDX_1, "custom_data", resultSet, "account_type", errCode);
    PutResultValue(holders[idx], BUCKET_IDX_2, "extend7", resultSet, "id", errCode);
}

// it closes resultSet after parse
GroupsData* parseResultSetForGroups(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    if (resultSet == nullptr) {
        HILOG_ERROR("ContactUtils::parseResultSetForGroups resultSet is nullptr");
        return nullptr;
    }
    int totalGroups = 0;
    resultSet->GetRowCount(totalGroups);
    HILOG_INFO("parseResultSetForGroups GetRowCount is %{public}d", totalGroups);
    if (totalGroups == 0) {
        return nullptr;
    }

    GroupsData* allGroups = (GroupsData*) malloc(sizeof(GroupsData));
    if (allGroups == nullptr) {
        HILOG_ERROR("ContactUtils::parseResultSetForGroups fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return nullptr;
    }
    allGroups->data = (struct ValuesBucket*) malloc(totalGroups * sizeof(struct ValuesBucket));
    if (allGroups->data == nullptr) {
        free(allGroups);
        HILOG_ERROR("ContactUtils::parseResultSetForGroups fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return NULL;
    }
    allGroups->bucketCount = totalGroups;

    int resultSetNum = resultSet->GoToFirstRow();
    int count = 0;
    while (resultSetNum == 0) {
        resultSetAsGroup(allGroups->data, count, resultSet, errCode);
        if (*errCode != ContactsApi::SUCCESS) {
            break;
        }
        resultSetNum = resultSet->GoToNextRow();
        count++;
    }
    resultSet->Close();

    if (*errCode != ContactsApi::SUCCESS) {
        allGroups->freeContent();
        free(allGroups);
        allGroups = nullptr;
    }

    return allGroups;
}

// it closes resultSet after parse
HoldersData* parseResultSetForHolders(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    if (resultSet == nullptr) {
        HILOG_ERROR("ContactUtils::parseResultSetForHolders resultSet is nullptr");
        return nullptr;
    }
    int totalHolders = 0;
    resultSet->GetRowCount(totalHolders);
    HILOG_INFO("parseResultSetForHolders GetRowCount is %{public}d", totalHolders);
    if (totalHolders == 0) {
        return nullptr;
    }

    HoldersData* allHolders = (HoldersData*) malloc(sizeof(HoldersData));
    if (allHolders == nullptr) {
        HILOG_ERROR("ContactUtils::parseResultSetForHolders fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return nullptr;
    }
    allHolders->data = (struct ValuesBucket*) malloc(totalHolders * sizeof(struct ValuesBucket));
    if (allHolders->data == nullptr) {
        free(allHolders);
        HILOG_ERROR("ContactUtils::parseResultSetForHolders fail to mem alloc");
        *errCode = ContactsApi::ERROR;
        return nullptr;
    }
    allHolders->bucketCount = totalHolders;

    int resultSetNum = resultSet->GoToFirstRow();
    int count = 0;
    while (resultSetNum == 0) {
        resultSetAsHolder(allHolders->data, count, resultSet, errCode);
        if (*errCode != ContactsApi::SUCCESS) {
            break;
        }
        resultSetNum = resultSet->GoToNextRow();
        count++;
    }
    resultSet->Close();

    if (*errCode != ContactsApi::SUCCESS) {
        allHolders->freeContent();
        free(allHolders);
        allHolders = nullptr;
    }

    return allHolders;
}

} // namespace ContactsFfi
} // namespace OHOS
