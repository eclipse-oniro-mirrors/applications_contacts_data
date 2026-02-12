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
#include "datashare_valuebucket_convert.h"
#include "contacts_data_ability.h"
#include "datashare_observer.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "os_account_manager.h"
#include "file_utils.h"
#include "uri_permission_manager_client.h"
#include "file_uri.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include "uri_utils.h"

namespace OHOS {
namespace DataShare {
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;
int g_operateDataTime = 200;
std::string ContactsDataShareStubImpl::bundleName_;
const std::string GROUP_ID = "group.1521646752317339712";
const std::string FILE_URI_CONTACTSDATAABILITY = "file://com.ohos.contactsdataability";
const std::string SANDBOX_PHOTO_PATH = "/data/storage/el2/base/files";
const std::string GROUP_PATH_PREFIX = "/data/storage/el2/group/";
const std::string APP_PATH_PREFIX = "/data/app/el2";
const std::string PHOTO_STR = "/photo/";
const std::string EMPTY_STR = "";
const int OPEN_FILE_FAILED = -1;

void ContactsDataShareStubImpl::SetContactsDataAbility(std::shared_ptr<DataShareExtAbility> extension)
{
    std::lock_guard<std::mutex> lock(contactsMutex_);
    contactsDataAbility_ = extension;
}

void ContactsDataShareStubImpl::SetCallLogAbility(std::shared_ptr<DataShareExtAbility> extension)
{
    std::lock_guard<std::mutex> lock(callogMutex_);
    callLogAbility_ = extension;
}

void ContactsDataShareStubImpl::SetVoiceMailAbility(std::shared_ptr<DataShareExtAbility> extension)
{
    std::lock_guard<std::mutex> lock(voiceMailMutex_);
    voiceMailAbility_ = extension;
}

std::shared_ptr<DataShareExtAbility> ContactsDataShareStubImpl::GetContactsDataAbility()
{
    std::lock_guard<std::mutex> lock(contactsMutex_);
    return contactsDataAbility_;
}

std::shared_ptr<DataShareExtAbility> ContactsDataShareStubImpl::GetCallLogAbility()
{
    std::lock_guard<std::mutex> lock(callogMutex_);
    if (callLogAbility_ == nullptr) {
        callLogAbility_.reset(CallLogAbility::Create());
    }
    return callLogAbility_;
}

std::shared_ptr<DataShareExtAbility> ContactsDataShareStubImpl::GetVoiceMailAbility()
{
    std::lock_guard<std::mutex> lock(voiceMailMutex_);
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
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingBundleName;
    if (!GetBundleNameByUid(uid, callingBundleName)) {
        HILOG_ERROR("ContactsDataShareStubImpl Insert getbundleName faild");
    }
    OHOS::Uri uriTemp = uri;
    std::chrono::milliseconds beginTime = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    HILOG_WARN("insert begin. uri = %{public}s,beginTime = %{public}lld,callingBundleName = %{public}s",
        Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(),
        beginTime.count(), callingBundleName.c_str());
    int ret = 0;
    auto extension = GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("insert failed, extension is null.");
        return ret;
    }
    ret = extension->Insert(uri, value);
    if (ret != Contacts::OPERATION_ERROR && uriTemp.ToString().find("noNotifyChange") == std::string::npos) {
        NotifyChangeExt(uri, {value}, Contacts::OPERATE_TYPE_INSERT);
    }
    std::chrono::milliseconds endTime = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    if (endTime.count() - beginTime.count() > g_operateDataTime) {
        HILOG_WARN("insert end successfully.ret: %{public}d,uri = %{public}s,contact db cost time = %{public}lld",
            ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), endTime.count() - beginTime.count());
    } else {
        HILOG_WARN("insert end successfully.ret: %{public}d,uri = %{public}s,endTime = %{public}lld",
            ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), endTime.count());
    }
    return ret;
}

