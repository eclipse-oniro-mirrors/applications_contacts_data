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

#ifndef CONTACTS_CONNECT_ABILITY_H
#define CONTACTS_CONNECT_ABILITY_H

#include <condition_variable>
#include <mutex>

#include "ability_connect_callback_interface.h"

namespace OHOS {
namespace Contacts {
class ContactsDataBase;
/**
 * @brief Indicates one second duration.
 */
constexpr int16_t WAIT_TIME_ONE_SECOND = 1;
constexpr int16_t CONTACTS_BIRTHDAY_INSERT = 1;
constexpr int16_t CONTACTS_BIRTHDAY_UPDATE = 2;
constexpr int16_t CONTACTS_BIRTHDAY_DELETE = 3;
constexpr int16_t CONTACTS_BIRTHDAY_SYNC = 4;
constexpr int16_t CONTACTS_BIRTHDAY_BATCH_INSERT = 5;

class ContactConnectAbility {
public:
    ContactConnectAbility();
    ~ContactConnectAbility();
    int ConnectAbility(std::string uuidToInsert, std::string uuidToUpdate, std::string uuidToDelete,
        std::string compareCloudAndRawContact, std::string refreshContact, std::string mergeContactArgs);
    void ConnectBackupAbility();
    void ConnectRelationChainAbility(std::string operation, std::string jsonData);
    void ConnectEventsHandleAbility(int operationType, std::string values, std::string syncBirthdays);
    void DisconnectAbility();
    void SetConnectFlag(bool isConnected);
    void NotifyAll();
    bool WaitForConnectResult();
    static std::shared_ptr<ContactConnectAbility> GetInstance();

private:
    sptr<AAFwk::IAbilityConnection> connectCallback_ = nullptr;
    static std::shared_ptr<ContactConnectAbility> contactsConnectAbility_;
    static std::shared_ptr<OHOS::Contacts::ContactsDataBase> contactDataBase_;
    bool isConnected_ = false;
    static std::condition_variable cv_;
    std::mutex mutex_;
};
}  // namespace Contacts
}  // namespace OHOS

#endif  // CONTACTS_CONNECT_ABILITY_H
