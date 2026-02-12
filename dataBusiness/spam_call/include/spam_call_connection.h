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

#ifndef CONTACT_SPAM_CALL_CONNECTION_H
#define CONTACT_SPAM_CALL_CONNECTION_H

#include "ability_connect_callback_stub.h"
#include "spam_call_adapter.h"
#include <memory>

namespace OHOS {
namespace Contacts {
class SpamCallConnection : public AAFwk::AbilityConnectionStub {
public:
    SpamCallConnection(const std::vector<std::string> &phoneNumbers, int blockType,
        std::shared_ptr<SpamCallAdapter> spamCallAdapter)
    {
        phoneNumbers_ = phoneNumbers;
        spamCallAdapter_ = spamCallAdapter;
        blockType_ = blockType;
    };

    virtual ~SpamCallConnection() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;

private:
    std::vector<std::string> phoneNumbers_;
    std::shared_ptr<SpamCallAdapter> spamCallAdapter_ {nullptr};
    int blockType_;
};
} // namespace Contacts
} // namespace OHOS

#endif // CONTACT_SPAM_CALL_CONNECTION_H
