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

#ifndef VOICEMAIL_DATABASE_H
#define VOICEMAIL_DATABASE_H

#include "datashare_predicates.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"
#include "value_object.h"

#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
class VoiceMailDataBase {
public:
    static std::shared_ptr<VoiceMailDataBase> GetInstance();
    static std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
    int64_t InsertVoiceMail(std::string tableName, OHOS::NativeRdb::ValuesBucket insertValues);
    int UpdateVoiceMail(OHOS::NativeRdb::ValuesBucket values, OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int DeleteVoiceMail(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> Query(
        OHOS::NativeRdb::RdbPredicates &rdbPredicates, const std::vector<std::string> columns);
    int BeginTransaction();
    int Commit();
    int RollBack();

private:
    VoiceMailDataBase();
    VoiceMailDataBase(const VoiceMailDataBase &);
    const VoiceMailDataBase &operator=(const VoiceMailDataBase &);
    static std::shared_ptr<VoiceMailDataBase> voiceMailDataBase_;
};
} // namespace Contacts
} // namespace OHOS
#endif // VOICEMAIL_DATABASE_H
