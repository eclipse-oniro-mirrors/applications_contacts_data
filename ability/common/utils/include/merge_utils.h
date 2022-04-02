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

#ifndef MERGE_UTILS_H
#define MERGE_UTILS_H

#include "candidate.h"
#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"
#include "rdb_errno.h"
#include "rdb_store.h"
#include "result_set.h"

namespace OHOS {
namespace Contacts {
class MergeUtils {
public:
    MergeUtils();
    ~MergeUtils();
    std::set<std::string> QueryRawContactByType(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, int typeId);
    std::set<std::string> QueryDataDetailInfo(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> &rawIds, int typeId);
    std::set<std::string> QueryDataExecute(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> rawIds, int typeId);
    std::vector<int> QueryByDataName(
        int rawId, std::set<std::string> data, std::shared_ptr<OHOS::NativeRdb::RdbStore> store);
    bool SetEqual(std::set<std::string> setLeft, std::set<std::string> setRight);
    void AddHasJudgeForRawId(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, OHOS::NativeRdb::ValuesBucket &value);
    void GetRawIdsByRawId(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, std::set<int> &rawIds);
};
} // namespace Contacts
} // namespace OHOS
#endif // MERGE_UTILS_H
