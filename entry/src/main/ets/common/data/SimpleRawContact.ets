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
import RawContactsColumns from './RawContactsColumns';
import rdb from '@ohos.data.relationalStore';

export default class SimpleRawContact {
  public rawId: number = -1;
  public uuid: string = '';
  public contactId: number = -1;
  public isDeleted: number = 0;
  public dirty: number = 0;
  public static columns: string [] = ['id', RawContactsColumns.UUID, RawContactsColumns.CONTACT_ID,
    RawContactsColumns.IS_DELETED, RawContactsColumns.DIRTY];

  // 注意：所有索引要与上面 columns 一一对应
  constructor() {
  }

  static fromRuleSet(resultSet: rdb.ResultSet): SimpleRawContact {
    const simpleRawContact: SimpleRawContact = new SimpleRawContact();
    let index = 0;
    simpleRawContact.rawId = resultSet.getLong(index++);
    simpleRawContact.uuid = resultSet.getString(index++);
    simpleRawContact.contactId = resultSet.getLong(index++);
    simpleRawContact.isDeleted = resultSet.getLong(index++);
    simpleRawContact.dirty = resultSet.getLong(index++);
    return simpleRawContact;
  }
}
