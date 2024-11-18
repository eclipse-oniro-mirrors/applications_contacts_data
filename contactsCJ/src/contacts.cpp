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

#include "ability_runtime/cj_ability_context.h"
#include "cj_data_share_predicates_impl.h"
#include "contacts.h"
#include "contacts_api.h" // from contacts_data/contacts
#include "contacts_control.h" // from contacts_data/contacts
#include "contacts_napi_common.h" // from contacts_data/contacts
#include "contacts_telephony_permission.h" // from contacts_data/contacts
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "hilog_wrapper_api.h" // from contacts_data/contacts
#include "native/ffi_remote_data.h"

using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::ContactsFfi;
using namespace OHOS::DataShare;
using namespace OHOS::FFI;

namespace OHOS {
namespace ContactsFfi {

// works for contactId and holderId
bool isInvalidId(int id) {
    return id <= 0;
}

std::shared_ptr<DataShareHelper> dsHelperFromContext(int64_t contextId) {
    sptr<CJAbilityContext> context = FFIData::GetData<CJAbilityContext>(contextId);
    return DataShareHelper::Creator(context->GetToken(), ContactsApi::CONTACTS_DATA_URI);
}

int64_t Contacts::CJaddContact(int64_t contextId, DataShareValuesBucket rawContact,
                               std::vector<DataShareValuesBucket> contactData, int32_t *errCode)
{
    HILOG_INFO("CJaddContact data size = %{public}d", (int) contactData.size());

    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJaddContact Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return ContactsApi::Contacts::INVALID_CONTACT_ID;
    }

    ContactsApi::ContactsControl contactsControl;
    int rawId = contactsControl.RawContactInsert(dataShareHelper, rawContact);    

    unsigned int size = contactData.size();
    for (unsigned int i = 0; i < size; ++i) {
        contactData[i].Put("raw_contact_id", rawId);
    }

    int code = contactsControl.ContactDataInsert(dataShareHelper, contactData);

    dataShareHelper->Release();
    dataShareHelper = nullptr;

    *errCode = code;
    if (code == ContactsApi::SUCCESS) {
        HILOG_INFO("CJaddContact returns id = %{public}d", rawId);
        return rawId;
    } else {
        HILOG_ERROR("CJaddContact gets errCode = %{public}d and returns id = -1", code);
        return ContactsApi::Contacts::INVALID_CONTACT_ID;
    }
}

void Contacts::CJdeleteContact(int64_t contextId, int64_t predicatesId, int32_t *errCode)
{
    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJdeleteContact Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return;
    }

    std::shared_ptr<DataSharePredicates> predicates = FFIData::GetData<DataSharePredicatesImpl>(predicatesId)->GetPredicates();

    ContactsApi::ContactsControl contactsControl;
    int code = contactsControl.ContactDelete(dataShareHelper, *predicates);

    dataShareHelper->Release();
    dataShareHelper = nullptr;

    *errCode = code;
}

DataSharePredicates buildQueryDataPredicates(int64_t contactId)
{
    DataSharePredicates predicates;
    if (contactId != 0) {
        std::vector<std::string> fields; fields.push_back("raw_contact_id");
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", std::to_string(contactId));
        predicates.GroupBy(fields);
    }
    return predicates;
}

