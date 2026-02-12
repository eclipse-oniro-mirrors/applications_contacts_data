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
import DbConstants from '../../DbConstants';

/**
 * ContactEventDataItem bean to batch insert table data
 */
export default class ContactEventDataItem extends DataItem {
  private mCustomData: string = '';

  /**
   * Create contact event data obj
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
   * @param customData the contact event label for detail info.
   * @returns ImDataItem obj
   */
  public setCustomData(customData: string): ContactEventDataItem {
    this.mCustomData = customData;
    return this;
  }

  /**
   * Build contact event info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param resultSet which to build
   * @param detailInfo the content detail info for data item to insert table data
   * @returns ValuesBucket to insert table data
   */
  public static buildContactEventData(newRawId: number, type: string, resultSet: rdb.ResultSet,
    detailInfo: string): ValuesBucket {
    let item: ContactEventDataItem = new ContactEventDataItem(newRawId, type, detailInfo);
    let labelId: string = resultSet.getString(resultSet.getColumnIndex(Data.DATA2));
    let newLabelId: string = DbConstants.EVENT_MAP.get(labelId) as string;
    item.setCustomData(StringUtil.isEmpty(newLabelId) ? '0' : newLabelId);
    return item.createValuesBucket();
  }

  /**
   * Create contact event ValuesBucket object
   *
   * @returns ValuesBucket with contact event data
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = super.createValuesBucket();
    values[Data.CUSTOM_DATA] = this.mCustomData;
    return values;
  }
}