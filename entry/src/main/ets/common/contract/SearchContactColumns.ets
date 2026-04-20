/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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
export default class SearchContactColumns extends BaseColumns {
  /**
   * Column 'contact_id' in table search_contact
   */
  public static readonly CONTACT_ID: string = 'contact_id';

  /**
   * Column 'account_id' in table search_contact
   */
  public static readonly ACCOUNT_ID: string = 'account_id';

  /**
   * Column 'raw_contact_id' in table search_contact
   */
  public static readonly RAW_CONTACT_ID: string = 'raw_contact_id';

  /**
   * Column 'search_name' in table search_contact
   */
  public static readonly SEARCH_NAME: string = 'search_name';

  /**
   * Column 'name' in table search_contact
   */
  public static readonly NAME: string = 'name';

  /**
   * Column 'display_name' in table search_contact
   */
  public static readonly DISPLAY_NAME: string = 'display_name';

  /**
   * Column 'phonetic_name' in table search_contact
   */
  public static readonly PHONETIC_NAME: string = 'phonetic_name';

  /**
   * Column 'photo_id' in table search_contact
   */
  public static readonly PHOTO_ID: string = 'photo_id';

  /**
   * Column 'photo_file_id' in table search_contact
   */
  public static readonly PHOTO_FILE_ID: string = 'photo_file_id';
}