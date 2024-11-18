/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CONTACTS_UTILS_H
#define CONTACTS_UTILS_H

#include <cstdint>
#include "errors.h"

#include "hilog_wrapper_api.h" // from contacts_data/contacts

#include "cj_data_share_predicates_utils.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"

namespace OHOS {
namespace ContactsFfi {

struct ValuesBucket {
    char** key = NULL;
    DataShare::CValueType* value = NULL;
    int64_t size = 0;
};

using RawContact = ValuesBucket;

struct Buckets {
    ValuesBucket* data = NULL;
    int64_t bucketCount = 0;
};
using ContactData = Buckets;
using GroupsData = Buckets;
using HoldersData = Buckets;


struct ContactsData {
    ContactData* contactsData = NULL;
    int64_t contactsCount = 0;
};

OHOS::DataShare::DataShareValuesBucket convertToDataShareVB(OHOS::ContactsFfi::ValuesBucket vb);

ContactsData* parseResultSetForContacts(std::shared_ptr<OHOS::DataShare::DataShareResultSet> &resultSet, int32_t *errCode);

GroupsData* parseResultSetForGroups(std::shared_ptr<OHOS::DataShare::DataShareResultSet> &resultSet, int32_t *errCode);

HoldersData* parseResultSetForHolders(std::shared_ptr<OHOS::DataShare::DataShareResultSet> &resultSet, int32_t *errCode);

} // namespace ContactsFfi
} // namespace OHOS

#endif // CONTACTS_UTILS_H
