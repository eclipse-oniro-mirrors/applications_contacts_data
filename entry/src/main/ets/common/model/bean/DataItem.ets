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

import { Data } from '../../contract/Data';

/**
 * DataItem bean to batch insert table data
 */
export default class DataItem {
  protected mRawContactId: number = -1;
  protected mContentType: string = '';
  protected mDetailInfo: string = '';

  /**
   * Set rawContactId in data item for batch insert
   *
   * @param rawContactId the id for contact
   * @returns DataItem obj
   */
  public setRawContactId(rawContactId: number): DataItem {
    this.mRawContactId = rawContactId;
    return this;
  }

  /**
   * Set detailInfo type in data item for batch insert
   *
   * @param contentType, type 'phone' means detailInfo is phone number, type 'name' means detailInfo is name.
   * @returns DataItem obj
   */
  public setContentType(contentType: string): DataItem {
    this.mContentType = contentType;
    return this;
  }

  /**
   * Set detailInfo in data item for batch insert
   *
   * @param detailInfo means phone or name
   * @returns DataItem obj
   */
  public setDetailInfo(detailInfo: string): DataItem {
    this.mDetailInfo = detailInfo;
    return this;
  }

  /**
   * Create data ValuesBucket object
   *
   * @returns ValuesBucket with data item
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = {};
    values[Data.RAW_CONTACT_ID] = this.mRawContactId;
    values[Data.CONTENT_TYPE] = this.mContentType;
    values[Data.DETAIL_INFO] = this.mDetailInfo;
    return values;
  }
}