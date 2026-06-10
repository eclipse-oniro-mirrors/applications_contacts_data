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

#include "spam_call_proxy.h"

#include "message_option.h"
#include "message_parcel.h"

#include "hilog_wrapper.h"
#include "callback_stub_helper.h"
#include <string_ex.h>
#include "common.h"

namespace OHOS {
namespace Contacts {
SpamCallProxy::SpamCallProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ISpamCall>(impl)
{}

int32_t SpamCallProxy::UpdateBlockedMsgCount(const std::vector<std::string> &phoneNumbers,
    int blockType, std::shared_ptr<SpamCallAdapter> spamCallAdapter)
{
    MessageParcel dataParcel;
    std::u16string myDescriptor = SpamCallProxy::GetDescriptor();
    if (!dataParcel.WriteInterfaceToken(myDescriptor)) {
        HILOG_ERROR("Write descriptor fail");
        return OPERATION_ERROR;
    }
    auto instance = new (std::nothrow) CallbackStubHelper(spamCallAdapter);
    if (instance == nullptr) {
        HILOG_ERROR("CallbackStubHelper is null!");
        return OPERATION_ERROR;
    }
    std::vector<std::u16string> v;
    for (auto rust_s : phoneNumbers) {
        std::u16string u16string = Str8ToStr16(std::string(rust_s));
        v.push_back(u16string);
    }
    if (!dataParcel.WriteString16Vector(v)) {
        HILOG_ERROR("WriteString16Vector failed");
    }
    if (!dataParcel.WriteRemoteObject(instance)) {
        HILOG_ERROR("WriteRemoteObject failed");
    }
    MessageParcel replyParcel;
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() return nullptr!");
        return OPERATION_ERROR;
    }
    MessageOption option;
    int32_t ret = remote->SendRequest(blockType, dataParcel, replyParcel, option);
    if (ret != OPERATION_OK) {
        HILOG_ERROR("UpdateBlockedMsgCount:%{public}d errCode:%{public}d", blockType, ret);
        return ret;
    }
    return replyParcel.ReadInt32();
}
} // namespace Contacts
} // namespace OHOS
