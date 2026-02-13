/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import BaseColumns from './BaseColumns';

/**
 * Field constants in the contact rawcontact table
 */
export default class RawContactsColumns extends BaseColumns {
  /**
   * Column 'contact_id' in table raw_contacts
   */
  public static readonly CONTACT_ID: string = 'contact_id';

  /**
   * Column 'photo_id' in table raw_contacts
   */
  public static readonly PHOTO_ID: string = 'photo_id';

  /**
   * Column 'photo_file_id' in table raw_contacts
   */
  public static readonly PHOTO_FILE_ID: string = 'photo_file_id';

  /**
   * Column 'is_transfer_voicemail' in table raw_contacts
   */
  public static readonly IS_TRANSFER_VOICEMAIL: string = 'is_transfer_voicemail';

  /**
   * Column 'personal_ringtone' in table raw_contacts
   */
  public static readonly PERSONAL_RINGTONE: string = 'personal_ringtone';

  /**
   * Column 'is_deleted' in table raw_contacts
   */
  public static readonly IS_DELETED: string = 'is_deleted';

  /**
   * Column 'personal_notification_ringtone' in table raw_contacts
   */
  public static readonly PERSONAL_NOTIFICATION_RINGTONE: string = 'personal_notification_ringtone';

  /**
   * Column 'photo_first_name' in table raw_contacts
   */
  public static readonly PHOTO_FIRST_NAME: string = 'photo_first_name';

  /**
   * Column 'account_id' in table raw_contacts
   */
  public static readonly ACCOUNT_ID: string = 'account_id';

  /**
   * Column 'version' in table raw_contacts
   */
  public static readonly VERSION: string = 'version';

  /**
   * Column 'display_name' in table raw_contacts
   */
  public static readonly DISPLAY_NAME: string = 'display_name';

   /**
   * Column 'sort' in table raw_contacts
   */
  public static readonly SORT: string = 'sort';

  /**
   * Column 'sort_key' in table raw_contacts
   */
  public static readonly SORT_KEY: string = 'sort_key';

  /**
   * Column 'contacted_count' in table raw_contacts
   */
  public static readonly CONTACTED_COUNT: string = 'contacted_count';

  /**
   * Column 'lastest_contacted_time' in table raw_contacts
   */
  public static readonly LASTEST_CONTACTED_TIME: string = 'lastest_contacted_time';

  /**
   * Column 'id' in table raw_contacts
   */
  public static readonly SINGLE_ID: string = 'id';

  /**
   * Column 'favorite' in table raw_contacts
   */
  public static readonly FAVORITE: string = 'favorite';

  /**
   * Column 'favorite_order' in table raw_contacts
   */
  public static readonly FAVORITE_ORDER: string = 'favorite_order';

  /**
   * Column 'phonetic_name' in table raw_contacts
   */
  public static readonly PHONETIC_NAME: string = 'phonetic_name';

  /**
   * Column 'phonetic_name_type' in table raw_contacts
   */
  public static readonly PHONETIC_NAME_TYPE: string = 'phonetic_name_type';

  /**
   * Column 'company' in table raw_contacts
   */
  public static readonly COMPANY: string = 'company';

  /**
   * Column 'position' in table raw_contacts
   */
  public static readonly POSITION: string = 'position';

  /**
   * Column 'title' in table raw_contacts
   */
  public static readonly TITLE: string = 'title';

  /**
   * Column 'read_only' in table raw_contacts
   */
  public static readonly READ_ONLY: string = 'read_only';

  /**
   * Column 'sort_first_letter' in table raw_contacts
   */
  public static readonly SORT_FIRST_LETTER: string = 'sort_first_letter';

  /**
   * Column 'vibration_setting' in table raw_contacts
   */
  public static readonly VIBRATION_SETTING: string = 'vibration_setting';

  /**
   * Column 'sync_id' in table raw_contacts
   */
  public static readonly SYNC_ID: string = 'sync_id';

  /**
   * Column 'syn_1' in table raw_contacts
   */
  public static readonly SYN_1: string = 'syn_1';

  /**
   * Column 'syn_2' in table raw_contacts
   */
  public static readonly SYN_2: string = 'syn_2';

  /**
   * Column 'syn_3' in table raw_contacts
   */
  public static readonly SYN_3: string = 'syn_3';

