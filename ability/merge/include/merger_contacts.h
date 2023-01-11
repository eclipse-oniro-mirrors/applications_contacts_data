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

#ifndef MERGERCONTACT_H
#define MERGERCONTACT_H

#include "datashare_predicates.h"
#include "rdb_store.h"
#include "result_set.h"

namespace OHOS {
namespace Contacts {
class MergerContacts {
public:
    MergerContacts();
    ~MergerContacts();
    int ForceContactMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore>, std::set<int>);
    int ContactMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore>);
    int ReContactMerge(std::shared_ptr<OHOS::NativeRdb::RdbStore>, const DataShare::DataSharePredicates &);
    std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> SelectCandidate(std::shared_ptr<OHOS::NativeRdb::RdbStore>);

private:
    void UpdateIdsByName(
        std::shared_ptr<OHOS::NativeRdb::RdbStore>, std::set<std::string>, std::set<int>, std::vector<std::set<int>> &);
    std::vector<std::set<int>> SelectIdsByName(std::shared_ptr<OHOS::NativeRdb::RdbStore>, std::set<int>);
    void UpdateCandidate(std::shared_ptr<OHOS::NativeRdb::RdbStore>, std::vector<int>, std::vector<std::set<int>> &);
    int MergeCircle(std::shared_ptr<OHOS::NativeRdb::RdbStore>, std::vector<std::set<int>>);
    std::vector<std::set<int>> QueryMergeContacts(std::shared_ptr<OHOS::NativeRdb::RdbStore>, int);
    std::string QueryCandidateName(std::shared_ptr<OHOS::NativeRdb::RdbStore>, int);
    void UpdateRawContacts(std::shared_ptr<OHOS::NativeRdb::RdbStore>, int, std::set<int>);
    void UpdateRawContactsExecute(
        std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int minContactId, std::set<int> Ids);
    std::set<int> HandleIds(std::shared_ptr<OHOS::NativeRdb::RdbStore>, std::set<int>);
    int DeleteContacts(std::shared_ptr<OHOS::NativeRdb::RdbStore>, int, std::set<int>);
    std::set<int> QueryTargetName(std::shared_ptr<OHOS::NativeRdb::RdbStore>, int, std::set<std::string>);
    bool isNameMatch(std::shared_ptr<OHOS::NativeRdb::RdbStore>, std::set<int>);
    int ManualMergeOperation(std::shared_ptr<OHOS::NativeRdb::RdbStore> store, int minId, std::set<int> handledIds);
};
} // namespace Contacts
} // namespace OHOS

#endif // MERGERCONTACT_H
