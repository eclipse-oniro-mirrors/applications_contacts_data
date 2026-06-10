/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef CALLLOGCHECK_ABILITY_H
#define CALLLOGCHECK_ABILITY_H

#include "abs_shared_result_set.h"
#include "datashare_ext_ability.h"
#include "datashare_values_bucket.h"
#include "telephony_permission.h"
#include "want.h"

#include "calllog_database.h"

namespace OHOS {
namespace AbilityRuntime {
class CallLogCheckAbility : public DataShare::DataShareExtAbility {
public:
    CallLogCheckAbility();
    virtual ~CallLogCheckAbility() override;
    virtual void OnStart(const Want &want) override;
    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;
    virtual std::shared_ptr<DataShare::DataShareResultSet> Query(const Uri &uri,
        const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
        DataShare::DatashareBusinessError &businessError) override;

private:
    static std::shared_ptr<Contacts::CallLogDataBase> callLogDataBase_;
    static std::map<std::string, int> uriValueMap_;
    int UriParse(Uri &uri);
    void AddQueryNotPrivacyCondition(OHOS::NativeRdb::RdbPredicates &rdbPredicates);
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // CALLLOGCHECK_ABILITY_H
