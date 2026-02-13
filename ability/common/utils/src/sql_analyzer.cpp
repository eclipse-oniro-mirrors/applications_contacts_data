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
#include "sql_analyzer.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace Contacts {
constexpr int POS_ADD_TWO = 2;

SqlAnalyzer::SqlAnalyzer()
{
}

SqlAnalyzer::~SqlAnalyzer()
{
}

bool SqlAnalyzer::CheckValuesBucket(const OHOS::NativeRdb::ValuesBucket &value)
{
    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    value.GetAll(valuesMap);
    for (auto it = valuesMap.begin(); it != valuesMap.end(); ++it) {
        std::string key = it->first;
        bool isKey = FindIllegalWords(key);
        if (isKey) {
            HILOG_ERROR("SqlAnalyzer CheckValuesBucket key is %{public}s error", key.c_str());
            return false;
        }
        NativeRdb::ValueObject value = it->second;
        if (value.GetType() == NativeRdb::ValueObjectType::TYPE_STRING) {
            std::string str;
            value.GetString(str);
            str = ParseSpecial(str);
            bool isValue = FindIllegalWords(str);
            if (isValue) {
                HILOG_ERROR("SqlAnalyzer CheckValuesBucket value is %{public}s error", str.c_str());
                return false;
            }
        }
    }
    return true;
}

bool SqlAnalyzer::CharCheck(char &ch, std::string sql, std::size_t &pos)
{
    if (ch == '[') {
        pos++;
        std::size_t found = sql.find(']', pos);
        if (found == std::string::npos) {
            return true;
        }
        pos++;
    }
    if (ch == '-' && PickChar(sql, pos + 1) == '-') {
        pos += POS_ADD_TWO;
        std::size_t found = sql.find('\n', pos);
        if (found == std::string::npos) {
            return true;
        }
        pos++;
    }
    if (ch == '/' && PickChar(sql, pos + 1) == '*') {
        pos += POS_ADD_TWO;
        std::size_t found = sql.find("*/", pos);
        if (found == std::string::npos) {
            return true;
        }
        pos += POS_ADD_TWO;
    }
    if (ch == ';') {
        return true;
    }
    pos++;
    return false;
}

bool SqlAnalyzer::StrCheck(char &ch, std::size_t strlen, std::string sql, std::size_t &pos)
{
    if (IsInStr(ch, "'\"`") == 0) {
        pos++;
        while (pos < strlen) {
            std::size_t found = sql.find(ch, pos);
            if (found == std::string::npos) {
                return true;
            }
            if (PickChar(sql, pos + 1) != ch) {
                break;
            }
            pos += POS_ADD_TWO;
        }
    }
    return false;
}

bool SqlAnalyzer::FindIllegalWords(std::string sql)
{
    if (sql.empty()) {
        return false;
    }
    std::size_t pos = 0;
    std::size_t strlen = sql.length();
    while (pos < strlen) {
        char ch = PickChar(sql, pos);
        if (IsLetter(ch)) {
            std::size_t start = pos;
            pos++;
            while (IsLetterNumber(PickChar(sql, pos))) {
                pos++;
            }
            std::size_t count = pos - start + 1;
            sql.substr(start, count);
        }
        if (IsInStr(ch, "'\"`") == 0) {
            if (StrCheck(ch, strlen, sql, pos)) {
                return true;
            } else {
                continue;
            }
        }
        if (CharCheck(ch, sql, pos)) {
            return true;
        } else {
            continue;
        }
    }
    return false;
}
bool SqlAnalyzer::CheckColumnExists(OHOS::NativeRdb::RdbStore &store, std::string table, std::string column)
{
    std::string querySql = "SELECT * FROM " + table + " LIMIT 0";
    auto resultSet = store.QuerySql(querySql);
    if (resultSet == nullptr) {
        HILOG_ERROR("CheckColumnExists QuerySqlResult is null");
        return false;
    }
    int columnIndex = -1;
    resultSet->GetColumnIndex(column, columnIndex);
    HILOG_WARN("SqlAnalyzer CheckColumnExists columnIndex is %{public}d", columnIndex);
    resultSet->Close();
    if (columnIndex == -1) {
        return false;
    } else {
        return true;
    }
}

std::vector<std::string> SqlAnalyzer::QueryAllTables(OHOS::NativeRdb::RdbStore &store)
{
    std::vector<std::string> tables{};
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table';";
    auto resultSet = store.QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryAllTables is nullptr.");
        return tables;
    }
    int rowRecord = resultSet->GoToFirstRow();
    while (rowRecord == OHOS::NativeRdb::E_OK) {
        std::string tableName;
        resultSet->GetString(INDEX_ZERO, tableName);
        tables.push_back(tableName);
        rowRecord = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return tables;
}

std::vector<std::string> SqlAnalyzer::QueryAllViews(OHOS::NativeRdb::RdbStore &store)
{
    std::vector<std::string> views{};
    std::string sql = "SELECT name FROM sqlite_master WHERE type='view';";
    auto resultSet = store.QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryAllViews is nullptr.");
        return views;
    }
    int rowRecord = resultSet->GoToFirstRow();
    while (rowRecord == OHOS::NativeRdb::E_OK) {
        std::string viewName;
        resultSet->GetString(INDEX_ZERO, viewName);
        views.push_back(viewName);
        rowRecord = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return views;
}

std::vector<std::string> SqlAnalyzer::QueryAllFieldsFromTable(
    OHOS::NativeRdb::RdbStore &store, const std::string &table)
{
    std::vector<std::string> fields{};
    std::string sql = "SELECT name FROM pragma_table_info('" + table + "');";
    auto resultSet = store.QuerySql(sql);
    if (resultSet == nullptr) {
        HILOG_ERROR("QueryAllFieldsFromTable %{public}s is nullptr.", table.c_str());
        return fields;
    }
    int rowRecord = resultSet->GoToFirstRow();
    while (rowRecord == OHOS::NativeRdb::E_OK) {
        std::string fieldName;
        resultSet->GetString(INDEX_ZERO, fieldName);
        fields.push_back(fieldName);
        rowRecord = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return fields;
}
} // namespace Contacts
} // namespace OHOS