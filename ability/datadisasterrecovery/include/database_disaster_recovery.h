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

#ifndef DATABASE_DISASTER_RECOVERY_H
#define DATABASE_DISASTER_RECOVERY_H

#include <string>

#include "rdb_store.h"

namespace OHOS {
namespace Contacts {
class DataBaseDisasterRecovery {
public:
    static std::shared_ptr<DataBaseDisasterRecovery> GetInstance();
    ~DataBaseDisasterRecovery();
    int SQLiteCheckDb();
    int BackDatabase();
    int BackDatabase(std::string dataBaseName);
    std::string GetBackUpDatabase(const std::shared_ptr<OHOS::NativeRdb::RdbStore> &store_);
    int RecoveryDatabase(std::string dataBaseName);

private:
    DataBaseDisasterRecovery();
    static std::shared_ptr<DataBaseDisasterRecovery> instance_;
    static std::map<std::string, std::shared_ptr<OHOS::NativeRdb::RdbStore>> redbStoreMap;
    int SQLiteCheckDb(std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore, std::string dataBaseName);
    static const std::string BACKUP_LINK_SYMBOL;
    static const std::string BACKUP_SUFFIX;
    static const std::string DB_OK;
};
} // namespace Contacts
} // namespace OHOS
#endif // DATABASE_DISASTER_RECOVERY_H
