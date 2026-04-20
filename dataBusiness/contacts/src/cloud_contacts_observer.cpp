/*
 * Copyright (C) 2023-2023 Huawei Device Co., Ltd.
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

#include "cloud_contacts_observer.h"

#include <vector>

#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mutex;
}
std::shared_ptr<ContactConnectAbility> ContactsObserver::contactsConnectAbility_ = nullptr;
std::shared_ptr<ContactsObserver> ContactsObserver::contactsObserver_ = nullptr;

std::shared_ptr<ContactsObserver> ContactsObserver::GetInstance()
{
    if (contactsObserver_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mutex);
        if (contactsObserver_ == nullptr) {
            contactsObserver_.reset(new ContactsObserver());
        }
    }
    return contactsObserver_;
}

ContactsObserver::ContactsObserver()
{
    contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
}

void ContactsObserver::OnChange(const std::vector<std::string> &devices)
{}

void ContactsObserver::OnChange(
    const OHOS::DistributedRdb::Origin &origin, const PrimaryFields &fields, ChangeInfo &&changeInfo)
{
    if (origin.dataType == DistributedRdb::Origin::ASSET_DATA) {
        HILOG_WARN("ContactsObserver OnChange dataType is assetData, ts = %{public}lld", (long long) time(NULL));
        return;
    }
    handleCloudSyncChange(&changeInfo);
}

bool ContactsObserver::handleCloudSyncChangeLogout(std::string *ptr, std::string table)
{
    if (ptr->compare("DELETE#ALL_CLOUDDATA") == 0 && table.compare("cloud_raw_contact") == 0) {
        // 账号退出，处理联系人；
        // 忽略此消息：不在通知位置处理；账号退出，会先将联系人同步开关置空，在联系人同步开关监听位置处理
        HILOG_WARN("ContactsDataBase handleCloudSyncChange cloud logout contact delete handle "
            "checkCursor data = %{public}s, ts = %{public}lld, return",
            (*ptr).c_str(), (long long) time(NULL));
        // 调到js后，直接返回
        contactsConnectAbility_->ConnectAbility("", "", "",
            "", "logOutDelContact", "");
        HILOG_WARN("ContactsDataBase handleCloudSyncChange cloud logout contact delete handle start js success");
        return true;
    }
    // 退账号保留联系人
    if (ptr->compare("RESERVE#ALL_CLOUDDATA") == 0 && table.compare("cloud_raw_contact") == 0) {
        // 账号退出，处理联系人；
        // 忽略此消息：不在通知位置处理；账号退出，会先将联系人同步开关置空，在联系人同步开关监听位置处理
        HILOG_WARN("ContactsDataBase handleCloudSyncChange cloud logout contact reserve handle "
            "checkCursor data = %{public}s, ts = %{public}lld, return",
            (*ptr).c_str(), (long long) time(NULL));
        // 调到js后，直接返回
        contactsConnectAbility_->ConnectAbility("", "", "",
            "", "logOutRetainContact", "");
        HILOG_WARN("ContactsDataBase handleCloudSyncChange cloud logout contact retain handle start js success");
        return true;
    }
    return false;
}

bool ContactsObserver::handleCloudSyncChangeLogoutGroup(std::string *ptr, std::string table)
{
    // 对于群组表的退账号删除通知，不处理；
    if ((ptr->compare("RESERVE#ALL_CLOUDDATA") == 0 || ptr->compare("DELETE#ALL_CLOUDDATA") == 0) &&
        table.compare("cloud_groups") == 0) {
        HILOG_INFO("ContactsDataBase handleCloudSyncChange cloud logout cloud_groups msg handle "
            "checkCursor data = %{public}s, ts = %{public}lld",
            (*ptr).c_str(), (long long) time(NULL));
        return true;
    }
    return false;
}

void ContactsObserver::handleCloudSyncChange(ChangeInfo *changeInfo)
{
    HILOG_WARN("ContactsDataBase handleCloudSyncChange, ts = %{public}lld", (long long) time(NULL));
    std::string uuidToInsert;
    std::string uuidToUpdate;
    std::string uuidToDelete;
    int subLength = 1;
    bool isLogoutHandleFlag = false;
    for (auto &[table, value] : *changeInfo) {
        HILOG_INFO("ContactsDataBase handleCloudSyncChange table = %{public}s, ts = %{public}lld",
            table.c_str(), (long long) time(NULL));
        for (auto &uuid : value[CHG_TYPE_INSERT]) {
            auto ptr = std::get_if<std::string>(&uuid);
            if (ptr != nullptr) {
                uuidToInsert.append((*ptr)).append(";").append(table.c_str()).append(",");
            }
        }
        uuidToInsert = uuidToInsert.substr(0, uuidToInsert.length() - subLength);
        for (auto &uuid : value[CHG_TYPE_UPDATE]) {
            auto ptr = std::get_if<std::string>(&uuid);
            if (ptr != nullptr) {
                uuidToUpdate.append((*ptr)).append(";").append(table.c_str()).append(",");
            }
        }
        uuidToUpdate = uuidToUpdate.substr(0, uuidToUpdate.length() - subLength);

        for (auto &uuid : value[CHG_TYPE_DELETE]) {
            auto ptr = std::get_if<std::string>(&uuid);
            // 对于云退出账号删除联系人场景，会发送此处通知，此时走cursor处理逻辑
            if (ptr == nullptr) {
                continue;
            }
            // 退账号删联系人会发两个消息，一个联系人表的，一个群组表的
            // （1）联系人消息和group消息一起到，
            // 如果先处理联系人消息，会直接返回；如果先处理到群组表，后边会在处理到联系人表，直接返回
            // （2）联系人和群组消息分开到
            // 处理联系人消息直接返回；处理群组消息，判断到是推账号处理，不处理cursor
            if (handleCloudSyncChangeLogout(ptr, table)) {
                isLogoutHandleFlag = true;
                return;
            }
            // 如果退账号删联系人和退账号删群组消息一起来，退账号处理联系人直接返回
            // 如果退账号删群组消息单独来，不处理后边的cursor处理，直接返回
            if (handleCloudSyncChangeLogoutGroup(ptr, table)) {
                isLogoutHandleFlag = true;
            }

            uuidToDelete.append((*ptr)).append(";").append(table.c_str()).append(",");
        }
        uuidToDelete = uuidToDelete.substr(0, uuidToDelete.length() - subLength);
    }
    if (!isLogoutHandleFlag) {
        contactsConnectAbility_->ConnectAbility(uuidToInsert, uuidToUpdate, uuidToDelete,
            "", "", "");
    } else {
        HILOG_INFO("ContactsDataBase handleCloudSyncChange cloud logout cloud_groups msg not handle cursor "
            " ts = %{public}lld, return",
            (long long) time(NULL));
    }
}
}  // namespace Contacts
}  // namespace OHOS