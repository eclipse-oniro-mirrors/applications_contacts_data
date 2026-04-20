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

#ifndef CONTACT_POSTER_CALL_ADAPTER_MANAGER_H
#define CONTACT_POSTER_CALL_ADAPTER_MANAGER_H

#include <string>

#include "singleton.h"
#include "ability_connect_callback_stub.h"
#include <memory>

namespace OHOS {
namespace Contacts {
class PosterCallAdapter : public std::enable_shared_from_this<PosterCallAdapter> {
public:
    PosterCallAdapter();
    ~PosterCallAdapter();
    bool DownLoadPosters(const std::string &payload);
    void DisconnectPosterCallAbility();

private:
    bool ConnectPosterCallAbility(const AAFwk::Want &want, const std::string &payload);
    std::recursive_mutex mutex_;
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_POSTER_CALL_ADAPTER_MANAGER_H