/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import BaseColumns from './BaseColumns';

/**
 * Field constants in the contact group table
 */
export default class GroupColumns extends BaseColumns {
  /**
   * Column 'group_id' in table group
   */
  public static readonly GROUP_ID: string = 'id';
  /**
   * Column 'group_name' in table group
   */
  public static readonly GROUP_NAME: string = 'group_name';
  /**
   * Column 'account_id' in table group
   */
  public static readonly ACCOUNT_ID: string = 'account_id';
  /**
   * Column 'group_notes' in table group
   */
  public static readonly GROUP_NOTES: string = 'group_notes';
  /**
   * Column 'is_deleted' in table group
   */
  public static readonly IS_DELETED: string = 'is_deleted';
  /**
   * Column 'group_ringtone' in table group
   */
  public static readonly GROUP_RINGTONE: string = 'group_ringtone';
  /**
   * Column 'ringtone_modify_time' in table group
   */
  public static readonly RINGTONE_MODIFY_TIME: string = 'ringtone_modify_time';
  /**
   * Column 'lastest_modify_time' in table group
   */
  public static readonly LASTEST_MODIFY_TIME: string = 'lastest_modify_time';
  /**
   * Column 'title' in table group
   */
  public static readonly TITLE: string = 'title';
  /**
   * Column 'notes' in table group
   */
  public static readonly NOTES: string = 'notes';
  /**
   * Column 'deleted' in table group
   */
  public static readonly DELETED: string = 'deleted';
  /**
   * Column 'group_is_read_only' in table group
   */
  public static readonly GROUP_IS_READ_ONLY: string = 'group_is_read_only';
  /**
   * Column 'sync1' in table group
   */
  public static readonly sync1: string = 'sync1';
}