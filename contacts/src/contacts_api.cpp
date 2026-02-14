/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "contacts_api.h"

#include <mutex>

#include "datashare_predicates.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"
#include "securec.h"

#include "contacts_napi_common.h"
#include "contacts_napi_utils.h"
#include "hilog_wrapper_api.h"
#include "result_convert.h"
#include "contacts_telephony_permission.h"
#include "ability.h"
#include "ability_context.h"
#include "context.h"
#include "ui_content.h"
#include "ui_extension_context.h"
#include "napi_common_want.h"
#include "modal_ui_extension_config.h"
#include "window.h"
#include "want.h"
#include "phone_number_format.h"
#include "locale_config.h"
#include "image_packer.h"
#include "file_uri.h"
#include "kit_pixel_map_util.h"

using i18n::phonenumbers::PhoneNumberUtil;

namespace OHOS {
namespace ContactsApi {
namespace {
std::mutex g_mutex;
}

static const std::string PHONE_NUMBER_PREFIX = "106";
static const int OPEN_FILE_FAILED = -1;
static const int ERR_OK = 0;
constexpr int INVALID_CONTACT_ID = -1;
const std::vector<int> needGroupVec = {
    QUERY_CONTACT,
    QUERY_CONTACTS,
    QUERY_CONTACTS_BY_EMAIL,
    QUERY_CONTACTS_BY_PHONE_NUMBER,
    QUERY_MY_CARD
};

/**
 * @brief Initialize NAPI object
 *
 * @param env Conditions for initialize operation
 * @param object Conditions for initialize operation
 * @param hold Attribute of object
 * @param attr Attribute of object
 * @param contact Attribute of object
 */
void ObjectInit(napi_env env, napi_value object, napi_value &hold, napi_value &attr, napi_value &contact)
{
    int type = GetType(env, object);
    switch (type) {
        case TYPE_HOLDER:
            hold = object;
            break;
        case TYPE_ATTR:
            attr = object;
            break;
        case TYPE_CONTACT:
            contact = object;
            break;
        default:
            break;
    }
}

/**
 * @brief Initialize NAPI number object
 *
 * @param env Conditions for initialize operation
 * @param object Conditions for initialize operation
 * @param id Number object
 */
void ObjectInitId(napi_env env, napi_value object, napi_value &id)
{
    int type = GetType(env, object);
    switch (type) {
        case TYPE_NAPI_NUMBER:
            id = object;
            break;
        default:
            break;
    }
}

/**
 * @brief Initialize NAPI string object
 *
 * @param env Conditions for initialize operation
 * @param object Conditions for initialize operation
 * @param key String object
 */
void ObjectInitString(napi_env env, napi_value object, napi_value &key)
{
    int type = GetType(env, object);
    switch (type) {
        case TYPE_NAPI_STRING:
            key = object;
            break;
        default:
            break;
    }
}

/**
 * @brief Get NAPI object type
 *
 * @param env Conditions for get type operation
 * @param value Conditions for get type operation
 *
 * @return The result returned by get type operation
 */
int GetType(napi_env env, napi_value value)
{
    napi_valuetype valueType;
    napi_typeof(env, value, &valueType);
    bool result = false;
    switch (valueType) {
        case napi_number:
            return TYPE_NAPI_NUMBER;
            break;
        case napi_string:
            return TYPE_NAPI_STRING;
            break;
        case napi_object:
            napi_value key;
            napi_create_string_utf8(env, "bundleName", NAPI_AUTO_LENGTH, &key);
            napi_has_property(env, value, key, &result);
            if (result) {
                return TYPE_HOLDER;
            }
            napi_create_string_utf8(env, "attributes", NAPI_AUTO_LENGTH, &key);
            napi_has_property(env, value, key, &result);
            if (result) {
                return TYPE_ATTR;
            }
            return TYPE_CONTACT;
            break;
        default:
            return TYPE_NAPI_ERROR;
            break;
    }
}

/**
 * @brief Get dataShareHelper
 *
 * @param env Conditions for get dataShareHelper operation
 *
 * @return The result returned by get dataShareHelper
 */
bool GetDataShareHelper(napi_env env, napi_callback_info info, ExecuteHelper *executeHelper)
{
    napi_value global;
    bool isStageMode = false;
    napi_value abilityContext = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (executeHelper->abilityContext != nullptr) {
        isStageMode = true;
        status = napi_ok;
        abilityContext = executeHelper->abilityContext;
    } else {
        if (status != napi_ok) {
            HILOG_ERROR("GetDataShareHelper napi_get_global != napi_ok");
        }
        napi_value globalThis;
        status = napi_get_named_property(env, global, "globalThis", &globalThis);
        if (status != napi_ok) {
            HILOG_ERROR("GetDataShareHelper napi_get_globalThis != napi_ok");
        }
        status = napi_get_named_property(env, globalThis, "abilityContext", &abilityContext);
        if (status != napi_ok) {
            HILOG_ERROR("GetDataShareHelper napi_get_abilityContext != napi_ok");
        }
        status = OHOS::AbilityRuntime::IsStageContext(env, abilityContext, isStageMode);
    }

    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = nullptr;
    if (status != napi_ok || !isStageMode) {
        HILOG_INFO("GetFAModeContext");
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            HILOG_ERROR("Failed to get native ability instance");
            return false;
        }
        auto context = ability->GetContext();
        if (context == nullptr) {
            HILOG_ERROR("Failed to get native context instance");
            return false;
        }
        executeHelper->dataShareHelper = DataShare::DataShareHelper::Creator(context->GetToken(), CONTACTS_DATA_URI);
    } else {
        HILOG_INFO("GetStageModeContext");
        auto context = OHOS::AbilityRuntime::GetStageModeContext(env, abilityContext);
        if (context == nullptr) {
            HILOG_ERROR("Failed to get native stage context instance");
            return false;
        }
        executeHelper->dataShareHelper = DataShare::DataShareHelper::Creator(context->GetToken(), CONTACTS_DATA_URI);
    }
    return false;
}

/**
 * @brief Establish predicates condition by holder object
 *
 * @param holder Conditions for establish predicates operation
 * @param predicates Conditions for establish predicates operation
 */
void HolderPredicates(Holder &holder, DataShare::DataSharePredicates &predicates)
{
    if (!holder.bundleName.empty()) {
        predicates.And();
        predicates.EqualTo("account_type", holder.bundleName);
    }
    if (!holder.displayName.empty()) {
        predicates.And();
        predicates.EqualTo("account_name", holder.displayName);
    }
    predicates.And();
    predicates.EqualTo("account_id", std::to_string(holder.holderId));
}

/**
 * @brief Establish predicates condition by attributes object
 *
 * @param attrs Conditions for establish predicates operation
 * @param predicates Conditions for establish predicates operation
 */
void AttributesPredicates(bool isBegin, ContactAttributes &attrs, DataShare::DataSharePredicates &predicates)
{
    unsigned int size = attrs.attributes.size();
    if (size > 0) {
        if (!isBegin) {
            predicates.And();
        }
        predicates.BeginWrap();
    }
    for (unsigned int i = 0; i < size; ++i) {
        predicates.EqualTo("type_id", std::to_string(attrs.attributes[i]));
        if (i != size - 1) {
            predicates.Or();
        }
    }
    if (size > 0) {
        predicates.EndWrap();
    }
}

void CheckAttributes(ContactAttributes &attrs)
{
    unsigned int size = attrs.attributes.size();
    if (size == 0) {
        HILOG_INFO("attributes not exist, it means all attribute");
        attrs.attributes.push_back(EMAIL);
        attrs.attributes.push_back(IM);
        attrs.attributes.push_back(NICKNAME);
        attrs.attributes.push_back(ORGANIZATION);
        attrs.attributes.push_back(PHONE);
        attrs.attributes.push_back(NAME);
        attrs.attributes.push_back(POSTAL_ADDRESS);
        attrs.attributes.push_back(PHOTO);
        attrs.attributes.push_back(GROUP_MEMBERSHIP);
        attrs.attributes.push_back(NOTE);
        attrs.attributes.push_back(CONTACT_EVENT);
        attrs.attributes.push_back(WEBSITE);
        attrs.attributes.push_back(RELATION);
        attrs.attributes.push_back(SIP_ADDRESS);
    }
}

/**
 * @brief Resolve object interface in DELETE_CONTACT case
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildDeleteContactPredicates(napi_env env, ExecuteHelper *executeHelper)
{
    DataShare::DataSharePredicates predicates;
    ContactsBuild contactsBuild;
    std::string keyValue = contactsBuild.NapiGetValueString(env, executeHelper->argv[0]);
    if (!keyValue.empty()) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("quick_search_key", keyValue);
    } else {
        HILOG_ERROR("BuildDeleteContactPredicates error");
        executeHelper->resultData = RDB_PARAMETER_ERROR;
    }
    return predicates;
}

/**
 * @brief Resolve object interface in QUERY_CONTACT case
 *
 * @param env Conditions for resolve object interface operation
 * @param key Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 * @param attr Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildQueryContactPredicates(
    napi_env env, napi_value key, napi_value hold, napi_value attr)
{
    ContactsBuild contactsBuild;
    std::string keyValue = contactsBuild.NapiGetValueString(env, key);
    Holder holder = contactsBuild.GetHolder(env, hold);
    DataShare::DataSharePredicates predicates;
    if (!keyValue.empty()) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("quick_search_key", keyValue);
        if (hold != nullptr) {
            HolderPredicates(holder, predicates);
        }
    }
    return predicates;
}

void HoldersStructure(std::map<std::string, std::string> &holders, Holder &holder)
{
    if (!holder.bundleName.empty()) {
        holders["account_type"] = holder.bundleName;
    }
    if (!holder.displayName.empty()) {
        holders["account_name"] = holder.displayName;
    }
    holders["account_id"] = std::to_string(holder.holderId);
}

/**
 * @brief Resolve object interface in QUERY_CONTACTS_COUNT case
 *
 * @param env Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 * @param attr Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildQueryContactCountPredicates()
{
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("is_deleted", "0");
    return predicates;
}

/**
 * @brief Resolve object interface in QUERY_CONTACTS case
 *
 * @param env Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 * @param attr Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildQueryContactsPredicates(napi_env env, napi_value hold, napi_value attr)
{
    ContactsBuild contactsBuild;
    Holder holder = contactsBuild.GetHolder(env, hold);
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    DataShare::DataSharePredicates predicates;
    std::map<std::string, std::string> holders;
    if (hold != nullptr) {
        HoldersStructure(holders, holder);
    }
    unsigned int size = attrs.attributes.size();
    unsigned int mapSize = holders.size();
    std::map<std::string, std::string>::iterator it;
    for (it = holders.begin(); it != holders.end(); ++it) {
        predicates.EqualTo(it->first, it->second);
        if (it != --holders.end()) {
            predicates.And();
        }
    }
    if (mapSize > 0) {
        predicates.And();
    }
    if (size > 0) {
        predicates.BeginWrap();
    }
    for (unsigned int i = 0; i < size; ++i) {
        predicates.EqualTo("type_id", std::to_string(attrs.attributes[i]));
        if (i != size - 1) {
            predicates.Or();
        }
    }
    if (size > 0) {
        predicates.EndWrap();
    }
    if (size > 0 || mapSize > 0) {
        predicates.And();
    }
    predicates.EqualTo("is_deleted", "0");
    return predicates;
}

/**
 * @brief Resolve object interface in QUERY_CONTACTS_BY_EMAIL case
 *
 * @param env Conditions for resolve object interface operation
 * @param emailobject Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 * @param attr Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildQueryContactsByEmailPredicates(
    napi_env env, napi_value emailobject, napi_value hold, napi_value attr)
{
    ContactsBuild contactsBuild;
    std::string email = contactsBuild.NapiGetValueString(env, emailobject);
    Holder holder = contactsBuild.GetHolder(env, hold);
    DataShare::DataSharePredicates predicates;
    if (!email.empty() || email != "") {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("detail_info", email);
        predicates.And();
        predicates.EqualTo("content_type", "email");
        if (hold != nullptr) {
            HolderPredicates(holder, predicates);
        }
    }
    return predicates;
}

/**
 * @brief Resolve object interface in QUERY_CONTACTS_BY_PHONE_NUMBER case
 *
 * @param env Conditions for resolve object interface operation
 * @param number Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 * @param attr Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildQueryContactsByPhoneNumberPredicates(
    napi_env env, napi_value number, napi_value hold, napi_value attr)
{
    ContactsBuild contactsBuild;
    std::string phoneNumber = contactsBuild.NapiGetValueString(env, number);
    std::string formatPhoneNumber = GetE164FormatPhoneNumber(phoneNumber);
    Holder holder = contactsBuild.GetHolder(env, hold);
    DataShare::DataSharePredicates predicates;
    if (!phoneNumber.empty() || phoneNumber != "") {
        predicates.EqualTo("is_deleted", "0");
        predicates.BeginWrap();
        predicates.EqualTo("detail_info", phoneNumber);
        predicates.Or();
        predicates.EqualTo("format_phone_number", formatPhoneNumber);
        predicates.EndWrap();
        predicates.And();
        predicates.EqualTo("content_type", "phone");
        if (hold != nullptr) {
            HolderPredicates(holder, predicates);
        }
    }
    return predicates;
}

std::string GetE164FormatPhoneNumber(std::string &phoneNumber)
{
    std::string result;
    std::map<std::string, std::string> options = {{"type", "E164"}};
    std::string countryCode = GetCountryCode();
    if (countryCode.empty()) {
        return phoneNumber;
    }
    Global::I18n::PhoneNumberFormat *phoneNumberFormat = new Global::I18n::PhoneNumberFormat(countryCode, options);
    bool isValidPhoneNumber = phoneNumberFormat->isValidPhoneNumber(phoneNumber);
    if (isValidPhoneNumber) {
        std::string formatPhoneNumber = FormatPhoneNumber(phoneNumber, countryCode);
        if (!formatPhoneNumber.empty()) {
            result = formatPhoneNumber;
        } else {
            HILOG_INFO("ContactsDataBase GetE164FormatPhoneNumber formatPhoneNumber is empty");
        }
    } else {
        HILOG_INFO("ContactsDataBase GetE164FormatPhoneNumber isValidPhoneNumber false");
    }
    delete phoneNumberFormat;
    return result;
}

std::string FormatPhoneNumber(const std::string &number, const std::string &country)
{
    PhoneNumberUtil *util = PhoneNumberUtil::GetInstance();
    if (util == nullptr) {
        HILOG_ERROR("FormatPhoneNumber: util is nullptr.");
        return "";
    }

    std::string formattedNumber;
    i18n::phonenumbers::PhoneNumber phoneNumber;
    PhoneNumberUtil::ErrorType type = util->ParseAndKeepRawInput(number, country, &phoneNumber);
    if (type != PhoneNumberUtil::ErrorType::NO_PARSING_ERROR) {
        return "";
    }
    if (number.compare(0, PHONE_NUMBER_PREFIX.length(), PHONE_NUMBER_PREFIX) == 0) {
        util->FormatInOriginalFormat(phoneNumber, country, &formattedNumber);
    } else {
        util->Format(phoneNumber, PhoneNumberUtil::PhoneNumberFormat::E164, &formattedNumber);
    }
    return formattedNumber;
}

std::string GetCountryCode()
{
    std::string countryCode;
#ifdef CELLULAR_DATA_SUPPORT
    int slotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    HILOG_INFO("ContactsDataBase GetCountryCode slotIds is %{public}d, ts = %{public}lld", slotId, (long long) time(NULL));
    sptr<Telephony::NetworkState> networkClient = nullptr;
    DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetNetworkState(slotId, networkClient);
    if (networkClient != nullptr && networkClient->IsRoaming()) {
        HILOG_INFO("ContactsDataBase GetCountryCode networkState is nullptr");
        HILOG_INFO("ContactsDataBase GetCountryCode Roaming is %{public}d", networkClient->IsRoaming());
        return countryCode;
    }
    std::u16string countryCodeForNetwork;
    DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetIsoCountryCodeForNetwork(
        slotId, countryCodeForNetwork);
    countryCode = Str16ToStr8(countryCodeForNetwork);
    if (countryCode.empty()) {
        std::u16string countryCodeForSim;
        DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetISOCountryCodeForSim(
            slotId, countryCodeForSim);
        countryCode = Str16ToStr8(countryCodeForSim);
    }
#endif
    if (countryCode.empty()) {
        countryCode = Global::I18n::LocaleConfig::GetSystemRegion();
    }
    transform(countryCode.begin(), countryCode.end(), countryCode.begin(), toupper);
    HILOG_INFO("ContactsDataBase GetCountryCode region is %{public}s",
        countryCode.c_str());
    return countryCode;
}

/**
 * @brief Resolve object interface in QUERY_GROUPS case
 *
 * @param env Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildQueryGroupsPredicates(napi_env env, napi_value hold)
{
    ContactsBuild contactsBuild;
    Holder holder = contactsBuild.GetHolder(env, hold);
    DataShare::DataSharePredicates predicates;
    std::map<std::string, std::string> holders;
    if (hold != nullptr) {
        HoldersStructure(holders, holder);
    }
    predicates.EqualTo("is_deleted", "0");
    unsigned int size = holders.size();
    if (size > 0) {
        predicates.And();
    }
    std::map<std::string, std::string>::iterator it;
    for (it = holders.begin(); it != holders.end(); ++it) {
        predicates.EqualTo(it->first, it->second);
        if (it != --holders.end()) {
            predicates.And();
        }
    }
    return predicates;
}

/**
 * @brief Resolve object interface in QUERY_KEY case
 *
 * @param env Conditions for resolve object interface operation
 * @param id Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildQueryKeyPredicates(napi_env env, napi_value id, napi_value hold)
{
    ContactsBuild contactsBuild;
    std::string value = contactsBuild.GetContactIdStr(env, id);
    Holder holder = contactsBuild.GetHolder(env, hold);
    DataShare::DataSharePredicates predicates;
    if (value != "0") {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", value);
        if (hold != nullptr) {
            HolderPredicates(holder, predicates);
        }
    }
    return predicates;
}

/**
 * @brief Resolve object interface in QUERY_MY_CARD case
 *
 * @param env Conditions for resolve object interface operation
 * @param attr Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildQueryMyCardPredicates(napi_env env, napi_value attr)
{
    ContactsBuild contactsBuild;
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    DataShare::DataSharePredicates predicates;
    unsigned int size = attrs.attributes.size();
    predicates.EqualTo("is_deleted", "0");
    if (size > 0) {
        predicates.And();
        predicates.BeginWrap();
    }
    for (unsigned int i = 0; i < size; ++i) {
        predicates.EqualTo("type_id", std::to_string(attrs.attributes[i]));
        if (i != size - 1) {
            predicates.Or();
        }
    }
    if (size > 0) {
        predicates.EndWrap();
    }
    return predicates;
}

DataShare::DataSharePredicates BuildQueryContactData(napi_env env, napi_value &contactObject, napi_value &attrObject,
    ExecuteHelper *executeHelper)
{
    ContactsBuild contactsBuild;
    Contacts contact;
    contactsBuild.GetContactDataByObject(env, contactObject, contact);
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attrObject);
    CheckAttributes(attrs);
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> fields;
    fields.push_back("raw_contact_id");
    if (contact.id != 0) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", std::to_string(contact.id));
        predicates.GroupBy(fields);
    }
    unsigned int size = attrs.attributes.size();
    for (unsigned int i = 0; i < size; i++) {
        contactsBuild.BuildValueContactDataByType(contact, attrs.attributes[i], executeHelper);
    }
    return predicates;
}

std::vector<std::string> BuildUpdateContactColumns()
{
    std::vector<std::string> columns;
    columns.push_back("raw_contact_id");
    return columns;
}

int GetRawIdByResultSet(const std::shared_ptr<DataShare::DataShareResultSet> &resultSet)
{
    if (resultSet == nullptr) {
        return -1;
    }
    int resultSetNum = resultSet->GoToFirstRow();
    int intValue = 0;
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        resultSet->GetInt(0, intValue);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return intValue;
}

DataShare::DataSharePredicates BuildUpdateContactConvertParams(napi_env env, napi_value &contact,
    napi_value &attr, ExecuteHelper *executeHelper)
{
    executeHelper->valueContactData.clear();
    DataShare::DataSharePredicates predicates =
        BuildQueryContactData(env, contact, attr, executeHelper);
    executeHelper->columns = BuildUpdateContactColumns();
    executeHelper->deletePredicates = BuildDeleteContactDataPredicates(env, attr);
    return predicates;
}

DataShare::DataSharePredicates BuildDeleteContactDataPredicates(napi_env env, napi_value attr)
{
    ContactsBuild contactsBuild;
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    CheckAttributes(attrs);
    DataShare::DataSharePredicates predicates;
    AttributesPredicates(true, attrs, predicates);
    return predicates;
}

/**
 * @brief Resolve object interface in IS_LOCAL_CONTACT case
 *
 * @param env Conditions for resolve object interface operation
 * @param id Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildIsLocalContactPredicates(napi_env env, napi_value id)
{
    ContactsBuild contactsBuild;
    DataShare::DataSharePredicates predicates;
    std::string value = contactsBuild.GetContactIdStr(env, id);
    if (value != "0") {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", value);
        predicates.And();
        predicates.EqualTo("account_type", "com.ohos.contacts");
        predicates.And();
        predicates.EqualTo("account_name", "phone");
    }
    return predicates;
}

/**
 * @brief Resolve object interface in IS_MY_CARD case
 *
 * @param env Conditions for resolve object interface operation
 * @param id Conditions for resolve object interface operation
 */
DataShare::DataSharePredicates BuildIsMyCardPredicates(napi_env env, napi_value id)
{
    ContactsBuild contactsBuild;
    int value = contactsBuild.GetInt(env, id);
    DataShare::DataSharePredicates predicates;
    if (value != 0) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", std::to_string(value));
    }
    return predicates;
}

