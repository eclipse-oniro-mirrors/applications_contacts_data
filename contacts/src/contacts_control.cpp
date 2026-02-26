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

int ContactsControl::RawContactInsert(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    DataShare::DataShareValuesBucket rawContactValues)
{
    int code = 0;
    OHOS::Uri uriRawContact("datashare:///com.ohos.contactsdataability/contacts/raw_contact");
    code = dataShareHelper->Insert(uriRawContact, rawContactValues);
    HILOG_INFO("ContactsControl::RawContactInsert insert code %{public}d", code);
    rawContactValues.Clear();
    return code;
}

int ContactsControl::ContactDataInsert(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    std::vector<DataShare::DataShareValuesBucket> contactDataValues)
{
    int code = 0;
    OHOS::Uri uriContactData("datashare:///com.ohos.contactsdataability/contacts/contact_data");
    code = dataShareHelper->BatchInsert(uriContactData, contactDataValues);
    HILOG_INFO("ContactsControl::ContactDataInsert insert code %{public}d", code);
    contactDataValues.clear();
    return code;
}

int ContactsControl::ContactBatchInsert(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    const std::vector<DataShare::OperationStatement> &statements, DataShare::ExecResultSet &result)
{
    int code = 0;
    std::string uri("datashare:///com.ohos.contactsdataability/contacts/applyBatch");
    code = dataShareHelper->ExecuteBatch(statements, result);
    HILOG_WARN("ContactsControl::ContactBatchInsert insert code %{public}d", code);
    return code;
}

int ContactsControl::ContactDataDelete(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    DataShare::DataSharePredicates predicates)
{
    int code = 0;
    OHOS::Uri uriContactData("datashare:///com.ohos.contactsdataability/contacts/contact_data");
    code = dataShareHelper->Delete(uriContactData, predicates);
    HILOG_INFO("ContactsControl::ContactDataDelete delete code %{public}d", code);
    return code;
}

int ContactsControl::RawContactUpdate(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    DataShare::DataShareValuesBucket updateValues, DataShare::DataSharePredicates predicates)
{
    int code = 0;
    OHOS::Uri uriRawContact("datashare:///com.ohos.contactsdataability/contacts/raw_contact");
    code = dataShareHelper->Update(uriRawContact, predicates, updateValues);
    HILOG_INFO("ContactsControl::RawContactUpdate Update code %{public}d", code);
    return code;
}

int ContactsControl::ContactDataUpdate(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    DataShare::DataShareValuesBucket updateValues, DataShare::DataSharePredicates predicates)
{
    int code = 0;
    OHOS::Uri uriContactData("datashare:///com.ohos.contactsdataability/contacts/contact_data");
    code = dataShareHelper->Update(uriContactData, predicates, updateValues);
    HILOG_INFO("ContactsControl::ContactDataUpdate Update code %{public}d", code);
    return code;
}

int ContactsControl::ContactDelete(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    DataShare::DataSharePredicates predicates)
{
    int code = 0;
    OHOS::Uri uriContact("datashare:///com.ohos.contactsdataability/contacts/contact");
    code = dataShareHelper->Delete(uriContact, predicates);
    HILOG_INFO("ContactsControl::ContactDelete Delete code %{public}d", code);
    return code;
}

std::shared_ptr<DataShare::DataShareResultSet> ContactsControl::HolderQuery(
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, std::vector<std::string> columns,
    DataShare::DataSharePredicates predicates)
{
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    OHOS::Uri uriContact("datashare:///com.ohos.contactsdataability/contacts/account");
    resultSet = dataShareHelper->Query(uriContact, predicates, columns);
    return resultSet;
}

std::shared_ptr<DataShare::DataShareResultSet> ContactsControl::ContactQuery(
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, std::vector<std::string> columns,
    DataShare::DataSharePredicates predicates)
{
    HILOG_INFO("ContactsControl::ContactQuery is start");
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    OHOS::Uri uriContact("datashare:///com.ohos.contactsdataability/contacts/contact_data");
    resultSet = dataShareHelper->Query(uriContact, predicates, columns);
    return resultSet;
}

