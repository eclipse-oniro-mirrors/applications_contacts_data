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

#ifndef CONTACTS_DATA_CONTACTS_COMMON_H
#define CONTACTS_DATA_CONTACTS_COMMON_H

namespace OHOS {
namespace Contacts {
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
    "[has_group] INTEGER NOT NULL DEFAULT 0, "
    "[focus_mode_list] TEXT, "
    "[form_id] TEXT, "
    "[ringtone_path] TEXT, "
    "[prefer_avatar] INTEGER, "
    "[contact_last_updated_timestamp] INTEGER)";

constexpr const char *CREATE_CONTACT_INDEX =
    "CREATE INDEX IF NOT EXISTS [contact_name_raw_contact_id_index] "
    "ON [contact] ([name_raw_contact_id])";

constexpr const char *CREATE_RAW_INDEX =
    "CREATE INDEX IF NOT EXISTS [raw_contact_del_index] "
    "ON [raw_contact] ([is_deleted], [primary_contact])";
 
constexpr const char *CREATE_DATA_INDEX =
    "CREATE INDEX IF NOT EXISTS [raw_data_type_index] "
    "ON [contact_data] ([type_id], [raw_contact_id])";
 
constexpr const char *CREATE_LOCATION_INDEX =
    "CREATE INDEX IF NOT EXISTS [data_location_index] "
    "ON [contact_data] ([location])";

constexpr const char *CREATE_CONTACT_BLOCKLIST_INDEX_PHONE =
    "CREATE UNIQUE INDEX IF NOT EXISTS [contact_blocklist_phone_index]  "
    "ON [contact_blocklist] ([phone_number], [types])";

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
    "[sort_key] TEXT, "
    "[contacted_count] INTEGER NOT NULL DEFAULT 0, "
    "[lastest_contacted_time] INTEGER NOT NULL DEFAULT 0, "
    "[favorite] INTEGER NOT NULL DEFAULT 0, "
    "[favorite_order] INTEGER, "
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
    "[syn_3] TEXT, "
    "[primary_contact] INTEGER DEFAULT 0, "
    "[extra1] TEXT, "
    "[extra2] TEXT, "
    "[extra3] TEXT, "
    "[extra4] TEXT, "
    "[dirty] INTEGER NOT NULL DEFAULT 0, "
    "[uuid] TEXT, "
    "[unique_key] TEXT, "
    "[focus_mode_list] TEXT, "
    "[aggregation_status] INTEGER NOT NULL DEFAULT 0)";

constexpr const char *CREATE_RAW_CONTACT_INDEX_UNIQUE_KEY =
    "CREATE INDEX IF NOT EXISTS [raw_contact_unique_key_index]  "
    "ON [raw_contact] ([unique_key])";

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
    "[format_phone_number] TEXT, "
    "[position] TEXT, "
    "[location] TEXT, "
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
    "[syn_3] TEXT, "
    "[extend8] TEXT, "
    "[extend9] TEXT, "
    "[extend10] TEXT, "
    "[extend11] TEXT, "
    "[calendar_event_id] TEXT, "
    "[is_sync_birthday_to_calendar] INTEGER NOT NULL DEFAULT 0, "
    "[blob_source] INTEGER NOT NULL DEFAULT 0)";

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
    "[mark] INTEGER NOT NULL DEFAULT 0, "
    "[name] TEXT, "
    "[phone_number] TEXT, "
    "[interception_call_count] INTEGER NOT NULL DEFAULT 0, "
    "[interception_msg_count] INTEGER NOT NULL DEFAULT 0, "
    "[content] TEXT, "
    "[time_stamp] INTEGER NOT NULL DEFAULT 0, "
    "[format_phone_number] TEXT);";

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

constexpr const char *CREATE_RAW_CONTACT_INDEX_SORT =
    "CREATE INDEX IF NOT EXISTS [raw_contact_sort_index]  "
    "ON [raw_contact] ([sort], [sort_key])";

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
    "[contact].[focus_mode_list] AS [focus_mode_list], "
    "[contact].[form_id] AS [form_id], "
    "[name_raw_contact_id], "
    "[name_raw_contact].[account_id] AS [account_id], "
    "[name_raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[name_raw_contact].[personal_ringtone] AS [personal_ringtone], "
    "[name_raw_contact].[is_deleted] AS [is_deleted], "
    "[name_raw_contact].[photo_first_name] AS [photo_first_name], "
    "[name_raw_contact].[version] AS [version], "
    "[name_raw_contact].[display_name] AS [display_name], "
    "[name_raw_contact].[sort] AS [sort], "
    "[name_raw_contact].[sort_key] AS [sort_key], "
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
    "[name_raw_contact].[primary_contact] AS [primary_contact], "
    "[name_raw_contact].[extra1] AS [extra1], "
    "[name_raw_contact].[extra2] AS [extra2], "
    "[name_raw_contact].[extra3] AS [extra3], "
    "[name_raw_contact].[extra4] AS [extra4], "
    "[name_raw_contact].[aggregation_status] AS [aggregation_status], "
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

constexpr const char *CREATE_VIEW_CONTACT_LOCATION =
    "CREATE VIEW IF NOT EXISTS [view_contact_location] AS "
	"WITH RankedData AS ( SELECT id, type_id, raw_contact_id, detail_info, format_phone_number, location "
    "FROM (SELECT id, type_id, raw_contact_id, detail_info,format_phone_number, location,"
    "ROW_NUMBER() OVER (PARTITION BY raw_contact_id, CASE WHEN INSTR(location, ' ') > 0 "
    "THEN SUBSTR(location, 1, INSTR(location, ' ') - 1) ELSE location END "
    "ORDER BY id) AS rn FROM contact_data WHERE type_id = 5 "
    "AND location IS NOT NULL AND location <> '') AS tmp WHERE rn = 1),"
    "RankedFilteredData AS (SELECT * FROM RankedData WHERE rn = 1) "
	"SELECT "
    "[contact].[id] AS [id], "
    "[contact].[photo_id] AS [photo_id], "
    "[contact].[photo_file_id] AS [photo_file_id], "
    "[contact].[quick_search_key] AS [quick_search_key], "
    "[contact].[has_phone_number] AS [has_phone_number], "
    "[contact].[has_display_name] AS [has_display_name], "
    "[contact].[has_email] AS [has_email], "
    "[contact].[has_group] AS [has_group], "
    "[contact].[focus_mode_list] AS [focus_mode_list], "
    "[contact].[form_id] AS [form_id], "
    "[name_raw_contact_id], "
    "[name_raw_contact].[account_id] AS [account_id], "
    "[name_raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[name_raw_contact].[personal_ringtone] AS [personal_ringtone], "
    "[name_raw_contact].[is_deleted] AS [is_deleted], "
    "[name_raw_contact].[photo_first_name] AS [photo_first_name], "
    "[name_raw_contact].[version] AS [version], "
    "[name_raw_contact].[display_name] AS [display_name], "
    "[name_raw_contact].[sort] AS [sort], "
    "[name_raw_contact].[sort_key] AS [sort_key], "
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
    "[name_raw_contact].[primary_contact] AS [primary_contact], "
    "[name_raw_contact].[extra1] AS [extra1], "
    "[name_raw_contact].[extra2] AS [extra2]," 
    "[name_raw_contact].[extra3] AS [extra3], "
    "[name_raw_contact].[extra4] AS [extra4], "
    "[name_raw_contact].[aggregation_status] AS [aggregation_status], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type], "
    "[photo_files].[file_size] AS [file_size], "
    "[photo_files].[file_height] AS [file_height], "
    "[photo_files].[file_width] AS [file_width], "
    "[RankedData].[location] AS [location] "
	"FROM [contact] JOIN [raw_contact] AS [name_raw_contact] "
	"ON ([name_raw_contact_id] = [name_raw_contact].[id]) "
	"LEFT JOIN [account] ON ([name_raw_contact].[account_id] = [account].[id]) "
	"LEFT JOIN [photo_files] ON ([photo_files].[id] = [contact].[photo_file_id]) "
	"LEFT JOIN [RankedData] ON ([name_raw_contact_id] = RankedData.[raw_contact_id]) "
	"AND ([RankedData].[location] IS NOT NULL AND [RankedData].[location] <> '')";

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
    "[raw_contact].[sort_key] AS [sort_key], "
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
    "[raw_contact].[primary_contact] AS [primary_contact], "
    "[raw_contact].[extra1] AS [extra1], "
    "[raw_contact].[extra2] AS [extra2], "
    "[raw_contact].[extra3] AS [extra3], "
    "[raw_contact].[extra4] AS [extra4], "
    "[raw_contact].[dirty] AS [dirty], "
    "[raw_contact].[uuid] AS [uuid], "
    "[raw_contact].[unique_key] AS [unique_key], "
    "[raw_contact].[aggregation_status] AS [aggregation_status], "
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
    "[contact_data].[format_phone_number] AS [format_phone_number], "
    "[contact_data].[detail_info] AS [detail_info], "
    "[contact_data].[alias_detail_info] AS [alias_detail_info], "
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
    "[contact_data].[extend8] AS [extend8], "
    "[contact_data].[extend9] AS [extend9], "
    "[contact_data].[extend10] AS [extend10], "
    "[contact_data].[extend11] AS [extend11], "
    "[contact_data].[blob_source] AS [blob_source], "
    "[contact_data].[calendar_event_id] AS [calendar_event_id], "
    "[contact_data].[is_sync_birthday_to_calendar] AS [is_sync_birthday_to_calendar], "
    "[contact_data].[location] AS [location], "
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
    "[contact].[read_only] AS [contact_read_only], "
    "[contact].[personal_notification_ringtone] AS "
    "[personal_notification_ringtone], "
    "[contact].[has_phone_number] AS [has_phone_number], "
    "[contact].[has_display_name] AS [has_display_name], "
    "[contact].[has_email] AS [has_email], "
    "[contact].[has_group] AS [has_group], "
    "[contact].[contact_last_updated_timestamp] AS [contact_last_updated_timestamp], "
    "[contact].[focus_mode_list] AS [focus_mode_list], "
    "[contact].[form_id] AS [form_id], "
    "[contact].[name_raw_contact_id] AS [name_raw_contact_id], "
    "[contact].[ringtone_path] AS [ringtone_path], "
    "[contact].[prefer_avatar] AS [prefer_avatar], "
    "[raw_contact].[is_transfer_voicemail] AS [raw_is_transfer_voicemail], "
    "[raw_contact].[personal_ringtone] AS [raw_personal_ringtone], "
    "[raw_contact].[personal_notification_ringtone] AS [raw_personal_notification_ringtone], "
	"[raw_contact].[version] AS [raw_version], "
    "[raw_contact].[contacted_count] AS [contacted_count], "
    "[raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], "
    "[raw_contact].[favorite] AS [favorite], "
    "[raw_contact].[favorite_order] AS [favorite_order], "
    "[raw_contact].[company] AS [raw_company], "
	"[raw_contact].[read_only] AS [raw_read_only], "
    "[raw_contact].[position] AS [position], "
    "[raw_contact].[display_name] AS [display_name], "
    "[raw_contact].[sort] AS [sort], "
    "[raw_contact].[sort_key] AS [sort_key], "
	"[raw_contact].[merge_mode] AS [merge_mode], "
	"[raw_contact].[is_need_merge] AS [is_need_merge], "
	"[raw_contact].[merge_status] AS [merge_status], "
	"[raw_contact].[is_merge_target] AS [is_merge_target], "
	"[raw_contact].[vibration_setting] AS [vibration_setting], "
	"[raw_contact].[sync_id] AS [sync_id], "
	"[raw_contact].[syn_1] AS [raw_syn_1], "
	"[raw_contact].[syn_2] AS [raw_syn_2], "
	"[raw_contact].[syn_3] AS [raw_syn_3], "
    "[raw_contact].[sort_first_letter] AS [sort_first_letter], "
    "[raw_contact].[is_deleted] AS [is_deleted], "
    "[raw_contact].[phonetic_name_type] AS [phonetic_name_type], "
	"[raw_contact].[phonetic_name] AS [raw_phonetic_name], "
    "[raw_contact].[photo_first_name] AS [photo_first_name], "
    "[raw_contact].[primary_contact] AS [primary_contact], "
    "[raw_contact].[extra1] AS [extra1], "
    "[raw_contact].[extra2] AS [extra2], "
    "[raw_contact].[extra3] AS [extra3], "
    "[raw_contact].[extra4] AS [extra4], "
	"[raw_contact].[dirty] AS [dirty], "
    "[raw_contact].[unique_key] AS [unique_key], "
    "[raw_contact].[aggregation_status] AS [aggregation_status] "
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

constexpr const char *UPDATE_CONTACT_BY_INSERT_CONTACT_DATA =
    "CREATE TRIGGER IF NOT EXISTS [update_contact_by_insert_contact_data] AFTER INSERT ON [contact_data] FOR EACH ROW "
    "BEGIN "
    "UPDATE contact "
    "SET "
    "has_display_name = CASE WHEN NEW.type_id = 6 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_display_name END,"
    "has_email = CASE WHEN NEW.type_id = 1 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_email END,"
    "has_group = CASE WHEN NEW.type_id = 9 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_group END,"
    "has_phone_number = CASE WHEN NEW.type_id = 5 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_phone_number END "
    "WHERE NEW.raw_contact_id = name_raw_contact_id;"
    "END";

constexpr const char *CREATE_CLOUD_CONTACT_BLOCKLIST =
    "CREATE TABLE IF NOT EXISTS [cloud_contact_blocklist]( "
    "[phone_number] TEXT PRIMARY KEY, "
    "[types] INTEGER DEFAULT 0, "
    "[mark] INTEGER DEFAULT 0, "
    "[name] TEXT, "
    "[content] TEXT, "
    "[time_stamp] INTEGER DEFAULT 0)";

constexpr const char *CREATE_PRIVACY_CONTACTS_BACKUP =
    "CREATE TABLE IF NOT EXISTS [privacy_contacts_backup]( "
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[contact_data_id] INTEGER, "
    "[raw_contact_id] INTEGER, "
    "[phone_number] TEXT, "
    "[format_phone_number] TEXT, "
    "[calls_synced] INTEGER DEFAULT 0)";

// contact poster table creation statement
constexpr const char *CREATE_POSTER =
    "CREATE TABLE IF NOT EXISTS [poster] ( "
    "[phone_number] TEXT PRIMARY KEY, "
    "[poster_version] TEXT, "
    "[poster_uri] TEXT, "
    "[poster_file_path] TEXT, "
    "[poster_crop_info] TEXT, "
    "[avatar_uri] TEXT, "
    "[avatar_file_path] TEXT, "
    "[video_poster_uri] TEXT, "
    "[video_poster_file_path] TEXT, "
    "[video_poster_crop_info] TEXT, "
    "[insert_time] INTEGER, "
    "[update_time] INTEGER, "
    "[extra_info] TEXT)";

const std::map<std::string, const char *> CONTACT_TABLES = {
    {ContactTableName::ACCOUNT, CREATE_ACCOUNT},
    {ContactTableName::CONTACT, CREATE_CONTACT},
    {ContactTableName::CONTACT_DATA, CREATE_CONTACT_DATA},
    {ContactTableName::RAW_CONTACT, CREATE_RAW_CONTACT},
    {ContactTableName::DELETE_RAW_CONTACT, CREATE_DELETED_RAW_CONTACT},
    {ContactTableName::CONTACT_TYPE, CREATE_CONTACT_TYPE},
    {ContactTableName::GROUPS, CREATE_GROUPS},
    {ContactTableName::PHOTO_FILES, CREATE_PHOTO_FILES},
    {ContactTableName::LOCAL_LANG, CREATE_LOCAL_LANG},
    {ContactTableName::CONTACT_BLOCKLIST, CREATE_CONTACT_BLOCKLIST},
    {ContactTableName::SEARCH_CONTACT, CREATE_SEARCH_CONTACT},
    {ContactTableName::MERGE_INFO, MERGE_INFO},
    {ContactTableName::CLOUD_RAW_CONTACT, CREATE_CLOUD_RAW_CONTACT},
    {ContactTableName::CLOUD_GROUP, CREATE_CLOUD_GROUPS},
    {ContactTableName::SETTINGS, CREATE_SETTINGS},
    {ContactTableName::HW_ACCOUNT, CREATE_HW_ACCOUNT},
    {ContactTableName::CLOUD_CONTACT_BLOCKLIST, CREATE_CLOUD_CONTACT_BLOCKLIST},
    {ContactTableName::PRIVACY_CONTACTS_BACKUP, CREATE_PRIVACY_CONTACTS_BACKUP},
    {ContactTableName::POSTER, CREATE_POSTER},
};

const std::map<std::string, const char *> CONTACT_VIEWS = {
    {ViewName::VIEW_CONTACT, CREATE_VIEW_CONTACT},
    {ViewName::VIEW_RAW_CONTACT, CREATE_VIEW_RAW_CONTACT},
    {ViewName::VIEW_CONTACT_DATA, CREATE_VIEW_CONTACT_DATA},
    {ViewName::VIEW_CONTACT_LOCATION, CREATE_VIEW_CONTACT_LOCATION},
    {ViewName::VIEW_GROUPS, CREATE_VIEW_GROUPS},
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACTS_DATA_CONTACTS_COMMON_H
