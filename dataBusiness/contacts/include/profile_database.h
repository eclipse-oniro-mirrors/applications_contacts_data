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

#ifndef PROFILE_DATABASE_H
#define PROFILE_DATABASE_H

#include <pthread.h>

#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "result_set.h"

namespace OHOS {
namespace Contacts {
class ProfileDatabase {
public:
    static std::shared_ptr<ProfileDatabase> GetInstance();
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
    int BeginTransaction();
    int Commit();
    int RollBack();
    static void DestroyInstanceAndRestore(std::string restorePath);

private:
    ProfileDatabase();
    static bool Restore(std::string restorePath);
    ProfileDatabase(const ProfileDatabase &);
    const ProfileDatabase &operator=(const ProfileDatabase &);
    static std::shared_ptr<ProfileDatabase> profileDatabase_;
};

class SqliteOpenHelperProfileCallback : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    int OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
};
} // namespace Contacts
} // namespace OHOS
#endif // PROFILE_DATABASE_H
