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
 
#include "spam_call_stub.h"
 
#include <string_ex.h>
 
#include "hilog_wrapper.h"
 
#include "message_option.h"
#include "message_parcel.h"
#include "common.h"
 
namespace OHOS {
namespace Contacts {
SpamCallStub::SpamCallStub() {}
 
SpamCallStub::~SpamCallStub() {}
 
int32_t SpamCallStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string myDescriptor = SpamCallStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        HILOG_ERROR("descriptor checked fail !");
        return OPERATION_ERROR;
    }
    HILOG_INFO("OnReceived, cmd = %{public}u", code);
    switch (code) {
        case RESULT_GET_TWO:
            do {
                int32_t errCodeVar = data.ReadInt32();
                std::string resultVar = Str16ToStr8(data.ReadString16());
                OnResult(errCodeVar, resultVar);
            } while (false);
            break;
        default:
            HILOG_ERROR("callback failed, default = %d", code);
            break;
    }
    return OPERATION_OK;
}
} // namespace Contacts
} // namespace OHOS