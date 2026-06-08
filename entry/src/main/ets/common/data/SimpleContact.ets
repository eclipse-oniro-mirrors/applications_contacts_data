/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
import rdb from '@ohos.data.relationalStore';
import ContactsColumns from '../contract/ContactsColumns';

export default class SimpleContact {
  public id: number = -1;
  public nameRawId: number = -1;
  public formId: string = '';
  public focusModeList: string = '';
  public personalRingtone: string = '';
  public personalNotificationRingtone: string = '';
  public ringtonePath: string = '';

  public static columns: string[] = [ContactsColumns.NEW_ID, ContactsColumns.NAME_RAW_CONTACT_ID,
  ContactsColumns.FORM_ID, ContactsColumns.FOCUS_MODE_LIST, ContactsColumns.PERSONAL_RINGTONE,
  ContactsColumns.PERSONAL_NOTIFICATION_RINGTONE, ContactsColumns.RINGTONE_PATH];

  // 注意：所有索引要与上面 columns 一一对应
  constructor() {
  }

  static fromResultSet(resultSet: rdb.ResultSet): SimpleContact {
    const simpleContact: SimpleContact = new SimpleContact();
    let index = 0;
    simpleContact.id = resultSet.getLong(index++);
    simpleContact.nameRawId = resultSet.getLong(index++);
    simpleContact.formId = resultSet.getString(index++);
    simpleContact.focusModeList = resultSet.getString(index++);
    simpleContact.personalRingtone = resultSet.getString(index++);
    simpleContact.personalNotificationRingtone = resultSet.getString(index++);
    simpleContact.ringtonePath = resultSet.getString(index++);
    return simpleContact;
  }
}
