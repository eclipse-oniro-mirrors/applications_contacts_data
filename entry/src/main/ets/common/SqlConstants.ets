/**
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

/**
 * Sql constants for operation database
 */
// 双升单以后数据库的版本号，要与 c++ 测开库版本号保持一致
export const LATEST_CHANNEL_STORE_VERSION: number = 42;
export const LATEST_BLOCKLIST_DB_VERSION : number = 1;

export default class SqlConstants {
  /**
   * Create table contact
   */
  public static readonly CREATE_CONTACT =
    `CREATE TABLE IF NOT EXISTS [contact]([id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[name_raw_contact_id] INTEGER REFERENCES ` +
      `raw_contact(id), [photo_id] INTEGER, ` +
      `[photo_file_id] INTEGER REFERENCES ` +
      `[photo_files]([id]),  [personal_ringtone] TEXT, ` +
      `[is_transfer_voicemail] INTEGER NOT NULL DEFAULT 0, ` +
      `[company] TEXT, ` +
      `[position] TEXT, ` +
      `[quick_search_key] TEXT, ` +
      `[read_only] INTEGER NOT NULL DEFAULT 0, ` +
      `[personal_notification_ringtone] TEXT, ` +
      `[has_phone_number] INTEGER NOT NULL DEFAULT 0, ` +
      `[has_display_name] INTEGER NOT NULL DEFAULT 0, ` +
      `[has_email] INTEGER NOT NULL DEFAULT 0, ` +
      `[has_group] INTEGER NOT NULL DEFAULT 0, ` +
      `[focus_mode_list] TEXT, ` +
      `[form_id] TEXT, ` +
      `[ringtone_path] TEXT, ` +
      `[prefer_avatar] INTEGER, ` +
      `[contact_last_updated_timestamp] INTEGER)`;

  /**
   * Create contact index for search
   */
  public static readonly CREATE_CONTACT_INDEX =
    `CREATE INDEX IF NOT EXISTS [contact_name_raw_contact_id_index] ` +
      `ON [contact] ([name_raw_contact_id])`;

  /**
   * Table data with raw_contacts
   */
  public static readonly TABLE_DATA_WITH_RAW_CONTACTS = `[data] LEFT JOIN [raw_contacts] ON ` +
    `[data].[raw_contact_id] = [raw_contacts].[_id]`;

  /**
   * Create table raw_contact
   */
  public static readonly CREATE_RAW_CONTACT =
    `CREATE TABLE IF NOT EXISTS [raw_contact]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[contact_id] INTEGER REFERENCES [contact]([id]), ` +
      `[photo_id] INTEGER, ` +
      `[photo_file_id] INTEGER REFERENCES [photo_files]([id]), ` +
      `[is_transfer_voicemail] INTEGER NOT NULL DEFAULT 0, ` +
      `[personal_ringtone] TEXT, ` +
      `[is_deleted] INTEGER NOT NULL DEFAULT 0, ` +
      `[personal_notification_ringtone] TEXT, ` +
      `[photo_first_name] TEXT, ` +
      `[account_id] INTEGER, ` +
      `[version] INTEGER NOT NULL DEFAULT 0, ` +
      `[display_name] TEXT, ` +
      `[sort] TEXT, ` +
      `[sort_key] TEXT, ` +
      `[contacted_count] INTEGER NOT NULL DEFAULT 0, ` +
      `[lastest_contacted_time] INTEGER NOT NULL DEFAULT 0, ` +
      `[favorite] INTEGER NOT NULL DEFAULT 0, ` +
      `[favorite_order] INTEGER, ` +
      `[phonetic_name] TEXT, ` +
      `[phonetic_name_type] INTEGER , ` +
      `[company] TEXT, ` +
      `[position] TEXT, ` +
      `[read_only] INTEGER NOT NULL DEFAULT 0, ` +
      `[sort_first_letter] TEXT, ` +
      `[merge_mode] INTEGER NOT NULL DEFAULT 0, ` +
      `[is_need_merge] INTEGER NOT NULL DEFAULT 1, ` +
      `[merge_status] INTEGER NOT NULL DEFAULT 1, ` +
      `[is_merge_target] INTEGER NOT NULL DEFAULT 0, ` +
      `[vibration_setting] INTEGER NOT NULL DEFAULT 0, ` +
      `[sync_id] INTEGER, ` +
      `[syn_1] TEXT, ` +
      `[syn_2] TEXT, ` +
      `[syn_3] TEXT, ` +
      `[primary_contact] INTEGER DEFAULT 0, ` +
      `[extra1] TEXT, ` +
      `[extra2] TEXT, ` +
      `[extra3] TEXT, ` +
      `[extra4] TEXT, ` +
      `[dirty] INTEGER NOT NULL DEFAULT 0, ` +
      `[uuid] TEXT, ` +
      `[unique_key] TEXT, ` +
      `[focus_mode_list] TEXT, ` +
      `[aggregation_status] INTEGER NOT NULL DEFAULT 0)`;

  /**
   * Create table contact_data
   */
  public static readonly CREATE_CONTACT_DATA =
    `CREATE TABLE IF NOT EXISTS [contact_data]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[type_id] INTEGER REFERENCES [contact_type]([id]), ` +
      `[raw_contact_id] INTEGER REFERENCES [raw_contact]([id]), ` +
      `[read_only] INTEGER NOT NULL DEFAULT 0, ` +
      `[version] INTEGER NOT NULL DEFAULT 0, ` +
      `[is_preferred_number] INTEGER NOT NULL DEFAULT 0, ` +
      `[detail_info] TEXT, ` +
      `[family_name] TEXT, ` +
      `[middle_name_phonetic] TEXT, ` +
      `[given_name] TEXT, ` +
      `[given_name_phonetic] TEXT, ` +
      `[alias_detail_info] TEXT, ` +
      `[phonetic_name] TEXT, ` +
      `[format_phone_number] TEXT, ` +
      `[position] TEXT, ` +
      `[location] TEXT, ` +
      `[extend1] TEXT, ` +
      `[extend2] TEXT, ` +
      `[extend3] TEXT, ` +
      `[extend4] TEXT, ` +
      `[city] TEXT, ` +
      `[country] TEXT, ` +
      `[neighborhood] TEXT, ` +
      `[pobox] TEXT, ` +
      `[postcode] TEXT, ` +
      `[region] TEXT, ` +
      `[street] TEXT, ` +
      `[alpha_name] TEXT, ` +
      `[other_lan_last_name] TEXT, ` +
      `[other_lan_first_name] TEXT, ` +
      `[extend5] TEXT, ` +
      `[lan_style] TEXT, ` +
      `[custom_data] TEXT, ` +
      `[extend6] TEXT, ` +
      `[extend7] TEXT, ` +
      `[blob_data] BLOB, ` +
      `[syn_1] TEXT, ` +
      `[syn_2] TEXT, ` +
      `[syn_3] TEXT, ` +
      `[extend8] TEXT, ` +
      `[extend9] TEXT, ` +
      `[extend10] TEXT, ` +
      `[extend11] TEXT, ` +
      `[blob_source] INTEGER NOT NULL DEFAULT 0,` +
      `[is_sync_birthday_to_calendar] INTEGER NOT NULL DEFAULT 0,` +
      `[calendar_event_id] TEXT)`;

