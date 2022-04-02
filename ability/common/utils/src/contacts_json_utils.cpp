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

#include "contacts_json_utils.h"

#include "contacts_columns.h"

namespace OHOS {
namespace Contacts {
ContactsJsonUtils::ContactsJsonUtils(void)
{
}

ContactsJsonUtils::~ContactsJsonUtils()
{
}

std::string ContactsJsonUtils::GetDeleteData(std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet)
{
    Json::Value dataResult;
    Json::Value arrayValue;
    ConvertResultSet(arrayValue, resultSet);
    dataResult[AliasName::DATA] = arrayValue;
    Json::StreamWriterBuilder builder;
    const std::string personal_ringtone = Json::writeString(builder, dataResult);
    return personal_ringtone;
}

void ContactsJsonUtils::ConvertResultSet(
    Json::Value &arrayValue, std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet)
{
    int resultSetNum = resultSet->GoToFirstRow();
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    while (resultSetNum == OHOS::NativeRdb::E_OK) {
        Json::Value data;
        unsigned int size = columnNames.size();
        for (unsigned int i = 0; i < size; i++) {
            GetValue(columnNames, i, data, resultSet);
        }
        arrayValue.append(data);
        resultSetNum = resultSet->GoToNextRow();
    }
}

void ContactsJsonUtils::GetValue(std::vector<std::string> &columnNames, unsigned int &index, Json::Value &data,
    std::unique_ptr<OHOS::NativeRdb::AbsSharedResultSet> &resultSet)
{
    std::string typeValue = columnNames[index];
    int columnIndex;
    resultSet->GetColumnIndex(typeValue, columnIndex);
    OHOS::NativeRdb::ColumnType columnType;
    resultSet->GetColumnType(columnIndex, columnType);
    if (columnType == OHOS::NativeRdb::ColumnType::TYPE_INTEGER) {
        int intValue;
        resultSet->GetInt(columnIndex, intValue);
        data[typeValue] = intValue;
    } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_FLOAT) {
        double doubleValue;
        resultSet->GetDouble(columnIndex, doubleValue);
        data[typeValue] = doubleValue;
    } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_STRING) {
        std::string stringValue;
        resultSet->GetString(columnIndex, stringValue);
        if (!stringValue.empty()) {
            data[typeValue] = stringValue;
        }
    }
}
} // namespace Contacts
} // namespace OHOS