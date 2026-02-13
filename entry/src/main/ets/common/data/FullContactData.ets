/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 * Licensed under the Apache License] =  Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing] =  software
 * distributed under the License is distributed on an "AS IS" BASIS] =
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND] =  either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import DataColumns from './DataColumns';
import rdb from '@ohos.data.relationalStore';
import { ValuesBucket } from '@ohos.data.ValuesBucket';

export default class FullContactData {
  public type_id: number = 0;
  public custom_data: number = 0;
  public detail_info: string = '';
  public raw_contact_id: number = 0;
  public position: string = '';
  public extend1: string = '';
  public extend2: string = '';
  public extend3: string = '';
  public extend4: string = '';
  public city: string = '';
  public country: string = '';
  public neighborhood: string = '';
  public pobox: string = '';
  public postcode: string = '';
  public region: string = '';
  public street: string = '';
  public extend5: string = '';
  public extend6: string = '';
  public extend7: string = '';
  public blob_data: Uint8Array = new Uint8Array();
  public blob_source: number = 0;
  public syn_1: string = '';
  public syn_2: string = '';
  public syn_3: string = '';
  public extend8: string = '';
  public extend9: string = '';
  public extend10: string = '';
  public extend11: string = '';
  public formatPhoneNumber: string = '';
  static columns: string[] = [DataColumns.TYPE_ID, DataColumns.CUSTOM_DATA, DataColumns.DETAIL_INFO,
  DataColumns.RAW_CONTACT_ID, DataColumns.POSITION, DataColumns.EXTEND1, DataColumns.EXTEND2, DataColumns.EXTEND3,
  DataColumns.EXTEND4, DataColumns.CITY, DataColumns.COUNTRY, DataColumns.NEIGHBORHOOD, DataColumns.POBOX,
  DataColumns.POSTCODE, DataColumns.REGION, DataColumns.STREET, DataColumns.EXTEND5, DataColumns.EXTEND6,
  DataColumns.EXTEND7, DataColumns.BLOB_DATA, DataColumns.BLOB_SOURCE, DataColumns.SYN_1, DataColumns.SYN_2,
  DataColumns.SYN_3, DataColumns.EXTEND8, DataColumns.EXTEND9, DataColumns.EXTEND10, DataColumns.EXTEND11,
  DataColumns.FORMAT_PHONE_NUMBER];

  // 注意：所有索引要与上面 columns 一一对应
  constructor(resultSet?: rdb.ResultSet) {
    if (resultSet) {
      let index = 0;
      this.type_id = resultSet.getLong(index++);
      //以long来获取TEXT，将1.0 与 1这2类值都转为1
      this.custom_data = resultSet.getLong(index++);
      this.detail_info = resultSet.getString(index++);
      this.raw_contact_id = resultSet.getLong(index++);
      this.position = resultSet.getString(index++);
      this.extend1 = resultSet.getString(index++);
      this.extend2 = resultSet.getString(index++);
      this.extend3 = resultSet.getString(index++);
      this.extend4 = resultSet.getString(index++);
      this.city = resultSet.getString(index++);
      this.country = resultSet.getString(index++);
      this.neighborhood = resultSet.getString(index++);
      this.pobox = resultSet.getString(index++);
      this.postcode = resultSet.getString(index++);
      this.region = resultSet.getString(index++);
      this.street = resultSet.getString(index++);
      this.extend5 = resultSet.getString(index++);
      this.extend6 = resultSet.getString(index++);
      this.extend7 = resultSet.getString(index++);
      this.blob_data = resultSet.getBlob(index++);
      this.blob_source = resultSet.getLong(index++);
      this.syn_1 = resultSet.getString(index++);
      this.syn_2 = resultSet.getString(index++);
      this.syn_3 = resultSet.getString(index++);
      this.extend8 = resultSet.getString(index++);
      this.extend9 = resultSet.getString(index++);
      this.extend10 = resultSet.getString(index++);
      this.extend11 = resultSet.getString(index++);
      this.formatPhoneNumber = resultSet.getString(index++);
    }
  }

  toValuesBucket() {
    const valuesBucket: ValuesBucket = {};
    valuesBucket[DataColumns.TYPE_ID] = this.type_id;
    valuesBucket[DataColumns.CUSTOM_DATA] = this.custom_data;
    valuesBucket[DataColumns.DETAIL_INFO] = this.detail_info;
    valuesBucket[DataColumns.RAW_CONTACT_ID] = this.raw_contact_id
    valuesBucket[DataColumns.POSITION] = this.position
    valuesBucket[DataColumns.EXTEND1] = this.extend1
    valuesBucket[DataColumns.EXTEND2] = this.extend2
    valuesBucket[DataColumns.EXTEND3] = this.extend3
    valuesBucket[DataColumns.EXTEND4] = this.extend4
    valuesBucket[DataColumns.CITY] = this.city
    valuesBucket[DataColumns.COUNTRY] = this.country
    valuesBucket[DataColumns.NEIGHBORHOOD] = this.neighborhood
    valuesBucket[DataColumns.POBOX] = this.pobox
    valuesBucket[DataColumns.POSTCODE] = this.postcode
    valuesBucket[DataColumns.REGION] = this.region
    valuesBucket[DataColumns.STREET] = this.street
    valuesBucket[DataColumns.EXTEND5] = this.extend5
    valuesBucket[DataColumns.EXTEND6] = this.extend6
    valuesBucket[DataColumns.EXTEND7] = this.extend7
    valuesBucket[DataColumns.BLOB_DATA] = this.blob_data
    valuesBucket[DataColumns.BLOB_SOURCE] = this.blob_source
    valuesBucket[DataColumns.SYN_1] = this.syn_1
    valuesBucket[DataColumns.SYN_2] = this.syn_2
    valuesBucket[DataColumns.SYN_3] = this.syn_3
    valuesBucket[DataColumns.EXTEND8] = this.extend8
    valuesBucket[DataColumns.EXTEND9] = this.extend9
    valuesBucket[DataColumns.EXTEND10] = this.extend10
    valuesBucket[DataColumns.EXTEND11] = this.extend11
    valuesBucket[DataColumns.FORMAT_PHONE_NUMBER] = this.formatPhoneNumber
    return valuesBucket;
  }
}
