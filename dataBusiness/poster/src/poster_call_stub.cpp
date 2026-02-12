/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
 
#include "poster_call_stub.h"
 
#include <string_ex.h>
 
#include "hilog_wrapper.h"
#include "contacts_common_event.h" 
#include "message_option.h"
#include "message_parcel.h"
#include "common.h"
 
namespace OHOS {
namespace Contacts {
PosterCallStub::PosterCallStub() {}
 
PosterCallStub::~PosterCallStub() {}
 
int32_t PosterCallStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::string payload = "";
    std::string event = "";
    switch (code) {
        case RESULT_GET_ONE:
            event = Str16ToStr8(data.ReadString16());
            payload = Str16ToStr8(data.ReadString16());
            HILOG_INFO("OnReceived, event = %{public}s", event.c_str());

            if (event == "download") {
                ContactsCommonEvent::SendPosterEvent(DOWNLOAD_DONE_EVENT_CODE, payload);
            } else if (event == "message") {
                ContactsCommonEvent::SendPosterEvent(DOWNLOAD_RESULT_EVENT_CODE, payload);
            }
            break;
        default:
            HILOG_ERROR("callback failed, default = %d", code);
            break;
    }
    return OPERATION_OK;
}
} // namespace Contacts
} // namespace OHOS
