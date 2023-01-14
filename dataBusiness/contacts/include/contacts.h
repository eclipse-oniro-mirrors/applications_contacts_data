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

#ifndef CONTACT_H
#define CONTACT_H

#include "datashare_predicates.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_store.h"

namespace OHOS {
namespace Contacts {
class Contacts {
public:
    Contacts();
    ~Contacts();
    int InsertContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, const int64_t &outRawContactId,
        OHOS::NativeRdb::ValuesBucket rawContactValues, int64_t &contactId);
    OHOS::NativeRdb::ValuesBucket StructureContactDataValueBucket(OHOS::NativeRdb::ValuesBucket rawContactValues);
    int UpdateContact(const int &rawContactId, std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore,
        OHOS::NativeRdb::ValuesBucket linkDataDataValues);
    int DeleteContactById(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int needDeleteContactId);

private:
    int ContactValueBucketGetInt(OHOS::NativeRdb::ValuesBucket rawContactValues, std::string ColumnName);
    std::string ContactValueBucketGetString(OHOS::NativeRdb::ValuesBucket rawContactValues, std::string ColumnName);
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_H
