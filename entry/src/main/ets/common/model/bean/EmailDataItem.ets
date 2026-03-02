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

/**
 * EmailDataItem bean to batch insert table data
 */
export default class EmailDataItem extends DataItem {
  private mCustomData: string = '';
  private mExtend7: string = '';

  /**
   * Create email data obj
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
   * @param customData the email label for detail info.
   * @returns EmailDataItem obj
   */
  public setCustomData(customData: string): EmailDataItem {
    this.mCustomData = customData;
    return this;
  }

  public setExtend7(extend7: string): EmailDataItem {
    this.mExtend7 = extend7;
    return this;
  }

  /**
   * Build email info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param resultSet which to build
   * @param detailInfo the content detail info for data item to insert table data
   * @returns ValuesBucket to insert table data
   */
  public static buildEmailData(newRawId: number, type: string, resultSet: rdb.ResultSet,
                               detailInfo: string): ValuesBucket {
    let item: EmailDataItem = new EmailDataItem(newRawId, type, detailInfo);
    let emailCustomData: string = resultSet.getString(resultSet.getColumnIndex(Data.DATA2));
    item.setCustomData(StringUtil.isEmpty(emailCustomData) ? '0' : emailCustomData);
    if (emailCustomData == '0') {
      let label = resultSet.getString(resultSet.getColumnIndex(Data.DATA3));
      item.setCustomData(StringUtil.isEmpty(emailCustomData) ? '3' : '10000');
      item.setExtend7(resultSet.getString(resultSet.getColumnIndex(Data.DATA3)));
    }
    return item.createValuesBucket();
  }

  /**
   * Create email ValuesBucket object
   *
   * @returns ValuesBucket with email data
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = super.createValuesBucket();
    values[Data.CUSTOM_DATA] = this.mCustomData;
    values[Data.EXTEND7] = this.mExtend7;
    return values;
  }
}