int ContactsDataShareStubImpl::Update(const Uri &uri, const DataSharePredicates &predicates,
    const DataShareValuesBucket &value)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingBundleName;
    if (!GetBundleNameByUid(uid, callingBundleName)) {
        HILOG_ERROR("ContactsDataShareStubImpl Update getbundleName faild");
    }
    OHOS::Uri uriTemp = uri;
    std::chrono::milliseconds beginTime = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    HILOG_WARN("update begin. uri = %{public}s,beginTime = %{public}lld,callingBundleName = %{public}s",
        Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(),
        beginTime.count(), callingBundleName.c_str());
    int ret = 0;
    auto extension = GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("update failed, extension is null.");
        return ret;
    }
    ret = extension->Update(uri, predicates, value);
    // 如果是畅连能力、设备相关相关变动，不需要发送联系人变更通知
    if (ret > 0 && uriTemp.ToString().find("noNotifyChange") == std::string::npos) {
        NotifyChangeExt(uri, {value}, Contacts::OPERATE_TYPE_UPDATE);
    }
    std::chrono::milliseconds endTime = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    if (endTime.count() - beginTime.count() > g_operateDataTime) {
        HILOG_WARN("update end successfully.ret: %{public}d,uri = %{public}s,contact db cost time = %{public}lld",
            ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), endTime.count() - beginTime.count());
    } else {
        HILOG_WARN("update end successfully.ret: %{public}d,uri = %{public}s,endTime = %{public}lld",
            ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), endTime.count());
    }
    return ret;
}

int ContactsDataShareStubImpl::Delete(const Uri &uri, const DataSharePredicates &predicates)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingBundleName;
    if (!GetBundleNameByUid(uid, callingBundleName)) {
        HILOG_ERROR("ContactsDataShareStubImpl delete getbundleName faild");
    }
    bundleName_ = callingBundleName;
    OHOS::Uri uriTemp = uri;
    std::chrono::milliseconds beginTime = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    HILOG_WARN("delete begin. uri = %{public}s,beginTime = %{public}lld,callingBundleName = %{public}s",
        Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), beginTime.count(), callingBundleName.c_str());
    int ret = 0;
    auto extension = GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("delete failed, extension is null.");
        return ret;
    }
    ret = extension->Delete(uri, predicates);
    if (ret != Contacts::OPERATION_ERROR && uriTemp.ToString().find("noNotifyChange") == std::string::npos) {
        if (uriTemp.ToString() == Contacts::ADD_CONTACT_INFO_BATCH_URI ||
            uriTemp.ToString() == Contacts::CONTACTS_HARD_DELETE_URI) {
            OHOS::Uri uriContact(Contacts::CONTACT_CHANGE_URI);
            NotifyChangeExt(uriContact, {}, Contacts::OPERATE_TYPE_DELETE);
        } else {
            NotifyChangeExt(uri, {}, Contacts::OPERATE_TYPE_DELETE);
        }
    }
    std::chrono::milliseconds endTime = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    if (endTime.count() - beginTime.count() > g_operateDataTime) {
        HILOG_WARN("delete end successfully.ret: %{public}d,uri = %{public}s,contact db cost time = %{public}lld",
            ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), endTime.count() - beginTime.count());
    } else {
        HILOG_WARN("delete end successfully.ret: %{public}d,uri = %{public}s,endTime = %{public}lld",
            ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), endTime.count());
    }
    return ret;
}

std::shared_ptr<DataShareResultSet> ContactsDataShareStubImpl::Query(const Uri &uri,
    const DataSharePredicates &predicates, std::vector<std::string> &columns, DatashareBusinessError &businessError)
{
    OHOS::Uri uriTemp = uri;
    HILOG_INFO("query begin. uri = %{public}s,beginTime = %{public}lld",
        Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), (long long) time(NULL));
    auto extension = GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("query failed, extension is null.");
        return nullptr;
    }
    auto resultSet = extension->Query(uri, predicates, columns, businessError);
    HILOG_INFO("query end successfully.uri = %{public}s,endTime = %{public}lld",
        Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), (long long) time(NULL));

    return resultSet;
}

