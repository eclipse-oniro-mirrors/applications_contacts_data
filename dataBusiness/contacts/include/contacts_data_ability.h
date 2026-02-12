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
#include "telephony_permission.h"
#include "want.h"

#include "blocklist_database.h"
#include "contacts_database.h"
#include "contact_connect_ability.h"

namespace OHOS {
namespace AbilityRuntime {
class ContactsDataAbility : public DataShare::DataShareExtAbility {
public:
    ContactsDataAbility();
    virtual ~ContactsDataAbility() override;
    static ContactsDataAbility *Create();
    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;
    virtual int Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value) override;
    virtual int BatchInsert(const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values) override;
    virtual int BatchInsertByMigrate(int code, const Uri &uri,
        const std::vector<DataShare::DataShareValuesBucket> &values);
    virtual void OnStart(const Want &want) override;
    virtual int Update(const Uri &uri, const DataShare::DataSharePredicates &predicates,
        const DataShare::DataShareValuesBucket &value) override;
    virtual int Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates) override;
    virtual std::shared_ptr<DataShare::DataShareResultSet> Query(const Uri &uri,
        const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
        DataShare::DatashareBusinessError &businessError) override;
    // 转换Bucket，根据DataShareValuesBucket，生成一个RdbBucket
    static void transferDataShareToRdbBucket(const std::vector<DataShare::DataShareValuesBucket> &values,
        std::vector<OHOS::NativeRdb::ValuesBucket> &valuesRdb);
    static std::string getStringValueFromRdbBucket(const OHOS::NativeRdb::ValuesBucket &value,
        const std::string &colName);
    static int getIntValueFromRdbBucket(const OHOS::NativeRdb::ValuesBucket &value,
        const std::string &colName);
    static int UriParseAndSwitch(Uri &uri);
    static void SwitchProfile(Uri &uri);
    void handleUpdateCalllogAfterBatchInsertData(int ret, int code,
        const std::vector<DataShare::DataShareValuesBucket> &values);
    void handleUpdateCalllogAfterInsertData(int code,
        const DataShare::DataShareValuesBucket &value);
private:
    static std::shared_ptr<Contacts::ContactsDataBase> contactDataBase_;
    static std::shared_ptr<Contacts::BlocklistDataBase> blocklistDataBase_;
    static std::shared_ptr<Contacts::ProfileDatabase> profileDataBase_;
    static std::shared_ptr<Contacts::ContactConnectAbility> contactsConnectAbility_;
    static std::map<std::string, int> uriValueMap_;
    int InsertExecute(int &code, const OHOS::NativeRdb::ValuesBucket &value, std::string isSync);
    std::string UriParseParam(Uri &uri);
    std::string UriParseBatchParam(Uri &uri);
    std::string UriParseHandleTypeParam(Uri &uri);
    bool QueryExecute(std::shared_ptr<OHOS::NativeRdb::ResultSet> &result,
        DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp, int &parseCode,
        OHOS::Uri &uri);
    void AddQueryNotCCardGroupCondition (OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    void AddQueryNotDeleteCondition (OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    bool QueryExecuteSwitchSplit(std::shared_ptr<OHOS::NativeRdb::ResultSet> &result,
        DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp, int &parseCode,
        OHOS::Uri &uri);
    bool QueryExecuteSwitchSplitOthers(std::shared_ptr<OHOS::NativeRdb::ResultSet> &result,
        DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp, int &parseCode,
        OHOS::Uri &uri);
    void UpdateExecute(int &retCode, int code, const OHOS::NativeRdb::ValuesBucket &value,
        DataShare::DataSharePredicates &dataSharePredicates, std::string isSync);
    void SwitchUpdate(int &retCode, int &code, const OHOS::NativeRdb::ValuesBucket &value,
        DataShare::DataSharePredicates &dataSharePredicates);
    void SwitchUpdateOthers(int &retCode, int &code, const OHOS::NativeRdb::ValuesBucket &value,
        DataShare::DataSharePredicates &dataSharePredicates);
    void DeleteExecute(int &retCode, int code, DataShare::DataSharePredicates &dataSharePredicates,
        std::string isSync, std::string handleType);
    void DeleteExecuteSwitchSplit(int &retCode, int code, DataShare::DataSharePredicates &dataSharePredicates,
        std::string isSync, std::string handleType);
    void DataBaseNotifyChange(int code, Uri uri);
    void DataBaseNotifyChange(int code, Uri uri, std::string isSync);
    void HandleHwRelationData(const std::vector<DataShare::DataShareValuesBucket> &values);
    bool IsBeginTransactionOK(int code, std::mutex &mutex);
    bool IsCommitOK(int code, std::mutex &mutex);
    int BackUp();
    int Recover(int &code);
    void generateDisplayNameBucket(std::vector<DataShare::DataShareValuesBucket> &values);
    std::string generateDisplayName(int &typeId, std::string &fromDetailInfo);
    std::string generateDisplayName(const std::vector<OHOS::NativeRdb::ValuesBucket> &valuesRdb);
    void putDisplayNameInfo(DataShare::DataShareValuesBucket &displayNameBucket,
        std::string &displayName, int rawContactId);
    void transferTypeInfo(std::vector<DataShare::DataShareValuesBucket> &values,
        std::vector<OHOS::NativeRdb::ValuesBucket> &valuesRdb);
    int batchInsertHandleOneByOne(const Uri &uri, std::string isSyncFromCloud,
        int code, std::vector<DataShare::DataShareValuesBucket> &valuesHandle);
    int HandleContactPortrait(DataShare::DataShareValuesBucket &value);
    std::string GetStringValueFromDataShareValue(const DataShare::DataShareValuesBucket &dataShareValue,
        const std::string &valueName, bool &isValid);
    void BuildPortraitValue(DataShare::DataShareValuesBucket &value, const std::string &contactId,
        const std::string &rawContactId, const std::vector<uint8_t> &blob, int32_t blobSource);
    void GetPortraitValue(const DataShare::DataShareValuesBucket &dataShareValue,
    std::string &portraitFileName, std::string &contactId, std::string &rawContactId, bool &isValid);
    int SavePortraitFileAndBlob(const std::string &srcFilePath, const std::string &filePath,
        std::vector<uint8_t> &blob, int32_t &blobSource);
    int ProcessSingleInsert(DataShare::DataShareValuesBucket &rawContactValues,
    bool &isContainEvent, int &rawContactId, int &code, std::string &isSyncFromCloud);
};
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // CONTACTSDATAABILITY_CONTACT_DATA_ABILITY_TEST_H
