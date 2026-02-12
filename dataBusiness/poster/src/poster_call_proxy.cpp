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

#include "poster_call_proxy.h"

#include <ctime>
#include <sys/time.h>
#include <cstdint>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <random>

#include "message_option.h"
#include "message_parcel.h"

#include "hilog_wrapper.h"
#include "poster_callback_stub.h"
#include <string_ex.h>
#include "common.h"

namespace OHOS {
namespace Contacts {

const std::string NUMBER_CODE_LIST = "1234567890";
const uint32_t SRCTRANID_RANDOM_LEN = 3;
const int32_t TIME_BUFFER_LEN = 128;

PosterCallProxy::PosterCallProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IPosterCall>(impl)
{}

int32_t PosterCallProxy::DownLoadPosters(const std::string &payload,
                                         std::shared_ptr<PosterCallAdapter> posterCallAdapter)
{
    MessageParcel dataParcel;
    std::u16string myDescriptor = PosterCallProxy::GetDescriptor();
    if (!dataParcel.WriteInterfaceToken(myDescriptor)) {
        HILOG_ERROR("Write descriptor fail");
        return OPERATION_ERROR;
    }


    std::string transId = GenSrcTransactionId();
    dataParcel.WriteString16(OHOS::to_utf16(transId));

    // callback1
    dataParcel.WriteString16(OHOS::to_utf16("MESSAGE"));
    auto callback = new (std::nothrow) PosterCallbackStub(posterCallAdapter);
    if (callback == nullptr) {
        HILOG_ERROR("CallbackStubHelper is null!");
        return OPERATION_ERROR;
    }
    if (!dataParcel.WriteRemoteObject(callback)) {
        HILOG_ERROR("WriteRemoteObject failed");
    }

    // callback2
    dataParcel.WriteString16(OHOS::to_utf16("DOWNLOAD"));
    if (!dataParcel.WriteRemoteObject(callback)) {
        HILOG_ERROR("WriteRemoteObject failed");
    }
    if (!dataParcel.WriteString16(OHOS::to_utf16(payload))) {
        HILOG_ERROR("Write MESSAGE failed");
    }

    MessageParcel replyParcel;
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() return nullptr!");
        return OPERATION_ERROR;
    }

    MessageOption option;
    int32_t ret = remote->SendRequest(POSTER_BATCH_DOWNLOAD_CODE, dataParcel, replyParcel, option);
    if (ret != OPERATION_OK) {
        HILOG_ERROR("DownLoadPosters errCode:%{public}d", ret);
        return ret;
    }
    
    ret = replyParcel.ReadInt32();
    HILOG_ERROR("errCode:%{public}d", ret);

    return ret;
}

std::string PosterCallProxy::GetRandomNumberStr(uint32_t len)
{
    std::random_device rd("/dev/random");
    std::mt19937 mt(rd());
    std::string numberStr;
    for (unsigned int i = 0; i < len; i++) {
        unsigned int index = mt() % NUMBER_CODE_LIST.length();
        numberStr.push_back(NUMBER_CODE_LIST.at(index));
    }
    return numberStr;
}

std::string PosterCallProxy::GenSrcTransactionId()
{
    // srctranid后增加3位随机数，防止并发请求时srctranid完全相同被云侧抑制
    return GetCurrentDateTimeCode() + GetRandomNumberStr(SRCTRANID_RANDOM_LEN);
}

std::string PosterCallProxy::GetCurrentDateTimeCode(void)
{
    char buffer[TIME_BUFFER_LEN] = {0};
    std::time_t timestamp = std::time(nullptr);
    std::tm* info = std::localtime(&timestamp);
    if (info != nullptr) {
        std::strftime(buffer, TIME_BUFFER_LEN - 1, "%Y%m%d%H%M%S", info);
    }

    return std::string(buffer);
}

} // namespace Contacts
} // namespace OHOS
