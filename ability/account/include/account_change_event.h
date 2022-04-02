/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ACCOUNT_CHANGE_EVENT_H
#define ACCOUNT_CHANGE_EVENT_H

#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace Contacts {
class AccountChangeEvent : public OHOS::EventFwk::CommonEventSubscriber {
public:
    ~AccountChangeEvent();
    explicit AccountChangeEvent(const OHOS::EventFwk::CommonEventSubscribeInfo &sp);
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
private:
    const static std::string SPLIT_SYMBOL_VERTICAL = "||";
    const static std::string SPLIT_SYMBOL_COMMA = ",";
};
} // namespace Contacts
} // namespace OHOS
#endif // ACCOUNT_CHANGE_EVENT_H