  /**
   * Column 'primary_contact' in table raw_contacts
   */
  public static readonly PRIMARY_CONTACT: string = 'primary_contact';

  /**
   * Column 'extra1' in table raw_contacts
   */
  public static readonly EXTRA1: string = 'extra1';

  /**
   * Column 'extra2' in table raw_contacts
   */
  public static readonly EXTRA2: string = 'extra2';

  /**
   * Column 'extra3' in table raw_contacts
   */
  public static readonly EXTRA3: string = 'extra3';

  /**
   * Column 'primary_contact' in table raw_contacts
   */
  public static readonly EXTRA4: string = 'extra4';

  /**
   * Column 'send_to_voicemail' in table raw_contacts
   */
  public static readonly SEND_TO_VOICEMAIL: string = 'send_to_voicemail';

  /**
   * Column 'custom_ringtone' in table raw_contacts
   */
  public static readonly CUSTOM_RINGTONE: string = 'custom_ringtone';

  /**
   * Column 'deleted' in table raw_contacts
   */
  public static readonly DELETED: string = 'deleted';

  /**
   * Column 'notification_tone' in table raw_contacts
   */
  public static readonly NOTIFICATION_TONE: string = 'notification_tone';

  /**
   * Column 'x_times_contacted' in table raw_contacts
   */
  public static readonly TIMES_CONTACTED: string = 'x_times_contacted';

  /**
   * Column 'x_last_time_contacted' in table raw_contacts
   */
  public static readonly LAST_TIME_CONTACTED: string = 'x_last_time_contacted';

  /**
   * Column 'starred' in table raw_contacts
   */
  public static readonly STARRED: string = 'starred';

  /**
   * Column 'starred_order' in table raw_contacts
   */
  public static readonly STARRED_ORDER: string = 'starred_order';

  /**
   * Column 'phonetic_name_style' in table raw_contacts
   */
  public static readonly PHONETIC_NAME_STYLE: string = 'phonetic_name_style';

  /**
   * Column 'raw_contact_is_read_only' in table raw_contacts
   */
  public static readonly RAW_CONTACT_IS_READ_ONLY: string = 'raw_contact_is_read_only';

  /**
   * Column 'phonebook_label' in table raw_contacts
   */
  public static readonly PHONEBOOK_LABEL: string = 'phonebook_label';

  /**
   * Column 'vibration_type' in table raw_contacts
   */
  public static readonly VIBRATION_TYPE: string = 'vibration_type';

  /**
   * Column 'aggregation_mode' in table raw_contacts
   */
  public static readonly AGGREGATION_MODE: string = 'aggregation_mode';

  /**
   *  Column 'merge_mode' in table raw_contacts
   */
  public static readonly MERGE_MODE: string = 'merge_mode';

  /**
   *  Column 'is_need_merge' in table raw_contacts
   */
  public static readonly IS_NEED_MERGE: string = 'is_need_merge';

  /**
   *  Column 'merge_status' in table raw_contacts
   */
  public static readonly MERGE_STATUS: string = 'merge_status';

  /**
   *  Column 'is_merge_target' in table raw_contacts
   */
  public static readonly IS_MERGE_TARGET: string = 'is_merge_target';

    /**
   * Column 'account_type' in table raw_contacts
   */
  public static readonly ACCOUNT_TYPE: string = 'account_type';

   /**
   * Column 'account_name' in table raw_contacts
   */
  public static readonly ACCOUNT_NAME: string = 'account_name';

    /**
   * Column 'is_private' in table raw_contacts
   */
  public static readonly IS_PRIVATE: string = 'is_private';
  
  /**
   * Column 'dirty' in table raw_contacts
   */
  public static readonly DIRTY: string = 'dirty';

  /**
   * Column 'sync1' in table raw_contacts
   */
  public static readonly SYNC1: string = 'sync1';

  /**
   * Column 'sync3' in table raw_contacts
   */
  public static readonly SYNC3: string = 'sync3';

  /**
   * Column 'uuid' in table raw_contacts
   */
  public static readonly UUID: string = 'uuid';

  public static readonly UNIQUE_KEY: string = 'unique_key';

  public static readonly AGGREGATION_STATUS: string = 'aggregation_status';
}