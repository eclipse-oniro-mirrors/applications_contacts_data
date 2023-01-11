/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "calllog_ability.h"
#include "common.h"
#include "contacts_datashare_stub_impl.h"
#include "dataobs_mgr_client.h"
#include "hilog_wrapper.h"
#include "voicemail_ability.h"

namespace OHOS {
namespace DataShare {
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;

std::shared_ptr<DataShareExtAbility> ContactsDataShareStubImpl::GetContactsDataAbility()
{
    return contactsDataAbility_;
}

std::shared_ptr<DataShareExtAbility> ContactsDataShareStubImpl::GetCallLogAbility()
{
    if (callLogAbility_ == nullptr) {
        callLogAbility_.reset(CallLogAbility::Create());
    }
    return callLogAbility_;
}

std::shared_ptr<DataShareExtAbility> ContactsDataShareStubImpl::GetVoiceMailAbility()
{
    if (voiceMailAbility_ == nullptr) {
        voiceMailAbility_.reset(VoiceMailAbility::Create());
    }
    return voiceMailAbility_;
}

std::shared_ptr<DataShareExtAbility> ContactsDataShareStubImpl::GetOwner(const Uri &uri)
{
    OHOS::Uri uriTemp = uri;
    std::string path = uriTemp.GetPath();
    if (path.find("com.ohos.contactsdataability") != std::string::npos) {
        return GetContactsDataAbility();
    }
    if (path.find("com.ohos.calllogability") != std::string::npos) {
        return GetCallLogAbility();
    }
    if (path.find("com.ohos.voicemailability") != std::string::npos) {
        return GetVoiceMailAbility();
    }
    return nullptr;
}

int ContactsDataShareStubImpl::Insert(const Uri &uri, const DataShareValuesBucket &value)
{
    HILOG_INFO("insert begin.");
    int ret = 0;
    auto client = sptr<ContactsDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("insert failed, extension is null.");
        return ret;
    }
    ret = extension->Insert(uri, value);
    HILOG_INFO("insert end successfully. ret: %{public}d", ret);
    if (ret != Contacts::OPERATION_ERROR) {
        NotifyChange(uri);
    }
    return ret;
}

int ContactsDataShareStubImpl::Update(const Uri &uri, const DataSharePredicates &predicates,
    const DataShareValuesBucket &value)
{
    HILOG_INFO("update begin.");
    int ret = 0;
    auto client = sptr<ContactsDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("update failed, extension is null.");
        return ret;
    }
    ret = extension->Update(uri, predicates, value);
    HILOG_INFO("update end successfully. ret: %{public}d", ret);
    if (ret != Contacts::OPERATION_ERROR) {
        NotifyChange(uri);
    }
    return ret;
}

int ContactsDataShareStubImpl::Delete(const Uri &uri, const DataSharePredicates &predicates)
{
    HILOG_INFO("delete begin.");
    int ret = 0;
    auto client = sptr<ContactsDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("delete failed, extension is null.");
        return ret;
    }
    ret = extension->Delete(uri, predicates);
    HILOG_INFO("delete end successfully. ret: %{public}d", ret);
    if (ret != Contacts::OPERATION_ERROR) {
        NotifyChange(uri);
    }
    return ret;
}

std::shared_ptr<DataShareResultSet> ContactsDataShareStubImpl::Query(const Uri &uri,
    const DataSharePredicates &predicates, std::vector<std::string> &columns)
{
    HILOG_INFO("query begin.");
    std::shared_ptr<DataShareResultSet> resultSet = nullptr;
    auto client = sptr<ContactsDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("query failed, extension is null.");
        return nullptr;
    }
    resultSet = extension->Query(uri, predicates, columns);
    HILOG_INFO("query end successfully.");
    return resultSet;
}

int ContactsDataShareStubImpl::BatchInsert(const Uri &uri, const std::vector<DataShareValuesBucket> &values)
{
    HILOG_INFO("batch insert begin.");
    int ret = 0;
    auto client = sptr<ContactsDataShareStubImpl>(this);
    auto extension = client->GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("batch insert failed, extension is null.");
        return ret;
    }
    ret = extension->BatchInsert(uri, values);
    HILOG_INFO("batch insert end successfully. ret: %{public}d", ret);
    if (ret != Contacts::OPERATION_ERROR) {
        NotifyChange(uri);
    }
    return ret;
}

std::vector<std::string> ContactsDataShareStubImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_INFO("GetFileTypes not supported");
    std::vector<std::string> result;
    return result;
}

int ContactsDataShareStubImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("OpenFile not supported");
    return -1;
}

int ContactsDataShareStubImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("OpenRawFile not supported");
    return -1;
}

std::string ContactsDataShareStubImpl::GetType(const Uri &uri)
{
    HILOG_INFO("GetType not supported");
    return "";
}

bool ContactsDataShareStubImpl::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->RegisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->RegisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

bool ContactsDataShareStubImpl::UnregisterObserver(const Uri &uri,
    const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->UnregisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->UnregisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

bool ContactsDataShareStubImpl::NotifyChange(const Uri &uri)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->NotifyChange(uri);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->NotifyChange error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

Uri ContactsDataShareStubImpl::NormalizeUri(const Uri &uri)
{
    HILOG_INFO("NormalizeUri not supported");
    return uri;
}

Uri ContactsDataShareStubImpl::DenormalizeUri(const Uri &uri)
{
    HILOG_INFO("DenormalizeUri not supported");
    return uri;
}
} // namespace DataShare
} // namespace OHOS
