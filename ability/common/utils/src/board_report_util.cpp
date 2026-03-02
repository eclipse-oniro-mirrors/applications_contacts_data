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

#include "board_report_util.h"
#include "contact_connect_ability.h"

namespace OHOS {
namespace Contacts {

const std::string SEPARATOR = ";";
void BoardReportUtil::BoardReportCloudState(int code)
{
    // 看板上报，云空间是否已满
    std::string paramStr = "CONTACTDB_CLOUD_STATE;";
    paramStr.append(std::to_string(code));
    paramStr.append(SEPARATOR);
    OHOS::Contacts::ContactConnectAbility::GetInstance()->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
}

void BoardReportUtil::BoardReportContactDbInfo(ContactDbInfo dbType, const std::string &dbName, int dbResult,
                                               const std::string &faultInfo) {
    // 看板上报，版本升级
    std::string paramStr = "DB_VERSION_INFO;";
    paramStr.append(std::to_string(static_cast<int>(dbType)));
    paramStr.append(SEPARATOR);
    paramStr.append(dbName);
    paramStr.append(SEPARATOR);
    paramStr.append(std::to_string(dbResult));
    paramStr.append(SEPARATOR);
    paramStr.append(faultInfo);
    paramStr.append(SEPARATOR);
    OHOS::Contacts::ContactConnectAbility::GetInstance()->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
}

void BoardReportUtil::InsertCallLogReport(ExecuteResult result, const std::string &quickSearchKey, int callDirection,
                                          const std::string &faultInfo, const int defaultMain) {
    // 插入上报异步发送通知
    std::string paramStr = "CONTACTDB_CALL_INSERT;";
    paramStr.append(std::to_string(static_cast<int>(result)));
    paramStr.append(SEPARATOR);
    paramStr.append(quickSearchKey);
    paramStr.append(SEPARATOR);
    paramStr.append(std::to_string(callDirection));
    paramStr.append(SEPARATOR);
    paramStr.append(faultInfo);
    paramStr.append(SEPARATOR);
    paramStr.append(std::to_string(defaultMain));
    OHOS::Contacts::ContactConnectAbility::GetInstance()->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
}

void BoardReportUtil::DeleteCallLogReport(ExecuteResult result, int size, const std::string &faultInfo) {
    // 删除上报异步发送通知
    std::string paramStr = "CONTACTDB_CALL_DELETE;";
    paramStr.append(std::to_string(static_cast<int>(result)));
    paramStr.append(SEPARATOR);
    paramStr.append(std::to_string(size));
    paramStr.append(SEPARATOR);
    paramStr.append(faultInfo);
    OHOS::Contacts::ContactConnectAbility::GetInstance()->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
}

void BoardReportUtil::ThirdLogoutReport()
{
    // 退出三方联系人上报异步发送通知
    std::string paramStr = "CONTACTDB_LOGOUT_THIRD_DELETE;";
    OHOS::Contacts::ContactConnectAbility::GetInstance()->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
}

void BoardReportUtil::MigrationReport(int type, int result, int count, const std::string &faultInfo)
{
    // EL1迁移EL5
    std::string paramStr = "CONTACTDB_DB_MIGRATION;";
    paramStr.append(std::to_string(type));
    paramStr.append(SEPARATOR);
    paramStr.append(std::to_string(result));
    paramStr.append(SEPARATOR);
    paramStr.append(std::to_string(count));
    paramStr.append(SEPARATOR);
    paramStr.append(faultInfo);
    OHOS::Contacts::ContactConnectAbility::GetInstance()->ConnectAbility("", "", "", "", "localChangeReport", paramStr);
}

} // namespace Contacts
} // namespace OHOS