void ExecuteDone(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("contactApi ExecuteDone start===>");
    ExecuteHelper *executeHelper = reinterpret_cast<ExecuteHelper *>(data);
    HILOG_INFO("ExecuteDone workName: %{public}d", executeHelper->actionCode);
    napi_value result = nullptr;
    napi_deferred deferred = executeHelper->deferred;
    // 处理结果
    HandleExecuteResult(env, executeHelper, result);
    if (executeHelper->abilityContext != nullptr) {
        HILOG_INFO("executeHelper->abilityContext != nullptr");
        // 如果有错误，根据错误信息return，否则根据result返回
        napi_value errorCode = nullptr;
        HandleExecuteErrorCode(env, executeHelper, errorCode);
        if (errorCode != nullptr) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, deferred, errorCode));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, deferred, result));
        }
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, deferred, result));
    }
    executeHelper->deferred = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, executeHelper->work));
    if (executeHelper->dataShareHelper != nullptr) {
        executeHelper->dataShareHelper->Release();
        executeHelper->dataShareHelper = nullptr;
    }
    delete executeHelper;
    executeHelper = nullptr;
}

void ExecuteSyncDone(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("contactApi ExecuteSyncDone start===>");
    if (status != napi_ok) {
        HILOG_ERROR("ExecuteSyncDone status is not ok===>");
        return;
    }
    if (data != nullptr) {
        ExecuteHelper *executeHelper = reinterpret_cast<ExecuteHelper *>(data);
        HILOG_INFO("ExecuteSyncDone workName: %{public}d", executeHelper->actionCode);
        napi_value global;
        napi_get_global(env, &global);
        // resultData[0] 为错误返回信息
        // resultData[1] 为正常返回信息
        napi_value resultData[RESULT_DATA_SIZE];
        // stage模型处理
        if (executeHelper->abilityContext != nullptr) {
            HandleExecuteErrorCode(env, executeHelper, resultData[0]);
            HandleExecuteResult(env, executeHelper, resultData[1]);
        } else {
            if (executeHelper->resultData < 0) {
                // FA 模型会调用到这里，怀疑这里需要改为处理错误信息；但是FA模型较老，暂不修改
                HandleExecuteErrorCode(env, executeHelper, resultData[0]);
                HandleExecuteResult(env, executeHelper, resultData[1]);
            } else {
                napi_get_undefined(env, &resultData[0]);
                // 处理执行结果
                HandleExecuteResult(env, executeHelper, resultData[1]);
            }
        }
        napi_value result;
        napi_value callBack;
        napi_get_reference_value(env, executeHelper->callBack, &callBack);
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, callBack, &valuetype);
        if (valuetype != napi_function) {
            HILOG_ERROR("contactApi params not is function");
            return;
        }
        napi_call_function(env, global, callBack, RESULT_DATA_SIZE, resultData, &result);
        if (executeHelper->work != nullptr) {
            napi_delete_async_work(env, executeHelper->work);
        }
        executeHelper->work = nullptr;
        executeHelper->deferred = nullptr;
        if (executeHelper->valueUpdateContact.capacity() != 0) {
            std::vector<DataShare::DataShareValuesBucket>().swap(executeHelper->valueUpdateContact);
        }
        if (executeHelper->valueContact.capacity() != 0) {
            std::vector<DataShare::DataShareValuesBucket>().swap(executeHelper->valueUpdateContact);
        }
        if (executeHelper->valueContactData.capacity() != 0) {
            std::vector<DataShare::DataShareValuesBucket>().swap(executeHelper->valueUpdateContact);
        }
        if (executeHelper->dataShareHelper != nullptr) {
            executeHelper->dataShareHelper->Release();
            executeHelper->dataShareHelper = nullptr;
        }
        delete executeHelper;
        executeHelper = nullptr;
    }
    HILOG_INFO("contactApi ExecuteSyncDone done===>");
}

