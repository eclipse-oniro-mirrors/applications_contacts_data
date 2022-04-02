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

#include "contacts_control.h"

#include "hilog_wrapper_api.h"

namespace OHOS {
namespace ContactsApi {
ContactsControl::ContactsControl(void)
{
}

ContactsControl::~ContactsControl()
{
}

int64_t ContactsControl::RawContactInsert(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
    OHOS::NativeRdb::ValuesBucket rawContactValues)
{
    int code = 0;
    OHOS::Uri uriRawContact("dataability:///com.ohos.contactsdataability/contacts/raw_contact");
    code = dataAbilityHelper->Insert(uriRawContact, rawContactValues);
    HILOG_INFO(" ContactsControl::RawContactInsert insert code %{public}d", code);
    rawContactValues.Clear();
    return code;
}

int ContactsControl::ContactDataInsert(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
    std::vector<OHOS::NativeRdb::ValuesBucket> contactDataValues)
{
    int code = 0;
    OHOS::Uri uriContactData("dataability:///com.ohos.contactsdataability/contacts/contact_data");
    code = dataAbilityHelper->BatchInsert(uriContactData, contactDataValues);
    HILOG_INFO(" ContactsControl::ContactDataInsert insert code %{public}d", code);
    contactDataValues.clear();
    return code;
}
int ContactsControl::ContactDataDelete(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
    OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    int code = 0;
    OHOS::Uri uriContactData("dataability:///com.ohos.contactsdataability/contacts/contact_data");
    code = dataAbilityHelper->Delete(uriContactData, predicates);
    HILOG_INFO(" ContactsControl::ContactDataDelete insert code %{public}d", code);
    return code;
}

int ContactsControl::RawContactUpdate(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
    OHOS::NativeRdb::ValuesBucket updateValues, OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    int code = 0;
    OHOS::Uri uriRawContact("dataability:///com.ohos.contactsdataability/contacts/raw_contact");
    code = dataAbilityHelper->Update(uriRawContact, updateValues, predicates);
    return code;
}

int ContactsControl::ContactDataUpdate(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
    OHOS::NativeRdb::ValuesBucket updateValues, OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    int code = 0;
    OHOS::Uri uriContactData("dataability:///com.ohos.contactsdataability/contacts/contact_data");
    code = dataAbilityHelper->Update(uriContactData, updateValues, predicates);
    return code;
}

int ContactsControl::ContactDelete(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
    OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    int code = 0;
    OHOS::Uri uriContact("dataability:///com.ohos.contactsdataability/contacts/contact");
    code = dataAbilityHelper->Delete(uriContact, predicates);
    return code;
}

std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactsControl::HolderQuery(
    std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
    OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet;
    OHOS::Uri uriContact("dataability:///com.ohos.contactsdataability/contacts/account");
    resultSet = dataAbilityHelper->Query(uriContact, columns, predicates);
    return resultSet;
}

std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactsControl::ContactQuery(
    std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
    OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    HILOG_INFO("ContactsControl::ContactQuery is start");
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet;
    OHOS::Uri uriContact("dataability:///com.ohos.contactsdataability/contacts/contact_data");
    resultSet = dataAbilityHelper->Query(uriContact, columns, predicates);
    return resultSet;
}

std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactsControl::ContactDataQuery(
    std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
    OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet;
    OHOS::Uri uriContactData("dataability:///com.ohos.contactsdataability/contacts/contact_data");
    resultSet = dataAbilityHelper->Query(uriContactData, columns, predicates);
    return resultSet;
}

std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactsControl::GroupsQuery(
    std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
    OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet;
    OHOS::Uri uriGroups("dataability:///com.ohos.contactsdataability/contacts/groups");
    resultSet = dataAbilityHelper->Query(uriGroups, columns, predicates);
    return resultSet;
}

std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactsControl::MyCardQuery(
    std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
    OHOS::NativeRdb::DataAbilityPredicates predicates)
{
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> resultSet;
    OHOS::Uri uriProfileContact("dataability:///com.ohos.contactsdataability/profile/contact_data");
    resultSet = dataAbilityHelper->Query(uriProfileContact, columns, predicates);
    return resultSet;
}
} // namespace ContactsApi
} // namespace OHOS