int ContactsDataShareStubImpl::BatchInsert(const Uri &uri, const std::vector<DataShareValuesBucket> &values)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingBundleName;
    if (!GetBundleNameByUid(uid, callingBundleName)) {
        HILOG_ERROR("ContactsDataShareStubImpl BatchInsert getbundleName faild");
    }
    unsigned int size = values.size();
    OHOS::Uri uriTemp = uri;
    std::chrono::milliseconds beginTime = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    HILOG_WARN("batch insert begin. uri = %{public}s,beginTime = %{public}lld,callingBundleName = %{public}s,"
        "values.size is %{public}d", Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(),
        beginTime.count(), callingBundleName.c_str(), size);
    int ret = 0;
    auto extension = GetOwner(uri);
    if (extension == nullptr) {
        HILOG_ERROR("batch insert failed, extension is null.");
        return ret;
    }
    ret = extension->BatchInsert(uri, values);
    if (ret != Contacts::OPERATION_ERROR && uriTemp.ToString().find("noNotifyChange") == std::string::npos) {
        if (uriTemp.GetQuery() == "isFromBatch=true") {
            std::string noQueryUri =
                uriTemp.ToString().substr(0, uriTemp.ToString().length() - uriTemp.GetQuery().length() - 1);
            HILOG_INFO("no query uri: %{public}s", noQueryUri.c_str());
            NotifyChangeExt(Uri(noQueryUri), values, Contacts::OPERATE_TYPE_INSERT);
        } else if (uriTemp.ToString() == Contacts::ADD_CONTACT_INFO_BATCH_URI ||
                   uriTemp.ToString() == Contacts::CONTACTS_HARD_DELETE_URI) {
            OHOS::Uri uriContact(Contacts::CONTACT_CHANGE_URI);
            NotifyChangeExt(uriContact, {}, Contacts::OPERATE_TYPE_INSERT);
        } else {
            uriTemp = getUriPrintByUri(uri);
            NotifyChangeExt(uriTemp, values, Contacts::OPERATE_TYPE_INSERT);
        }
    }
    std::chrono::milliseconds endTime = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
    if (endTime.count() - beginTime.count() > g_operateDataTime) {
        HILOG_WARN("batch insert end successfully.ret: %{public}d,uri = %{public}s,contact db cost time = %{public}lld",
            ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), endTime.count() - beginTime.count());
    } else {
        HILOG_WARN("batch insert end successfully.ret: %{public}d,uri = %{public}s,endTime = %{public}lld",
            ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), endTime.count());
    }
    return ret;
}

Uri ContactsDataShareStubImpl::getUriPrintByUri(const Uri &uriTemp)
{
    std::string uriStr = uriTemp.ToString();
    std::size_t index = uriStr.rfind("?");
    std::string uriLogPrint = uriStr;
    if (index >= 0 && index < uriStr.length()) {
        uriLogPrint = uriStr.substr(0, index);
    } else {
        uriLogPrint = uriStr;
    }
    OHOS::Uri uriContact(uriLogPrint);
    return uriContact;
}

std::vector<std::string> ContactsDataShareStubImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_INFO("GetFileTypes not supported");
    std::vector<std::string> result;
    return result;
}

int ContactsDataShareStubImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CONTACTS)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    std::string openFileType;
    std::string fileName;
    Contacts::UriUtils uriUtils;
    uriUtils.GetFileTypeAndFileName(uri, openFileType, fileName);
    if (openFileType != "savePhoto" || fileName.empty()) {
        HILOG_INFO("OpenFile not supported");
        return OPEN_FILE_FAILED;
    }
    std::string groupPath;
    Contacts::FileUtils fileUtils;
    if (!fileUtils.GetGroupPath(groupPath) || groupPath.empty()) {
        HILOG_ERROR("get group path failed");
        return OPEN_FILE_FAILED;
    }
    std::string filePath = groupPath + fileName;
    mode_t filePermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    int fileDescriptor = open(filePath.c_str(), O_CREAT | O_WRONLY, filePermissions);
    if (fileDescriptor == -1) {
        HILOG_ERROR("OpenFile failed");
        return OPEN_FILE_FAILED;
    }
    return fileDescriptor;
}

int ContactsDataShareStubImpl::addFailedDeleteFile(const std::string &fileName)
{
    if (fileName.empty()) {
        HILOG_INFO("delete file failed,fileName is null");
        return OPEN_FILE_FAILED;
    }
    std::string groupPath;
    Contacts::FileUtils fileUtils;
    if (!fileUtils.GetGroupPath(groupPath) || groupPath.empty()) {
        HILOG_ERROR("get group path failed");
        return OPEN_FILE_FAILED;
    }
    std::string filePath = groupPath + fileName;
    fileUtils.DeleteFile(filePath);
    return ERR_OK;
}

int ContactsDataShareStubImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("OpenRawFile not supported");
    return -1;
}

bool ContactsDataShareStubImpl::TryDeleteFile(const Uri &uri)
{
    std::string fileType;
    std::string fileName;
    Contacts::UriUtils uriUtils;
    uriUtils.GetFileTypeAndFileName(uri, fileType, fileName);
    if (fileType != "addFailedDeleteFile") {
        return false;
    }
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CONTACTS)) {
        HILOG_ERROR("Permission denied!");
        return false;
    }
    addFailedDeleteFile(fileName);
    return true;
}