void HandleExecuteErrorCode(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    HILOG_INFO("HandleExecuteErrorCode, actionCode = %{public}d", executeHelper->actionCode);
    ResultConvert resultConvert;
    switch (executeHelper->actionCode) {
        case ADD_CONTACT:
        case DELETE_CONTACT:
        case UPDATE_CONTACT:
        case SELECT_CONTACT:
        case IS_LOCAL_CONTACT:
        case IS_MY_CARD:
        case QUERY_CONTACT:
        case QUERY_MY_CARD:
        case QUERY_KEY:
        case QUERY_CONTACTS:
        case QUERY_CONTACTS_BY_EMAIL:
        case QUERY_CONTACTS_BY_PHONE_NUMBER:
        case QUERY_GROUPS:
        case QUERY_HOLDERS:
        case QUERY_CONTACT_COUNT:
            // 参数错误
            HILOG_INFO("HandleExecuteErrorCode resultData");
            if (executeHelper->resultData == RDB_PARAMETER_ERROR || executeHelper->resultData == ERROR) {
                HILOG_ERROR("handleExecuteErrorCode handle param error: %{public}d", executeHelper->resultData);
                result = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
            } else if (executeHelper->resultData == VERIFICATION_PARAMETER_ERROR) {
                HILOG_ERROR("parameter verification failed");
                if (executeHelper->actionCode == ADD_CONTACTS) {
                    result = ContactsNapiUtils::CreateErrorByVerification(env, INVALID_PARAMETER);
                } else {
                    result = ContactsNapiUtils::CreateErrorByVerification(env, PARAMETER_ERROR);
                }
            } else if (executeHelper->resultData == RDB_PERMISSION_ERROR) {
                HILOG_ERROR("permission error");
                result = ContactsNapiUtils::CreateError(env, PERMISSION_ERROR);
            }
            break;
        case ADD_CONTACTS: {
            HandleAddContactsErrorCode(env, executeHelper, result);
            break;
        }
        default:
            break;
    }
}

void HandleExecuteResult(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    ResultConvert resultConvert;
    napi_value results = nullptr;
    HILOG_INFO("HandleExecuteResult, actionCode = %{public}d", executeHelper->actionCode);
    switch (executeHelper->actionCode) {
        case ADD_CONTACT:
        case DELETE_CONTACT:
        case UPDATE_CONTACT:
        case SELECT_CONTACT:
            HandleSelectContactResult(env, executeHelper, result);
            break;
        case ADD_CONTACTS:
            HandleAddContactsResult(env, executeHelper, result);
            break;
        case IS_LOCAL_CONTACT:
        case IS_MY_CARD:
            napi_get_boolean(env, executeHelper->resultData != 0, &result);
            break;
        case QUERY_CONTACT:
        case QUERY_MY_CARD:
        case QUERY_KEY:
            results = resultConvert.ResultSetToObject(env, executeHelper->resultSet, executeHelper->grantUri);
            if (results != nullptr) {
                napi_get_element(env, results, 0, &result);
                if (executeHelper->actionCode == QUERY_KEY) {
                    napi_get_named_property(env, result, "key", &result);
                }
            }
            break;
        case QUERY_CONTACTS:
        case QUERY_CONTACTS_BY_EMAIL:
        case QUERY_CONTACTS_BY_PHONE_NUMBER:
            result = resultConvert.ResultSetToObject(env, executeHelper->resultSet, executeHelper->grantUri);
            break;
        case QUERY_GROUPS:
            result = resultConvert.ResultSetToGroup(env, executeHelper->resultSet);
            break;
        case QUERY_HOLDERS:
            result = resultConvert.ResultSetToHolder(env, executeHelper->resultSet);
            break;
        case QUERY_CONTACT_COUNT:
            HandleQueryContactCountResult(env, executeHelper, result);
            break;
        default:
            break;
    }
}

void HandleAddContactsResult(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    auto resultSize = executeHelper->operationResultData.size();
    HILOG_WARN("HandleAddContactsResult result: %{public}d", executeHelper->resultData);
    napi_create_array_with_length(env, resultSize, &result);
    if (executeHelper->resultData == DataShare::ExecErrorCode::EXEC_SUCCESS ||
        executeHelper->resultData == DataShare::ExecErrorCode::EXEC_PARTIAL_SUCCESS) {
        for (size_t i = 0; i < resultSize; i++) {
            napi_value element;
            napi_create_int64(env, executeHelper->operationResultData[i], &element);
            napi_set_element(env, result, i, element);
        }
    }
}

void HandleAddContactsErrorCode(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    HILOG_ERROR("handleExecuteErrorCode resultData: %{public}d", executeHelper->resultData);
    if (executeHelper->resultData == RDB_PARAMETER_ERROR || executeHelper->resultData == ERROR) {
        HILOG_ERROR("internal error: %{public}d", executeHelper->resultData);
        result = ContactsNapiUtils::CreateError(env, CONTACT_GENERAL_ERROR);
    } else if (executeHelper->resultData == VERIFICATION_PARAMETER_ERROR) {
        HILOG_ERROR("parameter invalid");
        result = ContactsNapiUtils::CreateErrorByVerification(env, INVALID_PARAMETER);
    } else if (executeHelper->resultData == RDB_PERMISSION_ERROR) {
        HILOG_ERROR("permission error");
        result = ContactsNapiUtils::CreateError(env, PERMISSION_ERROR);
    }
}

void HandleSelectContactResult(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    if (executeHelper->resultData == RDB_PARAMETER_ERROR || executeHelper->resultData == ERROR
        || executeHelper->resultData == RDB_PERMISSION_ERROR) {
        // execute error
        HILOG_ERROR("handleExecuteResult handle error: %{public}d", executeHelper->resultData);
        napi_create_int64(env, ERROR, &result);
    } else {
        napi_create_int64(env, executeHelper->resultData, &result);
    }
}

void HandleQueryContactCountResult(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    if (executeHelper->resultSet == nullptr) {
        HILOG_ERROR("resultSet is null");
        napi_create_int64(env, ERROR, &result);
        return;
    }

    int rowCount = 0;
    executeHelper->resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        napi_create_int64(env, ERROR, &result);
        executeHelper->resultSet->Close();
        return;
    }

    executeHelper->resultSet->GoToFirstRow();
    int contactCount = 0;
    executeHelper->resultSet->GetInt(0, contactCount);
    napi_create_int64(env, contactCount, &result);

    executeHelper->resultSet->Close();
}

void LocalExecuteAddContact(napi_env env, ExecuteHelper *executeHelper)
{
    // 如果contact_data没有数据，不能插入，直接设置插入结果为错误
    if (executeHelper->valueContactData.empty()) {
        HILOG_ERROR("addContact, contact_data can not be empty");
        executeHelper->resultData = RDB_PARAMETER_ERROR;
        return;
    }
    ContactsControl contactsControl;
    // 插入 RawContact
    int rawId = contactsControl.RawContactInsert(
        executeHelper->dataShareHelper, (executeHelper->valueContact)[0]);
    std::vector<DataShare::DataShareValuesBucket> value = executeHelper->valueContactData;
    unsigned int size = value.size();
    for (unsigned int i = 0; i < size; ++i) {
        (executeHelper->valueContactData)[i].Put("raw_contact_id", rawId);
    }
    if (executeHelper->portrait.isNeedHandlePhoto) {
        int result = InsertContactPortrait(executeHelper, contactsControl, rawId, true);
        if (result != ERR_OK) {
            return;
        }
    }
    // 插入 ContactData
    int code = contactsControl.ContactDataInsert(executeHelper->dataShareHelper, executeHelper->valueContactData);
    if (code == 0) {
        executeHelper->resultData = rawId;
    } else {
        executeHelper->resultData = code;
    }
}

