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

#include "contacts_api.h"

#include <mutex>

#include "ability.h"
#include "ability_context.h"
#include "context.h"
#include "data_ability_helper.h"
#include "data_ability_predicates.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"
#include "securec.h"

#include "contacts_control.h"
#include "contacts_napi_common.h"
#include "hilog_wrapper_api.h"
#include "result_convert.h"

namespace OHOS {
namespace ContactsApi {
namespace {
std::mutex g_mutex;
}
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
            HILOG_ERROR("ObjectInit type is error");
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
            HILOG_ERROR("ObjectInitId type is error");
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
            HILOG_ERROR("ObjectInitString type is error");
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
            napi_create_string_utf8(env, "id", NAPI_AUTO_LENGTH, &key);
            napi_has_property(env, value, key, &result);
            if (result) {
                return TYPE_CONTACT;
            }
            return TYPE_NAPI_ERROR;
            break;
        default:
            return TYPE_NAPI_ERROR;
            break;
    }
}

std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> g_dataAbilityHelper = nullptr;
/**
 * @brief Get dataAbilityHelper
 *
 * @param env Conditions for get dataAbilityHelper operation
 *
 * @return The result returned by get dataAbilityHelper
 */
std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> GetDataAbilityHelper(napi_env env)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));
    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));
    if (abilityObj == nullptr) {
        HILOG_ERROR("abilityObj is nullptr!");
    }
    OHOS::AppExecFwk::Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr!");
    }
    std::shared_ptr<OHOS::Uri> uriPtr = std::make_shared<OHOS::Uri>("dataability:///com.ohos.contactsdataability");
    if (g_dataAbilityHelper == nullptr) {
        HILOG_ERROR("g_dataAbilityHelper is not nullptr!");
        g_dataAbilityHelper = OHOS::AppExecFwk::DataAbilityHelper::Creator(ability->GetContext(), uriPtr);
    }
    return g_dataAbilityHelper;
}

/**
 * @brief Establish predicates condition by holder object
 *
 * @param holder Conditions for establish predicates operation
 * @param predicates Conditions for establish predicates operation
 */
void HolderPredicates(Holder &holder, NativeRdb::DataAbilityPredicates &predicates)
{
    if (!holder.bundleName.empty()) {
        predicates.And();
        predicates.EqualTo("account_type", holder.bundleName);
    }
    if (!holder.displayName.empty()) {
        predicates.And();
        predicates.EqualTo("account_name", holder.displayName);
    }
    if (holder.holderId > 0) {
        predicates.And();
        predicates.EqualTo("account_id", std::to_string(holder.holderId));
    }
}

/**
 * @brief Establish predicates condition by attributes object
 *
 * @param attrs Conditions for establish predicates operation
 * @param predicates Conditions for establish predicates operation
 */
void AttributesPredicates(ContactAttributes &attrs, NativeRdb::DataAbilityPredicates &predicates)
{
    unsigned int size = attrs.attributes.size();
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
}

