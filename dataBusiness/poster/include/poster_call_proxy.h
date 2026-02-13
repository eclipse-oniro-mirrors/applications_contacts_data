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

#ifndef CONTACT_POSTER_CALL_PROXY_H
#define CONTACT_POSTER_CALL_PROXY_H

#include <iremote_proxy.h>
#include "i_poster_call.h"
#include "poster_call_adapter.h"
#include <memory>

namespace OHOS {
namespace Contacts {
class PosterCallProxy : public OHOS::IRemoteProxy<IPosterCall> {
public:
    explicit PosterCallProxy(const sptr<IRemoteObject> &impl);

    virtual ~PosterCallProxy() = default;

    int32_t DownLoadPosters(const std::string &payload,
                            std::shared_ptr<PosterCallAdapter> posterCallAdapter) override;

private:
    std::shared_ptr<PosterCallAdapter> posterCallAdapter_ {nullptr};
    std::string GenSrcTransactionId();
    std::string GetRandomNumberStr(uint32_t len);
    std::string GetCurrentDateTimeCode(void);

private:
    DISALLOW_COPY_AND_MOVE(PosterCallProxy);

    // the delegator object is to register this proxy class
    static inline BrokerDelegator<PosterCallProxy> delegator_;
};
} // namespace Contacts
} // namespace OHOS

#endif //CONTACT_POSTER_CALL_PROXY_H