void LocalExecuteAddContacts(napi_env env, ExecuteHelper *executeHelper)
{
    // 如果operationStatement没有数据，不能插入，直接设置插入结果为错误
    if (executeHelper->operationStatements.empty()) {
        HILOG_ERROR("addContacts, operationStatement can not be empty");
        executeHelper->resultData = VERIFICATION_PARAMETER_ERROR;
        return;
    }
    executeHelper->resultData = DataShare::ExecErrorCode::EXEC_SUCCESS;
    ContactsControl contactsControl;
    // 插入 operationStatement
    size_t operationSize = 0;
    std::vector<DataShare::ExecResult> results;
    for (auto &statements : executeHelper->operationStatements) {
        DataShare::ExecResultSet batchInsertResult;
        contactsControl.ContactBatchInsert(executeHelper->dataShareHelper, statements, batchInsertResult);
        HandleContactBatchInsertResult(batchInsertResult, statements, executeHelper);
        operationSize += statements.size();
        statements.clear();
        results.insert(results.end(), batchInsertResult.results.begin(), batchInsertResult.results.end());
    }
    executeHelper->operationStatements.clear();
    if (operationSize == static_cast<size_t>(executeHelper->resultData)) {
        executeHelper->resultData = DataShare::ExecErrorCode::EXEC_FAILED;
    } else if (executeHelper->resultData > 0) {
        executeHelper->resultData = DataShare::ExecErrorCode::EXEC_PARTIAL_SUCCESS;
    }
    BatchInsertPortrait(results, executeHelper);
}

void BatchInsertPortrait(const std::vector<DataShare::ExecResult> &results, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    ContactsBuild contactsBuild;
    for (const auto &[index, portrait] : executeHelper->portraits) {
        if (results[index].code != DataShare::ExecErrorCode::EXEC_SUCCESS) {
            HILOG_ERROR("BatchInsertPortrait Insert Contact[%{public}zu] failed, skip insert portrait", index);
            continue;
        }
        auto rawId = std::stoi(results[index].message);
        Contacts contact;
        contact.portrait = portrait;
        contactsBuild.BuildExecuteHelperPortrait(contact, executeHelper);
        int insertPortraitResult = InsertContactPortrait(executeHelper, contactsControl, rawId, true);
        if (insertPortraitResult != ERR_OK) {
            HILOG_ERROR("BatchInsertPortrait InsertContactPortrait failed: %{public}d", insertPortraitResult);
            auto &opResult = executeHelper->operationResultData;
            auto iter = std::find(opResult.begin(), opResult.end(), rawId); 
            if (iter != opResult.end()) {
                *iter = INVALID_CONTACT_ID;
            }
        }
    }
    if (!executeHelper->valueContactData.empty()) {
        int code = contactsControl.ContactDataInsert(executeHelper->dataShareHelper, executeHelper->valueContactData);
        if (code == 0) {
            HILOG_WARN("BatchInsertPortrait ContactDataInsert, result: %{public}d", code);
        } else {
            HILOG_ERROR("BatchInsertPortrait ContactDataInsert failed, result: %{public}d", code);
        }
        executeHelper->valueContactData.clear();
    }
}

void HandleContactBatchInsertResult(const DataShare::ExecResultSet &execResultSet,
    const std::vector<DataShare::OperationStatement> &statements, ExecuteHelper *executeHelper)
{
    for (size_t i = 0; i < statements.size(); ++i) {
        if (statements[i].HasBackReference()) {
            continue;
        }
        auto &execResult = execResultSet.results[i];
        executeHelper->operationResultData.emplace_back(std::atoi(execResult.message.c_str()));
        if (execResult.code != DataShare::ExecErrorCode::EXEC_SUCCESS) {
            executeHelper->resultData++;
        }
    }
}

int HandleConverPortraitFailed(ExecuteHelper *executeHelper, ContactsControl &contactsControl, int rawContactId,
    const std::string &contactId, int errorCode)
{
    executeHelper->resultData =
        (errorCode == RDB_PERMISSION_ERROR ? RDB_PERMISSION_ERROR : VERIFICATION_PARAMETER_ERROR);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", rawContactId);
    std::string fileName = contactId + "_" + std::to_string(rawContactId) + ".jpg";
    return contactsControl.HandleAddFailed(executeHelper->dataShareHelper, predicates, fileName);
}

int InsertContactPortrait(ExecuteHelper *executeHelper, ContactsControl &contactsControl, int rawContactId,
    bool isAddType)
{
    std::string contactId = QueryContactIdByRawContactId(
        executeHelper->dataShareHelper, contactsControl, rawContactId);
    if (contactId.empty()) {
        HILOG_ERROR("InsertContactPortrait failed, contactId is null %{public}d", rawContactId);
        return OPEN_FILE_FAILED;
    }
    std::string fileName = contactId + "_" + std::to_string(rawContactId) + ".jpg";
    int fd = contactsControl.OpenFileByDataShare(fileName, executeHelper->dataShareHelper);
    if (fd == OPEN_FILE_FAILED || fd == RDB_PERMISSION_ERROR) {
        HILOG_ERROR("InsertContactPortrait OpenFileByDataShare failed");
        HandleConverPortraitFailed(executeHelper, contactsControl, rawContactId, contactId, fd);
        return fd;
    }
    int32_t srcHeight = 0;
    int32_t srcWidth = 0;
    OHOS::Contacts::KitPixelMapUtil::GetPixelMapSize(executeHelper->portrait.photo, srcHeight, srcWidth);
    int result = OHOS::Contacts::KitPixelMapUtil::SavePixelMapToFile(executeHelper->portrait.photo, fd);
    close(fd);
    if (result != ERR_OK) {
        HandleConverPortraitFailed(executeHelper, contactsControl, rawContactId, contactId, result);
        return result;
    }
    DataShare::DataShareValuesBucket valuesBucketPortrait;

    valuesBucketPortrait.Put("PortraitFileName", fileName);
    valuesBucketPortrait.Put("contactId", contactId);
    valuesBucketPortrait.Put("rawContactId", std::to_string(rawContactId));
    valuesBucketPortrait.Put("srcHeight", srcHeight);
    valuesBucketPortrait.Put("srcWidth", srcWidth);
    valuesBucketPortrait.Put("addPortraitType", 
        OHOS::Contacts::KitPixelMapUtil::GetAddPortraitType(isAddType, executeHelper->portrait.isUriPortrait));
    executeHelper->valueContactData.push_back(valuesBucketPortrait);
    return result;
}

std::string QueryContactIdByRawContactId(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
  ContactsControl &contactsControl, int rawContactId)
{
    std::vector<std::string> columns;
    columns.push_back("contact_id");

    auto resultSet = contactsControl.QueryContactByRawContactId(dataShareHelper, columns, rawContactId);

    if (resultSet == nullptr) {
        HILOG_ERROR("QueryContactByRawContactId failed, resultSet is nullptr");
        return "";
    }
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        resultSet->Close();
        HILOG_ERROR("QueryContactByRawContactId failed, rowCount is 0");
        return "";
    }
    std::vector<std::string> contactIdVector;
    int resultSetNum = resultSet->GoToFirstRow();
    while (resultSetNum == 0) {
        std::string contactId;
        int columnIndex = 0;
        resultSet->GetColumnIndex("contact_id", columnIndex);
        resultSet->GetString(columnIndex, contactId);
        contactIdVector.push_back(contactId);
        resultSetNum = resultSet->GoToNextRow();
    }
    resultSet->Close();
    for (std::string contactId : contactIdVector) {
        if (!contactId.empty()) {
            return contactId;
        }
    }
    HILOG_ERROR("QueryContactByRawContactId failed, contactId is empty");
    return "";
}

void LocalExecuteDeleteContact(napi_env env, ExecuteHelper *executeHelper)
{
    // 如果key为空，返回失败
    if (executeHelper->resultData == RDB_PARAMETER_ERROR) {
        HILOG_ERROR("LocalExecuteDeleteContact, key can not be empty");
        return;
    }
    ContactsControl contactsControl;
    int ret = contactsControl.ContactDelete(executeHelper->dataShareHelper, executeHelper->predicates);
    HILOG_INFO("LocalExecuteDeleteContact contact ret = %{public}d", ret);
    executeHelper->resultData = ret;
}

void LocalExecuteQueryContactCount(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    std::vector<std::string> columns;
    columns.push_back("count(*) as count");
    executeHelper->columns = columns;
    executeHelper->resultSet = contactsControl.ContactCountQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    executeHelper->resultData = SUCCESS;
}

void LocalExecuteQueryContact(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    executeHelper->resultSet = contactsControl.ContactQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    executeHelper->resultData = SUCCESS;
}

void LocalExecuteQueryContactsOrKey(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    executeHelper->resultSet = contactsControl.ContactQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    executeHelper->resultData = SUCCESS;
}

void LocalExecuteQueryAppGroupDir(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    executeHelper->grantUri = contactsControl.QueryAppGroupDir(executeHelper->dataShareHelper);
    executeHelper->resultData = SUCCESS;
}

void LocalExecuteQueryContactsByData(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    executeHelper->resultSet = contactsControl.ContactDataQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = executeHelper->resultSet;
    int rowCount = 0;
    if (resultSet == nullptr) {
        HILOG_ERROR("LocalExecuteQueryContactsByData resultSet is nullprt");
        executeHelper->resultData = RDB_PARAMETER_ERROR;
        return;
    }
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOG_ERROR("LocalExecuteQueryContactsByData parameter verification failed");
        executeHelper->resultData = RDB_PARAMETER_ERROR;
        resultSet->Close();
    } else {
        executeHelper->resultData = SUCCESS;
    }
}

void LocalExecuteQueryGroup(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    executeHelper->resultSet = contactsControl.GroupsQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    executeHelper->resultData = SUCCESS;
}

void LocalExecuteQueryHolders(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    executeHelper->resultSet = contactsControl.HolderQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    executeHelper->resultData = SUCCESS;
}

void LocalExecuteQueryMyCard(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    executeHelper->resultSet = contactsControl.MyCardQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    executeHelper->resultData = SUCCESS;
}