  /**
   * Create table contact_type
   */
  public static readonly CREATE_CONTACT_TYPE =
    `CREATE TABLE IF NOT EXISTS [contact_type]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[content_type] TEXT )`;

  /**
   * Create table deleted_raw_contact
   */
  public static readonly CREATE_DELETED_RAW_CONTACT =
    `CREATE TABLE IF NOT EXISTS [deleted_raw_contact]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[contact_id] INTEGER REFERENCES [contact]([id]),` +
      `[raw_contact_id] INTEGER REFERENCES [raw_contact]([id]), ` +
      `[delete_source] TEXT, ` +
      `[delete_time] INTEGER NOT NULL DEFAULT 0, ` +
      `[display_name] TEXT, ` +
      `[delete_account] TEXT, ` +
      `[backup_data] TEXT, ` +
      `[delete_date] TEXT, ` +
      `[is_deleted] INTEGER NOT NULL DEFAULT 0)`;

  /**
   * Create table groups
   */
  public static readonly CREATE_GROUPS =
    `CREATE TABLE IF NOT EXISTS [groups]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[account_id] INTEGER REFERENCES [account]([id]), ` +
      `[group_name] TEXT, ` +
      `[group_notes] TEXT, ` +
      `[is_deleted] INTEGER NOT NULL DEFAULT 0, ` +
      `[group_ringtone] TEXT, ` +
      `[ringtone_modify_time] INTEGER NOT NULL DEFAULT 0, ` +
      `[lastest_modify_time] INTEGER NOT NULL DEFAULT 0)`;

  /**
   * Create table account
   */
  public static readonly CREATE_ACCOUNT =
    `CREATE TABLE IF NOT EXISTS [account]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[account_name] TEXT, ` +
      `[account_type] TEXT, ` +
      `[data_info] TEXT)`;

  /**
   * Create table contact_blocklist
   */
  public static readonly CREATE_CONTACT_BLOCKLIST =
    `CREATE TABLE IF NOT EXISTS [contact_blocklist]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[types] INTEGER NOT NULL DEFAULT 0, ` +
      `[mark] INTEGER NOT NULL DEFAULT 0, ` +
      `[name] TEXT, ` +
      `[phone_number] TEXT, ` +
      `[interception_call_count] INTEGER NOT NULL DEFAULT 0, ` +
      `[interception_msg_count] INTEGER NOT NULL DEFAULT 0, ` +
      `[content] TEXT, ` +
      `[time_stamp] INTEGER NOT NULL DEFAULT 0,` +
      `[format_phone_number] TEXT)`;

        /**
   * Create table contact_blocklist
   */
  public static readonly CREATE_CONTACT_HW_ACCOUNT =
  `CREATE TABLE IF NOT EXISTS [hw_account](` +
    `[contact_id] INTEGER NOT NULL DEFAULT 0, ` +
    `[account_id] TEXT, ` +
    `[phone_number] TEXT, ` +
    `[email] TEXT)`;

    public static readonly CREATE_CLOUD_CONTACT_BLOCKLIST =
    `CREATE TABLE IF NOT EXISTS [cloud_contact_blocklist]( ` +
    `[phone_number] TEXT PRIMARY KEY, ` +
    `[types] INTEGER DEFAULT 0, ` +
    `[mark] INTEGER DEFAULT 0, ` +
    `[name] TEXT, ` +
    `[content] TEXT, ` +
    `[time_stamp] INTEGER DEFAULT 0)`;

  /**
   * Create table search_contact
   */
  public static readonly CREATE_SEARCH_CONTACT =
    `CREATE TABLE IF NOT EXISTS [search_contact]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[account_id] INTEGER   REFERENCES [account]([id]), ` +
      `[contact_id] INTEGER REFERENCES [contact]([id]), ` +
      `[raw_contact_id] INTEGER REFERENCES [raw_contact]([id]), ` +
      `[search_name] TEXT, ` +
      `[display_name] TEXT, ` +
      `[phonetic_name] TEXT, ` +
      `[photo_id] TEXT, ` +
      `[photo_file_id] INTEGER REFERENCES [photo_files]([id]))`;

  /**
   * Create table photo_files
   */
  public static readonly CREATE_PHOTO_FILES =
    `CREATE TABLE IF NOT EXISTS [photo_files]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[file_height] INTEGER NOT NULL DEFAULT 0, ` +
      `[file_width] INTEGER NOT NULL DEFAULT 0, ` +
      `[file_size]  INTEGER NOT NULL DEFAULT 0)`;

  /**
   * Create table local_lang
   */
  public static readonly CREATE_LOCAL_LANG =
    `CREATE TABLE IF NOT EXISTS[local_lang]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[local] TEXT)`;

  /**
   * Create index for contact data and type
   */
  public static readonly CREATE_CONTACT_INDEX_DATA1 =
    `CREATE INDEX IF NOT EXISTS [contact_data_type_contact_data_index] ` +
      `ON [contact_data] ([type_id],[detail_info])`;

  /**
   * Create index for contact data and raw contact
   */
  public static readonly CREATE_CONTACT_INDEX_DATA2 =
    `CREATE INDEX IF NOT EXISTS [contact_data_raw_contact_id] ` +
      `ON [contact_data] ([raw_contact_id])`;

  /**
   * Create index for search contact
   */
  public static readonly CREATE_SEARCH_CONTACT_INDEX1 =
    `CREATE INDEX IF NOT EXISTS [search_contact_id_index] ` +
      `ON [search_contact] ([contact_id])`;

  /**
   * Create index for search contact
   */
  public static readonly CREATE_SEARCH_CONTACT_INDEX2 =
    `CREATE INDEX IF NOT EXISTS [search_raw_contact_id_index] ` +
      `ON [search_contact] ([raw_contact_id])`;

  /**
   * Create index for raw contact
   */
  public static readonly CREATE_RAW_CONTACT_INDEX =
    `CREATE INDEX IF NOT EXISTS [raw_contact_id_index] ` +
      `ON [raw_contact] ([contact_id])`;

  /**
   * Create index for raw contact sort
   */
  public static readonly CREATE_RAW_CONTACT_INDEX_SORT =
    `CREATE INDEX IF NOT EXISTS [raw_contact_sort_index] ` +
      `ON [raw_contact] ([sort], [sort_key])`;

