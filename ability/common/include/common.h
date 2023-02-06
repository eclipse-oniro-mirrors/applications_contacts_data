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

#ifndef COMMON_H
#define COMMON_H

#include <string>

#include "contacts_path.h"

namespace OHOS {
namespace Contacts {
// error code
constexpr int RDB_EXECUTE_OK = 0;
constexpr int RDB_EXECUTE_FAIL = -1;
constexpr int OPERATION_ERROR = -1;
constexpr int OPERATION_OK = 0;
constexpr int RDB_OBJECT_EMPTY = -1;
constexpr int PARAMETER_EMPTY = -1;

constexpr int DELETE_MARK = 1;
constexpr int ID_EMPTY = 0;
constexpr int BATCH_INSERT_COUNT = 40;

// ResultSet get Num
constexpr int RESULT_GET_ONE = 1;
constexpr int RESULT_GET_TWO = 2;

// contact table has Judge
constexpr int HAS_NAME = 1;
constexpr int HAS_PHONE = 2;
constexpr int HAS_EMAIL = 3;
constexpr int HAS_GROUP = 4;

// Contacts DataBase Code
constexpr int CONTACTS_CONTACT = 10000;
constexpr int CONTACTS_RAW_CONTACT = 10001;
constexpr int CONTACTS_CONTACT_DATA = 10002;
constexpr int CONTACTS_CONTACT_TYPE = 10003;
constexpr int CONTACTS_DELETED_RAW_CONTACT = 10004;
constexpr int CONTACTS_GROUPS = 10005;
constexpr int CONTACTS_ACCOUNT = 10006;
constexpr int CONTACTS_BLOCKLIST = 10007;
constexpr int CONTACTS_SEARCH_CONTACT = 10008;
constexpr int CONTACTS_SYNC_STATUS = 10009;
constexpr int CONTACTS_PHOTO_FILES = 100010;
constexpr int CONTACTS_LOCAL_LANG = 10011;
constexpr int CONTACTS_DATABASE_BACKUP_TASK = 10012;
constexpr int CONTACTS_DELETE = 10013;
constexpr int QUERY_MERGE_LIST = 10014;
constexpr int SPLIT_CONTACT = 10015;
constexpr int MANUAL_MERGE = 10016;
constexpr int AUTO_MERGE = 10017;
constexpr int CONTACTS_DELETE_RECORD = 10018;
constexpr int CONTACT_TYPE = 10019;
constexpr int CONTACT_BACKUP = 10020;
constexpr int CONTACT_RECOVER = 10021;

constexpr int ACCOUNT = 10020;
constexpr int CALLLOG = 20000;
constexpr int VOICEMAIL = 20001;
constexpr int REPLAYING = 20002;

// PROFILE DATABASE CODE
constexpr int PROFILE_CONTACT = 30000;
constexpr int PROFILE_RAW_CONTACT = 30001;
constexpr int PROFILE_CONTACT_DATA = 30002;
constexpr int PROFILE_CONTACT_TYPE = 30003;
constexpr int PROFILE_DELETED_RAW_CONTACT = 30004;
constexpr int PROFILE_GROUPS = 30005;
constexpr int PROFILE_ACCOUNT = 30006;
constexpr int PROFILE_BLOCKLIST = 30007;
constexpr int PROFILE_SEARCH_CONTACT = 30008;
constexpr int PROFILE_SYNC_STATUS = 30009;
constexpr int PROFILE_PHOTO_FILES = 300010;
constexpr int PROFILE_LOCAL_LANG = 30011;
constexpr int PROFILE_DATABASE_BACKUP_TASK = 30012;
constexpr int PROFILE_DELETE_RECORD = 30013;
constexpr int PROFILE_TYPE = 30014;
constexpr int PROFILE_DELETE = 30015;
constexpr int PROFILE_BACKUP = 30016;
constexpr int PROFILE_RECOVER = 30017;

// DATABASE OPEN VERSION
constexpr int DATABASE_OPEN_VERSION = 1;

// DATABASE NEW VERSION
constexpr int DATABASE_NEW_VERSION = 2;

// REQUEST PARAMS ARGS NUM
constexpr int REQUEST_PARAMS_NUM = 2;

constexpr const char *PROFILE_DATABASE_NAME = "profile";
constexpr const char *CONTACT_DATABASE_NAME = "contacts";

// Rebase
constexpr int REBASE_SETTING = 0;
constexpr int REBASE_COUNT = 5;

// MergeMode Number
constexpr int MERGE_MODE_DEFAULT = 0;
constexpr int MERGE_MODE_MANUAL = 1;
constexpr int MERGE_MODE_AUTO = 2;

// Uri
constexpr const char *CONTACT_URI = "datashare:///com.ohos.contactsdataability";
constexpr const char *VOICEMAIL_URI = "datashare:///com.ohos.voicemailability";
constexpr const char *CALL_LOG_URI = "datashare:///com.ohos.calllogability";

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
    "[extra1] TEXT, "
    "[extra2] TEXT, "
    "[extra3] TEXT, "
    "[extra4] TEXT, "
    "[extra5] TEXT, "
    "[extra6] TEXT)";

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
    "CREATE TABLE [replying]("
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

constexpr const char *CREATE_CONTACT =
    "CREATE TABLE IF NOT EXISTS [contact]("
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[name_raw_contact_id] INTEGER REFERENCES raw_contact(id), "
    "[photo_id] INTEGER, "
    "[photo_file_id] INTEGER REFERENCES [photo_files]([id]), "
    "[personal_ringtone] TEXT, "
    "[is_transfer_voicemail] INTEGER NOT NULL DEFAULT 0, "
    "[company] TEXT, "
    "[position] TEXT, "
    "[quick_search_key] TEXT, "
    "[read_only] INTEGER NOT NULL DEFAULT 0, "
    "[personal_notification_ringtone] TEXT, "
    "[has_phone_number] INTEGER NOT NULL DEFAULT 0, "
    "[has_display_name] INTEGER NOT NULL DEFAULT 0, "
    "[has_email] INTEGER NOT NULL DEFAULT 0, "
    "[has_group] INTEGER NOT NULL DEFAULT 0)";

constexpr const char *CREATE_CONTACT_INDEX =
    "CREATE INDEX IF NOT EXISTS [contact_name_raw_contact_id_index] "
    "ON [contact] ([name_raw_contact_id])";

constexpr const char *CREATE_RAW_CONTACT =
    "CREATE TABLE IF NOT EXISTS [raw_contact]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[contact_id] INTEGER REFERENCES [contact]([id]), "
    "[photo_id] INTEGER, "
    "[photo_file_id] INTEGER REFERENCES [photo_files]([id]), "
    "[is_transfer_voicemail] INTEGER NOT NULL DEFAULT 0, "
    "[personal_ringtone] TEXT, "
    "[is_deleted] INTEGER NOT NULL DEFAULT 0, "
    "[personal_notification_ringtone] TEXT, "
    "[photo_first_name] TEXT, "
    "[account_id] INTEGER, "
    "[version] INTEGER NOT NULL DEFAULT 0, "
    "[display_name] TEXT, "
    "[sort] TEXT, "
    "[contacted_count] INTEGER NOT NULL DEFAULT 0, "
    "[lastest_contacted_time] INTEGER NOT NULL DEFAULT 0, "
    "[favorite] INTEGER NOT NULL DEFAULT 0, "
    "[favorite_order] TEXT, "
    "[phonetic_name] TEXT, "
    "[phonetic_name_type] INTEGER , "
    "[company] TEXT, "
    "[position] TEXT, "
    "[read_only] INTEGER NOT NULL DEFAULT 0, "
    "[sort_first_letter] TEXT, "
    "[merge_mode] INTEGER NOT NULL DEFAULT 0, "
    "[is_need_merge] INTEGER NOT NULL DEFAULT 1, "
    "[merge_status] INTEGER NOT NULL DEFAULT 1, "
    "[is_merge_target] INTEGER NOT NULL DEFAULT 0, "
    "[vibration_setting] INTEGER NOT NULL DEFAULT 0, "
    "[sync_id] INTEGER, "
    "[syn_1] TEXT, "
    "[syn_2] TEXT, "
    "[syn_3] TEXT)";

constexpr const char *CREATE_CONTACT_DATA =
    "CREATE TABLE IF NOT EXISTS [contact_data]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[type_id] INTEGER REFERENCES [contact_type]([id]), "
    "[raw_contact_id] INTEGER REFERENCES [raw_contact]([id]), "
    "[read_only] INTEGER NOT NULL DEFAULT 0, "
    "[version] INTEGER NOT NULL DEFAULT 0, "
    "[is_preferred_number] INTEGER NOT NULL DEFAULT 0, "
    "[detail_info] TEXT, "
    "[family_name] TEXT, "
    "[middle_name_phonetic] TEXT, "
    "[given_name] TEXT, "
    "[given_name_phonetic] TEXT, "
    "[alias_detail_info] TEXT, "
    "[phonetic_name] TEXT, "
    "[position] TEXT, "
    "[extend1] TEXT, "
    "[extend2] TEXT, "
    "[extend3] TEXT, "
    "[extend4] TEXT, "
    "[city] TEXT, "
    "[country] TEXT, "
    "[neighborhood] TEXT, "
    "[pobox] TEXT, "
    "[postcode] TEXT, "
    "[region] TEXT, "
    "[street] TEXT, "
    "[alpha_name] TEXT, "
    "[other_lan_last_name] TEXT, "
    "[other_lan_first_name] TEXT, "
    "[extend5] TEXT, "
    "[lan_style] TEXT, "
    "[custom_data] TEXT, "
    "[extend6] TEXT, "
    "[extend7] TEXT, "
    "[blob_data] BLOB, "
    "[syn_1] TEXT, "
    "[syn_2] TEXT, "
    "[syn_3] TEXT)";
constexpr const char *CREATE_CONTACT_TYPE =
    "CREATE TABLE IF NOT EXISTS [contact_type]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[content_type] TEXT )";

constexpr const char *CREATE_DELETED_RAW_CONTACT =
    "CREATE TABLE IF NOT EXISTS [deleted_raw_contact]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[contact_id] INTEGER REFERENCES [contact]([id]),"
    "[raw_contact_id] INTEGER REFERENCES [raw_contact]([id]), "
    "[delete_source] TEXT, "
    "[delete_time] INTEGER NOT NULL DEFAULT 0, "
    "[display_name] TEXT, "
    "[delete_account] TEXT, "
    "[backup_data] TEXT, "
    "[delete_date] TEXT, "
    "[is_deleted] INTEGER NOT NULL DEFAULT 0)";

constexpr const char *CREATE_GROUPS =
    "CREATE TABLE IF NOT EXISTS [groups]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[account_id] INTEGER REFERENCES [account]([id]), "
    "[group_name] TEXT, "
    "[group_notes] TEXT, "
    "[is_deleted] INTEGER NOT NULL DEFAULT 0, "
    "[group_ringtone] TEXT, "
    "[ringtone_modify_time] INTEGER NOT NULL DEFAULT 0, "
    "[lastest_modify_time] INTEGER NOT NULL DEFAULT 0)";

constexpr const char *CREATE_ACCOUNT =
    "CREATE TABLE IF NOT EXISTS [account]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[account_name] TEXT, "
    "[account_type] TEXT, "
    "[data_info] TEXT)";

constexpr const char *CREATE_CONTACT_BLOCKLIST =
    "CREATE TABLE IF NOT EXISTS [contact_blocklist]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[types] INTEGER NOT NULL DEFAULT 0, "
    "[phone_number] TEXT, "
    "[content] TEXT, "
    "[time_stamp] INTEGER NOT NULL DEFAULT 0)";

constexpr const char *CREATE_SEARCH_CONTACT =
    "CREATE TABLE IF NOT EXISTS [search_contact]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[account_id] INTEGER   REFERENCES [account]([id]), "
    "[contact_id] INTEGER REFERENCES [contact]([id]), "
    "[raw_contact_id] INTEGER REFERENCES [raw_contact]([id]), "
    "[search_name] TEXT, "
    "[display_name] TEXT, "
    "[phonetic_name] TEXT, "
    "[photo_id] TEXT, "
    "[photo_file_id] INTEGER REFERENCES [photo_files]([id]))";

constexpr const char *CREATE_PHOTO_FILES =
    "CREATE TABLE IF NOT EXISTS [photo_files]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[file_height] INTEGER NOT NULL DEFAULT 0, "
    "[file_width] INTEGER NOT NULL DEFAULT 0, "
    "[file_size]  INTEGER NOT NULL DEFAULT 0)";

constexpr const char *CREATE_LOCAL_LANG =
    "CREATE TABLE IF NOT EXISTS[local_lang]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[local] TEXT)";

constexpr const char *CREATE_DATABASE_BACKUP_TASK =
    "CREATE TABLE IF NOT EXISTS[database_backup_task]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[backup_time] TEXT, "
    "[backup_path] TEXT, "
    "[remarks] TEXT)";

constexpr const char *CREATE_INSERT_BACKUP_TIME =
    "CREATE TRIGGER [insert_database_backup_task] AFTER INSERT ON [database_backup_task] "
    " BEGIN "
    " UPDATE "
    " [database_backup_task] "
    " SET "
    " [backup_time] = STRFTIME ('%s', 'now') "
    " WHERE "
    " [id] = [NEW].[id]; "
    " END ";

constexpr const char *CREATE_CONTACT_INDEX_DATA1 =
    "CREATE INDEX IF NOT EXISTS [contact_data_type_contact_data_index] "
    "ON [contact_data] ([type_id],[detail_info])";
constexpr const char *CREATE_CONTACT_INDEX_DATA2 =
    "CREATE INDEX IF NOT EXISTS [contact_data_raw_contact_id] "
    "ON [contact_data] ([raw_contact_id])";

constexpr const char *CREATE_SEARCH_CONTACT_INDEX1 =
    "CREATE INDEX IF NOT EXISTS [search_contact_id_index] "
    "ON [search_contact] ([contact_id])";
constexpr const char *CREATE_SEARCH_CONTACT_INDEX2 =
    "CREATE INDEX IF NOT EXISTS [search_raw_contact_id_index] "
    "ON [search_contact] ([raw_contact_id])";

constexpr const char *CREATE_RAW_CONTACT_INDEX =
    "CREATE INDEX IF NOT EXISTS [raw_contact_id_index] "
    "ON [raw_contact] ([contact_id])";

constexpr const char *CREATE_VIEW_CONTACT =
    "CREATE VIEW IF NOT EXISTS[view_contact] AS SELECT "
    "[contact].[id] AS [id], "
    "[contact].[photo_id] AS [photo_id], "
    "[contact].[photo_file_id] AS [photo_file_id], "
    "[contact].[quick_search_key] AS [quick_search_key], "
    "[contact].[has_phone_number] AS [has_phone_number], "
    "[contact].[has_display_name] AS [has_display_name], "
    "[contact].[has_email] AS [has_email], "
    "[contact].[has_group] AS [has_group], "
    "[name_raw_contact_id], "
    "[name_raw_contact].[account_id] AS [account_id], "
    "[name_raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[name_raw_contact].[personal_ringtone] AS [personal_ringtone], "
    "[name_raw_contact].[is_deleted] AS [is_deleted], "
    "[name_raw_contact].[photo_first_name] AS [photo_first_name], "
    "[name_raw_contact].[version] AS [version], "
    "[name_raw_contact].[display_name] AS [display_name], "
    "[name_raw_contact].[sort] AS [sort], "
    "[name_raw_contact].[contacted_count] AS [contacted_count], "
    "[name_raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], "
    "[name_raw_contact].[favorite] AS [favorite], "
    "[name_raw_contact].[favorite_order] AS [favorite_order], "
    "[name_raw_contact].[phonetic_name] AS [phonetic_name], "
    "[name_raw_contact].[phonetic_name_type] AS [phonetic_name_type], "
    "[name_raw_contact].[company] AS [company], "
    "[name_raw_contact].[position] AS [position], "
    "[name_raw_contact].[read_only] AS [read_only], "
    "[name_raw_contact].[sort_first_letter] AS [sort_first_letter], "
    "[name_raw_contact].[merge_mode] AS [merge_mode], "
    "[name_raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], "
    "[name_raw_contact].[is_need_merge] AS [is_need_merge], "
    "[name_raw_contact].[merge_status] AS [merge_status], "
    "[name_raw_contact].[is_merge_target] AS [is_merge_target], "
    "[name_raw_contact].[vibration_setting] AS [vibration_setting], "
    "[name_raw_contact].[sync_id] AS [sync_id], "
    "[name_raw_contact].[syn_1] AS [syn_1], "
    "[name_raw_contact].[syn_2] AS [syn_2], "
    "[name_raw_contact].[syn_3] AS [syn_3], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type], "
    "[photo_files].[file_size] AS [file_size], "
    "[photo_files].[file_height] AS [file_height], "
    "[photo_files].[file_width] AS [file_width] "
    "FROM [contact] "
    "JOIN [raw_contact] AS [name_raw_contact] "
    "ON ([name_raw_contact_id] = [name_raw_contact].[id]) "
    "LEFT JOIN [account] ON "
    "([name_raw_contact].[account_id] = [account].[id]) "
    "LEFT JOIN [photo_files] ON "
    "([photo_files].[id] = [contact].[photo_file_id])";

constexpr const char *CREATE_VIEW_RAW_CONTACT =
    "CREATE VIEW IF NOT EXISTS [view_raw_contact] AS SELECT "
    "[contact_id], "
    "[raw_contact].[id] AS [id], "
    "[raw_contact].[account_id] AS [account_id], "
    "[raw_contact].[photo_id] AS [photo_id], "
    "[raw_contact].[photo_file_id] AS [photo_file_id], "
    "[raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[raw_contact].[personal_ringtone] AS [personal_ringtone], "
    "[raw_contact].[is_deleted] AS [is_deleted], "
    "[raw_contact].[photo_first_name] AS [photo_first_name], "
    "[raw_contact].[version] AS [version], "
    "[raw_contact].[display_name] AS [display_name], "
    "[raw_contact].[sort] AS [sort], "
    "[raw_contact].[contacted_count] AS [contacted_count], "
    "[raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], "
    "[raw_contact].[favorite] AS [favorite], "
    "[raw_contact].[favorite_order] AS [favorite_order], "
    "[raw_contact].[phonetic_name] AS [phonetic_name], "
    "[raw_contact].[phonetic_name_type] AS [phonetic_name_type], "
    "[raw_contact].[company] AS [company], "
    "[raw_contact].[position] AS [position], "
    "[raw_contact].[read_only] AS [read_only], "
    "[raw_contact].[sort_first_letter] AS [sort_first_letter], "
    "[raw_contact].[merge_mode] AS [merge_mode], "
    "[raw_contact].[is_need_merge] AS [is_need_merge], "
    "[raw_contact].[merge_status] AS [merge_status], "
    "[raw_contact].[is_merge_target] AS [is_merge_target], "
    "[raw_contact].[vibration_setting] AS [vibration_setting], "
    "[raw_contact].[sync_id] AS [sync_id], "
    "[raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], "
    "[raw_contact].[syn_1] AS [syn_1], "
    "[raw_contact].[syn_2] AS [syn_2], "
    "[raw_contact].[syn_3] AS [syn_3], "
    "[photo_files].[file_size] AS [file_size], "
    "[photo_files].[file_height] AS [file_height], "
    "[photo_files].[file_width] AS [file_width], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type] "
    "FROM [raw_contact] "
    "LEFT JOIN [account] ON "
    "([raw_contact].[account_id] = [account].[id]) "
    "LEFT JOIN [photo_files] ON "
    "([photo_files].[id] = [photo_file_id])";

constexpr const char *CREATE_VIEW_CONTACT_DATA =
    "CREATE VIEW IF NOT EXISTS [view_contact_data] AS SELECT "
    "[contact_data].[id] AS [id], "
    "[contact_data].[type_id] AS [type_id], "
    "[contact_data].[raw_contact_id] AS [raw_contact_id], "
    "[contact_data].[read_only] AS [read_only], "
    "[contact_data].[version] AS [version], "
    "[contact_data].[family_name] AS [family_name], "
    "[contact_data].[middle_name_phonetic] AS [middle_name_phonetic], "
    "[contact_data].[given_name] AS [given_name], "
    "[contact_data].[given_name_phonetic] AS [given_name_phonetic], "
    "[contact_data].[is_preferred_number] AS [is_preferred_number], "
    "[contact_data].[phonetic_name] AS [phonetic_name], "
    "[contact_data].[detail_info] AS [detail_info], "
    "[contact_data].[alias_detail_info] AS [alias_detail_info], "
    "[contact_data].[position] AS [position], "
    "[contact_data].[extend1] AS [extend1], "
    "[contact_data].[extend2] AS [extend2], "
    "[contact_data].[extend3] AS [extend3], "
    "[contact_data].[extend4] AS [extend4], "
    "[contact_data].[alpha_name] AS [alpha_name], "
    "[contact_data].[other_lan_last_name] AS [other_lan_last_name], "
    "[contact_data].[other_lan_first_name] AS [other_lan_first_name], "
    "[contact_data].[extend5] AS [extend5], "
    "[contact_data].[lan_style] AS [lan_style], "
    "[contact_data].[custom_data] AS [custom_data], "
    "[contact_data].[extend6] AS [extend6], "
    "[contact_data].[extend7] AS [extend7], "
    "[contact_data].[blob_data] AS [blob_data], "
    "[contact_data].[syn_1] AS [syn_1], "
    "[contact_data].[city] AS [city], "
    "[contact_data].[syn_2] AS [syn_2], "
    "[contact_data].[syn_3] AS [syn_3], "
    "[contact_data].[country] AS [country], "
    "[contact_data].[neighborhood] AS [neighborhood], "
    "[contact_data].[pobox] AS [pobox], "
    "[contact_data].[postcode] AS [postcode], "
    "[contact_data].[region] AS [region], "
    "[contact_data].[street] AS [street], "
    "[contact_type].[content_type] AS [content_type], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type], "
    "[account].[data_info] AS [data_info], "
    "[account].[id] AS [account_id], "
    "[groups].[id] AS [group_id], "
    "[groups].[group_name] AS [group_name], "
    "[groups].[group_notes] AS [group_notes], "
    "[groups].[is_deleted] AS [group_is_deleted], "
    "[groups].[group_ringtone] AS [group_ringtone], "
    "[groups].[ringtone_modify_time] AS [ringtone_modify_time], "
    "[groups].[lastest_modify_time] AS [lastest_modify_time], "
    "[photo_files].[file_size] AS [file_size], "
    "[photo_files].[file_height] AS [file_height], "
    "[photo_files].[file_width] AS [file_width], "
    "[contact].[id] AS [contact_id], "
    "[contact].[photo_id] AS [photo_id], "
    "[contact].[photo_file_id] AS [photo_file_id], "
    "[contact].[personal_ringtone] AS [personal_ringtone], "
    "[contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[contact].[company] AS [company], "
    "[contact].[quick_search_key] AS [quick_search_key], "
    "[contact].[personal_notification_ringtone] AS "
    "[personal_notification_ringtone], "
    "[contact].[has_phone_number] AS [has_phone_number], "
    "[contact].[has_display_name] AS [has_display_name], "
    "[contact].[has_email] AS [has_email], "
    "[contact].[has_group] AS [has_group], "
    "[raw_contact].[contacted_count] AS [contacted_count], "
    "[raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], "
    "[raw_contact].[favorite] AS [favorite], "
    "[raw_contact].[favorite_order] AS [favorite_order], "
    "[raw_contact].[display_name] AS [display_name], "
    "[raw_contact].[sort] AS [sort], "
    "[raw_contact].[sort_first_letter] AS [sort_first_letter], "
    "[raw_contact].[is_deleted] AS [is_deleted], "
    "[raw_contact].[phonetic_name_type] AS [phonetic_name_type], "
    "[raw_contact].[photo_first_name] AS [photo_first_name] "
    "FROM [contact_data] "
    "JOIN [raw_contact] ON "
    "([contact_data].[raw_contact_id] = [raw_contact].[id]) "
    "JOIN [contact] ON ([contact].[id] = [raw_contact].[contact_id]) "
    "JOIN [raw_contact] AS [name_raw_contact] ON "
    "([name_raw_contact_id] = [name_raw_contact].[id]) "
    "LEFT JOIN [account] ON "
    "([raw_contact].[account_id] = [account].[id]) "
    "LEFT JOIN [contact_type] ON "
    "([contact_type].[id] = [contact_data].[type_id]) "
    "LEFT JOIN [groups] ON "
    "([contact_type].[content_type] = 'group_membership' "
    "AND [groups].[id] = [contact_data].[detail_info]) "
    "LEFT JOIN [photo_files] ON "
    "([contact_type].[content_type] = 'photo' "
    "AND [photo_files].[id] = [raw_contact].[photo_file_id])";

constexpr const char *CREATE_SEARCH_CONTACT_VIEW =
    "CREATE VIEW IF NOT EXISTS [search_contact_view] AS SELECT "
    "[search_contact].[id] AS [id], "
    "[search_contact].[account_id] AS [account_id], "
    "[search_contact].[contact_id] AS [contact_id], "
    "[search_contact].[raw_contact_id] AS [raw_contact_id], "
    "[search_contact].[search_name] AS [search_name], "
    "[search_contact].[photo_id] AS [photo_id], "
    "[search_contact].[photo_file_id] AS [photo_file_id], "
    "[contact_type].[content_type] AS [content_type], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type], "
    "[account].[data_info] AS [data_info], "
    "[groups].[group_name] AS [group_name], "
    "[groups].[group_notes] AS [group_notes], "
    "[groups].[is_deleted] AS [group_is_deleted], "
    "[groups].[group_ringtone] AS [group_ringtone], "
    "[groups].[ringtone_modify_time] AS [ringtone_modify_time], "
    "[groups].[lastest_modify_time] AS [lastest_modify_time], "
    "[photo_files].[file_size] AS [file_size], "
    "[photo_files].[file_height] AS [file_height], "
    "[photo_files].[file_width] AS [file_width], "
    "[contact].[personal_ringtone] AS [personal_ringtone], "
    "[contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[contact].[personal_notification_ringtone] AS "
    "[personal_notification_ringtone], "
    "[contact].[has_phone_number] AS [has_phone_number], "
    "[contact].[has_display_name] AS [has_display_name], "
    "[contact].[has_email] AS [has_email], "
    "[contact].[has_group] AS [has_group], "
    "[raw_contact].[contacted_count] AS [contacted_count], "
    "[raw_contact].[favorite] AS [favorite], "
    "[raw_contact].[favorite_order] AS [favorite_order], "
    "[raw_contact].[display_name] AS [display_name], "
    "[raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], "
    "[raw_contact].[sort_first_letter] AS [sort_first_letter], "
    "[raw_contact].[sort] AS [sort], "
    "[raw_contact].[photo_first_name] AS [photo_first_name], "
    "[raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], "
    "[raw_contact].[is_deleted] AS [is_deleted], "
    "[contact_data].[type_id] AS [type_id], "
    "[contact_data].[phonetic_name] AS [phonetic_name], "
    "[contact_data].[raw_contact_id] AS [raw_contact_id], "
    "[contact_data].[read_only] AS [read_only], "
    "[contact_data].[version] AS [version], "
    "[contact_data].[alias_detail_info] AS [alias_detail_info], "
    "[contact_data].[is_preferred_number] AS [is_preferred_number], "
    "[contact_data].[detail_info] AS [detail_info], "
    "[contact_data].[city] AS [city], "
    "[contact_data].[position] AS [position], "
    "[contact_data].[middle_name_phonetic] AS [middle_name_phonetic], "
    "[contact_data].[given_name] AS [given_name], "
    "[contact_data].[family_name] AS [family_name], "
    "[contact_data].[given_name_phonetic] AS [given_name_phonetic], "
    "[contact_data].[country] AS [country], "
    "[contact_data].[neighborhood] AS [neighborhood], "
    "[contact_data].[pobox] AS [pobox], "
    "[contact_data].[postcode] AS [postcode], "
    "[contact_data].[region] AS [region], "
    "[contact_data].[street] AS [street], "
    "[contact_data].[extend1] AS [extend1], "
    "[contact_data].[extend2] AS [extend2], "
    "[contact_data].[extend3] AS [extend3], "
    "[contact_data].[extend4] AS [extend4], "
    "[contact_data].[alpha_name] AS [alpha_name], "
    "[contact_data].[other_lan_last_name] AS [other_lan_last_name], "
    "[contact_data].[other_lan_first_name] AS [other_lan_first_name], "
    "[contact_data].[extend5] AS [extend5], "
    "[contact_data].[lan_style] AS [lan_style], "
    "[contact_data].[custom_data] AS [custom_data], "
    "[contact_data].[extend6] AS [extend6], "
    "[contact_data].[extend7] AS [extend7], "
    "[contact_data].[blob_data] AS [blob_data], "
    "[contact_data].[syn_1] AS [syn_1], "
    "[contact_data].[syn_2] AS [syn_2], "
    "[contact_data].[syn_3] AS [syn_3] "
    "FROM [search_contact] "
    "JOIN [raw_contact] ON "
    "([search_contact].[raw_contact_id] = [raw_contact].[id]) "
    "JOIN [contact] ON "
    "([contact].[id] = [raw_contact].[contact_id]) "
    "JOIN [raw_contact] AS [name_raw_contact] ON "
    "([name_raw_contact_id] = [name_raw_contact].[id]) "
    "LEFT JOIN [account] ON "
    "([search_contact].[account_id] = [account].[id]) "
    "LEFT JOIN [contact_data] ON "
    "([contact_data].[raw_contact_id] = [raw_contact].[id]) "
    "LEFT JOIN [contact_type] ON "
    "([contact_data].[type_id] = [contact_type].[id]) "
    "LEFT JOIN [groups] ON "
    "([contact_type].[content_type] = 'group_membership' "
    "AND [groups].[id] = [contact_data].[detail_info]) "
    "LEFT JOIN [photo_files] ON "
    "([contact_type].[content_type] = 'photo' "
    "AND [photo_files].[id] = [search_contact].[photo_file_id])";

constexpr const char *CREATE_VIEW_GROUPS =
    "CREATE VIEW IF NOT EXISTS [view_groups] "
    "AS SELECT "
    "[groups].[id] AS [id], "
    "[groups].[account_id] AS [account_id], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type], "
    "[groups].[group_name] AS [group_name] , "
    "[groups].[group_notes] AS [group_notes] , "
    "[groups].[is_deleted]  AS [is_deleted] , "
    "[groups].[group_ringtone]  AS [group_ringtone], "
    "[groups].[ringtone_modify_time]  AS [ringtone_modify_time] , "
    "[groups].[lastest_modify_time]  AS [lastest_modify_time] "
    "FROM [groups] "
    "LEFT JOIN [account] ON ([groups].[account_id] = [account].[id])";

constexpr const char *CREATE_VIEW_DELETED =
    "CREATE VIEW IF NOT EXISTS [view_deleted] "
    "AS  SELECT "
    "[deleted_raw_contact].[id] AS [id], "
    "[contact].[id] AS [contact_id], "
    "[contact].[photo_id] AS [photo_id], "
    "[contact].[photo_file_id] AS [photo_file_id], "
    "[contact].[quick_search_key] AS [quick_search_key], "
    "[contact].[has_phone_number] AS [has_phone_number], "
    "[contact].[has_display_name] AS [has_display_name], "
    "[contact].[has_email] AS [has_email], "
    "[contact].[has_group] AS [has_group], "
    "[name_raw_contact].[account_id] AS [account_id], "
    "[name_raw_contact].[id] AS [raw_contact_id], "
    "[name_raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[name_raw_contact].[personal_ringtone] AS [personal_ringtone], "
    "[name_raw_contact].[is_deleted] AS [is_deleted], "
    "[name_raw_contact].[version] AS [version], "
    "[name_raw_contact].[display_name] AS [display_name], "
    "[name_raw_contact].[sort] AS [sort], "
    "[name_raw_contact].[contacted_count] AS [contacted_count], "
    "[name_raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], "
    "[name_raw_contact].[favorite] AS [favorite], "
    "[name_raw_contact].[favorite_order] AS [favorite_order], "
    "[name_raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], "
    "[name_raw_contact].[phonetic_name] AS [phonetic_name], "
    "[name_raw_contact].[phonetic_name_type] AS [phonetic_name_type], "
    "[name_raw_contact].[company] AS [company], "
    "[name_raw_contact].[position] AS [position], "
    "[name_raw_contact].[read_only] AS [read_only], "
    "[name_raw_contact].[sort_first_letter] AS [sort_first_letter], "
    "[name_raw_contact].[merge_mode] AS [merge_mode], "
    "[name_raw_contact].[is_need_merge] AS [is_need_merge], "
    "[name_raw_contact].[merge_status] AS [merge_status], "
    "[name_raw_contact].[is_merge_target] AS [is_merge_target], "
    "[name_raw_contact].[vibration_setting] AS [vibration_setting], "
    "[name_raw_contact].[photo_first_name] AS [photo_first_name], "
    "[name_raw_contact].[sync_id] AS [sync_id], "
    "[name_raw_contact].[syn_1] AS [syn_1], "
    "[name_raw_contact].[syn_2] AS [syn_2], "
    "[name_raw_contact].[syn_3] AS [syn_3], "
    "[deleted_raw_contact].[delete_source] AS [delete_source],"
    "[deleted_raw_contact].[delete_time] AS [delete_time], "
    "[deleted_raw_contact].[delete_account] AS [delete_account], "
    "[deleted_raw_contact].[backup_data] AS [backup_data], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type], "
    "[photo_files].[file_size] AS [file_size], "
    "[photo_files].[file_height] AS [file_height], "
    "[photo_files].[file_width] AS [file_width] "
    "FROM   [deleted_raw_contact] "
    "LEFT JOIN [contact] ON ([contact].[id] = [deleted_raw_contact].[contact_id]) "
    "LEFT JOIN [raw_contact] AS [name_raw_contact] ON ([name_raw_contact].[id] = "
    "[deleted_raw_contact].[raw_contact_id] "
    "AND [name_raw_contact].[is_deleted] = 1)"
    "LEFT JOIN [account] ON ([name_raw_contact].[account_id] = [account].[id]) "
    "LEFT JOIN [photo_files] ON ([photo_files].[id] = [name_raw_contact].[photo_file_id]) ";

constexpr const char *UPDATE_CONTACT_BY_INSERT_CONTACT_DATA =
    "CREATE TRIGGER IF NOT EXISTS [update_contact_by_insert_contact_data] AFTER INSERT ON [contact_data] FOR EACH ROW "
    "BEGIN "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_display_name] = 1 "
    "WHERE "
    "[NEW].[raw_contact_id] = [name_raw_contact_id] "
    "AND [NEW].[type_id] = 6 "
    "AND [NEW].[detail_info] IS NOT NULL; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_email] = 1 "
    "WHERE "
    "[NEW].[raw_contact_id] = [name_raw_contact_id] "
    "AND [NEW].[type_id] = 1 "
    "AND [NEW].[detail_info] IS NOT NULL; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_group] = 1 "
    "WHERE "
    "[NEW].[raw_contact_id] = [name_raw_contact_id] "
    "AND [NEW].[type_id] = 9 "
    "AND [NEW].[detail_info] IS NOT NULL; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_phone_number] = 1 "
    "WHERE "
    "[NEW].[raw_contact_id] = [name_raw_contact_id] "
    "AND [NEW].[type_id] = 5 "
    "AND [NEW].[detail_info] IS NOT NULL; "
    "END";

constexpr const char *UPDATE_CONTACT_BY_DELETE_CONTACT_DATA =
    "CREATE TRIGGER IF NOT EXISTS [update_contact_by_delete_contact_data] AFTER DELETE ON [contact_data] FOR EACH ROW "
    "BEGIN "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_display_name] = 0 "
    "WHERE "
    "[OLD].[raw_contact_id] = [name_raw_contact_id] "
    "AND [OLD].[type_id] = 6; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_email] = 0 "
    "WHERE "
    "[OLD].[raw_contact_id] = [name_raw_contact_id] "
    "AND [OLD].[type_id] = 1; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_group] = 0 "
    "WHERE "
    "[OLD].[raw_contact_id] = [name_raw_contact_id] "
    "AND [OLD].[type_id] = 9; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_phone_number] = 0 "
    "WHERE "
    "[OLD].[raw_contact_id] = [name_raw_contact_id] "
    "AND [OLD].[type_id] = 5; "
    "END";