std::shared_ptr<DataShare::DataShareResultSet> ContactsControl::QueryContactByRawContactId(
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, std::vector<std::string> &columns,
    int rawContactId)
{
    HILOG_INFO("ContactsControl::QueryContactByRawContactId is start");
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    OHOS::Uri uriContact("datashare:///com.ohos.contactsdataability/contacts/raw_contact");
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("id", rawContactId);
    resultSet = dataShareHelper->Query(uriContact, predicates, columns);
    return resultSet;
}

std::string ContactsControl::QueryAppGroupDir(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper)
{
    HILOG_INFO("ContactsControl::QueryAppGroupDir is start");
    std::string result;
    OHOS::Uri uriContact("datashare:///com.ohos.contactsdataability/contacts/get_grant_uri");
    result = dataShareHelper->GetType(uriContact);
    if (result == "") {
        HILOG_INFO("ContactsControl::QueryAppGroupDir result is empty");
    }
    return result;
}

std::shared_ptr<DataShare::DataShareResultSet> ContactsControl::ContactCountQuery(
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, std::vector<std::string> columns,
    DataShare::DataSharePredicates predicates)
{
    HILOG_INFO("ContactsControl::ContactCountQuery is start");
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    OHOS::Uri uriContact("datashare:///com.ohos.contactsdataability/contacts/contact");
    resultSet = dataShareHelper->Query(uriContact, predicates, columns);
    return resultSet;
}

std::shared_ptr<DataShare::DataShareResultSet> ContactsControl::ContactDataQuery(
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, std::vector<std::string> columns,
    DataShare::DataSharePredicates predicates)
{
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    OHOS::Uri uriContactData("datashare:///com.ohos.contactsdataability/contacts/contact_data");
    resultSet = dataShareHelper->Query(uriContactData, predicates, columns);
    return resultSet;
}

std::shared_ptr<DataShare::DataShareResultSet> ContactsControl::GroupsQuery(
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, std::vector<std::string> columns,
    DataShare::DataSharePredicates predicates)
{
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    OHOS::Uri uriGroups("datashare:///com.ohos.contactsdataability/contacts/groups");
    resultSet = dataShareHelper->Query(uriGroups, predicates, columns);
    return resultSet;
}

std::shared_ptr<DataShare::DataShareResultSet> ContactsControl::MyCardQuery(
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, std::vector<std::string> columns,
    DataShare::DataSharePredicates predicates)
{
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    OHOS::Uri uriProfileContact("datashare:///com.ohos.contactsdataability/profile/contact_data");
    resultSet = dataShareHelper->Query(uriProfileContact, predicates, columns);
    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    if (rowCount > 0) {
        HILOG_INFO("ContactsControl::MyCardQuery profile");
        return resultSet;
    }
    OHOS::Uri uriContactsContactData("datashare:///com.ohos.contactsdataability/contacts/contact_data");
    predicates.EqualTo("primary_contact", "1");
    resultSet = dataShareHelper->Query(uriContactsContactData, predicates, columns);
    HILOG_INFO("ContactsControl::MyCardQuery contacts");
    return resultSet;
}

int ContactsControl::OpenFileByDataShare(const std::string &fileName,
    const std::shared_ptr<DataShare::DataShareHelper> &dataShareHelper)
{
    OHOS::Uri uri("datashare:///com.ohos.contactsdataability/savePhoto/" + fileName);
    int result = -1;
    result = dataShareHelper->OpenFile(uri, "rw");
    HILOG_INFO("ContactsControl::OpenFile result %{public}d", result);
    return result;
}

int ContactsControl::HandleAddFailed(const std::shared_ptr<DataShare::DataShareHelper> &dataShareHelper,
    const DataShare::DataSharePredicates &predicates, const std::string &fileName)
{
    int code = 0;
    OHOS::Uri uriDeleteFile("datashare:///com.ohos.contactsdataability/addFailedDeleteFile/" + fileName);
    dataShareHelper->GetType(uriDeleteFile);
    OHOS::Uri uriDeleteContact("datashare:///com.ohos.contactsdataability/contacts/add_failed_delete");
    code = dataShareHelper->Delete(uriDeleteContact, predicates);
    HILOG_INFO("ContactsControl::HandleAddFailed code %{public}d", code);
    return code;
}
} // namespace ContactsApi
} // namespace OHOS