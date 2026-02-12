/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#include <thread>

#include "auto_sync_detail_progress_observer.h"
#include "contacts_database.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
namespace {
std::mutex g_mutex;
}
std::shared_ptr<AutoSyncDetailProgressObserver> AutoSyncDetailProgressObserver::instance_ = nullptr;
std::shared_ptr<ContactConnectAbility> AutoSyncDetailProgressObserver::contactsConnectAbility_ = nullptr;
std::shared_ptr<AutoSyncDetailProgressObserver> AutoSyncDetailProgressObserver::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> lock(g_mutex);
        if (instance_ == nullptr) {
            instance_.reset(new AutoSyncDetailProgressObserver());
        }
    }
    return instance_;
}

AutoSyncDetailProgressObserver::AutoSyncDetailProgressObserver()
{}

void AutoSyncDetailProgressObserver::ProgressNotification(const DistributedRdb::Details &details)
{
    if (details.empty()) {
        return;
    }
    HILOG_INFO(
        "AutoSync ProgressNotification details size is:%{public}ld, ts = %{public}lld", (long) details.size(), (long long) time(NULL));
    for (auto &detail : details) {
        HILOG_INFO("AutoSync ProgressNotification detail.first is is:%{public}s, ts is :%{public}lld",
            detail.first.c_str(), (long long) time(NULL));
        if (detail.first == "default") {
            HILOG_INFO("AutoSync ProgressNotification details progress is:%{public}d, ts = %{public}lld",
                detail.second.progress, (long long) time(NULL));
            if (detail.second.progress == OHOS::DistributedRdb::SYNC_FINISH) {
                HILOG_WARN("AutoSync ProgressNotification finished, detail.second.code = %{public}d, ts = %{public}lld",
                    detail.second.code, (long long) time(NULL));
                if (detail.second.code == OHOS::DistributedRdb::ProgressCode::SUCCESS) {
                    contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
                    contactsConnectAbility_->ConnectAbility("", "", "", "", "checkCursorAndUpload", "");
                    auto contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
                    contactDataBase_->ClearRetryCloudSyncCount();
                } else {
                    auto contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
                    contactDataBase_->RetryCloudSync(details.begin()->second.code);
                }
            }
        }
    }
    HILOG_WARN(
        "AutoSync ProgressNotification code is:%{public}d, ts = %{public}lld", details.begin()->second.code, (long long) time(NULL));
}
}  // namespace Contacts
}  // namespace OHOS