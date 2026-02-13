/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

import { ValuesBucket } from '@ohos.data.ValuesBucket';
import rdb from '@ohos.data.relationalStore';

import { Data } from '../../contract/Data';
import { StringUtil } from '../../../common/utils/StringUtil';
import DataItem from './DataItem';
import SimpleContactData from '../../../common/data/SimpleContactData';
import { Contact } from '../../../components/PlatformInterface/Contact';

/**
 * PhoneDataItem bean to batch insert table data
 */
export default class PhoneDataItem extends DataItem {
  private mCustomData: string = '';
  private mIsPreferredNumber: number = 0;
  private mFormatNumber: string = '';
  private mExtend7: string = '';

  /**
   * Create phone data obj
   *
   * @param rawContactId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param detailInfo the content detail info for data item to insert table data
   */
  constructor(rawContactId: number, type: string, detailInfo: string) {
    super();
    this.mRawContactId = rawContactId;
    this.mContentType = type;
    this.mDetailInfo = detailInfo;
  }

  /**
   * Set custom_data in data item for batch insert
   *
   * @param customData phone type is phone or custom.
   * @returns PhoneDataItem obj
   */
  public setCustomData(customData: string): PhoneDataItem {
    this.mCustomData = customData;
    return this;
  }

  /**
   * Set format_phone_number in data item for batch insert
   *
   * @param format_phone_number to insert the data table
   * @returns PhoneDataItem obj
   */
  public setFormatNumber(formatNumber: string): PhoneDataItem {
    this.mFormatNumber = formatNumber;
    return this;
  }

  /**
   * Set is_preferred_number in data item for batch insert
   *
   * @param is_preferred_number phone number is primary.
   * @returns PhoneDataItem obj
   */
  public setPreferredNumber(isPreferredNumber: number): PhoneDataItem {
    this.mIsPreferredNumber = isPreferredNumber;
    return this;
  }

  public setExtend7(extend7: string): PhoneDataItem {
    this.mExtend7 = extend7;
    return this;
  }

  /**
   * Build phone info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param resultSet which to build
   * @param detailInfo the content detail info for data item to insert table data
   * @returns ValuesBucket to insert table data
   */
  public static buildPhoneData(newRawId: number, type: string, resultSet: rdb.ResultSet,
                               detailInfo: string): ValuesBucket {
    let labelId = resultSet.getString(resultSet.getColumnIndex(Data.DATA2));
    let targetId: string = labelId;
    if (labelId === '1') {
      targetId = '2';
    } else if (labelId === '2') {
      targetId = '1';
    }
    detailInfo = StringUtil.removeSpaceNew(detailInfo);
    let item: PhoneDataItem = new PhoneDataItem(newRawId, type, detailInfo);
    item.setCustomData(StringUtil.isEmpty(targetId) ? '7' : targetId)
      .setPreferredNumber(resultSet.getLong(resultSet.getColumnIndex(Data.IS_PRIMARY)))
      .setFormatNumber(resultSet.getString(resultSet.getColumnIndex(Data.DATA4)));
    if (labelId === '0') {
      item.setCustomData('10000');
      item.setExtend7(resultSet.getString(resultSet.getColumnIndex(Data.DATA3)));
    }
    return item.createValuesBucket();
  }

  public static buildPhoneSimpleData(resultSet: rdb.ResultSet): SimpleContactData {
    let labelId = resultSet.getString(resultSet.getColumnIndex(Data.DATA2));
    let targetId: string = labelId;
    if (labelId === '1') {
      targetId = '2';
    } else if (labelId === '2') {
      targetId = '1';
    }
    const phone = new Contact.PhoneNumber();
    phone.phoneNumber = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
    phone.labelId = targetId === '0' ? 10000 : (Number.parseInt(targetId) || 7);
    phone.labelName = resultSet.getString(resultSet.getColumnIndex(Data.DATA3));
    phone.formatPhoneNumber = resultSet.getString(resultSet.getColumnIndex(Data.DATA4));
    return SimpleContactData.fromPhone(phone);
  }


  /**
   * Create phone ValuesBucket object
   *
   * @returns ValuesBucket with phone data
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = super.createValuesBucket();
    values[Data.CUSTOM_DATA] = this.mCustomData ? this.mCustomData.toString() : null;
    values[Data.IS_PREFERRED_NUMBER] = this.mIsPreferredNumber;
    values[Data.FORMAT_PHONE_NUMBER] = this.mFormatNumber;
    values[Data.EXTEND5] = this.mIsPreferredNumber.toString();
    values[Data.EXTEND7] = this.mExtend7;
    return values;
  }
}