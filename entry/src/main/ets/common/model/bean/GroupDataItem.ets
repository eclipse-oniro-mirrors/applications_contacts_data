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

import DataItem from './DataItem';

/**
 * GroupDataItem bean to batch insert table data
 */
export default class GroupDataItem extends DataItem {
  /**
   * Create group data obj
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
   * Build group info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param groupId the table groups id for data item to insert table data
   * @returns ValuesBucket to insert table data
   */
  public static buildGroupData(newRawId: number, type: string, groupId: number): ValuesBucket | null {
    if (groupId < 0) {
      return null;
    }
    let item: GroupDataItem = new GroupDataItem(newRawId, type, String(groupId));
    return item.createValuesBucket();
  }

  /**
   * Create group ValuesBucket object
   *
   * @returns ValuesBucket with group data
   */
  public createValuesBucket(): ValuesBucket {
    return super.createValuesBucket();
  }
}