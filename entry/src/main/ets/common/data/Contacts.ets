/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export class Contacts {
  public static readonly ID: string = 'id';
  public static readonly NAME_RAW_CONTACT_ID: string = 'name_raw_contact_id';
  public static readonly PHOTO_ID: string = 'photo_id';
  public static readonly PHOTO_FILE_ID: string = 'photo_file_id';
  public static readonly PERSONAL_RINGTONE: string = 'personal_ringtone';
  public static readonly IS_TRANSFER_VOICEMAIL: string = 'is_transfer_voicemail';
  public static readonly COMPANY: string = 'company';
  public static readonly POSITION: string = 'position';
  public static readonly QUICK_SEARCH_KEY: string = 'quick_search_key';
  public static readonly READ_ONLY: string = 'read_only';
  public static readonly PERSONAL_NOTIFICATION_RINGTONE: string = 'personal_notification_ringtone';
  public static readonly HAS_PHONE_NUMBER: string = 'has_phone_number';
  public static readonly HAS_DISPLAY_NAME: string = 'has_display_name';
  public static readonly HAS_EMAIL: string = 'has_email';
  public static readonly HAS_GROUP: string = 'has_group';
  public static readonly CONTACT_LAST_UPDATED_TIMESTAMP: string = 'contact_last_updated_timestamp';
  public static readonly CONTENT_URI: string = 'datashare:///com.ohos.contactsdataability';
  public static readonly CONTACT_URI: string = Contacts.CONTENT_URI + '/contacts/contact';
  public static readonly BACKUP_URI: string = Contacts.CONTENT_URI + '/contacts/backup';
  public static readonly SETTING_PROXY_URI: string = 'datashareproxy://com.ohos.contactsdataability/contacts/setting';
  public static readonly CALL_SETTING_PROXY_URI: string = Contacts.CONTENT_URI + '/calls/call_setting';
  public static readonly CALLLOG_URI: string = 'datashare:///com.ohos.calllogability';
}