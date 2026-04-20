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

#ifndef I_POSTER_CALL_H
#define I_POSTER_CALL_H

#include "poster_call_adapter.h"
#include <iremote_broker.h>
#include <iremote_object.h>
#include <memory>

namespace OHOS {
namespace Contacts {
class IPosterCall : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"idl.IAccountContactsService");

    virtual int32_t DownLoadPosters(const std::string &payload,
                                    std::shared_ptr<PosterCallAdapter> posterCallAdapter) = 0;
};
} // namespace Contacts
} // namespace OHOS

#endif // I_POSTER_CALL_H