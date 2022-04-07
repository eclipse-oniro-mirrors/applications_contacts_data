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

#include "ability.h"
#include "ability_loader.h"
#include "predicates_convert.h"
#include "rdb_predicates.h"
#include "want.h"

#include "contacts_database.h"

namespace OHOS {
namespace AppExecFwk {
class ContactsDataAbility : public Ability {
public:
    ContactsDataAbility();
    ~ContactsDataAbility();
    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override;
    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override;
    virtual void OnStart(const Want &want) override;
    virtual int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override;
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override;
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri &uri,
        const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates) override;
    virtual void Dump(const std::string &extra) override;

private:
    static std::shared_ptr<Contacts::ContactsDataBase> contactDataBase_;
    static std::shared_ptr<Contacts::ProfileDatabase> profileDataBase_;
    static std::map<std::string, int> uriValueMap_;
    int InsertExecute(int &code, const NativeRdb::ValuesBucket &value);
    int UriParseAndSwitch(Uri &uri);
    void SwitchProfile(Uri &uri);
    void QueryExecute(std::shared_ptr<NativeRdb::AbsSharedResultSet> &result,
        OHOS::NativeRdb::DataAbilityPredicates &dataAbilityPredicates, std::vector<std::string> &columnsTemp,
        int &parseCode);
    void QueryExecuteSwitchSplit(std::shared_ptr<NativeRdb::AbsSharedResultSet> &result,
        OHOS::NativeRdb::DataAbilityPredicates &dataAbilityPredicates, std::vector<std::string> &columnsTemp,
        int &parseCode);
    void UpdateExecute(int &retCode, int code, const NativeRdb::ValuesBucket &value,
        OHOS::NativeRdb::DataAbilityPredicates &dataAbilityPredicates);
    void SwitchUpdate(int &retCode, int &code, const NativeRdb::ValuesBucket &value,
        OHOS::NativeRdb::DataAbilityPredicates &dataAbilityPredicates);
    void DeleteExecute(int &retCode, int code, OHOS::NativeRdb::DataAbilityPredicates &dataAbilityPredicates);
    void DataBaseNotifyChange(int code, Uri uri);
    bool IsBeginTransactionOK(int code, std::mutex &mutex);
    bool IsCommitOK(int code, std::mutex &mutex);
    int BackUp();
    int Recover(int &code);
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // CONTACTSDATAABILITY_CONTACT_DATA_ABILITY_TEST_H
