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

#ifndef RAW_CONTACT_H
#define RAW_CONTACT_H

#include "datashare_predicates.h"
#include "rdb_errno.h"
#include "rdb_store.h"

namespace OHOS {
namespace Contacts {
class RawContacts {
public:
    RawContacts();
    ~RawContacts();
    int InsertRawContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int64_t &outRawContactId,
        OHOS::NativeRdb::ValuesBucket rawContactValues);
    int UpdateRawContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore,
        OHOS::NativeRdb::ValuesBucket rawContactValues, std::string whereClause, std::vector<std::string> whereArgs);
    int UpdateRawContactById(int &rawContactId, std::string type, std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore,
        OHOS::NativeRdb::ValuesBucket rawContactValues);
    int GetDeleteContactIdByAccountId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int accountId);
    int GetDeleteRawContactIdByAccountId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int accountId);
    int DeleteRawcontactByRawId(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int needDeleteRawContactId);
};
} // namespace Contacts
} // namespace OHOS
#endif // RAW_CONTACT_H