void Contacts::CJupdateContact(int64_t contextId, int64_t contactId, std::vector<DataShareValuesBucket> contactData,
                               int64_t predicatesId, int32_t *errCode)
{
    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJupdateContact Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return;
    }
    if (isInvalidId(contactId)) {
        HILOG_ERROR("CJupdateContact Parameter invalid! %{public}ld", contactId);
        *errCode = ContactsApi::PARAMETER_ERROR;
        return;
    }

    std::shared_ptr<DataSharePredicates> deletePredicates = FFIData::GetData<DataSharePredicatesImpl>(predicatesId)->GetPredicates();
    ContactsApi::ContactsControl contactsControl;

    // query raw_contact_id
    std::vector<std::string> queryDataColumns; queryDataColumns.push_back("raw_contact_id");
    DataSharePredicates queryDataPredicates = buildQueryDataPredicates(contactId);

    std::shared_ptr<DataShareResultSet> resultSet = contactsControl.ContactQuery(dataShareHelper, queryDataColumns, queryDataPredicates);
    int rawId = ContactsApi::GetRawIdByResultSet(resultSet);
    if (rawId <= 0) {
        HILOG_ERROR("CJupdateContact contact rawId equals %{public}d", rawId);
        *errCode = ContactsApi::ERROR;

        dataShareHelper->Release();
        dataShareHelper = nullptr;

        return;
    }

    unsigned int size = contactData.size();
    for (unsigned int i = 0; i < size; ++i) {
        contactData[i].Put("raw_contact_id", rawId);
    }

    deletePredicates->And();
    deletePredicates->EqualTo("raw_contact_id", std::to_string(rawId));

    int resultCode = contactsControl.ContactDataDelete(dataShareHelper, *(deletePredicates));
    if (resultCode >= 0) {
        resultCode = contactsControl.ContactDataInsert(dataShareHelper, contactData);
    }

    dataShareHelper->Release();
    dataShareHelper = nullptr;

    *errCode = resultCode;
}
// It is similar to code in contacts/src/contacts_api.cpp for js, consider reusing.
DataSharePredicates buildIsLocalContactPredicates(int64_t id)
{
    DataShare::DataSharePredicates predicates;
    if (id != 0) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", std::to_string(id));
        predicates.And();
        predicates.EqualTo("account_type", "com.ohos.contacts");
        predicates.And();
        predicates.EqualTo("account_name", "phone");
    }
    return predicates;
}

bool Contacts::CJisLocalContact(int64_t contextId, int64_t contactId, int32_t *errCode)
{
    ContactsApi::ContactsTelephonyPermission permission;
    if (!permission.CheckPermission(ContactsApi::Permission::READ_CONTACTS)) {
        HILOG_ERROR("CJisLocalContact Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return false;
    }
    if (isInvalidId(contactId)) {
        HILOG_ERROR("CJisLocalContact Parameter invalid! %{public}ld", contactId);
        *errCode = ContactsApi::PARAMETER_ERROR;
        return false;
    }

    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJisLocalContact Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return false;
    }

    ContactsApi::ContactsControl contactsControl;

    DataSharePredicates predicates = buildIsLocalContactPredicates(contactId);
    std::vector<std::string> columns;
    std::shared_ptr<DataShareResultSet> resultSet = contactsControl.ContactQuery(dataShareHelper, columns, predicates);
    if (resultSet == nullptr) {
        return false;
    }
    int resultSetNum = resultSet->GoToFirstRow();
    bool isLocal = resultSetNum == OHOS::NativeRdb::E_OK;

    resultSet->Close();

    dataShareHelper->Release();
    dataShareHelper = nullptr;

    return isLocal;
}

// It is similar to code in contacts/src/contacts_api.cpp for js, consider reusing.
DataSharePredicates buildIsMyCardPredicates(int64_t id)
{
    DataShare::DataSharePredicates predicates;
    if (id != 0) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", std::to_string(id));
    }
    return predicates;
}

bool Contacts::CJisMyCard(int64_t contextId, int64_t contactId, int32_t *errCode)
{
    ContactsApi::ContactsTelephonyPermission permission;
    if (!permission.CheckPermission(ContactsApi::Permission::READ_CONTACTS)) {
        HILOG_ERROR("CJisMyCard Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return false;
    }
    if (isInvalidId(contactId)) {
        HILOG_ERROR("CJisMyCard Parameter invalid! %{public}ld", contactId);
        *errCode = ContactsApi::PARAMETER_ERROR;
        return false;
    }

    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJisMyCard Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return false;
    }

    ContactsApi::ContactsControl contactsControl;

    DataSharePredicates predicates = buildIsMyCardPredicates(contactId);
    std::vector<std::string> columns;
    std::shared_ptr<DataShareResultSet> resultSet = contactsControl.MyCardQuery(dataShareHelper, columns, predicates);
    if (resultSet == nullptr) {
        dataShareHelper->Release();
        dataShareHelper = nullptr;

        return false;
    }
    int resultSetNum = resultSet->GoToFirstRow();
    bool isMyCard = resultSetNum == OHOS::NativeRdb::E_OK;

    resultSet->Close();
    dataShareHelper->Release();
    dataShareHelper = nullptr;

    return isMyCard;
}

