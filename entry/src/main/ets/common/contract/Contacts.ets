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
 * WITHOUT WARRANTIES OR CONDITIONS OF KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import ContactsColumns from './ContactsColumns';

/**
 * To get table contacts data
 */
export class Contacts extends ContactsColumns {
  /**
   * Contact database uri
   */
  public static readonly CONTENT_URI: string = 'datashare:///com.ohos.contactsdataability';
  /**
   * Contacts table uri
   */
  public static readonly CONTACT_URI: string = Contacts.CONTENT_URI + '/contacts/contact';

  constructor() {
    super();
  }
}