void LocalExecuteUpdateContact(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsControl contactsControl;
    // query raw_contact_id
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = contactsControl.ContactDataQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    int rawId = GetRawIdByResultSet(resultSet);
    if (rawId == 0) {
        HILOG_ERROR("LocalExecuteUpdateContact contact rawId equals 0");
        executeHelper->resultData = ERROR;
        return;
    }
    std::vector<DataShare::DataShareValuesBucket> value = executeHelper->valueContactData;
    unsigned int size = value.size();
    for (unsigned int i = 0; i < size; ++i) {
        (executeHelper->valueContactData)[i].Put("raw_contact_id", rawId);
    }
    executeHelper->deletePredicates.And();
    executeHelper->deletePredicates.EqualTo("raw_contact_id", std::to_string(rawId));
    int resultCode = contactsControl.ContactDataDelete(
        executeHelper->dataShareHelper, executeHelper->deletePredicates);
    if (executeHelper->portrait.isNeedHandlePhoto) {
        int result = InsertContactPortrait(executeHelper, contactsControl, rawId, false);
        if (result != ERR_OK) {
            executeHelper->resultData = RDB_PARAMETER_ERROR;
            return;
        }
    }
    if (resultCode >= 0) {
        resultCode = contactsControl.ContactDataInsert(
            executeHelper->dataShareHelper, executeHelper->valueContactData);
    }
    executeHelper->resultData = resultCode;
}

void LocalExecuteIsLocalContact(napi_env env, ExecuteHelper *executeHelper)
{
    int64_t isLocal = 0;
    ContactsControl contactsControl;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = contactsControl.ContactQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    if (resultSet == nullptr) {
        executeHelper->resultData = isLocal;
        return;
    }
    int resultSetNum = resultSet->GoToFirstRow();
    if (resultSetNum == OHOS::NativeRdb::E_OK) {
        isLocal = 1;
    }
    executeHelper->resultData = isLocal;
    resultSet->Close();
}

void LocalExecuteIsMyCard(napi_env env, ExecuteHelper *executeHelper)
{
    int64_t isMyCard = 0;
    ContactsControl contactsControl;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = contactsControl.MyCardQuery(
        executeHelper->dataShareHelper, executeHelper->columns, executeHelper->predicates);
    if (resultSet == nullptr) {
        executeHelper->resultData = isMyCard;
        return;
    }
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    int resultSetNum = resultSet->GoToFirstRow();
    if (resultSetNum == OHOS::NativeRdb::E_OK) {
        isMyCard = 1;
    }
    executeHelper->resultData = isMyCard;
    resultSet->Close();
}

void LocalExecute(napi_env env, ExecuteHelper *executeHelper)
{
    if (executeHelper->dataShareHelper == nullptr) {
        HILOG_ERROR("create dataShareHelper is null, please check your permission");
        executeHelper->resultData = RDB_PERMISSION_ERROR;
        return;
    }
    HILOG_INFO("LocalExecute, actionCode = %{public}d", executeHelper->actionCode);
    switch (executeHelper->actionCode) {
        case ADD_CONTACT:
            // 执行添加联系人操作
            LocalExecuteAddContact(env, executeHelper);
            break;
        case ADD_CONTACTS:
            // 执行批量添加联系人操作
            LocalExecuteAddContacts(env, executeHelper);
            break;
        case DELETE_CONTACT:
            LocalExecuteDeleteContact(env, executeHelper);
            break;
        case UPDATE_CONTACT:
            LocalExecuteUpdateContact(env, executeHelper);
            break;
        default:
            LocalExecuteSplit(env, executeHelper);
            HILOG_INFO("LocalExecute case error===>");
            break;
    }
}

void LocalExecuteSplit(napi_env env, ExecuteHelper *executeHelper)
{
    ContactsTelephonyPermission permission;
    if (!permission.CheckPermission(ContactsApi::Permission::READ_CONTACTS)) {
        HILOG_ERROR("LocalExecuteQueryContactsByData Permission denied!");
        executeHelper->resultData = RDB_PERMISSION_ERROR;
        return;
    } else if (executeHelper->resultData == VERIFICATION_PARAMETER_ERROR) {
        HILOG_ERROR("PARAMETER_ERROR, please check your PARAMETER");
        return;
    }
    if (std::find(needGroupVec.begin(), needGroupVec.end(), executeHelper->actionCode) != needGroupVec.end()) {
        LocalExecuteQueryAppGroupDir(env, executeHelper);
    }
    switch (executeHelper->actionCode) {
        case QUERY_CONTACT_COUNT:
            LocalExecuteQueryContactCount(env, executeHelper);
            break;
        case QUERY_CONTACT:
            LocalExecuteQueryContact(env, executeHelper);
            break;
        case QUERY_CONTACTS:
            LocalExecuteQueryContactsOrKey(env, executeHelper);
            break;
        case QUERY_CONTACTS_BY_EMAIL:
        case QUERY_CONTACTS_BY_PHONE_NUMBER:
            LocalExecuteQueryContactsByData(env, executeHelper);
            break;
        case QUERY_GROUPS:
            LocalExecuteQueryGroup(env, executeHelper);
            break;
        case QUERY_HOLDERS:
            LocalExecuteQueryHolders(env, executeHelper);
            break;
        case QUERY_KEY:
            LocalExecuteQueryContactsOrKey(env, executeHelper);
            break;
        case QUERY_MY_CARD:
            LocalExecuteQueryMyCard(env, executeHelper);
            break;
        case IS_LOCAL_CONTACT:
            LocalExecuteIsLocalContact(env, executeHelper);
            break;
        case IS_MY_CARD:
            LocalExecuteIsMyCard(env, executeHelper);
            break;
        default:
            HILOG_INFO("LocalExecute case error===>");
            break;
    }
}

void Execute(napi_env env, void *data)
{
    ExecuteHelper *executeHelper = static_cast<ExecuteHelper *>(data);
    HILOG_INFO("Execute start workName: %{public}d", executeHelper->actionCode);
    LocalExecute(env, executeHelper);
}

napi_value CreateAsyncWork(napi_env env, ExecuteHelper *executeHelper)
{
    napi_value workName;
    napi_value result = nullptr;
    // 执行处理方法：Execute
    // 执行完处理后执行方法：ExecuteSyncDone，ExecuteDone
    if (executeHelper->sync == NAPI_CALL_TYPE_CALLBACK) {
        HILOG_INFO("CreateAsyncWork ExecuteSyncDone");
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &workName);
        napi_create_reference(env, executeHelper->argv[executeHelper->argc - 1], 1, &executeHelper->callBack);
        napi_create_async_work(env, nullptr, workName, Execute, ExecuteSyncDone,
            reinterpret_cast<void *>(executeHelper), &(executeHelper->work));
        napi_get_null(env, &result);
    } else {
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &workName);
        napi_create_promise(env, &(executeHelper->deferred), &result);
        napi_create_async_work(env, nullptr, workName, Execute, ExecuteDone,
            reinterpret_cast<void *>(executeHelper), &(executeHelper->work));
    }
    napi_queue_async_work(env, executeHelper->work);
    executeHelper->promise = result;
    return result;
}

DataShare::DataSharePredicates ConvertParamsSwitchSplit(int code, napi_env env, const napi_value &key,
    const napi_value &hold, const napi_value &attr, ExecuteHelper *executeHelper)
{
    DataShare::DataSharePredicates predicates;
    switch (code) {
        case QUERY_CONTACT_COUNT:
            VerificationParameterHolderId(env, executeHelper, hold);
            predicates = BuildQueryContactCountPredicates();
            break;
        case QUERY_CONTACT:
            VerificationParameterHolderId(env, executeHelper, hold);
            predicates = BuildQueryContactPredicates(env, key, hold, attr);
            break;
        case QUERY_CONTACTS:
            VerificationParameterHolderId(env, executeHelper, hold);
            predicates = BuildQueryContactsPredicates(env, hold, attr);
            break;
        case QUERY_CONTACTS_BY_EMAIL:
            VerificationParameterHolderId(env, executeHelper, hold);
            predicates = BuildQueryContactsByEmailPredicates(env, key, hold, attr);
            break;
        case QUERY_CONTACTS_BY_PHONE_NUMBER:
            VerificationParameterHolderId(env, executeHelper, hold);
            predicates = BuildQueryContactsByPhoneNumberPredicates(env, key, hold, attr);
            break;
        case QUERY_GROUPS:
            VerificationParameterHolderId(env, executeHelper, hold);
            predicates = BuildQueryGroupsPredicates(env, hold);
            break;
        case QUERY_HOLDERS:
            break;
        case QUERY_MY_CARD:
            predicates = BuildQueryMyCardPredicates(env, attr);
            break;
        default:
            HILOG_ERROR("ConvertParamsSwitchSplit code is no match");
            break;
    }
    return predicates;
}

void SetChildActionCodeAndConvertParams(napi_env env, ExecuteHelper *executeHelper)
{
    napi_value id = nullptr;
    napi_value key = nullptr;
    napi_value hold = nullptr;
    napi_value attr = nullptr;
    napi_value contact = nullptr;
    unsigned int size = executeHelper->argc;
    for (unsigned int i = 0; i < size; i++) {
        ObjectInitId(env, executeHelper->argv[i], id);
        ObjectInitString(env, executeHelper->argv[i], key);
        ObjectInit(env, executeHelper->argv[i], hold, attr, contact);
    }
    ContactsBuild contactsBuild;
    HILOG_INFO("SetChildActionCodeAndConvertParams, actionCode = %{public}d", executeHelper->actionCode);
    switch (executeHelper->actionCode) {
        case ADD_CONTACT:
            contactsBuild.GetContactData(env, executeHelper);
            break;
        case ADD_CONTACTS:
            contactsBuild.BuildOperationStatements(env, executeHelper);
            break;
        case DELETE_CONTACT:
            executeHelper->predicates = BuildDeleteContactPredicates(env, executeHelper);
            break;
        case UPDATE_CONTACT:
            executeHelper->predicates = BuildUpdateContactConvertParams(env, contact, attr, executeHelper);
            break;
        case IS_LOCAL_CONTACT:
            VerificationParameterId(env, id, executeHelper, hold);
            executeHelper->predicates = BuildIsLocalContactPredicates(env, id);
            break;
        case IS_MY_CARD:
            VerificationParameterId(env, id, executeHelper, hold);
            executeHelper->predicates = BuildIsMyCardPredicates(env, id);
            break;
        case QUERY_KEY:
            VerificationParameterId(env, id, executeHelper, hold);
            executeHelper->predicates = BuildQueryKeyPredicates(env, id, hold);
            break;
        default:
            executeHelper->predicates = ConvertParamsSwitchSplit(executeHelper->actionCode, env, key, hold,
                attr, executeHelper);
            break;
    }
}

void VerificationParameterId(napi_env env, napi_value id, ExecuteHelper *executeHelper, napi_value hold)
{
    ContactsBuild contactsBuild;
    Holder holder = contactsBuild.GetHolder(env, hold);
    int holderId = holder.holderId;
    int valueId = contactsBuild.GetInt(env, id);
    if (valueId <= 0 || isinf(valueId)) {
        executeHelper->resultData = VERIFICATION_PARAMETER_ERROR;
        HILOG_ERROR("PARAMETER_ERROR valueId: %{public}d", valueId);
    } else if (hold != nullptr && holderId != 1) {
        executeHelper->resultData = VERIFICATION_PARAMETER_ERROR;
        HILOG_ERROR("PARAMETER_ERROR holderId: %{public}d", holderId);
    }
}