ContactsData* Contacts::CJqueryMyCard(int64_t contextId, int64_t predicatesId, int32_t *errCode)
{
    ContactsApi::ContactsTelephonyPermission permission;
    if (!permission.CheckPermission(ContactsApi::Permission::READ_CONTACTS)) {
        HILOG_ERROR("CJqueryMyCard Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return nullptr;
    }

    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJqueryMyCard Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return nullptr;
    }

    std::shared_ptr<DataSharePredicates> predicates = FFIData::GetData<DataSharePredicatesImpl>(predicatesId)->GetPredicates();
    ContactsApi::ContactsControl contactsControl;

    std::vector<std::string> columns;
    std::shared_ptr<DataShareResultSet> resultSet = contactsControl.MyCardQuery(dataShareHelper, columns, *predicates);
    ContactsData* contacts = parseResultSetForContacts(resultSet, errCode); // resultSet is closed inside

    dataShareHelper->Release();
    dataShareHelper = nullptr;

    return contacts;
}

GroupsData* Contacts::CJqueryGroups(int64_t contextId, int64_t predicatesId, int32_t *errCode)
{
    ContactsApi::ContactsTelephonyPermission permission;
    if (!permission.CheckPermission(ContactsApi::Permission::READ_CONTACTS)) {
        HILOG_ERROR("CJqueryGroups Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return nullptr;
    }

    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJqueryGroups Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return nullptr;
    }

    std::shared_ptr<DataSharePredicates> predicates = FFIData::GetData<DataSharePredicatesImpl>(predicatesId)->GetPredicates();
    ContactsApi::ContactsControl contactsControl;

    std::vector<std::string> columns;
    std::shared_ptr<DataShareResultSet> resultSet = contactsControl.GroupsQuery(dataShareHelper, columns, *predicates);
    GroupsData* groups = parseResultSetForGroups(resultSet, errCode); // resultSet is closed inside

    dataShareHelper->Release();
    dataShareHelper = nullptr;

    return groups;
}

HoldersData* Contacts::CJqueryHolders(int64_t contextId, int32_t *errCode)
{
    ContactsApi::ContactsTelephonyPermission permission;
    if (!permission.CheckPermission(ContactsApi::Permission::READ_CONTACTS)) {
        HILOG_ERROR("CJqueryHolders Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return nullptr;
    }

    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJqueryHolders Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return nullptr;
    }

    ContactsApi::ContactsControl contactsControl;

    DataSharePredicates predicates;
    std::vector<std::string> columns;
    std::shared_ptr<DataShareResultSet> resultSet = contactsControl.HolderQuery(dataShareHelper, columns, predicates);
    HoldersData* holders = parseResultSetForHolders(resultSet, errCode); // resultSet is closed inside

    dataShareHelper->Release();
    dataShareHelper = nullptr;

    return holders;
}

ContactsData* Contacts::CJqueryContacts(int64_t contextId, int64_t predicatesId, int32_t *errCode)
{
    ContactsApi::ContactsTelephonyPermission permission;
    if (!permission.CheckPermission(ContactsApi::Permission::READ_CONTACTS)) {
        HILOG_ERROR("CJqueryContacts Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return nullptr;
    }

    std::shared_ptr<DataShareHelper> dataShareHelper = dsHelperFromContext(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJqueryContacts Permission denied!");
        *errCode = ContactsApi::PERMISSION_ERROR;
        return nullptr;
    }

    std::shared_ptr<DataSharePredicates> predicates = FFIData::GetData<DataSharePredicatesImpl>(predicatesId)->GetPredicates();
    ContactsApi::ContactsControl contactsControl;

    std::vector<std::string> columns;
    std::shared_ptr<DataShareResultSet> resultSet = contactsControl.ContactQuery(dataShareHelper, columns, *predicates);
    ContactsData* contacts = parseResultSetForContacts(resultSet, errCode); // resultSet is closed inside

    dataShareHelper->Release();
    dataShareHelper = nullptr;

    return contacts;
}

} // namespace ContactsFfi
} // namespace OHOS
