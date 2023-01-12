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

#ifndef CALLLOG_DATABASE_H
#define CALLLOG_DATABASE_H

#include <pthread.h>

#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "value_object.h"

#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
class CallLogDataBase {
public:
    static std::shared_ptr<CallLogDataBase> GetInstance();
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
    int64_t InsertCallLog(OHOS::NativeRdb::ValuesBucket insertValues);
    int UpdateCallLog(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteCallLog(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> Query(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> columns);
    int BeginTransaction();
    int Commit();
    int RollBack();
    void QueryContactsByInsertCalls(OHOS::NativeRdb::ValuesBucket &insertValues);

private:
    CallLogDataBase();
    CallLogDataBase(const CallLogDataBase &);
    const CallLogDataBase &operator=(const CallLogDataBase &);
    static std::shared_ptr<CallLogDataBase> callLogDataBase_;
    int UpdateTopContact(OHOS::NativeRdb::ValuesBucket &insertValues);
};

class SqliteOpenHelperCallLogCallback : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    int OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
};
} // namespace Contacts
} // namespace OHOS

#endif // CALLLOG_DATABASE_H