constexpr const char *UPDATE_CONTACT_BY_UPDATE_CONTACT_DATA =
    "CREATE TRIGGER IF NOT EXISTS [update_contact_by_update_contact_data] AFTER UPDATE ON [contact_data] FOR EACH ROW "
    "BEGIN "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_display_name] = 1 "
    "WHERE "
    "[NEW].[raw_contact_id] = [name_raw_contact_id] "
    "AND [NEW].[type_id] = 6 "
    "AND [NEW].[detail_info] IS NOT NULL; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_email] = 1 "
    "WHERE "
    "[NEW].[raw_contact_id] = [name_raw_contact_id] "
    "AND [NEW].[type_id] = 1 "
    "AND [NEW].[detail_info] IS NOT NULL; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_group] = 1 "
    "WHERE "
    "[NEW].[raw_contact_id] = [name_raw_contact_id] "
    "AND [NEW].[type_id] = 9 "
    "AND [NEW].[detail_info] IS NOT NULL; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_phone_number] = 1 "
    "WHERE "
    "[NEW].[raw_contact_id] = [name_raw_contact_id] "
    "AND [NEW].[type_id] = 5 "
    "AND [NEW].[detail_info] IS NOT NULL; "
    "END";

constexpr const char *INSERT_DELETE_RAW_CONTACT =
    "CREATE TRIGGER IF NOT EXISTS insert_delete_raw_contact AFTER  INSERT ON deleted_raw_contact "
    "BEGIN "
    "UPDATE deleted_raw_contact SET delete_time = STRFTIME('%s','now')   WHERE id = NEW.id; "
    "END";

