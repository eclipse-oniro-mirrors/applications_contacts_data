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

/**
 * ImDataItem bean to batch insert table data
 */
export default class ImDataItem extends DataItem {
  private mCustomData: string = '';
  private mExtend7: string = '';

  /**
   * Create im data obj
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
   * @param customData the im label for detail info.
   * @returns ImDataItem obj
   */
  public setCustomData(customData: string): ImDataItem {
    this.mCustomData = customData;
    return this;
  }

  public setExtend7(extend7: string): ImDataItem {
    this.mExtend7 = extend7;
    return this;
  }

  /**
   * Build im info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param resultSet which to build
   * @param detailInfo the content detail info for data item to insert table data
   * @returns ValuesBucket to insert table data
   */
  public static buildImData(newRawId: number, type: string, resultSet: rdb.ResultSet,
                            detailInfo: string): ValuesBucket {
    let item: ImDataItem = new ImDataItem(newRawId, type, detailInfo);
    let data5: string = resultSet.getString(resultSet.getColumnIndex(Data.DATA5));
    item.setCustomData(ImDataItem.getCustomData(data5));
    if (data5 == '-1') {
      item.setCustomData('10000');
      item.setExtend7(resultSet.getString(resultSet.getColumnIndex(Data.DATA6)));
    }
    return item.createValuesBucket();
  }

  private static getCustomData(dbCustomData: string): string {
    let customDataNumber = Number.parseInt(dbCustomData);
    if (isNaN(customDataNumber) || customDataNumber > 7 || customDataNumber < -1) {
      return '0';
    } else {
      return (customDataNumber + 1).toString();
    }
  }

  /**
   * Create im ValuesBucket object
   *
   * @returns ValuesBucket with im data
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = super.createValuesBucket();
    values[Data.CUSTOM_DATA] = this.mCustomData;
    values[Data.EXTEND7] = this.mExtend7;
    return values;
  }
}