/**
 * @brief Resolve object interface in DELETE_CONTACT case
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 */
NativeRdb::DataAbilityPredicates BuildDeleteContactPredicates(napi_env env, napi_callback_info info)
{
    NativeRdb::DataAbilityPredicates predicates;
    napi_value argv[MAX_PARAMS];
    size_t argc = MAX_PARAMS;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    ContactsBuild contactsBuild;
    std::string keyValue = contactsBuild.NapiGetValueString(env, argv[0]);
    if (!keyValue.empty()) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("quick_search_key", keyValue);
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
NativeRdb::DataAbilityPredicates BuildQueryContactPredicates(
    napi_env env, napi_value key, napi_value hold, napi_value attr)
{
    ContactsBuild contactsBuild;
    std::string keyValue = contactsBuild.NapiGetValueString(env, key);
    Holder holder = contactsBuild.GetHolder(env, hold);
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    NativeRdb::DataAbilityPredicates predicates;
    if (!keyValue.empty()) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("quick_search_key", keyValue);
        HolderPredicates(holder, predicates);
        AttributesPredicates(attrs, predicates);
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
    if (holder.holderId > 0) {
        holders["account_id"] = std::to_string(holder.holderId);
    }
}

/**
 * @brief Resolve object interface in QUERY_CONTACTS case
 *
 * @param env Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 * @param attr Conditions for resolve object interface operation
 */
NativeRdb::DataAbilityPredicates BuildQueryContactsPredicates(napi_env env, napi_value hold, napi_value attr)
{
    ContactsBuild contactsBuild;
    Holder holder = contactsBuild.GetHolder(env, hold);
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    NativeRdb::DataAbilityPredicates predicates;
    std::map<std::string, std::string> holders;
    HoldersStructure(holders, holder);
    unsigned int size = attrs.attributes.size();
    unsigned int mapSize = holders.size();
    std::map<std::string, std::string>::iterator it;
    for (it = holders.begin(); it != holders.end(); ++it) {
        predicates.EqualTo(it->first, it->second);
        if (it != holders.end()--) {
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
NativeRdb::DataAbilityPredicates BuildQueryContactsByEmailPredicates(
    napi_env env, napi_value emailobject, napi_value hold, napi_value attr)
{
    ContactsBuild contactsBuild;
    std::string email = contactsBuild.NapiGetValueString(env, emailobject);
    Holder holder = contactsBuild.GetHolder(env, hold);
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    NativeRdb::DataAbilityPredicates predicates;
    if (!email.empty() || email != "") {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("detail_info", email);
        predicates.And();
        predicates.EqualTo("content_type", "email");
        HolderPredicates(holder, predicates);
        AttributesPredicates(attrs, predicates);
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
NativeRdb::DataAbilityPredicates BuildQueryContactsByPhoneNumberPredicates(
    napi_env env, napi_value number, napi_value hold, napi_value attr)
{
    ContactsBuild contactsBuild;
    std::string phoneNumber = contactsBuild.NapiGetValueString(env, number);
    Holder holder = contactsBuild.GetHolder(env, hold);
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    NativeRdb::DataAbilityPredicates predicates;
    if (!phoneNumber.empty() || phoneNumber != "") {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("detail_info", phoneNumber);
        predicates.And();
        predicates.EqualTo("content_type", "phone");
        HolderPredicates(holder, predicates);
        AttributesPredicates(attrs, predicates);
    }
    return predicates;
}

/**
 * @brief Resolve object interface in QUERY_GROUPS case
 *
 * @param env Conditions for resolve object interface operation
 * @param hold Conditions for resolve object interface operation
 */
NativeRdb::DataAbilityPredicates BuildQueryGroupsPredicates(napi_env env, napi_value hold)
{
    ContactsBuild contactsBuild;
    Holder holder = contactsBuild.GetHolder(env, hold);
    NativeRdb::DataAbilityPredicates predicates;
    std::map<std::string, std::string> holders;
    HoldersStructure(holders, holder);
    predicates.EqualTo("is_deleted", "0");
    unsigned int size = holders.size();
    if (size > 0) {
        predicates.And();
    }
    std::map<std::string, std::string>::iterator it;
    for (it = holders.begin(); it != holders.end(); ++it) {
        predicates.EqualTo(it->first, it->second);
        if (it != holders.end()--) {
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
NativeRdb::DataAbilityPredicates BuildQueryKeyPredicates(napi_env env, napi_value id, napi_value hold)
{
    ContactsBuild contactsBuild;
    int value = contactsBuild.GetInt(env, id);
    Holder holder = contactsBuild.GetHolder(env, hold);
    NativeRdb::DataAbilityPredicates predicates;
    if (value != 0) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", std::to_string(value));
        HolderPredicates(holder, predicates);
    }
    return predicates;
}

/**
 * @brief Resolve object interface in QUERY_MY_CARD case
 *
 * @param env Conditions for resolve object interface operation
 * @param attr Conditions for resolve object interface operation
 */
NativeRdb::DataAbilityPredicates BuildQueryMyCardPredicates(napi_env env, napi_value attr)
{
    ContactsBuild contactsBuild;
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    NativeRdb::DataAbilityPredicates predicates;
    unsigned int size = attrs.attributes.size();
    predicates.EqualTo("is_deleted", "0");
    if (size > 0) {
        predicates.And();
    }
    for (unsigned int i = 0; i < size; ++i) {
        predicates.EqualTo("type_id", std::to_string(attrs.attributes[i]));
        if (i != size - 1) {
            predicates.Or();
        }
    }
    return predicates;
}

NativeRdb::DataAbilityPredicates BuildQueryContactData(napi_env env, napi_value &contactObject, napi_value &attrObject,
    std::vector<NativeRdb::ValuesBucket> &valueContactData)
{
    ContactsBuild contactsBuild;
    Contacts contact;
    contactsBuild.GetContactDataByObject(env, contactObject, contact);
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attrObject);
    NativeRdb::DataAbilityPredicates predicates;
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
        contactsBuild.BuildValueContactDataByType(contact, attrs.attributes[i], valueContactData);
    }
    return predicates;
}

std::vector<std::string> BuildUpdateContactColumns()
{
    std::vector<std::string> columns;
    columns.push_back("raw_contact_id");
    return columns;
}

int GetRawIdByResultSet(const std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet)
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

void BuildUpdateContactConvertParams(napi_env env, napi_value &contact, napi_value &attr, ExecuteHelper *executeHelper)
{
    executeHelper->valueContactData.clear();
    NativeRdb::DataAbilityPredicates predicates =
        BuildQueryContactData(env, contact, attr, executeHelper->valueContactData);
    executeHelper->columns = BuildUpdateContactColumns();
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet =
        contactsControl.ContactDataQuery(GetDataAbilityHelper(env), executeHelper->columns, predicates);
    int rawId = GetRawIdByResultSet(resultSet);
    std::vector<NativeRdb::ValuesBucket> value = executeHelper->valueContactData;
    unsigned int size = value.size();
    for (unsigned int i = 0; i < size; ++i) {
        (executeHelper->valueContactData)[i].PutInt("raw_contact_id", rawId);
    }
    executeHelper->predicates = BuildDeleteContactDataPredicates(env, rawId, attr);
}

NativeRdb::DataAbilityPredicates BuildDeleteContactDataPredicates(napi_env env, int rawId, napi_value attr)
{
    ContactsBuild contactsBuild;
    ContactAttributes attrs = contactsBuild.GetContactAttributes(env, attr);
    NativeRdb::DataAbilityPredicates predicates;
    if (rawId != 0) {
        predicates.EqualTo("raw_contact_id", std::to_string(rawId));
        AttributesPredicates(attrs, predicates);
    }
    return predicates;
}

/**
 * @brief Resolve object interface in IS_LOCAL_CONTACT case
 *
 * @param env Conditions for resolve object interface operation
 * @param id Conditions for resolve object interface operation
 */
NativeRdb::DataAbilityPredicates BuildIsLocalContactPredicates(napi_env env, napi_value id)
{
    ContactsBuild contactsBuild;
    int value = contactsBuild.GetInt(env, id);
    NativeRdb::DataAbilityPredicates predicates;
    if (value != 0) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", std::to_string(value));
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
NativeRdb::DataAbilityPredicates BuildIsMyCardPredicates(napi_env env, napi_value id)
{
    ContactsBuild contactsBuild;
    int value = contactsBuild.GetInt(env, id);
    NativeRdb::DataAbilityPredicates predicates;
    if (value != 0) {
        predicates.EqualTo("is_deleted", "0");
        predicates.And();
        predicates.EqualTo("contact_id", std::to_string(value));
    }
    return predicates;
}

void ExecuteDone(napi_env env, napi_status status, void *data)
{
    ExecuteHelper *executeHelper = reinterpret_cast<ExecuteHelper *>(data);
    napi_value result = executeHelper->dataValue;
    executeHelper->dataValue = nullptr;
    napi_deferred deferred = executeHelper->deferred;
    executeHelper->deferred = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, executeHelper->work));
    delete executeHelper;
    executeHelper = nullptr;
}

void ExecuteSyncDone(napi_env env, napi_status status, void *data)
{
    if (status != napi_ok) {
        HILOG_ERROR("ExecuteSyncDone status is not ok===>");
        return;
    }
    if (data != nullptr) {
        ExecuteHelper *executeHelper = reinterpret_cast<ExecuteHelper *>(data);
        napi_value global;
        napi_get_global(env, &global);
        napi_value resultData[1];
        resultData[0] = executeHelper->dataValue;
        executeHelper->dataValue = nullptr;
        napi_value result;
        napi_value callBack;
        napi_get_reference_value(env, executeHelper->callBack, &callBack);
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, callBack, &valuetype);
        if (valuetype != napi_function) {
            HILOG_ERROR("contactApi params not is function");
            return;
        }
        napi_call_function(env, global, callBack, 1, resultData, &result);
        if (executeHelper->work != nullptr) {
            napi_delete_async_work(env, executeHelper->work);
        }
        executeHelper->work = nullptr;
        executeHelper->deferred = nullptr;
        delete executeHelper;
    }
    HILOG_INFO("contactApi ExecuteSyncDone done===>");
}

void LocalExecuteAddContact(napi_env env, ExecuteHelper *executeHelper, napi_value &result)
{
    ContactsControl contactsControl;
    int64_t rawId =
        contactsControl.RawContactInsert(executeHelper->dataAbilityHelper, (executeHelper->valueContact)[0]);
    std::vector<NativeRdb::ValuesBucket> value = executeHelper->valueContactData;
    unsigned int size = value.size();
    for (unsigned int i = 0; i < size; ++i) {
        (executeHelper->valueContactData)[i].PutInt("raw_contact_id", rawId);
    }
    int code = contactsControl.ContactDataInsert(executeHelper->dataAbilityHelper, executeHelper->valueContactData);
    if (code == 0) {
        napi_create_int64(env, rawId, &result);
    } else {
        napi_create_int64(env, code, &result);
    }
}

void LocalExecuteDeleteContact(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    ContactsControl contactsControl;
    int ret = contactsControl.ContactDelete(executeHelper->dataAbilityHelper, executeHelper->predicates);
    HILOG_INFO("LocalExecuteDeleteContact contact ret = %{public}d", ret);
    napi_create_int64(env, ret, &result);
}

void LocalExecuteQueryContact(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    ResultConvert resultConvert;
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = contactsControl.ContactQuery(
        executeHelper->dataAbilityHelper, executeHelper->columns, executeHelper->predicates);
    napi_value results = resultConvert.ResultSetToObject(env, resultSet);
    if (results != nullptr) {
        napi_get_element(env, results, 0, &result);
    }
}

void LocalExecuteQueryContactsOrKey(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    ResultConvert resultConvert;
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = contactsControl.ContactQuery(
        executeHelper->dataAbilityHelper, executeHelper->columns, executeHelper->predicates);
    result = resultConvert.ResultSetToObject(env, resultSet);
}

void LocalExecuteQueryContactsByData(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    ResultConvert resultConvert;
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = contactsControl.ContactDataQuery(
        executeHelper->dataAbilityHelper, executeHelper->columns, executeHelper->predicates);
    result = resultConvert.ResultSetToObject(env, resultSet);
}

void LocalExecuteQueryGroup(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    ResultConvert resultConvert;
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = contactsControl.GroupsQuery(
        executeHelper->dataAbilityHelper, executeHelper->columns, executeHelper->predicates);
    result = resultConvert.ResultSetToGroup(env, resultSet);
}

void LocalExecuteQueryHolders(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    ResultConvert resultConvert;
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = contactsControl.HolderQuery(
        executeHelper->dataAbilityHelper, executeHelper->columns, executeHelper->predicates);
    result = resultConvert.ResultSetToHolder(env, resultSet);
}

void LocalExecuteQueryMyCard(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    ResultConvert resultConvert;
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = contactsControl.MyCardQuery(
        executeHelper->dataAbilityHelper, executeHelper->columns, executeHelper->predicates);
    result = resultConvert.ResultSetToObject(env, resultSet);
}

void LocalExecuteUpdateContact(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    ContactsControl contactsControl;
    int resultCode = contactsControl.ContactDataDelete(executeHelper->dataAbilityHelper, executeHelper->predicates);
    if (resultCode == 0) {
        resultCode =
            contactsControl.ContactDataInsert(executeHelper->dataAbilityHelper, executeHelper->valueContactData);
        napi_create_int64(env, resultCode, &result);
    }
}

void LocalExecuteIsLocalContact(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    int64_t isLocal = 0;
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = contactsControl.ContactQuery(
        executeHelper->dataAbilityHelper, executeHelper->columns, executeHelper->predicates);
    if (resultSet == nullptr) {
        napi_create_int64(env, isLocal, &result);
        return;
    }
    int resultSetNum = resultSet->GoToFirstRow();
    if (resultSetNum == OHOS::NativeRdb::E_OK) {
        isLocal = 1;
    }
    napi_create_int64(env, isLocal, &result);
    resultSet->Close();
}

void LocalExecuteIsMyCard(napi_env env, const ExecuteHelper *executeHelper, napi_value &result)
{
    int64_t isLocal = 0;
    ContactsControl contactsControl;
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet = contactsControl.MyCardQuery(
        executeHelper->dataAbilityHelper, executeHelper->columns, executeHelper->predicates);
    if (resultSet == nullptr) {
        HILOG_ERROR("LocalExecute is_my_card resultSet is nullptr===>");
        napi_create_int64(env, isLocal, &result);
        return;
    }
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    int resultSetNum = resultSet->GoToFirstRow();
    if (resultSetNum == OHOS::NativeRdb::E_OK) {
        isLocal = 1;
    }
    napi_create_int64(env, isLocal, &result);
    resultSet->Close();
}

napi_value LocalExecute(napi_env env, ExecuteHelper *executeHelper)
{
    napi_value result = nullptr;
    switch (executeHelper->actionCode) {
        case ADD_CONTACT:
            LocalExecuteAddContact(env, executeHelper, result);
            break;
        case DELETE_CONTACT:
            LocalExecuteDeleteContact(env, executeHelper, result);
            break;
        case QUERY_CONTACT:
            LocalExecuteQueryContact(env, executeHelper, result);
            break;
        case QUERY_CONTACTS:
            LocalExecuteQueryContactsOrKey(env, executeHelper, result);
            break;
        case QUERY_CONTACTS_BY_EMAIL:
        case QUERY_CONTACTS_BY_PHONE_NUMBER:
            LocalExecuteQueryContactsByData(env, executeHelper, result);
            break;
        case QUERY_GROUPS:
            LocalExecuteQueryGroup(env, executeHelper, result);
            break;
        case QUERY_HOLDERS:
            LocalExecuteQueryHolders(env, executeHelper, result);
            break;
        case QUERY_KEY:
            LocalExecuteQueryContactsOrKey(env, executeHelper, result);
            break;
        case QUERY_MY_CARD:
            LocalExecuteQueryMyCard(env, executeHelper, result);
            break;
        case UPDATE_CONTACT:
            LocalExecuteUpdateContact(env, executeHelper, result);
            break;
        case IS_LOCAL_CONTACT:
            LocalExecuteIsLocalContact(env, executeHelper, result);
            break;
        case IS_MY_CARD:
            LocalExecuteIsMyCard(env, executeHelper, result);
            break;
        default:
            HILOG_INFO("LocalExecute case error===>");
            break;
    }
    return result;
}

void Execute(napi_env env, void *data)
{
    ExecuteHelper *executeHelper = static_cast<ExecuteHelper *>(data);
    executeHelper->dataValue = LocalExecute(env, executeHelper);
}

napi_value CreateAsyncWork(napi_env env, ExecuteHelper *executeHelper, napi_callback_info info)
{
    napi_value workName;
    napi_value result = nullptr;
    if (executeHelper->sync == 0) {
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &workName);
        napi_value argv[6];
        size_t argc = 6;
        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[argc - 1], &valuetype);
        if (valuetype == napi_function) {
            HILOG_INFO("AddContact  is callback function===>");
            napi_create_reference(env, argv[argc - 1], 1, &executeHelper->callBack);
        }
        napi_create_async_work(
            env, nullptr, workName, Execute, ExecuteSyncDone,
             reinterpret_cast<void *>(executeHelper), &(executeHelper->work));
        napi_get_null(env, &result);
    } else {
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &workName);
        napi_create_promise(env, &(executeHelper->deferred), &result);
        napi_create_async_work(
            env, nullptr, workName, Execute, ExecuteDone,
             reinterpret_cast<void *>(executeHelper), &(executeHelper->work));
    }
    napi_queue_async_work(env, executeHelper->work);
    executeHelper->promise = result;
    return result;
}

NativeRdb::DataAbilityPredicates ConvertParamsSwitchSplit(
    int code, napi_env env, napi_value &key, napi_value &hold, napi_value &attr)
{
    NativeRdb::DataAbilityPredicates predicates;
    switch (code) {
        case QUERY_CONTACT:
            predicates = BuildQueryContactPredicates(env, key, hold, attr);
            break;
        case QUERY_CONTACTS:
            predicates = BuildQueryContactsPredicates(env, hold, attr);
            break;
        case QUERY_CONTACTS_BY_EMAIL:
            predicates = BuildQueryContactsByEmailPredicates(env, key, hold, attr);
            break;
        case QUERY_CONTACTS_BY_PHONE_NUMBER:
            predicates = BuildQueryContactsByPhoneNumberPredicates(env, key, hold, attr);
            break;
        case QUERY_GROUPS:
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

void SetChildActionCodeAndConvertParams(napi_env env, napi_callback_info info, ExecuteHelper *executeHelper)
{
    napi_value id = nullptr;
    napi_value key = nullptr;
    napi_value hold = nullptr;
    napi_value attr = nullptr;
    napi_value contact = nullptr;
    napi_value argv[MAX_PARAMS];
    size_t argc = MAX_PARAMS;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    unsigned int size = argc;
    for (unsigned int i = 0; i < size; i++) {
        ObjectInitId(env, argv[i], id);
        ObjectInitString(env, argv[i], key);
        ObjectInit(env, argv[i], hold, attr, contact);
    }
    ContactsBuild contactsBuild;
    switch (executeHelper->actionCode) {
        case ADD_CONTACT:
            contactsBuild.GetContactData(env, info, executeHelper->valueContact, executeHelper->valueContactData);
            break;
        case DELETE_CONTACT:
            executeHelper->predicates = BuildDeleteContactPredicates(env, info);
            break;
        case UPDATE_CONTACT:
            BuildUpdateContactConvertParams(env, contact, attr, executeHelper);
            break;
        case IS_LOCAL_CONTACT:
            executeHelper->predicates = BuildIsLocalContactPredicates(env, id);
            break;
        case IS_MY_CARD:
            executeHelper->predicates = BuildIsMyCardPredicates(env, id);
            break;
        case QUERY_KEY:
            executeHelper->predicates = BuildQueryKeyPredicates(env, id, hold);
            break;
        default:
            executeHelper->predicates = ConvertParamsSwitchSplit(executeHelper->actionCode, env, key, hold, attr);
            break;
    }
}

napi_value Scheduling(napi_env env, napi_callback_info info, ExecuteHelper *executeHelper, int actionCode)
{
    napi_value argv[MAX_PARAMS];
    size_t argc = MAX_PARAMS;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    executeHelper->dataAbilityHelper = GetDataAbilityHelper(env);
    executeHelper->argc = argc;
    executeHelper->actionCode = actionCode;
    SetChildActionCodeAndConvertParams(env, info, executeHelper);
    napi_value result = CreateAsyncWork(env, executeHelper, info);
    return result;
}

int GetMethodType(napi_env env, napi_callback_info info)
{
    napi_value argv[MAX_PARAMS];
    size_t argc = MAX_PARAMS;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, argv[argc - 1], &valuetype);
    // last params is function as callback
    if (valuetype == napi_function) {
        HILOG_INFO("GetMethodType   callback function===>");
        return 0;
    }
    return 1;
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
        result = Scheduling(env, info, executeHelper, ADD_CONTACT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
        result = Scheduling(env, info, executeHelper, UPDATE_CONTACT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

/**
 * @brief Test interface SELECT_CONTACT
 *
 * @param env Conditions for resolve object interface operation
 * @param info Conditions for resolve object interface operation
 *
 * @return The result returned by test
 */
napi_value SelectContact(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
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
    ExecuteHelper *executeHelper = new (std::nothrow) ExecuteHelper();
    napi_value result = nullptr;
    if (executeHelper != nullptr) {
        executeHelper->sync = GetMethodType(env, info);
        result = Scheduling(env, info, executeHelper, IS_LOCAL_CONTACT);
        return result;
    }
    napi_create_int64(env, ERROR, &result);
    return result;
}

void Init(napi_env env, napi_value exports)
{
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_FUNCTION("addContact", OHOS::ContactsApi::AddContact),
        DECLARE_NAPI_FUNCTION("deleteContact", OHOS::ContactsApi::DeleteContact),
        DECLARE_NAPI_FUNCTION("updateContact", OHOS::ContactsApi::UpdateContact),
        DECLARE_NAPI_FUNCTION("selectContact", OHOS::ContactsApi::SelectContact),
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
    };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
}
} // namespace ContactsApi
} // namespace OHOS
