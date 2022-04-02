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

#ifndef MATCHCANDIDATE_H
#define MATCHCANDIDATE_H

#include "candidate_status.h"

namespace OHOS {
namespace Contacts {
class MatchCandidate {
public:
    MatchCandidate();
    ~MatchCandidate();
    int FindMatchContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId);
    int Split(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId);
    void AddHasByRawId(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId);

private:
    int UpdateSearch(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, int contactsId);
    int ExecuteUpdateMode(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, std::set<int> Ids, int mode);
    int UpdateMergeMode(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, Candidate candidate);
    int64_t AddNewContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId);
    int UpdateRawContact(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int rawId, int64_t contactsId);
    std::vector<OHOS::NativeRdb::ValuesBucket> ResultSetToValuesBucket(
        std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet);
};
} // namespace Contacts
} // namespace OHOS
#endif // MATCHCANDIDATE_H
