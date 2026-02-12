/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef CONTACT_SPAM_CALL_ADAPTER_MANAGER_H
#define CONTACT_SPAM_CALL_ADAPTER_MANAGER_H

#include <string>

#include "singleton.h"
#include "ability_connect_callback_stub.h"
#include <memory>

namespace OHOS {
namespace Contacts {
class SpamCallAdapter : public std::enable_shared_from_this<SpamCallAdapter> {
public:
    SpamCallAdapter();
    ~SpamCallAdapter();
    bool UpdateBlockedMsgCount(const std::vector<std::string> &phoneNumbers, int blockType);
    void DisconnectSpamCallAbility();

private:
    bool ConnectSpamCallAbility(const AAFwk::Want &want, const std::vector<std::string> &phoneNumbers, int blockType);
    std::recursive_mutex mutex_;
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_SPAM_CALL_ADAPTER_MANAGER_H