  /**
   * Create view table view_contact
   */
  public static readonly CREATE_VIEW_CONTACT =
    `CREATE VIEW IF NOT EXISTS[view_contact] AS SELECT ` +
      `[contact].[id] AS [id], ` +
      `[contact].[photo_id] AS [photo_id], ` +
      `[contact].[photo_file_id] AS [photo_file_id], ` +
      `[contact].[quick_search_key] AS [quick_search_key], ` +
      `[contact].[has_phone_number] AS [has_phone_number], ` +
      `[contact].[has_display_name] AS [has_display_name], ` +
      `[contact].[has_email] AS [has_email], ` +
      `[contact].[has_group] AS [has_group], ` +
      `[contact].[form_id] AS [form_id], ` +
      `[contact].[focus_mode_list] AS [focus_mode_list], ` +
      `[name_raw_contact_id], ` +
      `[name_raw_contact].[account_id] AS [account_id], ` +
      `[name_raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], ` +
      `[name_raw_contact].[personal_ringtone] AS [personal_ringtone], ` +
      `[name_raw_contact].[is_deleted] AS [is_deleted], ` +
      `[name_raw_contact].[photo_first_name] AS [photo_first_name], ` +
      `[name_raw_contact].[version] AS [version], ` +
      `[name_raw_contact].[display_name] AS [display_name], ` +
      `[name_raw_contact].[sort] AS [sort], ` +
      `[name_raw_contact].[sort_key] AS [sort_key], ` +
      `[name_raw_contact].[contacted_count] AS [contacted_count], ` +
      `[name_raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], ` +
      `[name_raw_contact].[favorite] AS [favorite], ` +
      `[name_raw_contact].[favorite_order] AS [favorite_order], ` +
      `[name_raw_contact].[phonetic_name] AS [phonetic_name], ` +
      `[name_raw_contact].[phonetic_name_type] AS [phonetic_name_type], ` +
      `[name_raw_contact].[company] AS [company], ` +
      `[name_raw_contact].[position] AS [position], ` +
      `[name_raw_contact].[read_only] AS [read_only], ` +
      `[name_raw_contact].[sort_first_letter] AS [sort_first_letter], ` +
      `[name_raw_contact].[merge_mode] AS [merge_mode], ` +
      `[name_raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], ` +
      `[name_raw_contact].[is_need_merge] AS [is_need_merge], ` +
      `[name_raw_contact].[merge_status] AS [merge_status], ` +
      `[name_raw_contact].[is_merge_target] AS [is_merge_target], ` +
      `[name_raw_contact].[vibration_setting] AS [vibration_setting], ` +
      `[name_raw_contact].[sync_id] AS [sync_id], ` +
      `[name_raw_contact].[syn_1] AS [syn_1], ` +
      `[name_raw_contact].[syn_2] AS [syn_2], ` +
      `[name_raw_contact].[syn_3] AS [syn_3], ` +
      `[name_raw_contact].[primary_contact] AS [primary_contact], ` +
      `[name_raw_contact].[extra1] AS [extra1], ` +
      `[name_raw_contact].[extra2] AS [extra2], ` +
      `[name_raw_contact].[extra3] AS [extra3], ` +
      `[name_raw_contact].[extra4] AS [extra4], ` +
      `[name_raw_contact].[aggregation_status] AS [aggregation_status], ` +
      `[account].[account_name] AS [account_name], ` +
      `[account].[account_type] AS [account_type], ` +
      `[photo_files].[file_size] AS [file_size], ` +
      `[photo_files].[file_height] AS [file_height], ` +
      `[photo_files].[file_width] AS [file_width] ` +
      `FROM [contact] ` +
      `JOIN [raw_contact] AS [name_raw_contact] ` +
      `ON ([name_raw_contact_id] = [name_raw_contact].[id]) ` +
      `LEFT JOIN [account] ON ` +
      `([name_raw_contact].[account_id] = [account].[id]) ` +
      `LEFT JOIN [photo_files] ON ` +
      `([photo_files].[id] = [contact].[photo_file_id])`;
      
  /**
   * Create view table view_contact_location
   */
  public static readonly CREATE_VIEW_CONTACT_LOCATION = 
    `CREATE VIEW IF NOT EXISTS[view_contact_location] AS ` +
    `WITH RankedData AS ( SELECT id, type_id, raw_contact_id, detail_info, format_phone_number, location ` +
    `FROM (SELECT id, type_id, raw_contact_id, detail_info,format_phone_number, location, ` +
    `ROW_NUMBER() OVER (PARTITION BY raw_contact_id, CASE WHEN INSTR(location, ' ') > 0 ` +
    `THEN SUBSTR(location, 1, INSTR(location, ' ') - 1) ELSE location END ` +
    `ORDER BY id) AS rn FROM contact_data WHERE type_id = 5 ` +
    `AND location IS NOT NULL AND location <> '') AS tmp WHERE rn = 1), ` +
    `RankedFilteredData AS (SELECT * FROM RankedData WHERE rn = 1) ` +
    `SELECT ` +
    `[contact].[id] AS [id], ` + 
    `[contact].[photo_id] AS [photo_id], ` +
    `[contact].[photo_file_id] AS [photo_file_id], ` +
    `[contact].[quick_search_key] AS [quick_search_key], ` +
    `[contact].[has_phone_number] AS [has_phone_number], ` + 
    `[contact].[has_display_name] AS [has_display_name], ` +
    `[contact].[has_email] AS [has_email], ` +
    `[contact].[has_group] AS [has_group], ` +
    `[contact].[focus_mode_list] AS [focus_mode_list], ` +
    `[contact].[form_id] AS [form_id], ` +
    `[name_raw_contact_id], ` +
    `[name_raw_contact].[account_id] AS [account_id], ` +
    `[name_raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], ` +
    `[name_raw_contact].[personal_ringtone] AS [personal_ringtone], ` +
    `[name_raw_contact].[is_deleted] AS [is_deleted], ` +
    `[name_raw_contact].[photo_first_name] AS [photo_first_name], ` +
    `[name_raw_contact].[version] AS [version], ` +
    `[name_raw_contact].[display_name] AS [display_name], ` +
    `[name_raw_contact].[sort] AS [sort], ` +
    `[name_raw_contact].[sort_key] AS [sort_key], ` +
    `[name_raw_contact].[contacted_count] AS [contacted_count], ` +
    `[name_raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], ` +
    `[name_raw_contact].[favorite] AS [favorite], ` +
    `[name_raw_contact].[favorite_order] AS [favorite_order], ` +
    `[name_raw_contact].[phonetic_name] AS [phonetic_name], ` +
    `[name_raw_contact].[phonetic_name_type] AS [phonetic_name_type], ` +
    `[name_raw_contact].[company] AS [company], ` +
    `[name_raw_contact].[position] AS [position], ` +
    `[name_raw_contact].[read_only] AS [read_only], ` +
    `[name_raw_contact].[sort_first_letter] AS [sort_first_letter], ` +
    `[name_raw_contact].[merge_mode] AS [merge_mode], ` +
    `[name_raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], ` +
    `[name_raw_contact].[is_need_merge] AS [is_need_merge], ` +
    `[name_raw_contact].[merge_status] AS [merge_status], ` +
    `[name_raw_contact].[is_merge_target] AS [is_merge_target], ` +
    `[name_raw_contact].[vibration_setting] AS [vibration_setting], ` +
    `[name_raw_contact].[sync_id] AS [sync_id], ` +
    `[name_raw_contact].[syn_1] AS [syn_1], ` +
    `[name_raw_contact].[syn_2] AS [syn_2], ` +
    `[name_raw_contact].[syn_3] AS [syn_3], ` +
    `[name_raw_contact].[primary_contact] AS [primary_contact], ` +
    `[name_raw_contact].[extra1] AS [extra1], ` +
    `[name_raw_contact].[extra2] AS [extra2], ` +
    `[name_raw_contact].[extra3] AS [extra3], ` +
    `[name_raw_contact].[extra4] AS [extra4], ` +
    `[name_raw_contact].[aggregation_status] AS [aggregation_status], ` +
    `[account].[account_name] AS [account_name], ` +
    `[account].[account_type] AS [account_type], ` +
    `[photo_files].[file_size] AS [file_size], ` +
    `[photo_files].[file_height] AS [file_height], ` +
    `[photo_files].[file_width] AS [file_width],` +
    `[RankedData].[location] AS [location]` +
    `FROM [contact] JOIN [raw_contact] AS [name_raw_contact] ` +
    `ON ([name_raw_contact_id] = [name_raw_contact].[id]) ` +
    `LEFT JOIN [account] ON ([name_raw_contact].[account_id] = [account].[id]) ` +
    `LEFT JOIN [photo_files] ON ([photo_files].[id] = [contact].[photo_file_id])` +
    `LEFT JOIN [RankedData] ON ([name_raw_contact_id] = RankedData.[raw_contact_id])` +
    `AND ([RankedData].[location] IS NOT NULL AND [RankedData].[location] <> '')`;

