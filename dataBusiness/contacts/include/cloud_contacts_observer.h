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

#ifndef CLOUD_CONTACTS_OBSERVER_H
#define CLOUD_CONTACTS_OBSERVER_H

#include "contact_connect_ability.h"
#include "rdb_predicates.h"

namespace OHOS {
namespace Contacts {
class ContactsObserver : public OHOS::DistributedRdb::RdbStoreObserver {
public:
    static std::shared_ptr<ContactsObserver> contactsObserver_;
    static std::shared_ptr<ContactConnectAbility> contactsConnectAbility_;
    static std::shared_ptr<ContactsObserver> GetInstance();
    virtual ~ContactsObserver()
    {}
    void OnChange(const std::vector<std::string> &devices) override;
    void OnChange(
        const OHOS::DistributedRdb::Origin &origin, const PrimaryFields &fields, ChangeInfo &&changeInfo) override;
    void handleCloudSyncChange(ChangeInfo *changeInfo);
    bool handleCloudSyncChangeLogout(std::string *ptr, std::string table);
    bool handleCloudSyncChangeLogoutGroup(std::string *ptr, std::string table);

private:
    ContactsObserver();
};
}  // namespace Contacts
}  // namespace OHOS
#endif  // CONTACTS_CONNECT_ABILITY_H