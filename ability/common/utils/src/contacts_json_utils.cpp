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

#include <regex>
#include "contacts_json_utils.h"

#include "common.h"
#include "contacts_columns.h"
#include "hilog_wrapper.h"
#include "common_tool_type.h"

namespace OHOS {
namespace Contacts {
ContactsJsonUtils::ContactsJsonUtils(void)
{
}

ContactsJsonUtils::~ContactsJsonUtils()
{
}

std::string ContactsJsonUtils::GetDeleteData(std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
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
    Json::Value &arrayValue, std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
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
    std::shared_ptr<OHOS::NativeRdb::ResultSet> &resultSet)
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

std::string ContactsJsonUtils::GetDataFromValueBucket(std::vector<OHOS::NativeRdb::ValuesBucket> &valuesRdb)
{
    Json::Value dataResult;
    Json::Value arrayValue;
    ParseData(arrayValue, valuesRdb);
    dataResult[AliasName::DATA] = arrayValue;
    Json::StreamWriterBuilder builder;
    const std::string contactInfoJson = Json::writeString(builder, dataResult);
    return contactInfoJson;
}

void ContactsJsonUtils::ParseData(
    Json::Value &arrayValue, std::vector<OHOS::NativeRdb::ValuesBucket> &valuesRdb)
{
    unsigned int size = valuesRdb.size();
    for (unsigned int i = 0; i < size; i++) {
        // get typeId
        int typeId = getIntValueFromRdbBucket(valuesRdb[i],
            Contacts::ContactDataColumns::TYPE_ID);
        if (typeId == Contacts::ContentTypeData::PHONE_INT_VALUE ||
         typeId == Contacts::ContentTypeData::EMAIL_INT_VALUE) {
            Json::Value data;
            std::string detailInfo = getStringValueFromRdbBucket(valuesRdb[i],
                Contacts::ContactDataColumns::DETAIL_INFO);
            int contact_id = getIntValueFromRdbBucket(valuesRdb[i],
                Contacts::ContactDataColumns::RAW_CONTACT_ID);
            data["contactId"] = std::to_string(contact_id);
            data["type"] = std::to_string(typeId);
            data["value"] = detailInfo;
            arrayValue.append(data);
        }
    }
}

int ContactsJsonUtils::getIntValueFromRdbBucket(
    const OHOS::NativeRdb::ValuesBucket &valuesBucket,
    const std::string &colName)
{
    OHOS::NativeRdb::ValueObject value;
    valuesBucket.GetObject(colName, value);
    int colVal = Contacts::OPERATION_ERROR;
    if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_NULL) {
        HILOG_ERROR("get %{public}s value is nullptr, ts = %{public}lld", colName.c_str(), (long long) time(NULL));
        colVal = Contacts::OPERATION_ERROR;
    } else if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_INT) {
        value.GetInt(colVal);
    } else if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_STRING) {
        std::string tempString;
        value.GetString(tempString);
        if (!CommonToolType::ConvertToInt(tempString, colVal)) {
            HILOG_ERROR("GetContactByValue ValueObjectType String tempString = %{public}s", tempString.c_str());
        }
    } else if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_DOUBLE) {
        double temp = 0;
        value.GetDouble(temp);
        colVal = ceil(temp);
    } else {
        HILOG_ERROR("get %{public}s int value from OHOS::NativeRdb::ValuesBucket error, is other type",
            colName.c_str());
    }
    return colVal;
}

std::string ContactsJsonUtils::getStringValueFromRdbBucket(const OHOS::NativeRdb::ValuesBucket &valuesBucket,
    const std::string &colName)
{
    OHOS::NativeRdb::ValueObject value;
    std::string colVal;
    valuesBucket.GetObject(colName, value);
    value.GetString(colVal);
    return colVal;
}

} // namespace Contacts
} // namespace OHOS