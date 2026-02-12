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

import { Contacts } from './Contacts';
import DataColumns from './DataColumns';

/**
 * To get table data columns
 */
export class Data extends DataColumns {
  /**
   * Data table uri -- contacts.db
   */
  public static readonly CONTENT_URI_CONTACTS: string = Contacts.CONTENT_URI + '/contacts/contact_data';

  /**
   * Data table uri -- profile.db
   */
  public static readonly CONTENT_URI_PROFILE: string = Contacts.CONTENT_URI + '/profile/contact_data';

  /**
   * Data table type_id
   */
  public static readonly TYPE_ID: string = 'type_id';

  /**
   * Data table content_type
   */
  public static readonly CONTENT_TYPE: string = 'content_type';

  constructor() {
    super();
  }
}