  /**
   * Create view table view_raw_contact
   */
  public static readonly CREATE_VIEW_RAW_CONTACT =
    `CREATE VIEW IF NOT EXISTS [view_raw_contact] AS SELECT ` +
      `[contact_id], ` +
      `[raw_contact].[id] AS [id], ` +
      `[raw_contact].[account_id] AS [account_id], ` +
      `[raw_contact].[photo_id] AS [photo_id], ` +
      `[raw_contact].[photo_file_id] AS [photo_file_id], ` +
      `[raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], ` +
      `[raw_contact].[personal_ringtone] AS [personal_ringtone], ` +
      `[raw_contact].[is_deleted] AS [is_deleted], ` +
      `[raw_contact].[photo_first_name] AS [photo_first_name], ` +
      `[raw_contact].[version] AS [version], ` +
      `[raw_contact].[display_name] AS [display_name], ` +
      `[raw_contact].[sort] AS [sort], ` +
      `[raw_contact].[sort_key] AS [sort_key], ` +
      `[raw_contact].[contacted_count] AS [contacted_count], ` +
      `[raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], ` +
      `[raw_contact].[favorite] AS [favorite], ` +
      `[raw_contact].[favorite_order] AS [favorite_order], ` +
      `[raw_contact].[phonetic_name] AS [phonetic_name], ` +
      `[raw_contact].[phonetic_name_type] AS [phonetic_name_type], ` +
      `[raw_contact].[company] AS [company], ` +
      `[raw_contact].[position] AS [position], ` +
      `[raw_contact].[read_only] AS [read_only], ` +
      `[raw_contact].[sort_first_letter] AS [sort_first_letter], ` +
      `[raw_contact].[merge_mode] AS [merge_mode], ` +
      `[raw_contact].[is_need_merge] AS [is_need_merge], ` +
      `[raw_contact].[merge_status] AS [merge_status], ` +
      `[raw_contact].[is_merge_target] AS [is_merge_target], ` +
      `[raw_contact].[vibration_setting] AS [vibration_setting], ` +
      `[raw_contact].[sync_id] AS [sync_id], ` +
      `[raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], ` +
      `[raw_contact].[syn_1] AS [syn_1], ` +
      `[raw_contact].[syn_2] AS [syn_2], ` +
      `[raw_contact].[syn_3] AS [syn_3], ` +
      `[raw_contact].[primary_contact] AS [primary_contact], ` +
      `[raw_contact].[extra1] AS [extra1], ` +
      `[raw_contact].[extra2] AS [extra2], ` +
      `[raw_contact].[extra3] AS [extra3], ` +
      `[raw_contact].[extra4] AS [extra4], ` +
      `[raw_contact].[dirty] AS [dirty], ` +
      `[raw_contact].[uuid] AS [uuid], ` +
      `[raw_contact].[unique_key] AS [unique_key], ` +
      `[raw_contact].[aggregation_status] AS [aggregation_status], ` +
      `[photo_files].[file_size] AS [file_size], ` +
      `[photo_files].[file_height] AS [file_height], ` +
      `[photo_files].[file_width] AS [file_width], ` +
      `[account].[account_name] AS [account_name], ` +
      `[account].[account_type] AS [account_type] ` +
      `FROM [raw_contact] ` +
      `LEFT JOIN [account] ON ` +
      `([raw_contact].[account_id] = [account].[id]) ` +
      `LEFT JOIN [photo_files] ON ` +
      `([photo_files].[id] = [photo_file_id])`;

