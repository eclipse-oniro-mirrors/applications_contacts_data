/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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

#ifndef NUMBER_IDENTITY_HELPER_H
#define NUMBER_IDENTITY_HELPER_H

#include "datashare_helper.h"
#include "datashare_predicates.h"

namespace OHOS {
namespace Contacts {
class NumberIdentityHelper {
public:
    static std::shared_ptr<NumberIdentityHelper> GetInstance();
    bool Query(std::string &numberLocation, DataShare::DataSharePredicates &predicates);
    ~NumberIdentityHelper();
private:
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(std::string uri);
    static std::shared_ptr<NumberIdentityHelper> instance_;
    std::shared_ptr<DataShare::DataShareHelper> helper_;
    static std::mutex identity_mutex;
    static std::mutex helperMutex_;
};
}
}

#endif //NUMBER_IDENTITY_HELPER_H