std::string ContactsDataShareStubImpl::GetType(const Uri &uri) {
    if (TryDeleteFile(uri)) {
        return "";
    }
    OHOS::Uri uriTemp = uri;
    if (Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp) != "/get_grant_uri") {
        HILOG_INFO("GetType not supported");
        return EMPTY_STR;
    }

    std::string groupPath;
    Contacts::FileUtils fileUtils;
    if (!fileUtils.GetGroupPath(groupPath) || groupPath.empty()) {
        HILOG_ERROR("ContactsDataShareStubImpl GetType GetGroupPath failed");
        return EMPTY_STR;
    }
    if (!fileUtils.IsPathExist(groupPath)) {
        HILOG_ERROR("ContactsDataShareStubImpl GetType groupPath is null");
        return EMPTY_STR;
    }

    std::vector<std::string> fileNames;
    fileUtils.DirIterator(groupPath, fileNames);
    std::vector<Uri> uriVec;
    std::string targetPath = SANDBOX_PHOTO_PATH + PHOTO_STR;
    for (const auto& fileName : fileNames) {
        std::string name = fileUtils.GetFileNameFromPath(fileName);
        if (!fileUtils.CopyPath(fileName, targetPath + name)) {
            HILOG_ERROR("ContactsDataShareStubImpl GetType CopyPath failed for file: %{private}s", fileName.c_str());
            continue;
        }
        AppFileService::ModuleFileUri::FileUri fileUri(targetPath + name);
        uriVec.push_back(fileUri.uri_);
    }

    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingBundleName;
    if (uriVec.empty() || !GetBundleNameByUid(uid, callingBundleName)) {
        HILOG_ERROR("ContactsDataShareStubImpl GetType uriVec is null or GetBundleNameByUid failed");
        return EMPTY_STR;
    }
    int res = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermission(
        uriVec, 1, callingBundleName);
    return res == 0 ? FILE_URI_CONTACTSDATAABILITY + SANDBOX_PHOTO_PATH : EMPTY_STR;
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

// 无法通知具体信息，会导致被通知的应用全量刷新，尽可能不用
bool ContactsDataShareStubImpl::NotifyChange(const Uri &uri)
{
    OHOS::Uri uriTemp = uri;
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr,uri = %{public}s", __func__,
            Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str());
        return false;
    }

    ErrCode ret = obsMgrClient->NotifyChange(uri);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->NotifyChange error return %{public}d,uri = %{public}s", __func__, ret,
            Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str());
        return false;
    } else {
        HILOG_INFO("%{public}s obsMgrClient->NotifyChange succeed return %{public}d,uri = %{public}s,ts = %{public}lld",
            __func__, ret, Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(), (long long) time(NULL));
    }
    return true;
}

/**
 * 是否操作contact 相关，contact相关，发送的消息会被联系人应用处理；
 * 如发送CONTACTS_RAW_CONTACT的消息，联系人就没有处理
*/
bool isOperateContactData(const Uri &uri)
{
    OHOS::Uri uriTemp = uri;
    std::string path = uriTemp.GetPath();
    if (path.find("com.ohos.contactsdataability") != std::string::npos) {
        int code = ContactsDataAbility::UriParseAndSwitch(uriTemp);
        if (code == Contacts::CONTACTS_CONTACT_DATA || code == Contacts::PROFILE_CONTACT_DATA ||
            code == Contacts::CONTACTS_CONTACT || code == Contacts::PROFILE_CONTACT ||
            code == Contacts::ADD_CONTACT_INFO_BATCH || code == Contacts::CONTACTS_HARD_DELETE) {
            return true;
        }
    }
    return false;
}

std::string getIdsStr(const std::vector<DataShareValuesBucket> &values)
{
    // DataShareValuesBucket转为ValuesBucket，从bucket获取值，从DataShareValuesBucket获取值有问题
    std::vector<OHOS::NativeRdb::ValuesBucket> valuesRdb;
    ContactsDataAbility::transferDataShareToRdbBucket(values, valuesRdb);
    std::string idsStr;
    for (auto &value : valuesRdb) {
        int rawContactId = ContactsDataAbility::getIntValueFromRdbBucket(value,
            Contacts::ContactDataColumns::RAW_CONTACT_ID);
        idsStr.append(std::to_string(rawContactId)).append(",");
    }
    if (!idsStr.empty()) {
        idsStr.pop_back();
    }
    return idsStr;
}

std::string transferToStr(std::vector<int> &idVector)
{
    std::string idsStr;
    for (auto rawContactId : idVector) {
        idsStr.append(std::to_string(rawContactId)).append(",");
    }
    if (!idsStr.empty()) {
        idsStr.pop_back();
    }
    return idsStr;
}