  /**
   * Create view table view_contact_data
   */
  public static readonly CREATE_VIEW_CONTACT_DATA =
  `CREATE VIEW IF NOT EXISTS [view_contact_data] AS SELECT ` +
  `[contact_data].[id] AS [id], ` +
  `[contact_data].[type_id] AS [type_id], ` +
  `[contact_data].[raw_contact_id] AS [raw_contact_id], ` +
  `[contact_data].[read_only] AS [read_only], ` +
  `[contact_data].[version] AS [version], ` +
  `[contact_data].[family_name] AS [family_name], ` +
  `[contact_data].[middle_name_phonetic] AS [middle_name_phonetic], ` +
  `[contact_data].[given_name] AS [given_name], ` +
  `[contact_data].[given_name_phonetic] AS [given_name_phonetic], ` +
  `[contact_data].[is_preferred_number] AS [is_preferred_number], ` +
  `[contact_data].[phonetic_name] AS [phonetic_name], ` +
  `[contact_data].[format_phone_number] AS [format_phone_number], ` +
  `[contact_data].[detail_info] AS [detail_info], ` +
  `[contact_data].[alias_detail_info] AS [alias_detail_info], ` +
  `[contact_data].[extend1] AS [extend1], ` +
  `[contact_data].[extend2] AS [extend2], ` +
  `[contact_data].[extend3] AS [extend3], ` +
  `[contact_data].[extend4] AS [extend4], ` +
  `[contact_data].[alpha_name] AS [alpha_name], ` +
  `[contact_data].[other_lan_last_name] AS [other_lan_last_name], ` +
  `[contact_data].[other_lan_first_name] AS [other_lan_first_name], ` +
  `[contact_data].[extend5] AS [extend5], ` +
  `[contact_data].[lan_style] AS [lan_style], ` +
  `[contact_data].[custom_data] AS [custom_data], ` +
  `[contact_data].[extend6] AS [extend6], ` +
  `[contact_data].[extend7] AS [extend7], ` +
  `[contact_data].[blob_data] AS [blob_data], ` +
  `[contact_data].[syn_1] AS [syn_1], ` +
  `[contact_data].[city] AS [city], ` +
  `[contact_data].[syn_2] AS [syn_2], ` +
  `[contact_data].[syn_3] AS [syn_3], ` +
  `[contact_data].[country] AS [country], ` +
  `[contact_data].[neighborhood] AS [neighborhood], ` +
  `[contact_data].[pobox] AS [pobox], ` +
  `[contact_data].[postcode] AS [postcode], ` +
  `[contact_data].[region] AS [region], ` +
  `[contact_data].[street] AS [street], ` +
  `[contact_data].[extend8] AS [extend8], ` +
  `[contact_data].[extend9] AS [extend9], ` +
  `[contact_data].[extend10] AS [extend10], ` +
  `[contact_data].[extend11] AS [extend11], ` +
  `[contact_data].[blob_source] AS [blob_source], ` +
  `[contact_data].[calendar_event_id] AS [calendar_event_id], ` +
  `[contact_data].[is_sync_birthday_to_calendar] AS [is_sync_birthday_to_calendar], ` +
  `[contact_data].[location] AS [location], ` +
  `[contact_type].[content_type] AS [content_type], ` +
  `[account].[account_name] AS [account_name], ` +
  `[account].[account_type] AS [account_type], ` +
  `[account].[data_info] AS [data_info], ` +
  `[account].[id] AS [account_id], ` +
  `[groups].[id] AS [group_id], ` +
  `[groups].[group_name] AS [group_name], ` +
  `[groups].[group_notes] AS [group_notes], ` +
  `[groups].[is_deleted] AS [group_is_deleted], ` +
  `[groups].[group_ringtone] AS [group_ringtone], ` +
  `[groups].[ringtone_modify_time] AS [ringtone_modify_time], ` +
  `[groups].[lastest_modify_time] AS [lastest_modify_time], ` +
  `[photo_files].[file_size] AS [file_size], ` +
  `[photo_files].[file_height] AS [file_height], ` +
  `[photo_files].[file_width] AS [file_width], ` +
  `[contact].[id] AS [contact_id], ` +
  `[contact].[photo_id] AS [photo_id], ` +
  `[contact].[photo_file_id] AS [photo_file_id], ` +
  `[contact].[personal_ringtone] AS [personal_ringtone], ` +
  `[contact].[is_transfer_voicemail] AS [is_transfer_voicemail], ` +
  `[contact].[company] AS [company], ` +
  `[contact].[quick_search_key] AS [quick_search_key], ` +
  `[contact].[read_only] AS [contact_read_only], ` +
  `[contact].[personal_notification_ringtone] AS ` +
  `[personal_notification_ringtone], ` +
  `[contact].[has_phone_number] AS [has_phone_number], ` +
  `[contact].[has_display_name] AS [has_display_name], ` +
  `[contact].[has_email] AS [has_email], ` +
  `[contact].[has_group] AS [has_group], ` +
  `[contact].[contact_last_updated_timestamp] AS [contact_last_updated_timestamp], ` +
  `[contact].[focus_mode_list] AS [focus_mode_list], ` +
  `[contact].[form_id] AS [form_id], ` +
  `[contact].[name_raw_contact_id] AS [name_raw_contact_id], ` +
  `[contact].[ringtone_path] AS [ringtone_path], ` +
  `[contact].[prefer_avatar] AS [prefer_avatar], ` +
  `[raw_contact].[is_transfer_voicemail] AS [raw_is_transfer_voicemail], ` +
  `[raw_contact].[personal_ringtone] AS [raw_personal_ringtone], ` +
  `[raw_contact].[personal_notification_ringtone] AS [raw_personal_notification_ringtone], ` +
  `[raw_contact].[version] AS [raw_version], ` +
  `[raw_contact].[contacted_count] AS [contacted_count], ` +
  `[raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], ` +
  `[raw_contact].[favorite] AS [favorite], ` +
  `[raw_contact].[favorite_order] AS [favorite_order], ` +
  `[raw_contact].[company] AS [raw_company], ` +
	`[raw_contact].[read_only] AS [raw_read_only], ` +
  `[raw_contact].[position] AS [position], ` +
  `[raw_contact].[display_name] AS [display_name], ` +
  `[raw_contact].[sort] AS [sort], ` +
  `[raw_contact].[sort_key] AS [sort_key], ` +
	`[raw_contact].[merge_mode] AS [merge_mode], ` +
	`[raw_contact].[is_need_merge] AS [is_need_merge], ` +
	`[raw_contact].[merge_status] AS [merge_status], ` +
	`[raw_contact].[is_merge_target] AS [is_merge_target], ` +
	`[raw_contact].[vibration_setting] AS [vibration_setting], ` +
	`[raw_contact].[sync_id] AS [sync_id], ` +
	`[raw_contact].[syn_1] AS [raw_syn_1], ` +
	`[raw_contact].[syn_2] AS [raw_syn_2], ` +
	`[raw_contact].[syn_3] AS [raw_syn_3], ` +
  `[raw_contact].[sort_first_letter] AS [sort_first_letter], ` +
  `[raw_contact].[is_deleted] AS [is_deleted], ` +
  `[raw_contact].[phonetic_name_type] AS [phonetic_name_type], ` +
  `[raw_contact].[phonetic_name] AS [raw_phonetic_name], ` +
  `[raw_contact].[photo_first_name] AS [photo_first_name], ` +
  `[raw_contact].[primary_contact] AS [primary_contact], ` +
  `[raw_contact].[extra1] AS [extra1], ` +
  `[raw_contact].[extra2] AS [extra2], ` +
  `[raw_contact].[extra3] AS [extra3], ` +
  `[raw_contact].[extra4] AS [extra4], ` +
	`[raw_contact].[dirty] AS [dirty], ` +
  `[raw_contact].[unique_key] AS [unique_key], ` +
  `[raw_contact].[aggregation_status] AS [aggregation_status] ` +
  `FROM [contact_data] ` +
  `JOIN [raw_contact] ON ` +
  `([contact_data].[raw_contact_id] = [raw_contact].[id]) ` +
  `JOIN [contact] ON ([contact].[id] = [raw_contact].[contact_id]) ` +
  `JOIN [raw_contact] AS [name_raw_contact] ON ` +
  `([name_raw_contact_id] = [name_raw_contact].[id]) ` +
  `LEFT JOIN [account] ON ` +
  `([raw_contact].[account_id] = [account].[id]) ` +
  `LEFT JOIN [contact_type] ON ` +
  `([contact_type].[id] = [contact_data].[type_id]) ` +
  `LEFT JOIN [groups] ON ` +
  `([contact_type].[content_type] = 'group_membership' ` +
  `AND [groups].[id] = [contact_data].[detail_info]) ` +
  `LEFT JOIN [photo_files] ON ` +
  `([contact_type].[content_type] = 'photo' ` +
  `AND [photo_files].[id] = [raw_contact].[photo_file_id])`;

  /**
   * Create view table view_groups
   */
  public static readonly CREATE_VIEW_GROUPS =
    `CREATE VIEW IF NOT EXISTS [view_groups] ` +
      `AS SELECT ` +
      `[groups].[id] AS [id], ` +
      `[groups].[account_id] AS [account_id], ` +
      `[account].[account_name] AS [account_name], ` +
      `[account].[account_type] AS [account_type], ` +
      `[groups].[group_name] AS [group_name] , ` +
      `[groups].[group_notes] AS [group_notes] , ` +
      `[groups].[is_deleted]  AS [is_deleted] , ` +
      `[groups].[group_ringtone]  AS [group_ringtone], ` +
      `[groups].[ringtone_modify_time]  AS [ringtone_modify_time] , ` +
      `[groups].[lastest_modify_time]  AS [lastest_modify_time] ` +
      `FROM [groups] ` +
      `LEFT JOIN [account] ON ([groups].[account_id] = [account].[id])`;

