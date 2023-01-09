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

#ifndef CONTACT_JSON_UTILS_H
#define CONTACT_JSON_UTILS_H

#include <string>

#include "datashare_result_set.h"
#include "json.h"
#include "json_utils.h"
#include "rdb_errno.h"
#include "rdb_open_callback.h"

namespace OHOS {
namespace Contacts {
class ContactsJsonUtils {
public:
    ContactsJsonUtils();
    ~ContactsJsonUtils();
    std::string GetDeleteData(std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet);
    void ConvertResultSet(Json::Value &arrayValue, std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet);
    void GetValue(std::vector<std::string> &columnNames, unsigned int &index, Json::Value &data,
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet);
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_JSON_UTILS_H
