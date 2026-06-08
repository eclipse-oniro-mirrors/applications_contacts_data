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

import DataShareResultSet from '@ohos.data.DataShareResultSet';
import RawContactsColumns from '../RawContactsColumns';
import MeeTimeAbilityInfoModel from './MeeTimeAbilityInfoModel';
import rdb from '@ohos.data.rdb';
import DataItem from '../../model/bean/DataItem';

type ValueType = number | string | boolean;

export default class RawContact {
  public readonly id: number;
  public readonly values: Map<string, ValueType>;
  public readonly dataItems: DataItem[];
  public meeTimeAbilityInfoModel?: MeeTimeAbilityInfoModel;

  constructor(id: number, values: Map<string, ValueType>) {
    this.id = id; // RAW_CONTACT_ID
    this.values = values;
    this.dataItems = [];
  }

  static fromResultSet(resultSet: DataShareResultSet | rdb.ResultSet): RawContact {
    let contentValues: Map<string, ValueType> = new Map();
    contentValues.set(RawContactsColumns.CONTACT_ID,
      resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.CONTACT_ID)));
    contentValues.set(RawContactsColumns.PHOTO_ID,
      resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.PHOTO_ID)));
    contentValues.set(RawContactsColumns.PHOTO_FILE_ID,
      resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.PHOTO_FILE_ID)));
    contentValues.set(RawContactsColumns.IS_TRANSFER_VOICEMAIL,
      resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.IS_TRANSFER_VOICEMAIL)) > 0 ? true : false);
    contentValues.set(RawContactsColumns.DISPLAY_NAME,
      resultSet.getString(resultSet.getColumnIndex(RawContactsColumns.DISPLAY_NAME)));
    contentValues.set(RawContactsColumns.COMPANY,
      resultSet.getString(resultSet.getColumnIndex(RawContactsColumns.COMPANY)));
    contentValues.set(RawContactsColumns.POSITION,
      resultSet.getString(resultSet.getColumnIndex(RawContactsColumns.POSITION)));
    contentValues.set(RawContactsColumns.READ_ONLY,
      resultSet.getString(resultSet.getColumnIndex(RawContactsColumns.READ_ONLY)));
    return new RawContact(resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.ID)), contentValues);
  }
}