  /**
   * Create trigger to update contact by insert contact data
   */
  public static readonly UPDATE_CONTACT_BY_INSERT_CONTACT_DATA =
  `CREATE TRIGGER IF NOT EXISTS [update_contact_by_insert_contact_data] AFTER INSERT ON [contact_data] FOR EACH ROW ` +
  `BEGIN ` +
  `UPDATE contact ` +
  `SET ` +
  `has_display_name = CASE WHEN NEW.type_id = 6 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_display_name END,` +
  `has_email = CASE WHEN NEW.type_id = 1 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_email END,` +
  `has_group = CASE WHEN NEW.type_id = 9 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_group END,` +
  `has_phone_number = CASE WHEN NEW.type_id = 5 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_phone_number END ` +
  `WHERE NEW.raw_contact_id = name_raw_contact_id;` +
  `END`;

  /**
   * Create view table search_contact_view
   */
  public static readonly CREATE_SEARCH_CONTACT_VIEW =
    `CREATE VIEW IF NOT EXISTS [search_contact_view] AS SELECT ` +
      `[search_contact].[id] AS [id], ` +
      `[search_contact].[account_id] AS [account_id], ` +
      `[search_contact].[contact_id] AS [contact_id], ` +
      `[search_contact].[raw_contact_id] AS [raw_contact_id], ` +
      `[search_contact].[search_name] AS [search_name], ` +
      `[search_contact].[photo_id] AS [photo_id], ` +
      `[search_contact].[photo_file_id] AS [photo_file_id], ` +
      `[contact_type].[content_type] AS [content_type], ` +
      `[account].[account_name] AS [account_name], ` +
      `[account].[account_type] AS [account_type], ` +
      `[account].[data_info] AS [data_info], ` +
      `[groups].[group_name] AS [group_name], ` +
      `[groups].[group_notes] AS [group_notes], ` +
      `[groups].[is_deleted] AS [group_is_deleted], ` +
      `[groups].[group_ringtone] AS [group_ringtone], ` +
      `[groups].[ringtone_modify_time] AS [ringtone_modify_time], ` +
      `[groups].[lastest_modify_time] AS [lastest_modify_time], ` +
      `[photo_files].[file_size] AS [file_size], ` +
      `[photo_files].[file_height] AS [file_height], ` +
      `[photo_files].[file_width] AS [file_width], ` +
      `[contact].[personal_ringtone] AS [personal_ringtone], ` +
      `[contact].[is_transfer_voicemail] AS [is_transfer_voicemail], ` +
      `[contact].[personal_notification_ringtone] AS ` +
      `[personal_notification_ringtone], ` +
      `[contact].[has_phone_number] AS [has_phone_number], ` +
      `[contact].[has_display_name] AS [has_display_name], ` +
      `[contact].[has_email] AS [has_email], ` +
      `[contact].[has_group] AS [has_group], ` +
      `[raw_contact].[contacted_count] AS [contacted_count], ` +
      `[raw_contact].[favorite] AS [favorite], ` +
      `[raw_contact].[favorite_order] AS [favorite_order], ` +
      `[raw_contact].[display_name] AS [display_name], ` +
      `[raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], ` +
      `[raw_contact].[sort_first_letter] AS [sort_first_letter], ` +
      `[raw_contact].[sort] AS [sort], ` +
      `[raw_contact].[photo_first_name] AS [photo_first_name], ` +
      `[raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], ` +
      `[raw_contact].[is_deleted] AS [is_deleted], ` +
      `[raw_contact].[primary_contact] AS [primary_contact], ` +
      `[raw_contact].[extra1] AS [extra1], ` +
      `[raw_contact].[extra2] AS [extra2], ` +
      `[raw_contact].[extra3] AS [extra3], ` +
      `[raw_contact].[extra4] AS [extra4], ` +
      `[contact_data].[type_id] AS [type_id], ` +
      `[contact_data].[phonetic_name] AS [phonetic_name], ` +
      `[contact_data].[raw_contact_id] AS [raw_contact_id], ` +
      `[contact_data].[read_only] AS [read_only], ` +
      `[contact_data].[version] AS [version], ` +
      `[contact_data].[alias_detail_info] AS [alias_detail_info], ` +
      `[contact_data].[is_preferred_number] AS [is_preferred_number], ` +
      `[contact_data].[detail_info] AS [detail_info], ` +
      `[contact_data].[city] AS [city], ` +
      `[contact_data].[position] AS [position], ` +
      `[contact_data].[middle_name_phonetic] AS [middle_name_phonetic], ` +
      `[contact_data].[given_name] AS [given_name], ` +
      `[contact_data].[family_name] AS [family_name], ` +
      `[contact_data].[given_name_phonetic] AS [given_name_phonetic], ` +
      `[contact_data].[country] AS [country], ` +
      `[contact_data].[neighborhood] AS [neighborhood], ` +
      `[contact_data].[pobox] AS [pobox], ` +
      `[contact_data].[postcode] AS [postcode], ` +
      `[contact_data].[region] AS [region], ` +
      `[contact_data].[street] AS [street], ` +
      `[contact_data].[extend1] AS [extend1], ` +
      `[contact_data].[extend2] AS [extend2], ` +
      `[contact_data].[extend3] AS [extend3], ` +
      `[contact_data].[extend4] AS [extend4], ` +
      `[contact_data].[alpha_name] AS [alpha_name], ` +
      `[contact_data].[other_lan_last_name] AS [other_lan_last_name], ` +
      `[contact_data].[other_lan_first_name] AS [other_lan_first_name], ` +
      `[contact_data].[extend5] AS [extend5], ` +
      `[contact_data].[lan_style] AS [lan_style], ` +
      `[contact_data].[custom_data] AS [custom_data], ` +
      `[contact_data].[extend6] AS [extend6], ` +
      `[contact_data].[extend7] AS [extend7], ` +
      `[contact_data].[blob_data] AS [blob_data], ` +
      `[contact_data].[syn_1] AS [syn_1], ` +
      `[contact_data].[syn_2] AS [syn_2], ` +
      `[contact_data].[syn_3] AS [syn_3], ` +
      `[contact_data].[extend8] AS [extend8], ` +
      `[contact_data].[extend9] AS [extend9], ` +
      `[contact_data].[extend10] AS [extend10], ` +
      `[contact_data].[extend11] AS [extend11] ` +
      `FROM [search_contact] ` +
      `JOIN [raw_contact] ON ` +
      `([search_contact].[raw_contact_id] = [raw_contact].[id]) ` +
      `JOIN [contact] ON ` +
      `([contact].[id] = [raw_contact].[contact_id]) ` +
      `JOIN [raw_contact] AS [name_raw_contact] ON ` +
      `([name_raw_contact_id] = [name_raw_contact].[id]) ` +
      `LEFT JOIN [account] ON ` +
      `([search_contact].[account_id] = [account].[id]) ` +
      `LEFT JOIN [contact_data] ON ` +
      `([contact_data].[raw_contact_id] = [raw_contact].[id]) ` +
      `LEFT JOIN [contact_type] ON ` +
      `([contact_data].[type_id] = [contact_type].[id]) ` +
      `LEFT JOIN [groups] ON ` +
      `([contact_type].[content_type] = 'group_membership' ` +
      `AND [groups].[id] = [contact_data].[detail_info]) ` +
      `LEFT JOIN [photo_files] ON ` +
      `([contact_type].[content_type] = 'photo' ` +
      `AND [photo_files].[id] = [search_contact].[photo_file_id])`;

