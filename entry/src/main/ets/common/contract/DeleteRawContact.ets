/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 * To get table groups columns
 */
export class DeleteRawContact extends BaseColumns {
  /**
   * Column 'raw_contact_id' in table delete_raw_contact
   */
  public static readonly RAW_CONTACT_ID: string = 'raw_contact_id';
  /**
   * Column 'contact_id' in table delete_raw_contact
   */
  public static readonly CONTACT_ID: string = 'contact_id';
  /**
   * Column 'delete_source' in table delete_raw_contact
   */
  public static readonly DELETE_SOURCE: string = 'delete_source';
  /**
   * Column 'delete_time' in table delete_raw_contact
   */
  public static readonly DELETE_TIME: string = 'delete_time';
  /**
   * Column 'display_name' in table delete_raw_contact
   */
  public static readonly DISPLAY_NAME: string = 'display_name';

  /**
   * Column 'delete_package' in table deleted_raw_contacts
   */
  public static readonly DELETE_PACKAGE: string = 'delete_package';
   
  /**
   * Column 'backup' in table deleted_raw_contacts
   */
  public static readonly DELETE_BACKUP: string = 'backup';
 
  /**
   * Column 'backup_data' in table delete_raw_contact
   */
  public static readonly DELETE_BACKUP_DATA: string = 'backup_data';
 
  /**
   * Column 'deleted' in table deleted_raw_contacts
   */
  public static readonly DELETED: string = 'deleted';

  /**
   * Column 'is_deleted' in table delete_raw_contact
   */
  public static readonly IS_DELETED: string = 'is_deleted';
  
  /**
   * Column 'sync1' in table delete_raw_contact
   */
  public static readonly SYNC1: string = 'sync1';

  /**
   * Column 'sync3' in table delete_raw_contact
   */
  public static readonly SYNC3: string = 'sync3';

  /**
   * Column 'dirty' in table delete_raw_contact
   */
  public static readonly DIRTY: string = 'dirty';

  /**
   * Column 'syn_1' in table delete_raw_contact
   */
  public static readonly SYN_1: string = 'syn_1';

  /**
   * Column 'uuid' in table delete_raw_contact
   */
  public static readonly UUID: string = 'uuid';

  /**
   * Column 'backup_unstruct' in table delete_raw_contact
   */
  public static readonly BACKUP_UNSTRUCT: string = 'backup_unstruct';
}