bool ContactsDataShareStubImpl::NotifyChangeExt(const Uri &uri,
    const std::vector<DataShareValuesBucket> &values, std::string operateType)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }
    // 通知对象
    DataShareObserver::ChangeInfo::VBuckets extends;
    AAFwk::ChangeInfo uriChanges;
    // 操作联系人相关，发送的消息会被应用处理，获取当前影响到的id，发送给应用
    if (isOperateContactData(uri)) {
        // 不管操作的啥了，统统从update和delete影响到的联系人集合获取
        std::vector<int> delIdVector = Contacts::ContactsDataBase::deleteContactIdVector.consume();
        std::string delIdStr = transferToStr(delIdVector);
        std::vector<int> updateIdVector = Contacts::ContactsDataBase::updateContactIdVector.consume();
        std::string updateIdStr = transferToStr(updateIdVector);
        // 获取userId，通知也携带这个信息，应用收到后，判断是否是自己所属的空间变化
        int32_t userId = -1;
        OHOS::AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(userId);
        std::string idsStr = getIdsStr(values);
        DataShareObserver::ChangeInfo::VBucket vBucket;
        vBucket["updateIdStr"] = updateIdStr;
        vBucket["delIdStr"] = delIdStr;
        vBucket["userId"] = userId;
        // 通知类型，默认为空
        vBucket["notifyModifyType"] = "";
        HILOG_INFO("NotifyChangeExtWithId, updateIdStr: %{public}s, delIdStr: %{public}s , ts = %{public}lld",
            updateIdStr.c_str(), delIdStr.c_str(), (long long) time(NULL));
        // 无id信息，联系人会全量刷新
        // 场景1：删除联系人，会删除type为9（群组成员）的contact_data，如果不存在，不会更新到，也就拿不到影响到的id，标记未改变
        if (updateIdStr.empty() && delIdStr.empty()) {
            std::string modifyType = Contacts::NOTIFY_MODIFY_TYPE_NOT_MODIFY;
            HILOG_INFO("NotifyChangeExtWithId, handle contact table, but not modify id, not notify!, %{public}s",
                modifyType.c_str());
            vBucket["notifyModifyType"] = modifyType;
        }
        extends.push_back(vBucket);
    }
  
    if (operateType == Contacts::OPERATE_TYPE_INSERT) {
        uriChanges = { AAFwk::ChangeInfo::ChangeType::INSERT, { uri }, nullptr, 0, extends};
    } else if (operateType == Contacts::OPERATE_TYPE_UPDATE) {
        uriChanges = { AAFwk::ChangeInfo::ChangeType::UPDATE, { uri }, nullptr, 0, extends};
    } else if (operateType == Contacts::OPERATE_TYPE_DELETE) {
        uriChanges = { AAFwk::ChangeInfo::ChangeType::DELETE, { uri }, nullptr, 0, extends};
    }
    
    ErrCode ret = obsMgrClient->NotifyChangeExt(uriChanges);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->NotifyChangeExt error return %{public}d, uri = %{public}s", __func__, ret,
            Contacts::ContactsDataBase::getUriLogPrintByUri(uri).c_str());
        return false;
    } else {
        HILOG_INFO("%{public}s obsMgrClient->NotifyChangeExt succeed return %{public}d, %{public}lld, uri = %{public}s",
            __func__, ret, (long long) time(NULL), Contacts::ContactsDataBase::getUriLogPrintByUri(uri).c_str());
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
 
bool ContactsDataShareStubImpl::GetBundleNameByUid(int32_t uid, std::string &bundleName)
{
    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smgr == nullptr) {
        HILOG_ERROR("ContactsDataShareStubImpl GetBundleNameByUid smgr is nullptr");
        return false;
    }
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("ContactsDataShareStubImpl GetBundleNameByUid remoteObject is nullptr");
        return false;
    }
    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContactsDataShareStubImpl GetBundleNameByUid bundleMgr is nullptr");
        return false;
    }
    int32_t error = bundleMgr->GetNameForUid(uid, bundleName);
    if (error != ERR_OK) {
        HILOG_ERROR("ContactsDataShareStubImpl GetBundleNameByUid is "
                    "fail,error:%{public}d,uid:%{public}d,bundleName:%{public}s",
            error,
            uid,
            bundleName.c_str());
        return false;
    }
    return true;
}
} // namespace DataShare
} // namespace OHOS