  /**
   * Create view table view_deleted
   */
  public static readonly CREATE_VIEW_DELETED =
    `CREATE VIEW IF NOT EXISTS [view_deleted] ` +
      `AS  SELECT ` +
      `[deleted_raw_contact].[id] AS [id], ` +
      `[contact].[id] AS [contact_id], ` +
      `[contact].[photo_id] AS [photo_id], ` +
      `[contact].[photo_file_id] AS [photo_file_id], ` +
      `[contact].[quick_search_key] AS [quick_search_key], ` +
      `[contact].[has_phone_number] AS [has_phone_number], ` +
      `[contact].[has_display_name] AS [has_display_name], ` +
      `[contact].[has_email] AS [has_email], ` +
      `[contact].[has_group] AS [has_group], ` +
      `[name_raw_contact].[account_id] AS [account_id], ` +
      `[name_raw_contact].[id] AS [raw_contact_id], ` +
      `[name_raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], ` +
      `[name_raw_contact].[personal_ringtone] AS [personal_ringtone], ` +
      `[name_raw_contact].[is_deleted] AS [is_deleted], ` +
      `[name_raw_contact].[version] AS [version], ` +
      `[name_raw_contact].[display_name] AS [display_name], ` +
      `[name_raw_contact].[sort] AS [sort], ` +
      `[name_raw_contact].[contacted_count] AS [contacted_count], ` +
      `[name_raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], ` +
      `[name_raw_contact].[favorite] AS [favorite], ` +
      `[name_raw_contact].[favorite_order] AS [favorite_order], ` +
      `[name_raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], ` +
      `[name_raw_contact].[phonetic_name] AS [phonetic_name], ` +
      `[name_raw_contact].[phonetic_name_type] AS [phonetic_name_type], ` +
      `[name_raw_contact].[company] AS [company], ` +
      `[name_raw_contact].[position] AS [position], ` +
      `[name_raw_contact].[read_only] AS [read_only], ` +
      `[name_raw_contact].[sort_first_letter] AS [sort_first_letter], ` +
      `[name_raw_contact].[merge_mode] AS [merge_mode], ` +
      `[name_raw_contact].[is_need_merge] AS [is_need_merge], ` +
      `[name_raw_contact].[merge_status] AS [merge_status], ` +
      `[name_raw_contact].[is_merge_target] AS [is_merge_target], ` +
      `[name_raw_contact].[vibration_setting] AS [vibration_setting], ` +
      `[name_raw_contact].[photo_first_name] AS [photo_first_name], ` +
      `[name_raw_contact].[sync_id] AS [sync_id], ` +
      `[name_raw_contact].[syn_1] AS [syn_1], ` +
      `[name_raw_contact].[syn_2] AS [syn_2], ` +
      `[name_raw_contact].[syn_3] AS [syn_3], ` +
      `[raw_contact].[primary_contact] AS [primary_contact], ` +
      `[raw_contact].[extra1] AS [extra1], ` +
      `[raw_contact].[extra2] AS [extra2], ` +
      `[raw_contact].[extra3] AS [extra3], ` +
      `[raw_contact].[extra4] AS [extra4], ` +
      `[deleted_raw_contact].[delete_source] AS [delete_source],` +
      `[deleted_raw_contact].[delete_time] AS [delete_time], ` +
      `[deleted_raw_contact].[delete_account] AS [delete_account], ` +
      `[deleted_raw_contact].[backup_data] AS [backup_data], ` +
      `[account].[account_name] AS [account_name], ` +
      `[account].[account_type] AS [account_type], ` +
      `[photo_files].[file_size] AS [file_size], ` +
      `[photo_files].[file_height] AS [file_height], ` +
      `[photo_files].[file_width] AS [file_width] ` +
      `FROM   [deleted_raw_contact] ` +
      `LEFT JOIN [contact] ON ([contact].[id] = [deleted_raw_contact].[contact_id]) ` +
      `LEFT JOIN [raw_contact] AS [name_raw_contact] ON ([name_raw_contact].[id] = ` +
      `[deleted_raw_contact].[raw_contact_id] ` +
      `AND [name_raw_contact].[is_deleted] = 1)` +
      `LEFT JOIN [account] ON ([name_raw_contact].[account_id] = [account].[id]) ` +
      `LEFT JOIN [photo_files] ON ([photo_files].[id] = [name_raw_contact].[photo_file_id]) `;

  /**
   * Create trigger to update contact by delete contact_data
   */
  public static readonly UPDATE_CONTACT_BY_DELETE_CONTACT_DATA =
    `CREATE TRIGGER IF NOT EXISTS [update_contact_by_delete_contact_data] ` +
      `AFTER DELETE ON [contact_data] FOR EACH ROW ` +
      `BEGIN ` +
      `UPDATE ` +
      `[contact] ` +
      `SET ` +
      `[has_display_name] = 0 ` +
      `WHERE ` +
      `[OLD].[raw_contact_id] = [name_raw_contact_id] ` +
      `AND [OLD].[type_id] = 6; ` +
      `UPDATE ` +
      `[contact] ` +
      `SET ` +
      `[has_email] = 0 ` +
      `WHERE ` +
      `[OLD].[raw_contact_id] = [name_raw_contact_id] ` +
      `AND [OLD].[type_id] = 1; ` +
      `UPDATE ` +
      `[contact] ` +
      `SET ` +
      `[has_group] = 0 ` +
      `WHERE ` +
      `[OLD].[raw_contact_id] = [name_raw_contact_id] ` +
      `AND [OLD].[type_id] = 9; ` +
      `UPDATE ` +
      `[contact] ` +
      `SET ` +
      `[has_phone_number] = 0 ` +
      `WHERE ` +
      `[OLD].[raw_contact_id] = [name_raw_contact_id] ` +
      `AND [OLD].[type_id] = 5; ` +
      `END`;

  /**
   * Create trigger to update contact by update contact_data
   */
  public static readonly UPDATE_CONTACT_BY_UPDATE_CONTACT_DATA =
  `CREATE TRIGGER IF NOT EXISTS [update_contact_by_update_contact_data] AFTER UPDATE ON [contact_data] FOR EACH ROW ` +
  `BEGIN ` +
  `UPDATE contact ` +
  `SET ` +
  `has_display_name = CASE WHEN NEW.type_id = 6 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_display_name END,` +
  `has_email = CASE WHEN NEW.type_id = 1 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_email END,` +
  `has_group = CASE WHEN NEW.type_id = 9 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_group END,` +
  `has_phone_number = CASE WHEN NEW.type_id = 5 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_phone_number END ` +
  `WHERE NEW.raw_contact_id = name_raw_contact_id;` +
  `END`;

