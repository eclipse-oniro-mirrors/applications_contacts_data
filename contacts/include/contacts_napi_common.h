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

#ifndef CONTACT_NAPI_COMMON_H
#define CONTACT_NAPI_COMMON_H

#include "data_ability_predicates.h"
#include "data_ability_helper.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"

namespace OHOS {
namespace ContactsApi {
constexpr int MAX_PARAMS = 6;
constexpr int ERROR = -1;

constexpr int NAPI_GET_STRING_SIZE = 256;
constexpr int REQUEST_PARAMS_COUNT_ONE = 1;

// NAPI type
constexpr int TYPE_NAPI_ERROR = -1;
constexpr int TYPE_NAPI_NUMBER = 0;
constexpr int TYPE_NAPI_STRING = 1;
constexpr int TYPE_HOLDER = 2;
constexpr int TYPE_ATTR = 3;
constexpr int TYPE_CONTACT = 4;

// Execute action code
constexpr int ADD_CONTACT = 1001;
constexpr int DELETE_CONTACT = 2001;
constexpr int UPDATE_CONTACT = 3001;

constexpr int QUERY_CONTACTS = 4001;
constexpr int QUERY_CONTACT = 4008;

constexpr int QUERY_CONTACTS_BY_EMAIL = 4002;
constexpr int QUERY_CONTACTS_BY_PHONE_NUMBER = 4003;
constexpr int QUERY_GROUPS = 4004;
constexpr int QUERY_HOLDERS = 4005;
constexpr int QUERY_KEY = 4006;
constexpr int QUERY_MY_CARD = 4007;
constexpr int IS_LOCAL_CONTACT = 5008;
constexpr int IS_MY_CARD = 5009;

// contactsData type
constexpr int EMAIL = 1;
constexpr int IM = 2;
constexpr int NICKNAME = 3;
constexpr int ORGANIZATION = 4;
constexpr int PHONE = 5;
constexpr int NAME = 6;
constexpr int POSTAL_ADDRESS = 7;
constexpr int PHOTO = 8;
constexpr int GROUP_MEMBERSHIP = 9;
constexpr int NOTE = 10;
constexpr int CONTACT_EVENT = 11;
constexpr int WEBSITE = 12;
constexpr int RELATION = 13;
constexpr int CONTACT_MISC = 14;
constexpr int HICALL_DEVICE = 15;
constexpr int CAMCARD = 16;
constexpr int SIP_ADDRESS = 17;

struct ExecuteHelper {
    ExecuteHelper()
        : work(nullptr), deferred(nullptr), dataValue(nullptr), sync(1), argc(0), actionCode(-1), callBack(nullptr),
          info(nullptr), childActionCode(0), promise(nullptr) {}
    napi_async_work work;
    napi_deferred deferred;
    napi_value dataValue;
    int sync;
    unsigned int argc;
    int actionCode;
    napi_ref callBack;
    napi_callback_info info;
    int childActionCode;
    napi_value promise;
    // query
    std::vector<std::string> columns;
    // condition
    NativeRdb::DataAbilityPredicates predicates;
    // update
    std::vector<NativeRdb::ValuesBucket> valueUpdateContact;
    // insert
    std::vector<NativeRdb::ValuesBucket> valueContact;
    // insertContactData
    std::vector<NativeRdb::ValuesBucket> valueContactData;
    // dataAbilityHelper
    std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> dataAbilityHelper;
};
} // namespace ContactsApi
} // namespace OHOS
#endif // CONTACT_NAPI_COMMON_H