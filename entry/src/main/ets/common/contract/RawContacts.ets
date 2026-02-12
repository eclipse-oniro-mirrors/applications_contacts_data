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

import RawContactsColumns from './RawContactsColumns';

/**
 * RawContact data type
 */
export class RawContacts extends RawContactsColumns {
  /**
   * RawContacts table uri --- contacts.db
   */
  public static readonly CONTENT_URI_CONTACTS: string = 'datashare:///com.ohos.contactsdataability/contacts/raw_contact';
  /**
   * RawContacts table uri --- profile.db
   */
  public static readonly CONTENT_URI_PROFILE: string = 'datashare:///com.ohos.contactsdataability/profile/raw_contact';

  constructor() {
    super()
  }
}