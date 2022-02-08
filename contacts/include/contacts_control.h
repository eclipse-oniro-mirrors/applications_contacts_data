/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef CONTACT_CONTROL_H
#define CONTACT_CONTROL_H

#include "ability.h"
#include "ability_context.h"
#include "context.h"
#include "data_ability_helper.h"
#include "data_ability_predicates.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"

#include "contacts_napi_object.h"

namespace OHOS {
namespace ContactsApi {
class ContactsControl {
public:
    ContactsControl();
    ~ContactsControl();
    int64_t RawContactInsert(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
        OHOS::NativeRdb::ValuesBucket rawContactValues);
    int ContactDataInsert(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
        std::vector<OHOS::NativeRdb::ValuesBucket> contactsDataValues);
    int RawContactUpdate(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
        OHOS::NativeRdb::ValuesBucket updateValues, OHOS::NativeRdb::DataAbilityPredicates predicates);
    int ContactDataUpdate(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
        OHOS::NativeRdb::ValuesBucket updateValues, OHOS::NativeRdb::DataAbilityPredicates predicates);
    int ContactDelete(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
    int ContactDataDelete(std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactQuery(
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> ContactDataQuery(
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> GroupsQuery(
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> MyCardQuery(
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> HolderQuery(
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper, std::vector<std::string> columns,
        OHOS::NativeRdb::DataAbilityPredicates predicates);
};
} // namespace ContactsApi
} // namespace OHOS
#endif // CONTACT_CONTROL_H