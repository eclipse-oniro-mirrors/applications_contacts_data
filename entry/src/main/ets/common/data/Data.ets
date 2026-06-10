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

import { Contacts } from './Contacts';

export class Data {
  public static readonly CLOUD_SYNC_CONTACT_DELETE: string = Contacts.CONTENT_URI + '/contacts/cloud_sync_contact_delete';
  public static readonly CONTENT_URI: string = Contacts.CONTENT_URI + '/contacts/contact_data';
  public static readonly CLOUD_URI: string = Contacts.CONTENT_URI + '/contacts/cloud';
  public static readonly TYPE_URI: string = Contacts.CONTENT_URI + '/contacts/contact_type';
  public static readonly RAW_URI: string = Contacts.CONTENT_URI + '/contacts/raw_contact';
  public static readonly CONTACT: string = Contacts.CONTENT_URI + '/contacts/contact';
  public static readonly SYNC_URI: string = Contacts.CONTENT_URI + '/contacts/query_uuid_not_in_raw_contact';
  public static readonly SETTING_URI: string = Contacts.CONTENT_URI + '/contacts/setting';
  public static readonly CONTACT_SYNC_URI: string = Contacts.CONTENT_URI + '/contacts/upload_data_to_cloud';
  public static readonly TRIGGER_UPLOAD_CLOUD_URI: string = Contacts.CONTENT_URI + '/contacts/trigger_upload_cloud';
  public static readonly BIG_NAME_URI: string = Contacts.CONTENT_URI + '/contacts/query_big_length_name';
  public static readonly DELETE_URI = Contacts.CONTENT_URI + '/contacts/deleted_raw_contact';
  public static readonly DEEP_SYNC_URI = Contacts.CONTENT_URI + '/contacts/sync_contact_data';
  public static readonly CLOUD_GROUP_URI = Contacts.CONTENT_URI + '/contacts/cloud_groups';
  public static readonly FULL_SEARCH_SWITCH_OPEN_URI: string = Contacts.CONTENT_URI + '/contacts/set_full_search_switch_open';
  public static readonly FULL_SEARCH_SWITCH_CLOSE_URI: string = Contacts.CONTENT_URI + '/contacts/set_full_search_switch_close';
  public static readonly CLEAR_AND_RECREATE_TRIGGER_SEARCH_TABLE: string = Contacts.CONTENT_URI +
    '/contacts/clear_and_recreate_trigger_search_table';
  public static readonly SPLIT_AGGREGATION_CONTACT: string = Contacts.CONTENT_URI + '/contacts/split_aggregation_contact';
  public static readonly CALLLOG_URI: string = Contacts.CALLLOG_URI + '/calls/calllog';
  public static readonly UPDATE_YELLOW_PAGE_INFO_FOR_CALLLOG: string =
    Contacts.CONTENT_URI + '/contacts/update_yellow_page_info_for_callLog';
}