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

#ifndef I_SPAM_CALL_H
#define I_SPAM_CALL_H

#include "spam_call_adapter.h"
#include <iremote_broker.h>
#include <iremote_object.h>
#include <memory>

namespace OHOS {
namespace Contacts {
class ISpamCall : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"idl.SpamShieldMmsService");

    virtual int32_t UpdateBlockedMsgCount(const std::vector<std::string> &phoneNumbers,
        int blockType, std::shared_ptr<SpamCallAdapter> spamCallAdapter) = 0;
};
} // namespace Contacts
} // namespace OHOS

#endif // I_SPAM_CALL_H