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
#ifndef APPLICATIONS_CONTACTS_DATA_BLOCKLIST_DATABASE_H
#define APPLICATIONS_CONTACTS_DATA_BLOCKLIST_DATABASE_H
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
#include "contacts_database.h"
#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"
#include "datashare_helper.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "dataobs_mgr_client.h"

namespace OHOS {
namespace Contacts {

static std::shared_ptr<ContactsDataBase> contactsDataBase_;
class BlocklistDataBase {
public:
    static std::shared_ptr<BlocklistDataBase> GetInstance();
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
    int BeginTransaction();
    int Commit();
    int RollBack();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> Query(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, std::vector<std::string> &columns);
    
private:
    BlocklistDataBase();
    BlocklistDataBase(const BlocklistDataBase &);
    const BlocklistDataBase &operator=(const BlocklistDataBase &);
    static std::shared_ptr<BlocklistDataBase> blocklistDataBase_;
    static std::shared_ptr<ContactsDataBase> contactsDataBase_;
    static std::shared_ptr<DataShare::DataShareHelper> dataShareBlocklistHelper_;
};

class SqliteOpenHelperBlocklistCallback : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    int OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
};
} // namespace Contacts
} // namespace OHOS

#endif //APPLICATIONS_CONTACTS_DATA_BLOCKLIST_DATABASE_H
