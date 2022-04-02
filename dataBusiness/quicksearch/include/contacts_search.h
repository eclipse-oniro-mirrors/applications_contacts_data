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

#ifndef CONTACT_SEARCH_H
#define CONTACT_SEARCH_H

#include "rdb_store.h"

namespace OHOS {
namespace Contacts {
class ContactsSearch {
public:
    ContactsSearch();
    ~ContactsSearch();
    int64_t Insert(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, int64_t contactId, int64_t rawContactId,
        OHOS::NativeRdb::ValuesBucket rawContactValues, int64_t &searchContactId);
    // Convert the rawcontact table insert parameter to the searchcontact table parameter
    OHOS::NativeRdb::ValuesBucket StructureSearchContactDataValueBucket(
        OHOS::NativeRdb::ValuesBucket searchContactValues);
    int UpdateSearchContact(int rawContactId, std::string type, std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore,
        OHOS::NativeRdb::ValuesBucket linkDataDataValues);
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_SEARCH_H