void VerificationParameterHolderId(napi_env env, ExecuteHelper *executeHelper, napi_value hold)
{
    ContactsBuild contactsBuild;
    Holder holder = contactsBuild.GetHolder(env, hold);
    int holderId = holder.holderId;
    if (hold != nullptr && holderId != 1) {
        executeHelper->resultData = VERIFICATION_PARAMETER_ERROR;
        HILOG_ERROR("PARAMETER_ERROR holderId: %{public}d", holderId);
    }
}

napi_value Scheduling(napi_env env, napi_callback_info info, ExecuteHelper *executeHelper, int actionCode)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    executeHelper->argc = argc;
    executeHelper->actionCode = actionCode;

    if (argc > 0) {
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[argc - 1], &valuetype);
        // last params is function as callback
        if (valuetype == napi_function) {
            executeHelper->sync = NAPI_CALL_TYPE_CALLBACK;
        } else {
            executeHelper->sync = NAPI_CALL_TYPE_PROMISE;
        }
    }
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        HILOG_INFO("use API 10 interface");
        for (int i = 1; i < MAX_PARAMS; i++) {
            executeHelper->argv[i - 1] = argv[i];
        }
        executeHelper->abilityContext = argv[0];
        executeHelper->argc -= 1;
    } else {
        HILOG_INFO("use API 7 interface");
        for (int i = 0; i < MAX_PARAMS; i++) {
            executeHelper->argv[i] = argv[i];
        }
    }

    SetChildActionCodeAndConvertParams(env, executeHelper);
    GetDataShareHelper(env, info, executeHelper);

    napi_value result = CreateAsyncWork(env, executeHelper);
    return result;
}

/**
 * @brief Test interface ADD_CONTACT
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value AddContact(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, ADD_CONTACT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface ADD_CONTACT
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value AddContacts(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, INVALID_PARAMETER);
        switch (argc) {
            HILOG_WARN("AddContacts argc: %{public}zu", argc);
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object })) {
                    HILOG_ERROR("AddContacts argc is 2 and param is invalid");
                    napi_throw(env, errorCode);
                    return errorCode;
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })) {
                    HILOG_ERROR("AddContacts argc is 3 and param is invalid");
                    napi_throw(env, errorCode);
                    return errorCode;
                }
                break;
            default:
                HILOG_ERROR("AddContacts Invalid number of parameters");
                napi_throw(env, errorCode);
                return errorCode;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, ADD_CONTACTS);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

static void StartContactsPickerExecute(napi_env env, void *data)
{
    HILOG_INFO("[ContactsPicker] StartContactsPickerExecute");
    auto *context = static_cast<OHOS::ContactsApi::BaseContext*>(data);
    while (!context->pickerCallBack->ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
}

static std::string SplicePickerData(std::string *pickerData, int32_t dataIndex)
{
    std::stringstream splicePickerData;
    splicePickerData << "["; // 开始的方括号
    // 遍历pickerData中的每一个字符串
    for (int32_t i = 0; i < (dataIndex + 1); ++i) {
        // 去除字符串两端的方括号
        std::string currentData = pickerData[i].substr(1, pickerData[i].length() - OFFSET_TWO);
        // 如果不是第一个元素，添加逗号
        if (i > 0 && pickerData[i].length() != 0) {
            splicePickerData << ",";
        }
        // 添加处理后的字符串
        splicePickerData << currentData;
    }
    splicePickerData << "]"; // 结束的方括号
    HILOG_WARN("[ContactsPicker] end of SplicePickerData");
    return splicePickerData.str();
}

static void StartSaveContactsPickerAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto *context = static_cast<OHOS::ContactsApi::BaseContext*>(data);
    CHECK_NULL_PTR_RETURN_VOID(context, "[ContactsPicker] Async context is null");

    auto jsContext = std::make_unique<JSAsyncContextOutput>();
    jsContext->status = false;
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->data), JS_ERR_PARAMETER_INVALID);
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->error), JS_ERR_PARAMETER_INVALID);
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value resultCode = nullptr;
    napi_create_int32(env, context->pickerCallBack->resultCode, &resultCode);
    status = napi_set_named_property(env, result, "resultCode", resultCode);
    if (status != napi_ok) {
        HILOG_ERROR("[ContactsPicker] napi_set_named_property resultCode failed");
    }
    napi_value jsContactId = nullptr;
    
    CHECK_ARGS_RET_VOID(env, napi_create_int32(env, context->pickerCallBack->dataIndex, &jsContactId), JS_INNER_FAIL);
    if (napi_set_named_property(env, result, "jsContactId", jsContactId) != napi_ok) {
        HILOG_ERROR("[ContactsPicker] napi_set_named_property total failed");
    }
    if (result != nullptr) {
        jsContext->data = result;
        jsContext->status = true;
    } else {
        HILOG_ERROR("[ContactsPicker] failed to create js object");
    }
    if (context->work != nullptr) {
        ContactsNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
            context->work, *jsContext);
    }
    delete context;
    HILOG_WARN("[ContactsPicker] StartContactsPickerExecute end");
}

static void StartContactsPickerAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto *context = static_cast<OHOS::ContactsApi::BaseContext*>(data);
    CHECK_NULL_PTR_RETURN_VOID(context, "[ContactsPicker] Async context is null");

    auto jsContext = std::make_unique<JSAsyncContextOutput>();
    jsContext->status = false;
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->data), JS_ERR_PARAMETER_INVALID);
    CHECK_ARGS_RET_VOID(env, napi_get_undefined(env, &jsContext->error), JS_ERR_PARAMETER_INVALID);
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value resultCode = nullptr;
    napi_create_int32(env, context->pickerCallBack->resultCode, &resultCode);
    status = napi_set_named_property(env, result, "resultCode", resultCode);
    if (status != napi_ok) {
        HILOG_ERROR("[ContactsPicker] napi_set_named_property resultCode failed");
    }

    std::string pickerTemp;
    if (context->pickerCallBack->resultCode == 1) {
        pickerTemp = "[]";
        context->pickerCallBack->total = 0;
    } else {
        pickerTemp = SplicePickerData(context->pickerCallBack->pickerData, context->pickerCallBack->dataIndex);
    }
    const std::string &pickerData = pickerTemp;

    napi_value jsPickerData = nullptr;
    CHECK_ARGS_RET_VOID(env, napi_create_string_utf8(env, pickerData.c_str(),
        NAPI_AUTO_LENGTH, &jsPickerData), JS_INNER_FAIL);
    if (napi_set_named_property(env, result, "pickerData", jsPickerData) != napi_ok) {
        HILOG_ERROR("[ContactsPicker] napi_set_named_property jsPickerData failed");
    }
    napi_value jsTotal = nullptr;
    CHECK_ARGS_RET_VOID(env, napi_create_int32(env, context->pickerCallBack->total, &jsTotal), JS_INNER_FAIL);
    if (napi_set_named_property(env, result, "total", jsTotal) != napi_ok) {
        HILOG_ERROR("[ContactsPicker] napi_set_named_property total failed");
    }
    if (result != nullptr) {
        jsContext->data = result;
        jsContext->status = true;
    } else {
        HILOG_ERROR("[ContactsPicker] failed to create js object");
    }
    if (context->work != nullptr) {
        ContactsNapiUtils::InvokeJSAsyncMethod(env, context->deferred, context->callbackRef,
            context->work, *jsContext);
    }
    delete context;
}

void StartUIExtensionAbility(OHOS::AAFwk::Want& request, std::unique_ptr<OHOS::ContactsApi::BaseContext>& asyncContext)
{
    HILOG_INFO("[ContactsPicker] begin StartUIExtensionAbility");
    if (asyncContext == nullptr) {
        HILOG_ERROR("[ContactsPicker] asyncContext is nullptr");
        return;
    }
    if (asyncContext->context == nullptr &&
        asyncContext->uiExtensionContext == nullptr) {
        HILOG_ERROR("[ContactsPicker] abilityContext && uiExtensionContext is nullptr");
        return;
    }
    auto uiContent = ContactsNapiUtils::GetUIContent(asyncContext);
    if (uiContent == nullptr) {
        HILOG_ERROR("[ContactsPicker] UIContent is nullptr");
        return;
    }
    asyncContext->pickerCallBack = std::make_shared<PickerCallBack>();
    auto callback = std::make_shared<ModalUICallback>(uiContent, asyncContext);
    // 构建回调，onRelease、OnResultForModal、OnError、OnDestroy
    OHOS::Ace::ModalUIExtensionCallbacks extensionCallbacks = {
        std::bind(&ModalUICallback::OnRelease, callback, std::placeholders::_1),
        std::bind(&ModalUICallback::OnResultForModal, callback, std::placeholders::_1, std::placeholders::_2),
        std::bind(&ModalUICallback::OnReceive, callback, std::placeholders::_1),
        std::bind(&ModalUICallback::OnError, callback, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
    };
    OHOS::Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    // 调用CreateModalUIExtension拉页面
    HILOG_WARN("[ContactsPicker] CreateModalUIExtension");
    int32_t sessionId = uiContent->CreateModalUIExtension(request, extensionCallbacks, config);
    callback->SetSessionId(sessionId);
    HILOG_INFO("[ContactsPicker] end StartUIExtensionAbility");
    return;
}

/* 解析AbilityContext */
static bool ParseAbilityContextReq(
    napi_env env, const napi_value& obj, std::shared_ptr<AbilityRuntime::AbilityContext>& abilityContext,
    std::shared_ptr<AbilityRuntime::UIExtensionContext>& uiExtensionContext)
{
    HILOG_INFO("[ContactsPicker] begin ParseAbilityContextReq");
    bool stageMode = false;
    napi_status status = AbilityRuntime::IsStageContext(env, obj, stageMode);
    if (status != napi_ok || !stageMode) {
        HILOG_ERROR("[ContactsPicker] it is not a stage mode");
        return false;
    }
    auto context = AbilityRuntime::GetStageModeContext(env, obj);
    if (context == nullptr) {
        HILOG_ERROR("[ContactsPicker] GetStageModeContext failed");
        return false;
    }
    abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (abilityContext == nullptr) {
        HILOG_ERROR("[ContactsPicker] get abilityContext failed");
        uiExtensionContext =
            AbilityRuntime::Context::ConvertTo<AbilityRuntime::UIExtensionContext>(context);
        if (uiExtensionContext == nullptr) {
            HILOG_ERROR("[ContactsPicker] get uiExtensionContext failed");
            return false;
        }
    }
    HILOG_INFO("[ContactsPicker] end ParseAbilityContextReq");
    return true;
}

