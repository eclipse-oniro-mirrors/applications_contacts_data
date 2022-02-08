/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef CONTACT_API_H
#define CONTACT_API_H

#include "contacts_build.h"
#include "contacts_napi_object.h"

namespace OHOS {
namespace ContactsApi {
void Init(napi_env env, napi_value exports);
napi_value AddContact(napi_env env, napi_callback_info info);
napi_value DeleteContact(napi_env env, napi_callback_info info);
napi_value UpdateContact(napi_env env, napi_callback_info info);
napi_value QueryContact(napi_env env, napi_callback_info info);
napi_value QueryContacts(napi_env env, napi_callback_info info);
napi_value QueryContactsByEmail(napi_env env, napi_callback_info info);
napi_value QueryContactsByPhoneNumber(napi_env env, napi_callback_info info);
napi_value QueryGroups(napi_env env, napi_callback_info info);
napi_value QueryHolders(napi_env env, napi_callback_info info);
napi_value QueryKey(napi_env env, napi_callback_info info);
napi_value QueryMyCard(napi_env env, napi_callback_info info);
napi_value IsMyCard(napi_env env, napi_callback_info info);
napi_value IsLocalContact(napi_env env, napi_callback_info info);
void Execute(napi_env env, void *data);
void ExecuteAsync(napi_env env, void *data);
void ExecuteDone(napi_env env, napi_status status, void *data);
void ExecuteSyncDone(napi_env env, napi_status status, void *data);
int GetMethodType(napi_env env, napi_callback_info info);
int GetRawIdByResultSet(const std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet);
napi_value CreateAsyncWork(napi_env env, ExecuteHelper *executeHelper, napi_callback_info info);
napi_value LocalExecute(napi_env env, ExecuteHelper *executeHelper);
void LocalExecuteAddContact(napi_env env, ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteDeleteContact(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteQueryContact(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteQueryContactsOrKey(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteQueryContactsByData(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteQueryGroup(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteQueryHolders(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteQueryMyCard(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteUpdateContact(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteIsLocalContact(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
void LocalExecuteIsMyCard(napi_env env, const ExecuteHelper *executeHelper, napi_value &result);
napi_value Scheduling(napi_env env, napi_callback_info info, ExecuteHelper *executeHelper, int actionCode);
void SetChildActionCodeAndConvertParams(napi_env env, napi_callback_info info, ExecuteHelper *executeHelper);
NativeRdb::DataAbilityPredicates ConvertParamsSwitchSplit(
    int code, napi_env env, napi_value &key, napi_value &hold, napi_value &attr);
std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> GetDataAbilityHelper(napi_env env);
void HolderPredicates(const Holder &holder, NativeRdb::DataAbilityPredicates &predicates);
void AttributesPredicates(ContactAttributes &attrs, NativeRdb::DataAbilityPredicates &predicates);
NativeRdb::DataAbilityPredicates BuildQureyContactPredicates(
    napi_env env, std::string key, napi_value hold, napi_value attr);
NativeRdb::DataAbilityPredicates BuildUpdateContactPredicates(napi_env env, napi_value contacts, napi_value attr);
NativeRdb::DataAbilityPredicates BuildQuerytContactData(napi_env env, napi_value &contactsObject,
    napi_value &attrObject, std::vector<NativeRdb::ValuesBucket> &valueContactData);
std::vector<std::string> BuildUpdateContactColumns();
void BuildUpdateContactConvertParams(
    napi_env env, napi_value &contacts, napi_value &attr, ExecuteHelper &executeHelper);
NativeRdb::DataAbilityPredicates BuildQureyContactsPredicates(napi_env env, napi_value hold, napi_value attr);
NativeRdb::DataAbilityPredicates BuildQureyContactsByEmailPredicates(
    napi_env env, std::string email, napi_value hold, napi_value attr);
NativeRdb::DataAbilityPredicates BuildQureyContactsByPhoneNumberPredicates(
    napi_env env, std::string phoneNumber, napi_value hold, napi_value attr);
NativeRdb::DataAbilityPredicates BuildQureyGroupsPredicates(napi_env env, napi_value hold);
NativeRdb::DataAbilityPredicates BuildQureyKeyPredicates(napi_env env, int id, napi_value hold);
NativeRdb::DataAbilityPredicates BuildQueryMyCardPredicates(napi_env env, napi_value attr);
NativeRdb::DataAbilityPredicates BuildIsLocalContactPredicates(napi_env env, napi_value id);
NativeRdb::DataAbilityPredicates BuildIsMyCardPredicates(napi_env env, napi_value id);
NativeRdb::DataAbilityPredicates BuildDeleteContactDataPredicates(napi_env env, int rawId, napi_value attr);
NativeRdb::DataAbilityPredicates BuildDeleteContactPredicates(napi_env env, napi_callback_info info);
void ObjectInit(napi_env env, napi_value object, napi_value &hold, napi_value &attr, napi_value &contacts);
void ObjectInitId(napi_env env, napi_value object, napi_value &id);
void ObjectInitString(napi_env env, napi_value object, napi_value &key);
int GetType(napi_env env, napi_value value);
} // namespace ContactsApi
} // namespace OHOS

#endif // CONTACT_API_H
