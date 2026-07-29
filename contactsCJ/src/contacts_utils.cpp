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
#include "contacts_utils.h"

using namespace OHOS;
using namespace OHOS::ContactsFfi;
using namespace OHOS::DataShare;

namespace OHOS {
namespace ContactsFfi {

// return true if succeeded; after it b is fully allocated or completely empty with errCode set
bool allocBucket(ValuesBucket* b, size_t total, int32_t *errCode)
{
    if (*errCode != SUCCESS) {
        return false;
    }
    if (total > 0) {
        b->size = total;
        // calloc zeroes memory so that freeContent() is safe on partially filled buckets:
        // key[i]==nullptr and value[i].tag==TYPE_NULL/value[i].string==nullptr by default.
        b->key = (char**) calloc(total, sizeof(char*));
        if (b->key == nullptr) {
            *errCode = ERROR;
            b->freeContent(); // actually. there is nothing to free, just set size to 0
            return false;
        }
        b->value = (struct CValueType*) calloc(total, sizeof(struct CValueType));
        if (b->value == nullptr) {
            *errCode = ERROR;
            b->freeContent();
            return false;
        }
    }
    return true;
}

DataShareValuesBucket convertToDataShareVB(ValuesBucket vb)
{
    uint64_t size = vb.size;
    DataShareValuesBucket dsvb = DataShareValuesBucket();
    for (uint64_t i = 0; i < size; i++) {
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

std::vector<ValuesBucket>& GetResultMapValue(std::map<int, std::vector<ValuesBucket>> &resultSetMap, int contactId)
{
    std::map<int, std::vector<ValuesBucket>>::iterator it = resultSetMap.find(contactId);
    if (it == resultSetMap.end()) {
        std::vector<ValuesBucket> contactData = std::vector<ValuesBucket>();
        resultSetMap.insert(std::pair<int, std::vector<ValuesBucket>>(contactId, contactData));
        it = resultSetMap.find(contactId);
    }
    return it->second;
}

// Allocates a NUL-terminated deep copy of srcStr via malloc + memcpy_s. Returns nullptr
// for a nullptr input (no error). On malloc/memcpy_s failure frees the partial buffer,
// sets *errCode = ERROR (if non-null), and returns nullptr; callers detect failure by
// observing a nullptr result for a non-null input.
static char* dupCString(const char* srcStr, int32_t *errCode)
{
    if (srcStr == nullptr) {
        return nullptr;
    }
    size_t len = strlen(srcStr) + 1;
    char* dst = static_cast<char*>(malloc(len));
    if (dst == nullptr || memcpy_s(dst, len, srcStr, len) != 0) {
        free(dst);
        HILOG_ERROR("dupCString failed to copy string of length %{public}llu",
            static_cast<unsigned long long>(len));
        if (errCode != nullptr) {
            *errCode = ERROR;
        }
        return nullptr;
    }
    return dst;
}

void copyBucket(ValuesBucket* dst, int dstIdx, ValuesBucket &src, int32_t *errCode)
{
    // Deep copy: dst owns its own key/value arrays so that freeing dst and src never
    // double-frees the same heap memory (e.g. allocCollectedContacts error path used to
    // free the shared pointers via both allContacts->freeContent() and releaseRresultSetMapBuckets).
    // On any allocation/copy failure sets *errCode = ERROR and returns early; the dst slot is
    // left in a freeContent()-safe state (calloc-zeroed arrays, nullptr for the calloc-fail case)
    // so callers can release the partial copy via dst[dstIdx].freeContent().
    dst[dstIdx].size = src.size;
    if (src.size == 0) {
        dst[dstIdx].key = nullptr;
        dst[dstIdx].value = nullptr;
        return;
    }
    dst[dstIdx].key = static_cast<char**>(calloc(src.size, sizeof(char*)));
    dst[dstIdx].value = static_cast<struct CValueType*>(calloc(src.size, sizeof(struct CValueType)));
    if (dst[dstIdx].key == nullptr || dst[dstIdx].value == nullptr) {
        free(dst[dstIdx].key);
        dst[dstIdx].key = nullptr;
        free(dst[dstIdx].value);
        dst[dstIdx].value = nullptr;
        dst[dstIdx].size = 0;
        if (errCode != nullptr) {
            *errCode = ERROR;
        }
        return;
    }
    for (uint64_t i = 0; i < src.size; i++) {
        if (src.key[i] != nullptr) {
            dst[dstIdx].key[i] = dupCString(src.key[i], errCode);
            if (dst[dstIdx].key[i] == nullptr) {
                return;
            }
        }
        dst[dstIdx].value[i].tag = src.value[i].tag;
        dst[dstIdx].value[i].integer = src.value[i].integer;
        dst[dstIdx].value[i].dou = src.value[i].dou;
        dst[dstIdx].value[i].boolean = src.value[i].boolean;
        if (src.value[i].tag == static_cast<int>(DataShareValueObjectType::TYPE_STRING) &&
            src.value[i].string != nullptr) {
            dst[dstIdx].value[i].string = dupCString(src.value[i].string, errCode);
            if (dst[dstIdx].value[i].string == nullptr) {
                return;
            }
        }
    }
}

char* TransformFromString(std::string &str, int32_t* errCode)
{
    size_t len = str.size() + 1;
    char* retValue = static_cast<char *>(malloc(len));
    if (retValue == nullptr) {
        *errCode = ERROR;
        return nullptr;
    }
    *errCode = memcpy_s(retValue, len, str.c_str(), len);
    if (*errCode != SUCCESS) {
        free(retValue);
        retValue = nullptr;
    }
    return retValue;
}

ValuesBucket allocBucketData(std::vector<KeyWithValueType> &bucketData, int32_t *errCode)
{
    struct ValuesBucket b;
    size_t total = bucketData.size();
    if (total > 0) {
        if (!allocBucket(&b, total, errCode)) {
            return b;
        }
        // Let's increment b.size one by one for b.freeContent() call in case of error
        b.size = 0;
        for (size_t i = 0; i < total; i++) {
            bucketData[i].allocToBucket(&b, 0, i, errCode);
            if (*errCode != SUCCESS) {
                b.freeContent();
                return b;
            } else {
                b.size = b.size + 1; // b.size is incremented one by one for b.freeContent() call in case of error
            }
        }
    }
    return b;
}

ValuesBucket singleStringAsValueBucket(std::string contentType, std::string value, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", contentType));
    bucketData.push_back(KeyWithValueType("detail_info", value));
    return allocBucketData(bucketData, errCode);
}

void PutResultValue(std::vector<KeyWithValueType> &bucket, std::string contentStoreKey,
                    std::shared_ptr<DataShareResultSet> &resultSet, std::string contentLoadKey,
                    int32_t *errCode)
{
    int columnIndex = 0;
    if (resultSet->GetColumnIndex(contentLoadKey, columnIndex) != 0) {
        HILOG_ERROR("PutResultValue GetColumnIndex failed for key %{public}s", contentLoadKey.c_str());
        return;
    }
    DataType columnType = DataType::TYPE_NULL;
    if (resultSet->GetDataType(columnIndex, columnType) != 0) {
        HILOG_ERROR("PutResultValue GetDataType failed for key %{public}s", contentLoadKey.c_str());
        return;
    }

    // NULL and BLOB are ignored here
    if (columnType == DataType::TYPE_STRING) {
        std::string stringValue;
        if (resultSet->GetString(columnIndex, stringValue) != 0) {
            HILOG_ERROR("PutResultValue GetString failed for key %{public}s", contentLoadKey.c_str());
            *errCode = ERROR;
            return;
        }
        bucket.push_back(KeyWithValueType(contentStoreKey, stringValue));
    } else if (columnType == DataType::TYPE_INTEGER) {
        int intValue = 0;
        if (resultSet->GetInt(columnIndex, intValue) != 0) {
            HILOG_ERROR("PutResultValue GetInt failed for key %{public}s", contentLoadKey.c_str());
            *errCode = ERROR;
            return;
        }
        bucket.push_back(KeyWithValueType(contentStoreKey, static_cast<int64_t>(intValue)));
    } else if (columnType == DataType::TYPE_FLOAT) {
        double doubleValue = 0;
        if (resultSet->GetDouble(columnIndex, doubleValue) != 0) {
            HILOG_ERROR("PutResultValue GetDouble failed for key %{public}s", contentLoadKey.c_str());
            *errCode = ERROR;
            return;
        }
        bucket.push_back(KeyWithValueType(contentStoreKey, doubleValue));
    } else if (columnType != DataType::TYPE_NULL) { // TYPE_NULL is just ignored
        HILOG_ERROR("PutResultValue unsupported columnType for key %{public}s is %{public}d",
            contentLoadKey.c_str(), columnType);
    }
}

void PutResultValue(std::vector<KeyWithValueType> &bucket,
                    std::shared_ptr<DataShareResultSet> &resultSet, std::string contentKey,
                    int32_t *errCode)
{
    PutResultValue(bucket, contentKey, resultSet, contentKey, errCode);
}

/**
 * @brief Converting resultset of a query to Email data ValuesBucket
 */
ValuesBucket resultSetAsEmail(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "email"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "alias_detail_info", errCode);
    PutResultValue(bucketData, resultSet, "custom_data", errCode);
    PutResultValue(bucketData, resultSet, "extend7", errCode);
    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Name data ValuesBucket
 */
ValuesBucket resultSetAsName(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "name"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "alpha_name", errCode);
    PutResultValue(bucketData, resultSet, "other_lan_last_name", errCode);
    PutResultValue(bucketData, resultSet, "other_lan_first_name", errCode);
    PutResultValue(bucketData, resultSet, "family_name", errCode);
    PutResultValue(bucketData, resultSet, "middle_name_phonetic", errCode);
    PutResultValue(bucketData, resultSet, "given_name", errCode);
    PutResultValue(bucketData, resultSet, "given_name_phonetic", errCode);
    PutResultValue(bucketData, resultSet, "phonetic_name", errCode);
    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Portrait data ValuesBucket
 */
ValuesBucket resultSetAsPortrait(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "photo"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Event data ValuesBucket
 */
ValuesBucket resultSetAsEvent(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "contact_event"));

    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "custom_data", errCode);
    PutResultValue(bucketData, resultSet, "extend7", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Group data ValuesBucket
 */
ValuesBucket resultSetAsGroup(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "group_membership"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "group_name", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to ImAddress data ValuesBucket
 */
ValuesBucket resultSetAsImAddress(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "im"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "custom_data", errCode);
    PutResultValue(bucketData, resultSet, "extend7", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to PhoneNumber data ValuesBucket
 */
ValuesBucket resultSetAsPhone(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "phone"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "custom_data", errCode);
    PutResultValue(bucketData, resultSet, "extend7", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to PostalAddress data ValuesBucket
 */
ValuesBucket resultSetAsPostAddress(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "postal_address"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "neighborhood", errCode);
    PutResultValue(bucketData, resultSet, "pobox", errCode);
    PutResultValue(bucketData, resultSet, "postcode", errCode);
    PutResultValue(bucketData, resultSet, "region", errCode);
    PutResultValue(bucketData, resultSet, "street", errCode);
    PutResultValue(bucketData, resultSet, "city", errCode);
    PutResultValue(bucketData, resultSet, "country", errCode);
    PutResultValue(bucketData, resultSet, "custom_data", errCode);
    PutResultValue(bucketData, resultSet, "extend7", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Relation data ValuesBucket
 */
ValuesBucket resultSetAsRelation(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "relation"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "custom_data", errCode);
    PutResultValue(bucketData, resultSet, "extend7", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to SipAddress data ValuesBucket
 */
ValuesBucket resultSetAsSipAddress(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "sip_address"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "custom_data", errCode);
    PutResultValue(bucketData, resultSet, "extend7", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Website data ValuesBucket
 */
ValuesBucket resultSetAsWebsite(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "website"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Nickname data ValuesBucket
 */
ValuesBucket resultSetAsNickname(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "nickname"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Note data ValuesBucket
 */
ValuesBucket resultSetAsNote(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "note"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to Organization data ValuesBucket
 */
ValuesBucket resultSetAsOrganization(std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    bucketData.push_back(KeyWithValueType("content_type", "organization"));
    PutResultValue(bucketData, resultSet, "detail_info", errCode);
    PutResultValue(bucketData, resultSet, "position", errCode);

    return allocBucketData(bucketData, errCode);
}

/**
 * @brief Converting resultset of a query to one of contact's ValuesBucket
 */
void addResultSetAsValuesBucket(std::vector<ValuesBucket> &contactData,
                                std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    int typeIdValue = 0;
    std::string typeId = "type_id";
    int columnIndexType = 0;
    if (resultSet->GetColumnIndex(typeId, columnIndexType) != 0) {
        HILOG_ERROR("addResultSetAsValuesBucket GetColumnIndex failed for type_id");
        return;
    }
    if (resultSet->GetInt(columnIndexType, typeIdValue) != 0) {
        HILOG_ERROR("addResultSetAsValuesBucket GetInt failed for type_id");
        return;
    }
    switch (typeIdValue) {
        case EMAIL:            contactData.push_back(resultSetAsEmail(resultSet, errCode)); return;
        case NAME:             contactData.push_back(resultSetAsName(resultSet, errCode)); return;
        case PHOTO:            contactData.push_back(resultSetAsPortrait(resultSet, errCode)); return;
        case CONTACT_EVENT:    contactData.push_back(resultSetAsEvent(resultSet, errCode)); return;
        case GROUP_MEMBERSHIP: contactData.push_back(resultSetAsGroup(resultSet, errCode)); return;
        case IM:               contactData.push_back(resultSetAsImAddress(resultSet, errCode)); return;
        case PHONE:            contactData.push_back(resultSetAsPhone(resultSet, errCode)); return;
        case POSTAL_ADDRESS:   contactData.push_back(resultSetAsPostAddress(resultSet, errCode)); return;
        case RELATION:         contactData.push_back(resultSetAsRelation(resultSet, errCode)); return;
        case SIP_ADDRESS:      contactData.push_back(resultSetAsSipAddress(resultSet, errCode)); return;
        case WEBSITE:          contactData.push_back(resultSetAsWebsite(resultSet, errCode)); return;
        case NICKNAME:         contactData.push_back(resultSetAsNickname(resultSet, errCode)); return;
        case NOTE:             contactData.push_back(resultSetAsNote(resultSet, errCode)); return;
        case ORGANIZATION:     contactData.push_back(resultSetAsOrganization(resultSet, errCode)); return;
        default:               return;
    }
}

// Releases dst slots [0..failIdx] of one ContactData (those touched by copyBucket) and the
// data array itself. allocateDataForContact's data array comes from malloc (not calloc), so
// untouched slots hold garbage and must NOT be passed to freeContent; that is why we sweep
// only the touched range here instead of calling ContactData::freeContent.
static void releaseTouchedContactBuckets(ContactData* contactDataSlot, int failIdx)
{
    if (contactDataSlot == nullptr || contactDataSlot->data == nullptr) {
        return;
    }
    for (int b = 0; b <= failIdx; b++) {
        contactDataSlot->data[b].freeContent();
    }
    free(contactDataSlot->data);
    contactDataSlot->data = nullptr;
    contactDataSlot->bucketCount = 0;
}

// returns false when mem allocation failed
bool allocateDataForContact(ContactsData* allContacts, int contactIndex, ContactInfo contactInfo, int32_t *errCode)
{
    int bucketIndex = 0;
    ValuesBucket idBucket = singleStringAsValueBucket("id", std::to_string(contactInfo.contactId), errCode);
    if (*errCode != SUCCESS) {
        free(allContacts->contactsData[contactIndex].data);
        allContacts->contactsData[contactIndex].bucketCount = 0;
        allContacts->contactsData[contactIndex].data = nullptr;
        return false;
    }
    ValuesBucket searchKeyBucket = singleStringAsValueBucket("key", contactInfo.searchKey, errCode);
    if (*errCode != SUCCESS) {
        idBucket.freeContent();
        free(allContacts->contactsData[contactIndex].data);
        allContacts->contactsData[contactIndex].bucketCount = 0;
        allContacts->contactsData[contactIndex].data = nullptr;
        return false;
    }

    copyBucket(allContacts->contactsData[contactIndex].data, bucketIndex, idBucket, errCode);
    idBucket.freeContent();
    if (*errCode != SUCCESS) {
        HILOG_ERROR("allocateDataForContact copyBucket failed for idBucket");
        releaseTouchedContactBuckets(&allContacts->contactsData[contactIndex], bucketIndex);
        searchKeyBucket.freeContent();
        return false;
    }
    bucketIndex++;
    copyBucket(allContacts->contactsData[contactIndex].data, bucketIndex, searchKeyBucket, errCode);
    searchKeyBucket.freeContent();
    if (*errCode != SUCCESS) {
        HILOG_ERROR("allocateDataForContact copyBucket failed for searchKeyBucket");
        releaseTouchedContactBuckets(&allContacts->contactsData[contactIndex], bucketIndex);
        return false;
    }
    bucketIndex++;

    // copy bucket pointers from vector
    for (std::vector<ValuesBucket>::size_type i = 0; i < contactInfo.contactDataVector.size(); i++, bucketIndex++) {
        // deep copy: resultSetMap keeps ownership of the originals, allContacts owns its own copies
        copyBucket(allContacts->contactsData[contactIndex].data, bucketIndex, contactInfo.contactDataVector[i],
            errCode);
        if (*errCode != SUCCESS) {
            HILOG_ERROR("allocateDataForContact copyBucket failed at vector index %{public}zu", i);
            releaseTouchedContactBuckets(&allContacts->contactsData[contactIndex], bucketIndex);
            return false;
        }
    }

    return true;
}

void releaseRresultSetMapBuckets(std::map<int, std::vector<ValuesBucket>> &resultSetMap)
{
    std::map<int, std::vector<ValuesBucket>>::iterator it;
    for (it = resultSetMap.begin(); it != resultSetMap.end(); it++) {
        std::vector<ValuesBucket> &contactDataVector = it->second;
        for (std::vector<ValuesBucket>::size_type i = 0; i < contactDataVector.size(); i++) {
            contactDataVector[i].freeContent();
        }
    }
}

ContactsData* allocContactsDataHeader(size_t totalContacts, int32_t *errCode)
{
    ContactsData* allContacts = (struct ContactsData*) malloc(sizeof(struct ContactsData));
    if (allContacts == nullptr) {
        HILOG_ERROR("ContactUtils::allocCollectedContacts fail to mem alloc");
        *errCode = ERROR;
        return nullptr;
    }

    if (totalContacts == 0 || totalContacts > MAX_CONTACTS) {
        HILOG_ERROR("ContactUtils::allocContactsDataHeader invalid totalContacts %{public}zu", totalContacts);
        *errCode = ERROR;
        free(allContacts);
        return nullptr;
    }

    allContacts->contactsData = (ContactData*) malloc(totalContacts * sizeof(ContactData));
    if (allContacts->contactsData == nullptr) {
        free(allContacts);
        HILOG_ERROR("ContactUtils::allocCollectedContacts fail to mem alloc");
        *errCode = ERROR;
        return nullptr;
    }
    allContacts->contactsCount = totalContacts;
    return allContacts;
}

void allocSingleContact(ContactsData* allContacts, int contactIndex,
                        std::pair<const int, std::vector<ValuesBucket>> &entry,
                        std::map<int, std::string> &quickSearchMap, int32_t *errCode)
{
    if (*errCode != SUCCESS) {
        allContacts->contactsData[contactIndex].bucketCount = 0;
        allContacts->contactsData[contactIndex].data = nullptr;
        return;
    }
    int contactId = entry.first;
    std::vector<ValuesBucket> contactDataVector = entry.second;
    auto searchIt = quickSearchMap.find(contactId);
    if (searchIt == quickSearchMap.end()) {
        HILOG_ERROR("ContactUtils::allocSingleContact quickSearchKey not found for contactId %{public}d", contactId);
        *errCode = ERROR;
        allContacts->contactsData[contactIndex].bucketCount = 0;
        allContacts->contactsData[contactIndex].data = nullptr;
        return;
    }
    std::string searchKey = searchIt->second;
    size_t totalBuckets = 2 + contactDataVector.size();
    allContacts->contactsData[contactIndex].bucketCount = totalBuckets;
    allContacts->contactsData[contactIndex].data =
        (struct ValuesBucket*) malloc(totalBuckets * sizeof(struct ValuesBucket));
    if (allContacts->contactsData[contactIndex].data == nullptr) {
        *errCode = ERROR;
        return;
    }

    ContactInfo contactInfo;
    contactInfo.contactId = contactId;
    contactInfo.searchKey = searchKey;
    contactInfo.contactDataVector = contactDataVector;
    allocateDataForContact(allContacts, contactIndex, contactInfo, errCode);
}

ContactsData* allocCollectedContacts(std::map<int, std::vector<ValuesBucket>> &resultSetMap,
                                     std::map<int, std::string> &quickSearchMap, int32_t *errCode)
{
    ContactsData* allContacts = allocContactsDataHeader(resultSetMap.size(), errCode);
    if (allContacts == nullptr) {
        return nullptr;
    }

    int contactIndex = 0;
    for (auto it = resultSetMap.begin(); it != resultSetMap.end(); it++, contactIndex++) {
        allocSingleContact(allContacts, contactIndex, *it, quickSearchMap, errCode);
    }

    if (*errCode == SUCCESS) {
        // allContacts holds deep copies; release the originals still owned by resultSetMap.
        releaseRresultSetMapBuckets(resultSetMap);
        return allContacts;
    }

    allContacts->freeContent();
    free(allContacts);
    releaseRresultSetMapBuckets(resultSetMap);

    return nullptr;
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
        std::vector<ValuesBucket>& contactData = GetResultMapValue(resultSetMap, contactIdValue);
        PutQuickSearchKey(resultSet, quickSearchMap, contactIdValue);
        addResultSetAsValuesBucket(contactData, resultSet, errCode);
        if (*errCode != SUCCESS) {
            HILOG_ERROR("ContactUtils::parseResultSetForContacts fail to mem alloc");
            break;
        }
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();

    if (*errCode != SUCCESS) {
        releaseRresultSetMapBuckets(resultSetMap);
        return nullptr;
    }
    return allocCollectedContacts(resultSetMap, quickSearchMap, errCode);
}

/**
 * @brief Converting resultset of a query to Group data ValuesBucket
 */
void resultSetAsGroup(ValuesBucket* groups, int idx, std::shared_ptr<DataShareResultSet> &resultSet, int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    // content_type for group is redundant
    PutResultValue(bucketData, "detail_info", resultSet, "id", errCode);
    PutResultValue(bucketData, "group_name", resultSet, "group_name", errCode);

    ValuesBucket bucket = allocBucketData(bucketData, errCode);
    if (*errCode != SUCCESS) {
        HILOG_ERROR("ContactUtils::resultSetAsGroup fail to mem alloc");
    } else {
        copyBucket(groups, idx, bucket, errCode);
        if (*errCode != SUCCESS) {
            // The caller sets bucketCount = count (excluding this slot) on failure, so its
            // freeContent sweep would skip idx; release the partial deep copy here to avoid
            // leaking the calloc'd key/value arrays of the failing slot.
            groups[idx].freeContent();
        }
    }
    bucket.freeContent();
}

/**
 * @brief Converting resultset of a query to Holder data ValuesBucket
 */
void resultSetAsHolder(ValuesBucket* holders, int idx, std::shared_ptr<DataShareResultSet> &resultSet,
                       int32_t *errCode)
{
    std::vector<KeyWithValueType> bucketData;
    // content_type for holder is redundant
    PutResultValue(bucketData, "detail_info", resultSet, "account_name", errCode);
    PutResultValue(bucketData, "custom_data", resultSet, "account_type", errCode);
    PutResultValue(bucketData, "extend7", resultSet, "id", errCode);

    ValuesBucket bucket = allocBucketData(bucketData, errCode);
    if (*errCode != SUCCESS) {
        HILOG_ERROR("ContactUtils::resultSetAsHolder fail to mem alloc");
    } else {
        copyBucket(holders, idx, bucket, errCode);
        if (*errCode != SUCCESS) {
            // The caller sets bucketCount = count (excluding this slot) on failure, so its
            // freeContent sweep would skip idx; release the partial deep copy here to avoid
            // leaking the calloc'd key/value arrays of the failing slot.
            holders[idx].freeContent();
        }
    }
    bucket.freeContent();
}

// Allocates a Buckets header plus a zero-initialised ValuesBucket array of totalBuckets slots.
// calloc zeroes both the header (so bucketCount=0/data=nullptr) and the array, making freeContent()
// safe on slots not filled by copyBucket. Returns nullptr and sets *errCode on failure.
// Shared by parseResultSetForGroups/Holders.
static Buckets* allocBucketsHeader(size_t totalBuckets, int32_t *errCode)
{
    if (errCode == nullptr) {
        HILOG_ERROR("ContactUtils::allocBucketsHeader errCode is nullptr");
        return nullptr;
    }
    if (totalBuckets == 0 || totalBuckets > MAX_GROUPS_HOLDERS) {
        HILOG_ERROR("ContactUtils::allocBucketsHeader invalid totalBuckets %{public}zu", totalBuckets);
        *errCode = ERROR;
        return nullptr;
    }
    Buckets* buckets = (Buckets*)calloc(1, sizeof(Buckets));
    if (buckets == nullptr) {
        HILOG_ERROR("ContactUtils::allocBucketsHeader fail to mem alloc");
        *errCode = ERROR;
        return nullptr;
    }
    buckets->data = (struct ValuesBucket*)calloc(totalBuckets, sizeof(struct ValuesBucket));
    if (buckets->data == nullptr) {
        free(buckets);
        HILOG_ERROR("ContactUtils::allocBucketsHeader fail to mem alloc");
        *errCode = ERROR;
        return nullptr;
    }
    return buckets;
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
    if (totalGroups <= 0) {
        return nullptr;
    }
    // bucketCount is updated to the actually processed count after the loop.
    GroupsData* allGroups = allocBucketsHeader(totalGroups, errCode);
    if (allGroups == nullptr) {
        return nullptr;
    }

    int resultSetNum = resultSet->GoToFirstRow();
    int count = 0;
    while (resultSetNum == 0) {
        if (count >= totalGroups) {
            HILOG_ERROR("ContactUtils::parseResultSetForGroups row count exceeded totalGroups");
            break;
        }
        resultSetAsGroup(allGroups->data, count, resultSet, errCode);
        if (*errCode != SUCCESS) {
            break;
        }
        resultSetNum = resultSet->GoToNextRow();
        count++;
    }
    resultSet->Close();
    allGroups->bucketCount = static_cast<uint64_t>(count);

    if (*errCode != SUCCESS) {
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
    if (totalHolders <= 0) {
        return nullptr;
    }
    // bucketCount is updated to the actually processed count after the loop.
    HoldersData* allHolders = allocBucketsHeader(totalHolders, errCode);
    if (allHolders == nullptr) {
        return nullptr;
    }

    int resultSetNum = resultSet->GoToFirstRow();
    int count = 0;
    while (resultSetNum == 0) {
        if (count >= totalHolders) {
            HILOG_ERROR("ContactUtils::parseResultSetForHolders row count exceeded totalHolders");
            break;
        }
        resultSetAsHolder(allHolders->data, count, resultSet, errCode);
        if (*errCode != SUCCESS) {
            break;
        }
        resultSetNum = resultSet->GoToNextRow();
        count++;
    }
    resultSet->Close();
    allHolders->bucketCount = static_cast<uint64_t>(count);

    if (*errCode != SUCCESS) {
        allHolders->freeContent();
        free(allHolders);
        allHolders = nullptr;
    }

    return allHolders;
}

} // namespace ContactsFfi
} // namespace OHOS