napi_value ContactsPickerSelect(napi_env env, napi_callback_info info)
{
    HILOG_WARN("[ContactsPicker] ContactsPickerSelect start");
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    void *data;
    // 获取JS接口传入的参数
    napi_get_cb_info(env, info, &argc, &(argv[0]), &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    // init request of Want，构建 want 请求
    OHOS::AAFwk::Want request;
    AppExecFwk::UnwrapWant(env, argv[1], request);
    request.SetElementName(CONTACT_PACKAGE_NAME, CONTACT_UI_EXT_ABILITY_NAME); // 设置包名 和 ability name
    std::string targetType = CONTACT_UI_EXT_TYPE;
    std::string targetUrl = "BatchSelectContactsPage";
    request.SetParam(UI_EXT_TYPE, targetType);
    request.SetParam(UI_EXT_TARGETURL, targetUrl);
    // init request Context，第一个参数是Context，解析参数，转成AbilityContext或者UIExtensionContext
    auto asyncContext = std::make_unique<OHOS::ContactsApi::BaseContext>();
    asyncContext->env = env;
    if (!ParseAbilityContextReq(env, argv[0],
        asyncContext->context, asyncContext->uiExtensionContext)) {
        HILOG_ERROR("[ContactsPicker] ParseAbilityContextReq failed");
        return result;
    }
    NAPI_CALL(env, ContactsNapiUtils::GetParamCallback(env, asyncContext, argc, argv)); // 解析Promise对象，用于返回JS结果
    // start UIExtension Ability，启动UIExtensionAbility
    StartUIExtensionAbility(request, asyncContext);
    HILOG_INFO("[ContactsPicker] ContactsPickerSelect end");
    return ContactsNapiUtils::NapiCreateAsyncWork(env, asyncContext, "ContactsPickerSelect",
        StartContactsPickerExecute, StartContactsPickerAsyncCallbackComplete);
}

napi_value ContactsPickerSave(napi_env env, napi_callback_info info)
{
    HILOG_WARN("[ContactsPicker] ContactsPickerSave start");
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, &(argv[0]), &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    napi_value result = nullptr;
    
    result = argv[1];
    HILOG_INFO("[ContactsPicker] SetChildActionCodeAndConvertParams, ");

    // init request of Want，构建 want 请求
    OHOS::AAFwk::Want request;
    AppExecFwk::UnwrapWant(env, argv[1], request);
    request.SetElementName(CONTACT_PACKAGE_NAME, CONTACT_UI_EXT_ABILITY_NAME); // 设置包名 和 ability name
    std::string targetType = CONTACT_UI_EXT_TYPE;
    std::string targetUrl = "Accountants";
    request.SetParam(UI_EXT_TYPE, targetType);
    request.SetParam(UI_EXT_TARGETURL, targetUrl);

    ResultConvert resultConvert;
    napi_value parameters = resultConvert.GetNapiValue(env, "parameters", argv[1]);
    if (parameters != nullptr) {
        napi_value contactNapi = resultConvert.GetNapiValue(env, "contact", parameters);
        ContactsBuild contactsBuild;
        Contacts contact;
        contactsBuild.GetContactDataByObject(env, contactNapi, contact);
        std::string uri = contact.portrait.uri;
        request.SetFlags(OHOS::AAFwk::Want::FLAG_AUTH_WRITE_URI_PERMISSION | OHOS::AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION);
        request.SetUri(uri);
    }
    
    // init request Context，第一个参数是Context，解析参数，转成AbilityContext或者UIExtensionContext
    auto asyncContext = std::make_unique<OHOS::ContactsApi::BaseContext>();
    asyncContext->env = env;
    if (!ParseAbilityContextReq(env, argv[0],
        asyncContext->context, asyncContext->uiExtensionContext)) {
        HILOG_ERROR("[ContactsPicker] ParseAbilityContextReq failed");
        return result;
    }
    NAPI_CALL(env, ContactsNapiUtils::GetParamCallback(env, asyncContext, argc, argv)); // 解析Promise对象，用于返回JS结果
    // start UIExtension Ability，启动UIExtensionAbility
    StartUIExtensionAbility(request, asyncContext);
    HILOG_INFO("[ContactsPicker] ContactsPickerSave end");
    return ContactsNapiUtils::NapiCreateAsyncWork(env, asyncContext, "contactsPickerSave",
        StartContactsPickerExecute, StartSaveContactsPickerAsyncCallbackComplete);
}

napi_value ContactsPickerSaveExist(napi_env env, napi_callback_info info)
{
    HILOG_WARN("[ContactsPicker] ContactsPickerSaveExist start");
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    void *data;
    // 获取JS接口传入的参数
    napi_get_cb_info(env, info, &argc, &(argv[0]), &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    // init request of Want，构建 want 请求
    OHOS::AAFwk::Want request;
    AppExecFwk::UnwrapWant(env, argv[1], request);
    request.SetElementName(CONTACT_PACKAGE_NAME, CONTACT_UI_EXT_ABILITY_NAME); // 设置包名 和 ability name
    std::string targetType = CONTACT_UI_EXT_TYPE;
    std::string targetUrl = "BatchSelectContactsPage";
    request.SetParam(UI_EXT_TYPE, targetType);
    request.SetParam(UI_EXT_TARGETURL, targetUrl);

    ResultConvert resultConvert;
    napi_value parameters = resultConvert.GetNapiValue(env, "parameters", argv[1]);
    if (parameters != nullptr) {
        napi_value contactNapi = resultConvert.GetNapiValue(env, "contact", parameters);
        ContactsBuild contactsBuild;
        Contacts contact;
        contactsBuild.GetContactDataByObject(env, contactNapi, contact);
        std::string uri = contact.portrait.uri;
        request.SetFlags(OHOS::AAFwk::Want::FLAG_AUTH_WRITE_URI_PERMISSION | OHOS::AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION);
        request.SetUri(uri);
    }
    
    // init request Context，第一个参数是Context，解析参数，转成AbilityContext或者UIExtensionContext
    auto asyncContext = std::make_unique<OHOS::ContactsApi::BaseContext>();
    asyncContext->env = env;
    if (!ParseAbilityContextReq(env, argv[0],
        asyncContext->context, asyncContext->uiExtensionContext)) {
        HILOG_ERROR("[ContactsPicker] ParseAbilityContextReq failed");
        return result;
    }
    NAPI_CALL(env, ContactsNapiUtils::GetParamCallback(env, asyncContext, argc, argv)); // 解析Promise对象，用于返回JS结果
    // start UIExtension Ability，启动UIExtensionAbility
    StartUIExtensionAbility(request, asyncContext);
    HILOG_INFO("[ContactsPicker] ContactsPickerSaveExist end");
    return ContactsNapiUtils::NapiCreateAsyncWork(env, asyncContext, "ContactsPickerSaveExist",
        StartContactsPickerExecute, StartSaveContactsPickerAsyncCallbackComplete);
}

/**
 * @brief Test interface DELETE_CONTACT
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value DeleteContact(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_string })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_string, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, DELETE_CONTACT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface UPDATE_CONTACT
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value UpdateContact(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })
                    && !ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FOUR:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_object, napi_object, napi_function
                })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, UPDATE_CONTACT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_CONTACT_COUNT
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryContactsCount(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_ONE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_CONTACT_COUNT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_CONTACT
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryContact(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_string })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_string, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FOUR:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_string, napi_object, napi_function }) &&
                    !ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_string, napi_object, napi_object
                })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FIVE:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    {
                        napi_object, napi_string, napi_object, napi_object, napi_function
                    })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_CONTACT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_CONTACTS
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryContacts(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_ONE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })
                && !ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_object })
                ) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FOUR:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_object, napi_object, napi_function
                })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_CONTACTS);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_CONTACTS_BY_EMAIL
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryContactsByEmail(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_string })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_string, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FOUR:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_string, napi_object, napi_function }) &&
                    !ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_string, napi_object, napi_object
                })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FIVE:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    {
                        napi_object, napi_string, napi_object, napi_object, napi_function
                    })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_CONTACTS_BY_EMAIL);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_CONTACTS_BY_PHONE_NUMBER
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryContactsByPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_string })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_string, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FOUR:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_string, napi_object, napi_function }) &&
                    !ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_string, napi_object, napi_object
                })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FIVE:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    {
                        napi_object, napi_string, napi_object, napi_object, napi_function
                    })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_CONTACTS_BY_PHONE_NUMBER);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_CONTACTS_BY_PHONE_NUMBER
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryGroups(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_ONE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_function })
                    && !ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_GROUPS);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_HOLDERS
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryHolders(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_ONE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_HOLDERS);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_KEY
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryKey(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_ONE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_function })
                && !ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_number })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_number, napi_function })
                && !ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_number, napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_FOUR:
                if (!ContactsNapiUtils::MatchParameters(env, argv,
                    { napi_object, napi_number, napi_object, napi_function
                })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_KEY);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface QUERY_MY_CARD
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value QueryMyCard(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_ONE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object })
                    && !ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_object, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, QUERY_MY_CARD);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface IS_MY_CARD
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value IsMyCard(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_number })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_number, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, IS_MY_CARD);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface IS_LOCAL_CONTACT
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value IsLocalContact(napi_env env, napi_callback_info info)
{
    size_t argc = MAX_PARAMS;
    napi_value argv[MAX_PARAMS] = {0};
    napi_value thisVar = nullptr;
    void *data;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    bool isStageMode = false;
    OHOS::AbilityRuntime::IsStageContext(env, argv[0], isStageMode);
    if (isStageMode) {
        napi_value errorCode = ContactsNapiUtils::CreateError(env, PARAMETER_ERROR);
        switch (argc) {
            case ARGS_TWO:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_number })) {
                    napi_throw(env, errorCode);
                }
                break;
            case ARGS_THREE:
                if (!ContactsNapiUtils::MatchParameters(env, argv, { napi_object, napi_number, napi_function })) {
                    napi_throw(env, errorCode);
                }
                break;
            default:
                napi_throw(env, errorCode);
                break;
        }
    }
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        result = Scheduling(env, info, executeHelper, IS_LOCAL_CONTACT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

napi_value DeclareContactConst(napi_env env, napi_value exports)
{
    // Contact
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("INVALID_CONTACT_ID",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Contacts::INVALID_CONTACT_ID))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "Contact", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "Contact", result);
    return exports;
}

napi_value DeclareEmailConst(napi_env env, napi_value exports)
{
    // Email
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CUSTOM_LABEL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Email::CUSTOM_LABEL))),
        DECLARE_NAPI_STATIC_PROPERTY("EMAIL_HOME",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Email::EMAIL_HOME))),
        DECLARE_NAPI_STATIC_PROPERTY("EMAIL_WORK",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Email::EMAIL_WORK))),
        DECLARE_NAPI_STATIC_PROPERTY("EMAIL_OTHER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Email::EMAIL_OTHER))),
        DECLARE_NAPI_STATIC_PROPERTY("INVALID_LABEL_ID",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Email::INVALID_LABEL_ID))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "Email", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "Email", result);
    return exports;
}

napi_value DeclareEventConst(napi_env env, napi_value exports)
{
    // Event
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CUSTOM_LABEL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Event::CUSTOM_LABEL))),
        DECLARE_NAPI_STATIC_PROPERTY("EVENT_ANNIVERSARY",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Event::EVENT_ANNIVERSARY))),
        DECLARE_NAPI_STATIC_PROPERTY("EVENT_OTHER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Event::EVENT_OTHER))),
        DECLARE_NAPI_STATIC_PROPERTY("EVENT_BIRTHDAY",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Event::EVENT_BIRTHDAY))),
        DECLARE_NAPI_STATIC_PROPERTY("INVALID_LABEL_ID",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Event::INVALID_LABEL_ID))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "Event", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "Event", result);
    return exports;
}

napi_value DeclareImAddressConst(napi_env env, napi_value exports)
{
    // ImAddress
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CUSTOM_LABEL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::CUSTOM_LABEL))),
        DECLARE_NAPI_STATIC_PROPERTY("IM_AIM",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::IM_AIM))),
        DECLARE_NAPI_STATIC_PROPERTY("IM_MSN",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::IM_MSN))),
        DECLARE_NAPI_STATIC_PROPERTY("IM_YAHOO",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::IM_YAHOO))),
        DECLARE_NAPI_STATIC_PROPERTY("IM_SKYPE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::IM_SKYPE))),
        DECLARE_NAPI_STATIC_PROPERTY("IM_QQ",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::IM_QQ))),
        DECLARE_NAPI_STATIC_PROPERTY("IM_ICQ",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::IM_ICQ))),
        DECLARE_NAPI_STATIC_PROPERTY("IM_JABBER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::IM_JABBER))),
        DECLARE_NAPI_STATIC_PROPERTY("INVALID_LABEL_ID",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(ImAddress::INVALID_LABEL_ID))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "ImAddress", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "ImAddress", result);
    return exports;
}

napi_value DeclarePhoneNumberConst(napi_env env, napi_value exports)
{
    // PhoneNumber
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CUSTOM_LABEL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::CUSTOM_LABEL))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_HOME",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_HOME))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_MOBILE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_MOBILE))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_WORK",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_WORK))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_FAX_WORK",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_FAX_WORK))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_FAX_HOME",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_FAX_HOME))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_PAGER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_PAGER))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_OTHER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_OTHER))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_CALLBACK",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_CALLBACK))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_CAR",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_CAR))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_COMPANY_MAIN",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_COMPANY_MAIN))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_ISDN",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_ISDN))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_MAIN",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_MAIN))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_OTHER_FAX",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_OTHER_FAX))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_RADIO",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_RADIO))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_TELEX",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_TELEX))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_TTY_TDD",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_TTY_TDD))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_WORK_MOBILE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_WORK_MOBILE))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_WORK_PAGER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_WORK_PAGER))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_ASSISTANT",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_ASSISTANT))),
        DECLARE_NAPI_STATIC_PROPERTY("NUM_MMS",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::NUM_MMS))),
        DECLARE_NAPI_STATIC_PROPERTY("INVALID_LABEL_ID",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PhoneNumber::INVALID_LABEL_ID))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "PhoneNumber", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "PhoneNumber", result);
    return exports;
}

napi_value DeclarePostalAddressConst(napi_env env, napi_value exports)
{
    // PostalAddress
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CUSTOM_LABEL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PostalAddress::CUSTOM_LABEL))),
        DECLARE_NAPI_STATIC_PROPERTY("ADDR_HOME",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PostalAddress::ADDR_HOME))),
        DECLARE_NAPI_STATIC_PROPERTY("ADDR_WORK",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PostalAddress::ADDR_WORK))),
        DECLARE_NAPI_STATIC_PROPERTY("ADDR_OTHER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PostalAddress::ADDR_OTHER))),
        DECLARE_NAPI_STATIC_PROPERTY("INVALID_LABEL_ID",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(PostalAddress::INVALID_LABEL_ID))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "PostalAddress", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "PostalAddress", result);
    return exports;
}

napi_value DeclareRelationConst(napi_env env, napi_value exports)
{
    // Relation
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CUSTOM_LABEL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::CUSTOM_LABEL))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_ASSISTANT",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_ASSISTANT))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_BROTHER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_BROTHER))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_CHILD",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_CHILD))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_DOMESTIC_PARTNER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_DOMESTIC_PARTNER))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_FATHER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_FATHER))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_FRIEND",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_FRIEND))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_MANAGER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_MANAGER))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_MOTHER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_MOTHER))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_PARENT",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_PARENT))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_PARTNER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_PARTNER))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_REFERRED_BY",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_REFERRED_BY))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_RELATIVE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_RELATIVE))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_SISTER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_SISTER))),
        DECLARE_NAPI_STATIC_PROPERTY("RELATION_SPOUSE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::RELATION_SPOUSE))),
        DECLARE_NAPI_STATIC_PROPERTY("INVALID_LABEL_ID",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Relation::INVALID_LABEL_ID))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "Relation", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "Relation", result);
    return exports;
}

napi_value DeclareSipAddressConst(napi_env env, napi_value exports)
{
    // SipAddress
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CUSTOM_LABEL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(SipAddress::CUSTOM_LABEL))),
        DECLARE_NAPI_STATIC_PROPERTY("SIP_HOME",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(SipAddress::SIP_HOME))),
        DECLARE_NAPI_STATIC_PROPERTY("SIP_WORK",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(SipAddress::SIP_WORK))),
        DECLARE_NAPI_STATIC_PROPERTY("SIP_OTHER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(SipAddress::SIP_OTHER))),
        DECLARE_NAPI_STATIC_PROPERTY("INVALID_LABEL_ID",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(SipAddress::INVALID_LABEL_ID))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "SipAddress", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "SipAddress", result);
    return exports;
}

napi_value DeclareAttributeConst(napi_env env, napi_value exports)
{
    // Attribute
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_CONTACT_EVENT",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_CONTACT_EVENT))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_EMAIL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_EMAIL))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_GROUP_MEMBERSHIP",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_GROUP_MEMBERSHIP))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_IM",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_IM))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_NAME",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_NAME))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_NICKNAME",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_NICKNAME))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_NOTE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_NOTE))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_ORGANIZATION",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_ORGANIZATION))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_PHONE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_PHONE))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_PORTRAIT",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_PORTRAIT))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_POSTAL_ADDRESS",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_POSTAL_ADDRESS))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_RELATION",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_RELATION))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_SIP_ADDRESS",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_SIP_ADDRESS))),
        DECLARE_NAPI_STATIC_PROPERTY("ATTR_WEBSITE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(Attribute::ATTR_WEBSITE))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "Attribute", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "Attribute", result);
    return exports;
}

napi_value DeclareFilterConditionConst(napi_env env, napi_value exports)
{
    // FilterCondition
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("IS_NOT_NULL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterCondition::IS_NOT_NULL))),
        DECLARE_NAPI_STATIC_PROPERTY("EQUAL_TO",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterCondition::EQUAL_TO))),
        DECLARE_NAPI_STATIC_PROPERTY("NOT_EQUAL_TO",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterCondition::NOT_EQUAL_TO))),
        DECLARE_NAPI_STATIC_PROPERTY("IN",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterCondition::IN))),
        DECLARE_NAPI_STATIC_PROPERTY("NOT_IN",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterCondition::NOT_IN))),
        DECLARE_NAPI_STATIC_PROPERTY("CONTAINS",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterCondition::CONTAINS))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "FilterCondition", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "FilterCondition", result);
    return exports;
}

napi_value DeclareDataFieldConst(napi_env env, napi_value exports)
{
    // DataField
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("EMAIL",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(DataField::EMAIL))),
        DECLARE_NAPI_STATIC_PROPERTY("PHONE",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(DataField::PHONE))),
        DECLARE_NAPI_STATIC_PROPERTY("ORGANIZATION",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(DataField::ORGANIZATION))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "DataField", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "DataField", result);
    return exports;
}

napi_value DeclareFilterTypeConst(napi_env env, napi_value exports)
{
    // FilterType
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("SHOW_FILTER",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterType::SHOW_FILTER))),
        DECLARE_NAPI_STATIC_PROPERTY("DEFAULT_SELECT",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterType::DEFAULT_SELECT))),
        DECLARE_NAPI_STATIC_PROPERTY("SHOW_FILTER_AND_DEFAULT_SELECT",
            ContactsNapiUtils::ToInt32Value(env, static_cast<int32_t>(FilterType::SHOW_FILTER_AND_DEFAULT_SELECT))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "FilterType", NAPI_AUTO_LENGTH, ContactsNapiUtils::CreateClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "FilterType", result);
    return exports;
}

void Init(napi_env env, napi_value exports)
{
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_FUNCTION("addContact", OHOS::ContactsApi::AddContact),
        DECLARE_NAPI_FUNCTION("addContacts", OHOS::ContactsApi::AddContacts),
        DECLARE_NAPI_FUNCTION("deleteContact", OHOS::ContactsApi::DeleteContact),
        DECLARE_NAPI_FUNCTION("updateContact", OHOS::ContactsApi::UpdateContact),
        DECLARE_NAPI_FUNCTION("queryContactsCount", OHOS::ContactsApi::QueryContactsCount),
        DECLARE_NAPI_FUNCTION("queryContact", OHOS::ContactsApi::QueryContact),
        DECLARE_NAPI_FUNCTION("queryContacts", OHOS::ContactsApi::QueryContacts),
        DECLARE_NAPI_FUNCTION("queryContactsByEmail", OHOS::ContactsApi::QueryContactsByEmail),
        DECLARE_NAPI_FUNCTION("queryContactsByPhoneNumber", OHOS::ContactsApi::QueryContactsByPhoneNumber),
        DECLARE_NAPI_FUNCTION("queryGroups", OHOS::ContactsApi::QueryGroups),
        DECLARE_NAPI_FUNCTION("queryHolders", OHOS::ContactsApi::QueryHolders),
        DECLARE_NAPI_FUNCTION("queryKey", OHOS::ContactsApi::QueryKey),
        DECLARE_NAPI_FUNCTION("queryMyCard", OHOS::ContactsApi::QueryMyCard),
        DECLARE_NAPI_FUNCTION("isMyCard", OHOS::ContactsApi::IsMyCard),
        DECLARE_NAPI_FUNCTION("isLocalContact", OHOS::ContactsApi::IsLocalContact),
        DECLARE_NAPI_FUNCTION("startContactsPicker", OHOS::ContactsApi::ContactsPickerSelect),
        DECLARE_NAPI_FUNCTION("startSaveContactsPicker", OHOS::ContactsApi::ContactsPickerSave),
        DECLARE_NAPI_FUNCTION("startSaveExistContactsPicker", OHOS::ContactsApi::ContactsPickerSaveExist)
    };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    // Declare class const initialization
    DeclareContactConst(env, exports);
    DeclareEmailConst(env, exports);
    DeclareEventConst(env, exports);
    DeclareImAddressConst(env, exports);
    DeclarePhoneNumberConst(env, exports);
    DeclarePostalAddressConst(env, exports);
    DeclareRelationConst(env, exports);
    DeclareSipAddressConst(env, exports);
    DeclareAttributeConst(env, exports);
    DeclareFilterConditionConst(env, exports);
    DeclareDataFieldConst(env, exports);
    DeclareFilterTypeConst(env, exports);
}
} // namespace ContactsApi
} // namespace OHOS
