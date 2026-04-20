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
import DataColumns from './DataColumns';
import rdb from '@ohos.data.relationalStore';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import { Contact } from '../../components/PlatformInterface/Contact';
import FullContactData from './FullContactData';
// 正则表达式匹配只包含数字、空格、加号、短横杠的字符串
const phoneRegex = /^[\d\s+-]+$/;
const spaceOrDashReg = /[\s-]/g;
export default class SimpleContactData {
  public detailInfo: string = '';
  public formatPhoneNumber: string = '';
  public labelId: number = 0;
  public labelName: string = '';
  public typeId: number = 0;
  public position: string = '';
  static columns: string [] = [DataColumns.DETAIL_INFO, DataColumns.FORMAT_PHONE_NUMBER,
    DataColumns.CUSTOM_DATA, DataColumns.EXTEND7, DataColumns.TYPE_ID, DataColumns.POSITION];

  // 注意：所有索引要与上面 columns 一一对应
  static fromResultSet(resultSet: rdb.ResultSet) {
    const simpleContactData: SimpleContactData = new SimpleContactData();
    simpleContactData.detailInfo = resultSet.getString(0);
    simpleContactData.formatPhoneNumber = resultSet.getString(1);
    simpleContactData.labelId = resultSet.getLong(2);
    simpleContactData.labelName = resultSet.getString(3);
    simpleContactData.typeId = resultSet.getLong(4);
    simpleContactData.position = resultSet.getString(5);
    return simpleContactData;
  }

  static fromDisplayName(displayName: string, removePhoneSpace = false) {
    // 双的 display name可能包含空格
    if (removePhoneSpace && phoneRegex.test(displayName)) {
      displayName = displayName.replace(spaceOrDashReg, '');
    }
    const simpleContactData: SimpleContactData = new SimpleContactData();
    simpleContactData.typeId = 6;
    simpleContactData.detailInfo = displayName;
    return simpleContactData;
  }

  static fromPhone(phone: Contact.PhoneNumber) {
    const simpleContactData: SimpleContactData = new SimpleContactData();
    simpleContactData.typeId = 5;
    simpleContactData.detailInfo = phone.phoneNumber!;
    simpleContactData.labelId = phone.labelId!;
    simpleContactData.labelName = phone.labelName!;
    return simpleContactData;
  }

  static fromFullContactData(fullContactData: FullContactData) {
    const simpleContactData: SimpleContactData = new SimpleContactData();
    simpleContactData.typeId = fullContactData.type_id;
    simpleContactData.detailInfo = fullContactData.detail_info;
    simpleContactData.labelId = fullContactData.custom_data;
    simpleContactData.labelName = fullContactData.extend7;
    simpleContactData.formatPhoneNumber = fullContactData.formatPhoneNumber;
    return simpleContactData;
  }

  toValuesBucket() {
    const valuesBucket: ValuesBucket = {};
    valuesBucket[DataColumns.DETAIL_INFO] = this.detailInfo;
    valuesBucket[DataColumns.FORMAT_PHONE_NUMBER] = this.formatPhoneNumber;
    valuesBucket[DataColumns.CUSTOM_DATA] = this.labelId;
    valuesBucket[DataColumns.EXTEND7] = this.labelName;
    valuesBucket[DataColumns.TYPE_ID] = this.typeId;
    return valuesBucket;
  }
}
