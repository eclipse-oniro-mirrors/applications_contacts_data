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

#ifndef CALLLOG_ABILITY_H
#define CALLLOG_ABILITY_H

#include "abs_shared_result_set.h"
#include "datashare_ext_ability.h"
#include "datashare_values_bucket.h"
#include "want.h"

#include "calllog_database.h"

namespace OHOS {
namespace AbilityRuntime {
class CallLogAbility : public DataShare::DataShareExtAbility {
public:
    CallLogAbility();
    virtual ~CallLogAbility() override;
    static CallLogAbility* Create();
    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;
    virtual int Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value) override;
    virtual int BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values) override;
    virtual void OnStart(const Want &want) override;
    virtual int Update(const Uri &uri, const DataShare::DataSharePredicates &predicates,
        const DataShare::DataShareValuesBucket &value) override;
    virtual int Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates) override;
    virtual std::shared_ptr<DataShare::DataShareResultSet> Query(const Uri &uri,
        const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns) override;

private:
    static std::shared_ptr<Contacts::CallLogDataBase> callLogDataBase_;
    static std::map<std::string, int> uriValueMap_;
    int UriParse(Uri &uri);
    int InsertExecute(const Uri &uri, const OHOS::NativeRdb::ValuesBucket &value);
    void DataBaseNotifyChange(int code, Uri uri);
    bool IsBeginTransactionOK(int code, std::mutex &mutex);
    bool IsCommitOk(int code, std::mutex &mutex);
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // CALLLOG_ABILITY_H
