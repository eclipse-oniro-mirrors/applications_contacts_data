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
 * PhotoDataItem bean to batch insert table data
 */
export default class PhotoDataItem extends DataItem {
  private mBlobData: Uint8Array | null = null;

  /**
   * Create photo data obj
   *
   * @param rawContactId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   */
  constructor(rawContactId: number, type: string) {
    super();
    this.mRawContactId = rawContactId;
    this.mContentType = type;
  }

  /**
   * Set blob_data in data item for batch insert
   *
   * @param blobData the photo data for detail info.
   * @returns PhotoDataItem obj
   */
  public setBlobData(blobData: Uint8Array): PhotoDataItem {
    this.mBlobData = blobData;
    return this;
  }

  /**
   * Build photo info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param resultSet which to build
   * @returns ValuesBucket to insert table data
   */
  public static buildPhotoData(newRawId: number, type: string, resultSet: rdb.ResultSet): ValuesBucket {
    let item: PhotoDataItem = new PhotoDataItem(newRawId, type);
    item.setBlobData(resultSet.getBlob(resultSet.getColumnIndex(Data.DATA15)));
    return item.createValuesBucket();
  }


  public static buildPhotoDataForClone(newRawId: number, type: string, blobData: Uint8Array): ValuesBucket {
    let item: PhotoDataItem = new PhotoDataItem(newRawId, type);
    item.setBlobData(blobData);
    return item.createValuesBucket();
  }

  /**
   * Create photo ValuesBucket object
   *
   * @returns ValuesBucket with photo data
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = super.createValuesBucket();
    values['blob_data'] = this.mBlobData;
    return values;
  }
}