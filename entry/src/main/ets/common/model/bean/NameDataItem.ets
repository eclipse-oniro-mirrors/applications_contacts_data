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
import DataItem from './DataItem';
import { StringUtil } from '../../../common/utils/StringUtil';
import LogUtils from '../../../common/utils/LogUtils';
import DbConstants from '../../DbConstants';

const TAG = 'NameDataItem';

/**
 * NameDataItem bean to batch insert table data
 */
export default class NameDataItem extends DataItem {
  private mGivenName: string = '';

  private mFamilyName: string = '';

  private mAlphaName: string = '';

  private mOtherLanLastName: string = '';

  private mOtherLanFirstName: string = '';

  private mGivenNamePhonetic: string = '';

  private mMiddleNamePhonetic: string = '';

  private mPhoneticName: string = '';

  private mLanStyle: string = '';

  /**
   * Create name data obj
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
   * Set given_name in data item for batch insert
   *
   * @param givenName which is contained the name info
   */
  public setGivenName(givenName: string) {
    this.mGivenName = givenName;
    return this;
  }

  /**
   * Set family_name in data item for batch insert
   *
   * @param familyName which is contained the name info
   */
  public setFamilyName(familyName: string) {
    this.mFamilyName = familyName;
    return this;
  }

  /**
   * Set alpha_name in data item for batch insert
   *
   * @param alphaName which is contained the name info
   */
  public setAlphaName(alphaName: string) {
    this.mAlphaName = alphaName;
    return this;
  }

  /**
   * Set other_lan_last_name in data item for batch insert
   *
   * @param otherLanLastName which is contained the name info
   */
  public setOtherLanLastName(otherLanLastName: string) {
    this.mOtherLanLastName = otherLanLastName;
    return this;
  }

  /**
   * Set other_lan_first_name in data item for batch insert
   *
   * @param otherLanFirstName which is contained the name info
   */
  public setOtherLanFirstName(otherLanFirstName: string) {
    this.mOtherLanFirstName = otherLanFirstName;
    return this;
  }

  /**
   * Set given_name_phonetic in data item for batch insert
   *
   * @param givenNamePhonetic which is contained the name info
   */
  public setGivenNamePhonetic(givenNamePhonetic: string) {
    this.mGivenNamePhonetic = givenNamePhonetic;
    return this;
  }

  /**
   * Set middle_name_phonetic in data item for batch insert
   *
   * @param middleNamePhonetic which is contained the name info
   */
  public setMiddleNamePhonetic(middleNamePhonetic: string) {
    this.mMiddleNamePhonetic = middleNamePhonetic;
    return this;
  }

  /**
   * Set phonetic_name in data item for batch insert
   *
   * @param phoneticName which is contained the name info
   */
  public setPhoneticName(phoneticName: string) {
    this.mPhoneticName = phoneticName;
    return this;
  }

  /**
   * Set lan_style in data item for batch insert
   *
   * @param lanStyle which is contained the name info
   */
  public setLanStyle(lanStyle: string) {
    this.mLanStyle = lanStyle;
    return this;
  }

  /**
   * Build name info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param resultSet which to build
   * @param detailInfo the content detail info for data item to insert table data
   * @returns ValuesBucket to insert table data
   */
  public static buildNameData(newRawId: number, type: string, resultSet: rdb.ResultSet,
    detailInfo: string): ValuesBucket {
    let item: NameDataItem = new NameDataItem(newRawId, type, detailInfo);
    return item.setGivenName(resultSet.getString(resultSet.getColumnIndex(Data.DATA2)))
      .setFamilyName(resultSet.getString(resultSet.getColumnIndex(Data.DATA3)))
      .setAlphaName(resultSet.getString(resultSet.getColumnIndex(Data.DATA4)))
      .setOtherLanLastName(resultSet.getString(resultSet.getColumnIndex(Data.DATA5)))
      .setOtherLanFirstName(resultSet.getString(resultSet.getColumnIndex(Data.DATA6)))
      .setGivenNamePhonetic(resultSet.getString(resultSet.getColumnIndex(Data.DATA7)))
      .setMiddleNamePhonetic(resultSet.getString(resultSet.getColumnIndex(Data.DATA8)))
      .setPhoneticName(resultSet.getString(resultSet.getColumnIndex(Data.DATA9)))
      .setLanStyle(resultSet.getString(resultSet.getColumnIndex(Data.DATA10)))
      .createValuesBucket();
  }

  public static getDisplayName(displayName: string, resultSetData: rdb.ResultSet): string {
    let data2 = resultSetData.getString(resultSetData.getColumnIndex(Data.DATA2));
    let data3 = resultSetData.getString(resultSetData.getColumnIndex(Data.DATA3));
    let data4 = resultSetData.getString(resultSetData.getColumnIndex(Data.DATA4));
    let data5 = resultSetData.getString(resultSetData.getColumnIndex(Data.DATA5));
    let data6 = resultSetData.getString(resultSetData.getColumnIndex(Data.DATA6));
    let data10 = resultSetData.getString(resultSetData.getColumnIndex(Data.DATA10));
    if (!StringUtil.isEmpty(data2) &&
      !StringUtil.isEmpty(data3) && !StringUtil.isEmpty(data5) && !StringUtil.isEmpty(data10) &&
      data10 == DbConstants.LAN_STYLE_CJK) {
      displayName = data3 + data5 + data2;
      LogUtils.i(TAG, `getDisplayName has data5 displayName: ${StringUtil.maskSensitiveInfo(displayName)}`);
    } else if (!StringUtil.isEmpty(data2) &&
      !StringUtil.isEmpty(data3) && !StringUtil.isEmpty(data10) && data10 == DbConstants.LAN_STYLE_CJK) {
      displayName = data3 + data2;
      LogUtils.i(TAG, `getDisplayName displayName: ${StringUtil.maskSensitiveInfo(displayName)}`);
    }
    //处理前缀名
    if (!StringUtil.isEmpty(data4)) {
      displayName = data4 + ' ' + displayName;
    }
    //处理后缀名
    if (!StringUtil.isEmpty(data6)) {
      displayName = displayName + ' ' + data6;
    }
    return displayName;
  }

  /**
   * Create name ValuesBucket object
   *
   * @returns ValuesBucket with name data
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = super.createValuesBucket();
    values[Data.GIVEN_NAME] = this.mGivenName;
    values[Data.FAMILY_NAME] = this.mFamilyName;
    values[Data.ALPHA_NAME] = this.mAlphaName;
    values[Data.OTHER_LAN_LAST_NAME] = this.mOtherLanLastName;
    values[Data.OTHER_LAN_FIRST_NAME] = this.mOtherLanFirstName;
    values[Data.GIVEN_NAME_PHONETIC] = this.mGivenNamePhonetic;
    values[Data.MIDDLE_NAME_PHONETIC] = this.mMiddleNamePhonetic;
    values[Data.PHONETIC_NAME] = this.mPhoneticName;
    values[Data.LAN_STYLE] = this.mLanStyle;
    return values;
  }
}