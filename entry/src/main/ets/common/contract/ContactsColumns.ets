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
 * Field constants in the contact table
 */
export default class ContactsColumns extends BaseColumns {
  /**
   * Column 'name_raw_contact_id' in table contacts
   */
  public static readonly NAME_RAW_CONTACT_ID: string = 'name_raw_contact_id';

  /**
   * Column 'photo_id' in table contacts
   */
  public static readonly PHOTO_ID: string = 'photo_id';

  /**
   * Column 'photo_file_id' in table contacts
   */
  public static readonly PHOTO_FILE_ID: string = 'photo_file_id';

  /**
   * Column 'personal_ringtone' in table contacts
   */
  public static readonly PERSONAL_RINGTONE: string = 'personal_ringtone';

  /**
   * Column 'is_transfer_voicemail' in table contacts
   */
  public static readonly IS_TRANSFER_VOICEMAIL: string = 'is_transfer_voicemail';

  /**
   * Column 'company' in table contacts
   */
  public static readonly COMPANY: string = 'company';

  /**
   * Column 'position' in table contacts
   */
  public static readonly POSITION: string = 'position';

  /**
   * Column 'quick_search_key' in table contacts
   */
  public static readonly QUICK_SEARCH_KEY: string = 'quick_search_key';

  /**
   * Column 'read_only' in table contacts
   */
  public static readonly READ_ONLY: string = 'read_only';

  /**
   * Column 'personal_notification_ringtone' in table contacts
   */
  public static readonly PERSONAL_NOTIFICATION_RINGTONE: string = 'personal_notification_ringtone';

  /**
   * Column 'has_phone_number' in table contacts
   */
  public static readonly HAS_PHONE_NUMBER: string = 'has_phone_number';

  /**
   * Column 'has_display_name' in table contacts
   */
  public static readonly HAS_DISPLAY_NAME: string = 'has_display_name';

  /**
   * Column 'has_email' in table contacts
   */
  public static readonly HAS_EMAIL: string = 'has_email';

  /**
   * Column 'has_group' in table contacts
   */
  public static readonly HAS_GROUP: string = 'has_group';

  /**
   * Column 'contact_last_updated_timestamp' in table contacts
   */
  public static readonly CONTACT_LAST_UPDATED_TIMESTAMP: string = 'contact_last_updated_timestamp';

  /**
   * Column 'custom_ringtone' in table contacts
   */
  public static readonly CUSTOM_RINGTONE: string = 'custom_ringtone';

  public static readonly RINGTONE_PATH: string = 'ringtone_path';

  /**
   * Column 'send_to_voicemail' in table contacts
   */
  public static readonly SEND_TO_VOICEMAIL: string = 'send_to_voicemail';

  /**
   * Column 'title' in table contacts
   */
  public static readonly TITLE: string = 'title';

  /**
   * Column 'has_name' in table contacts
   */
  public static readonly HAS_NAME: string = 'has_name';

  public static readonly FORM_ID: string = 'form_id';

  public static readonly FOCUS_MODE_LIST: string = 'focus_mode_list';

  public static readonly PREFER_AVATAR: string = 'prefer_avatar';
}