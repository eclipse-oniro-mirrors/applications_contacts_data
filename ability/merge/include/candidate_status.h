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

#ifndef CANDIDATESCORE_H
#define CANDIDATESCORE_H

#include "rdb_errno.h"
#include "rdb_store.h"
#include "result_set.h"

#include "candidate.h"

namespace OHOS {
namespace Contacts {
class CandidateStatus {
public:
    CandidateStatus();
    ~CandidateStatus();
    Candidate QueryAllForMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId);

private:
    int PhoneMatch(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<std::string> phones, int id);
    bool IsMerged(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId);
    bool IsNeedMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId);
    bool IsMergeStatus(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId);
    bool JudgeDataDifferent(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::vector<int> ids, int rawId);
    void AddMergedStatus(Candidate &candidate);
};
} // namespace Contacts
} // namespace OHOS
#endif // CANDIDATESCORE_H
