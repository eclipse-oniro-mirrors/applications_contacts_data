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

#ifndef CONTACTSDATAABILITY_CONTACT_DATA_ABILITY_TEST_H
#define CONTACTSDATAABILITY_CONTACT_DATA_ABILITY_TEST_H

#include <map>

#include "abs_shared_result_set.h"
#include "datashare_ext_ability.h"
#include "datashare_values_bucket.h"
#include "predicates_convert.h"
#include "rdb_predicates.h"
#include "want.h"

#include "contacts_database.h"

namespace OHOS {
namespace AbilityRuntime {
class ContactsDataAbility : public DataShare::DataShareExtAbility {
public:
    ContactsDataAbility();
    virtual ~ContactsDataAbility() override;
    static ContactsDataAbility* Create();
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
    static std::shared_ptr<Contacts::ContactsDataBase> contactDataBase_;
    static std::shared_ptr<Contacts::ProfileDatabase> profileDataBase_;
    static std::map<std::string, int> uriValueMap_;
    int InsertExecute(int &code, const OHOS::NativeRdb::ValuesBucket &value);
    int UriParseAndSwitch(Uri &uri);
    void SwitchProfile(Uri &uri);
    bool QueryExecute(std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &result,
        DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp,
        int &parseCode);
    bool QueryExecuteSwitchSplit(std::shared_ptr<OHOS::NativeRdb::AbsSharedResultSet> &result,
        DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp,
        int &parseCode);
    void UpdateExecute(int &retCode, int code, const OHOS::NativeRdb::ValuesBucket &value,
        DataShare::DataSharePredicates &dataSharePredicates);
    void SwitchUpdate(int &retCode, int &code, const OHOS::NativeRdb::ValuesBucket &value,
        DataShare::DataSharePredicates &dataSharePredicates);
    void DeleteExecute(int &retCode, int code, DataShare::DataSharePredicates &dataSharePredicates);
    void DataBaseNotifyChange(int code, Uri uri);
    bool IsBeginTransactionOK(int code, std::mutex &mutex);
    bool IsCommitOK(int code, std::mutex &mutex);
    int BackUp();
    int Recover(int &code);
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // CONTACTSDATAABILITY_CONTACT_DATA_ABILITY_TEST_H