  /**
   * Create trigger to update raw_contact_version
   */
  public static readonly UPDATE_RAW_CONTACT_VERSION =
    `CREATE TRIGGER IF NOT EXISTS [update_raw_contact_version] AFTER UPDATE ON [raw_contact] ` +
      `BEGIN ` +
      `UPDATE ` +
      `[raw_contact] ` +
      `SET ` +
      `[version] = [OLD].[version] + 1 ` +
      `WHERE ` +
      `[id] = [OLD].[id] AND [NEW].[is_deleted] != [OLD].[is_deleted]; ` +
      `END`;

  /**
   * Create trigger to update contact_data_version
   */
  public static readonly UPDATE_CONTACT_DATA_VERSION =
    `CREATE TRIGGER IF NOT EXISTS [update_contact_data_version] AFTER UPDATE ON [contact_data] ` +
      `BEGIN ` +
      `IF UPDATE ` +
      `[version] RETURN; ` +
      `UPDATE ` +
      `[contact_data] ` +
      `SET ` +
      `[version] = [OLD].[version] + 1 ` +
      `WHERE ` +
      `[id] = [OLD].[id]; ` +
      `END`;

  /**
   * Create trigger to insert contact_quick_searc
   */
  public static readonly INSERT_CONTACT_QUICK_SEARCH =
    `CREATE TRIGGER IF NOT EXISTS [insert_contact_quick_search]AFTER INSERT ON [contact] BEGIN ` +
      `UPDATE [contact] SET [quick_search_key] = [NEW].[id] WHERE [id] = [NEW].[id]; END`;

  /**
   * Create table merge_info
   */
  public static readonly MERGE_INFO =
    `CREATE TABLE IF NOT EXISTS [merge_info](` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[raw_contact_id] INTEGER NOT NULL DEFAULT 0 )`;

  /**
   * Create index for merge_info
   */
  public static readonly MERGE_INFO_INDEX =
    `CREATE INDEX IF NOT EXISTS [merge_info_index] ON [merge_info] ([raw_contact_id])`;

  /**
   * Create table cloud_raw_contact
   */
  public static readonly CREATE_CLOUD_RAW_CONTACT =
    `CREATE TABLE IF NOT EXISTS [cloud_raw_contact](` +
      `[uuid] TEXT PRIMARY KEY, ` +
      `[data] TEXT, ` +
      `[recycled] boolean, ` +
      `[recycledTime] INTEGER, ` +
      `[attachments] ASSETS)`;

  /**
   * Create table cloud_groups
   */
  public static readonly CREATE_CLOUD_GROUPS =
    `CREATE TABLE IF NOT EXISTS [cloud_groups](` +
      `[uuid] TEXT PRIMARY KEY, ` +
      `[data] TEXT, ` +
      `[recycled] boolean, ` +
      `[recycledTime] INTEGER, ` +
      `[attachments] ASSETS)`;

  /**
   * Create table settings
   */
  public static readonly CREATE_SETTINGS =
    `CREATE TABLE IF NOT EXISTS [settings](` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[contact_change_time] DATETIME, ` +
      `[contacts_on_card] TEXT DEFAULT ',', ` +
      `[refresh_contacts]  INTEGER NOT NULL DEFAULT 0,` +
      `[clone_time_stamp] INTEGER NOT NULL DEFAULT 0,` +
      `[cloud_raw_contact_cursor] INTEGER NOT NULL DEFAULT -1,` +
      `[main_space_need_sync_flag] INTEGER NOT NULL DEFAULT 0,` +
      `[refresh_location] INTEGER NOT NULL DEFAULT 0,` +
      `[blocklist_migrate_status] INTEGER NOT NULL DEFAULT 0,` +
      `[cloud_sync_delete_flag] INTEGER NOT NULL DEFAULT 0)`;



  /**
   * Init change time for table settings
   */
  public static readonly INIT_CHANGE_TIME =
    `INSERT INTO settings (contact_change_time) VALUES (datetime('now'))`;

  /**
   * Create table database_backup_task
   */
  public static readonly CREATE_DATABASE_BACKUP_TASK =
    `CREATE TABLE IF NOT EXISTS[database_backup_task]( ` +
      `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
      `[backup_time] TEXT, ` +
      `[backup_path] TEXT, ` +
      `[remarks] TEXT)`;

  /**
   * Create trigger to insert database_backup_task
   */
  public static readonly CREATE_INSERT_BACKUP_TIME =
    `CREATE TRIGGER IF NOT EXISTS [insert_database_backup_task] AFTER INSERT ON [database_backup_task] ` +
      ` BEGIN ` +
      ` UPDATE ` +
      ` [database_backup_task] ` +
      ` SET ` +
      ` [backup_time] = STRFTIME ('%s', 'now') ` +
      ` WHERE ` +
      ` [id] = [NEW].[id]; ` +
      ` END `;

  /**
   * Create unique index for search and unique
   */
  public static readonly CREATE_CONTACT_BLOCKLIST_INDEX_PHONE =
    `CREATE UNIQUE INDEX IF NOT EXISTS [contact_blocklist_phone_index]  ` +
  `ON [contact_blocklist] ([phone_number], [types])`;

  /**
   * Create index for unique_key
   */
  public static readonly CREATE_RAW_CONTACT_INDEX_UNIQUE_KEY =
  `CREATE INDEX IF NOT EXISTS [raw_contact_unique_key_index] ON [raw_contact] ([unique_key])`;

  public static readonly CREATE_PRIVACY_CONTACTS_BACKUP =
    `CREATE TABLE IF NOT EXISTS [privacy_contacts_backup]( ` +
    `[id] INTEGER PRIMARY KEY AUTOINCREMENT, ` +
    `[contact_data_id] INTEGER, ` +
    `[raw_contact_id] INTEGER, ` +
    `[phone_number] TEXT, ` +
    `[format_phone_number] TEXT, ` +
    `[calls_synced] INTEGER DEFAULT 0)`;

  public static readonly CREATE_POSTER =
    `CREATE TABLE IF NOT EXISTS [poster] ( ` +
    `[phone_number] TEXT PRIMARY KEY, ` +
    `[poster_version] TEXT, ` +
    `[poster_uri] TEXT, ` +
    `[poster_file_path] TEXT, ` +
    `[poster_crop_info] TEXT, ` +
    `[avatar_uri] TEXT, ` +
    `[avatar_file_path] TEXT, ` +
    `[video_poster_uri] TEXT, ` +
    `[video_poster_file_path] TEXT, ` +
    `[video_poster_crop_info] TEXT, ` +
    `[insert_time] INTEGER, ` +
    `[update_time] INTEGER, ` +
    `[extra_info] TEXT)`;

  /**
   * All sql to create blocklist tables for contacts.db
   */
  public static readonly CREATE_CONTACT_BLOCK_TABLES: string[] = [

    SqlConstants.CREATE_CONTACT_BLOCKLIST,
    SqlConstants.CREATE_CONTACT_BLOCKLIST_INDEX_PHONE,

  ];

}
