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

#ifndef AUTO_SYNC_DETAIL_PROGRESS_OBSERVER_H
#define AUTO_SYNC_DETAIL_PROGRESS_OBSERVER_H

#include "rdb_predicates.h"
#include "rdb_store.h"
#include "contact_connect_ability.h"
namespace OHOS {
namespace Contacts {
class AutoSyncDetailProgressObserver : public OHOS::DistributedRdb::DetailProgressObserver {
public:
    static std::shared_ptr<AutoSyncDetailProgressObserver> instance_;
    static std::shared_ptr<ContactConnectAbility> contactsConnectAbility_;
    void ProgressNotification(const DistributedRdb::Details &details) override;
    static std::shared_ptr<AutoSyncDetailProgressObserver> GetInstance();
    virtual ~AutoSyncDetailProgressObserver() {}

private:
    AutoSyncDetailProgressObserver();
};
}  // namespace Contacts
}  // namespace OHOS
#endif  // CONTACTS_CONNECT_ABILITY_H