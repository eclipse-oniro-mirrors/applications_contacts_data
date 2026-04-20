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

import { Contacts } from './Contacts';
import SearchContactColumns from './SearchContactColumns';

/**
 * To get table groups columns
 */
export class SearchContact extends SearchContactColumns {
  /**
   * search_contact table uri -- contacts.db
   */
  public static readonly CONTENT_URI_CONTACTS: string = Contacts.CONTENT_URI + '/contacts/search_contact';

  /**
   * search_contact table uri -- profile.db
   */
  public static readonly CONTENT_URI_PROFILE: string = Contacts.CONTENT_URI + '/profile/search_contact';

  constructor() {
    super();
  }
}