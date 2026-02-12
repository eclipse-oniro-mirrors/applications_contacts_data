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
 * OrganizationDataItem bean to batch insert table data
 */
export default class OrganizationDataItem extends DataItem {
  private mPosition: string = '';

  /**
   * Create organization data obj
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
   * Set position in data item for batch insert
   *
   * @param position the organization label for detail info.
   * @returns OrganizationDataItem obj
   */
  public setPosition(position: string): OrganizationDataItem {
    this.mPosition = position;
    return this;
  }

  /**
   * Build organization info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param resultSet which to build
   * @param detailInfo the content detail info for data item to insert table data
   * @returns ValuesBucket to insert table data
   */
  public static buildOrganizationData(newRawId: number, type: string, resultSet: rdb.ResultSet,
    detailInfo: string): ValuesBucket {
    let item: OrganizationDataItem = new OrganizationDataItem(newRawId, type, detailInfo);
    item.setPosition(resultSet.getString(resultSet.getColumnIndex(Data.DATA4)));
    return item.createValuesBucket();
  }

  /**
   * Create organization ValuesBucket object
   *
   * @returns ValuesBucket with organization data
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = super.createValuesBucket();
    values[Data.POSITION] = this.mPosition;
    return values;
  }
}