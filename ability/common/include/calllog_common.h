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

#ifndef CONTACTS_DATA_CALLLOG_COMMON_H
#define CONTACTS_DATA_CALLLOG_COMMON_H

#include <string>
#include <map>
#include "contacts_columns.h"

namespace OHOS {
namespace Contacts {
constexpr const char *CREATE_CALLLOG =
    "CREATE TABLE IF NOT EXISTS [calllog]("
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[slot_id] INTEGER NOT NULL DEFAULT 0, "
    "[phone_number] TEXT, "
    "[display_name] TEXT, "
    "[call_direction] INTEGER NOT NULL DEFAULT 0, "
    "[voicemail_uri] TEXT, "
    "[sim_type] INTEGER , "
    "[is_hd] INTEGER , "
    "[is_read] INTEGER NOT NULL DEFAULT 0, "
    "[ring_duration] INTEGER NOT NULL DEFAULT 0, "
    "[talk_duration] INTEGER NOT NULL DEFAULT 0, "
    "[format_number] TEXT, "
    "[quicksearch_key] TEXT, "
    "[number_type] INTEGER, "
    "[number_type_name] TEXT, "
    "[begin_time] INTEGER NOT NULL DEFAULT 0, "
    "[end_time] INTEGER NOT NULL DEFAULT 0, "
    "[answer_state] INTEGER , "
    "[create_time] INTEGER, "
    "[number_location] TEXT, "
    "[photo_id] INTEGER, "
    "[photo_uri] TEXT, "
    "[country_iso_code] INTEGER, "
    "[countryiso] TEXT, "
    "[mark_type] TEXT, "
    "[mark_content] TEXT, "
    "[mark_details] TEXT, "
    "[mark_source] TEXT, "
    "[block_reason] INTEGER, "
    "[is_cloud_mark] INTEGER DEFAULT 0, "
    "[mark_count] INTEGER DEFAULT 0, "
    "[hw_is_satellite] INTEGER DEFAULT 0, "
    "[is_block] INTEGER DEFAULT 0, "
    "[celia_call_type] INTEGER DEFAULT -1, "
    "[privacy_tag] INTEGER DEFAULT -1, "
    "[extra1] TEXT, "
    "[extra2] TEXT, "
    "[extra3] TEXT, "
    "[extra4] TEXT, "
    "[extra5] TEXT, "
    "[extra6] TEXT, "
    "[features] INTEGER DEFAULT 0, "
    "[format_phone_number] TEXT, "
    "[is_cnap] INTEGER DEFAULT 0, "
    "[new_calling] INTEGER DEFAULT 0, "
    "[detect_details] TEXT, "
    "[start_time] INTEGER DEFAULT 0, "
    "[abstract_type] TEXT, "
    "[abstract_profile] VARCHAR(256), "
    "[abs_status] TEXT, "
    "[abs_code] TEXT DEFAULT '-1', "
    "[fail_abs_record_id] INTEGER DEFAULT 0, "
    "[fail_abs_record_status] TEXT, "
    "[fail_abs_record_updater] TEXT, "
    "[fail_abs_record_update_time] TEXT, "
    "[abs_fin_times_code] TEXT, "
    "[notes_id] TEXT, "
    "[notes_status] TEXT, "
    "[notes_status_updater] TEXT, "
    "[notes_status_update_time] TEXT);";

constexpr const char *CREATE_VOICEMAIL =
    "CREATE TABLE IF NOT EXISTS [voicemail]("
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[phone_number] TEXT, "
    "[quicksearch_key] TEXT, "
    "[display_name] TEXT, "
    "[voicemail_uri] TEXT, "
    "[voicemail_type] INTEGER NOT NULL DEFAULT 0, "
    "[voice_file_size] INTEGER NOT NULL DEFAULT 0, "
    "[voice_duration] INTEGER NOT NULL DEFAULT 0, "
    "[voice_status] INTEGER NOT NULL DEFAULT 0, "
    "[origin_type] TEXT, "
    "[create_time] INTEGER);";

constexpr const char *CREATE_REPLYING =
    "CREATE TABLE IF NOT EXISTS [replying]("
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[name] TEXT, "
    "[replying_uri] TEXT, "
    "[replying_path] TEXT, "
    "[phone_account_id] TEXT, "
    "[has_content] NOT NULL DEFAULT 0, "
    "[duration] INTEGER NOT NULL DEFAULT 0, "
    "[content_type] TEXT, "
    "[last_modified] INTEGER NOT NULL DEFAULT 0, "
    "[synced] INTEGER NOT NULL DEFAULT 0 );";

constexpr const char *CREATE_DATABASE_BACKUP_TASK =
    "CREATE TABLE IF NOT EXISTS [database_backup_task]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[backup_time] TEXT, "
    "[backup_path] TEXT, "
    "[remarks] TEXT)";

constexpr const char *CREATE_INSERT_BACKUP_TIME =
    "CREATE TRIGGER IF NOT EXISTS [insert_database_backup_task] AFTER INSERT ON [database_backup_task] "
    " BEGIN "
    " UPDATE "
    " [database_backup_task] "
    " SET "
    " [backup_time] = STRFTIME ('%s', 'now') "
    " WHERE "
    " [id] = [NEW].[id]; "
    " END ";

constexpr const char *CALL_LOG_PHONE_NUMBER_INDEX =
    "CREATE INDEX IF NOT EXISTS [calllog_phone_number_index] ON [calllog] ([phone_number])";

constexpr const char *CREATE_CALL_SETTINGS =
    "CREATE TABLE IF NOT EXISTS [call_settings]("
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[calllog_change_time] DATETIME) ";

constexpr const char *INIT_CALLlOG_CHANGE_TIME =
    "INSERT INTO call_settings (calllog_change_time) VALUES (datetime('now'))";

constexpr const char *UPDATE_CONTACT_CHANGE_TIME =
    "UPDATE settings set contact_change_time = datetime('now')";

constexpr const char *SETTING_URI =
    "datashareproxy://com.ohos.contactsdataability/contacts/setting";

constexpr const char *SETTING_URI_NOT_PROXY =
    "datashare://com.ohos.contactsdataability/contacts/setting";

constexpr const char *CALL_SETTING_URI =
    "datashareproxy://com.ohos.contactsdataability/calls/call_setting";

constexpr const char *CALL_CHANGER_URI =
    "datashare:///com.ohos.calllogability/calls/calllog";

constexpr const char *CONTACT_DATA = "datashare:///com.ohos.contactsdataability/contacts/contact_data";

constexpr const char *UPDATE_CALLLOG_CHANGE_TIME =
    "UPDATE call_settings set calllog_change_time = datetime('now')";

constexpr const char *UPDATE_CALLLOG_AVATAR =
    "CREATE TRIGGER IF NOT EXISTS [update_calllog_avatar] AFTER INSERT ON [calllog] "
    "WHEN NEW.quicksearch_key = '' OR NEW.quicksearch_key IS NULL "
    "BEGIN "
    "UPDATE [calllog] "
    "SET "
    "extra4 = (SELECT extra4 FROM calllog WHERE format_phone_number = NEW.format_phone_number "
    "AND format_phone_number !='' AND extra4 IS NOT NULL LIMIT 1) "
    "WHERE rowid = NEW.rowid;"
    "END";

constexpr const char *CALL_LOG_FAIL_ABS_RECORD_ID_INDEX =
    "CREATE INDEX IF NOT EXISTS [fail_abs_record_id_index] ON [calllog] ([fail_abs_record_id])";

constexpr const char *CALL_LOG_NOTES_ID_INDEX =
    "CREATE INDEX IF NOT EXISTS [notes_id_index] ON [calllog] ([notes_id])";

// v21版本后新增字段统计
constexpr const char *CALL_LOG_ADD_IS_CNAP = "ALTER TABLE calllog ADD COLUMN is_cnap INTEGER DEFAULT 0;";
constexpr const char *CALL_LOG_ADD_PRIVACY_TAG = "ALTER TABLE calllog ADD COLUMN privacy_tag INTEGER DEFAULT -1;";
constexpr const char *CALL_LOG_ADD_NEW_CALLING = "ALTER TABLE calllog ADD COLUMN new_calling INTEGER DEFAULT 0;";
constexpr const char *CALL_LOG_ADD_START_TIME = "ALTER TABLE calllog ADD COLUMN start_time INTEGER DEFAULT 0;";
constexpr const char *CALL_LOG_ADD_ABSTRACT_TYPE = "ALTER TABLE calllog ADD COLUMN abstract_type TEXT;";
constexpr const char *CALL_LOG_ADD_ABSTRACT_PROFILE = "ALTER TABLE calllog ADD COLUMN abstract_profile VARCHAR(256);";
constexpr const char *CALL_LOG_ADD_ABS_STATUS = "ALTER TABLE calllog ADD COLUMN abs_status TEXT;";
constexpr const char *CALL_LOG_ADD_ABS_CODE = "ALTER TABLE calllog ADD COLUMN abs_code TEXT DEFAULT '-1';";
constexpr const char *CALL_LOG_ADD_FAIL_ABS_RECORD_ID =
    "ALTER TABLE calllog ADD COLUMN fail_abs_record_id INTEGER DEFAULT 0;";
constexpr const char *CALL_LOG_ADD_FAIL_ABS_RECORD_STATUS =
    "ALTER TABLE calllog ADD COLUMN fail_abs_record_status TEXT;";
constexpr const char *CALL_LOG_ADD_FAIL_ABS_RECORD_UPDATER =
    "ALTER TABLE calllog ADD COLUMN fail_abs_record_updater TEXT;";
constexpr const char *CALL_LOG_ADD_FAIL_ABS_RECORD_UPDATE_TIME =
    "ALTER TABLE calllog ADD COLUMN fail_abs_record_update_time TEXT;";
constexpr const char *CALL_LOG_ADD_ABS_FIN_TIMES_CODE = "ALTER TABLE calllog ADD COLUMN abs_fin_times_code TEXT;";
constexpr const char *CALL_LOG_ADD_NOTES_ID = "ALTER TABLE calllog ADD COLUMN notes_id TEXT;";
constexpr const char *CALL_LOG_ADD_NOTES_STATUS = "ALTER TABLE calllog ADD COLUMN notes_status TEXT;";
constexpr const char *CALL_LOG_ADD_NOTES_STATUS_UPDATER = "ALTER TABLE calllog ADD COLUMN notes_status_updater TEXT;";
constexpr const char *CALL_LOG_ADD_NOTES_STATUS_UPDATE_TIME =
    "ALTER TABLE calllog ADD COLUMN notes_status_update_time TEXT;";

enum class PrivacyTag {
    CALL_LOG_NON_PRIVACY_TAG = -1,
    CALL_LOG_UNREAD_MISSED_PRIVACY_TAG,
    CALL_LOG_PRIVACY_TAG,
    CALL_LOG_MISSED_PRIVACY_TAG,
    CALL_LOG_MIGRATED_PRIVACY_TAG,
};

const std::map<std::string, const char *> CALL_LOG_TABLES = {
    {CallsTableName::CALLLOG, CREATE_CALLLOG},
    {CallsTableName::VOICEMAIL, CREATE_VOICEMAIL},
    {CallsTableName::REPLYING, CREATE_REPLYING},
};

const std::map<std::string, const char *> CALL_LOG_ADD_COLUMNS = {
    {CallLogColumns::IS_CNAP, CALL_LOG_ADD_IS_CNAP},
    {CallLogColumns::PRIVACY_TAG, CALL_LOG_ADD_PRIVACY_TAG},
    {CallLogColumns::NEWCALL, CALL_LOG_ADD_NEW_CALLING},
    {CallLogColumns::START_TIME, CALL_LOG_ADD_START_TIME},
    {CallLogColumns::ABSTRACT_TYPE, CALL_LOG_ADD_ABSTRACT_TYPE},
    {CallLogColumns::ABSTRACT_PROFILE, CALL_LOG_ADD_ABSTRACT_PROFILE},
    {CallLogColumns::ABS_STATUS, CALL_LOG_ADD_ABS_STATUS},
    {CallLogColumns::ABS_CODE, CALL_LOG_ADD_ABS_CODE},
    {CallLogColumns::FAIL_ABS_RECORD_ID, CALL_LOG_ADD_FAIL_ABS_RECORD_ID},
    {CallLogColumns::FAIL_ABS_RECORD_STATUS, CALL_LOG_ADD_FAIL_ABS_RECORD_STATUS},
    {CallLogColumns::FAIL_ABS_RECORD_UPDATER, CALL_LOG_ADD_FAIL_ABS_RECORD_UPDATER},
    {CallLogColumns::FAIL_ABS_RECORD_UPDATE_TIME, CALL_LOG_ADD_FAIL_ABS_RECORD_UPDATE_TIME},
    {CallLogColumns::ABS_FIN_TIMES_CODE, CALL_LOG_ADD_ABS_FIN_TIMES_CODE},
    {CallLogColumns::NOTES_ID, CALL_LOG_ADD_NOTES_ID},
    {CallLogColumns::NOTES_STATUS, CALL_LOG_ADD_NOTES_STATUS},
    {CallLogColumns::NOTES_STATUS_UPDATER, CALL_LOG_ADD_NOTES_STATUS_UPDATER},
    {CallLogColumns::NOTES_STATUS_UPDATE_TIME, CALL_LOG_ADD_NOTES_STATUS_UPDATE_TIME},
};

} // namespace Contacts
} // namespace OHOS
#endif //CONTACTS_DATA_CALLLOG_COMMON_H
