/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef CONTACTSDATAABILITY_BOARD_REPORT_UTIL_H
#define CONTACTSDATAABILITY_BOARD_REPORT_UTIL_H

#include <string>
#include "common.h"

namespace OHOS {
namespace Contacts {
class BoardReportUtil {
public:
    // 云空间状态打点
    static void BoardReportCloudState(int code);
    // 数据库迁移打点
    static void BoardReportContactDbInfo(ContactDbInfo dbType, const std::string &dbName, int dbResult,
                                         const std::string &faultInfo = "");
    static void InsertCallLogReport(ExecuteResult result = ExecuteResult::SUCCESS,
                                    const std::string &quickSearchKey = "", int callDirection = -1,
                                    const std::string &faultInfo = "", const int defaultMain = -1);
    static void DeleteCallLogReport(ExecuteResult result = ExecuteResult::SUCCESS, int size = 1,
                                    const std::string &faultInfo = "");
    static void ThirdLogoutReport();
    static void MigrationReport(int type, int result, int count, const std::string &faultInfo = "");
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACTSDATAABILITY_BOARD_REPORT_UTIL_H