constexpr const char *UPDATE_RAW_CONTACT_VERSION =
    "CREATE TRIGGER IF NOT EXISTS [update_raw_contact_version] AFTER UPDATE ON [raw_contact] "
    "BEGIN "
    "UPDATE "
    "[raw_contact] "
    "SET "
    "[version] = [OLD].[version] + 1 "
    "WHERE "
    "[id] = [OLD].[id] AND [NEW].[is_deleted] != [OLD].[is_deleted]; "
    "END";

constexpr const char *UPDATE_CONTACT_DATA_VERSION =
    "CREATE TRIGGER IF NOT EXISTS [update_contact_data_version] AFTER UPDATE ON [contact_data] "
    "BEGIN "
    "UPDATE "
    "[contact_data] "
    "SET "
    "[version] = [OLD].[version] + 1 "
    "WHERE "
    "[id] = [OLD].[id]; "
    "END";

constexpr const char *INSERT_CONTACT_QUICK_SEARCH =
    "CREATE TRIGGER IF NOT EXISTS [insert_contact_quick_search]AFTER INSERT ON [contact] BEGIN "
    "UPDATE [contact] SET [quick_search_key] = [NEW].[id] WHERE [id] = [NEW].[id]; END";

constexpr const char *CALL_LOG_PHONE_NUMBER_INDEX =
    "CREATE INDEX IF NOT EXISTS [calllog_phone_number_index] ON [calllog] ([phone_number])";

constexpr const char *MERGE_INFO =
    "CREATE TABLE IF NOT EXISTS [merge_info]("
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[raw_contact_id] INTEGER NOT NULL DEFAULT 0 )";

constexpr const char *MERGE_INFO_INDEX =
    "CREATE INDEX IF NOT EXISTS [merge_info_index] ON [merge_info] ([raw_contact_id])";
} // namespace Contacts
} // namespace OHOS
#endif // COMMON_H
