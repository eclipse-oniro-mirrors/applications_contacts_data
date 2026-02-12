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
import DataItem from './DataItem';
import { Data } from '../../contract/Data';

/**
 * PostalAddressDataItem bean to batch insert table data
 */
export default class PostalAddressDataItem extends DataItem {
  private mCustomData: string = '';
  private mStreet: string = '';
  private mPobox: string = '';
  private mNeighborhood: string = '';
  private mCity: string = '';
  private mRegion: string = '';
  private mPostcode: string = '';
  private mCountry: string = '';
  private mExtend7: string = '';

  /**
   * Create postalAddress data obj
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
   * Set street in data item for batch insert
   *
   * @param street which is contained the postal address
   * @return PostalAddressDataItem obj
   */
  public setStreet(street: string): PostalAddressDataItem {
    this.mStreet = street;
    return this;
  }

  /**
   * Set pobox in data item for batch insert
   *
   * @param pobox which is contained the postal address
   * @return PostalAddressDataItem obj
   */
  public setPobox(pobox: string) {
    this.mPobox = pobox;
    return this;
  }

  /**
   * Set neighborhood in data item for batch insert
   *
   * @param neighborhood which is contained the postal address
   * @return PostalAddressDataItem obj
   */
  public setNeighborhood(neighborhood: string) {
    this.mNeighborhood = neighborhood;
    return this;
  }

  /**
   * Set city in data item for batch insert
   *
   * @param city which is contained the postal address
   * @return PostalAddressDataItem obj
   */
  public setCity(city: string) {
    this.mCity = city;
    return this;
  }

  /**
   * Set region in data item for batch insert
   *
   * @param region which is contained the postal address
   * @return PostalAddressDataItem obj
   */
  public setRegion(region: string) {
    this.mRegion = region;
    return this;
  }

  /**
   * Set postcode in data item for batch insert
   *
   * @param postcode which is contained the postal address
   * @return PostalAddressDataItem obj
   */
  public setPostcode(postcode: string) {
    this.mPostcode = postcode;
    return this;
  }

  /**
   * Set country in data item for batch insert
   *
   * @param country which is contained the postal address
   * @return PostalAddressDataItem obj
   */
  public setCountry(country: string) {
    this.mCountry = country;
    return this;
  }

  /**
   * Set custom_data in data item for batch insert
   *
   * @param mCustomData label type for postal address.
   * @return PostalAddressDataItem obj
   */
  public setCustomData(customData: string) {
    this.mCustomData = customData;
    return this;
  }

  public setExtend7(extend7: string): PostalAddressDataItem {
    this.mExtend7 = extend7;
    return this;
  }

  /**
   * Build postal info for insert data table
   *
   * @param newRawId the raw contact id for data item to insert table data
   * @param type the content type for data item to insert table data
   * @param resultSet which to build
   * @param detailInfo the content detail info for data item to insert table data
   * @returns ValuesBucket to insert table data
   */
  public static buildPostalAddressData(newRawId: number, type: string, resultSet: rdb.ResultSet,
                                       detailInfo: string): ValuesBucket {
    let item: PostalAddressDataItem = new PostalAddressDataItem(newRawId, type, detailInfo);
    let data2: string = resultSet.getString(resultSet.getColumnIndex(Data.DATA2));
    item.setCustomData(resultSet.getString(resultSet.getColumnIndex(Data.DATA2)));
    if (data2 == '0') {
      item.setCustomData('10000');
      item.setExtend7(resultSet.getString(resultSet.getColumnIndex(Data.DATA3)));
    }
    return item.setStreet(resultSet.getString(resultSet.getColumnIndex(Data.DATA4)))
      .setPobox(resultSet.getString(resultSet.getColumnIndex(Data.DATA5)))
      .setNeighborhood(resultSet.getString(resultSet.getColumnIndex(Data.DATA6)))
      .setCity(resultSet.getString(resultSet.getColumnIndex(Data.DATA7)))
      .setRegion(resultSet.getString(resultSet.getColumnIndex(Data.DATA8)))
      .setPostcode(resultSet.getString(resultSet.getColumnIndex(Data.DATA9)))
      .setCountry(resultSet.getString(resultSet.getColumnIndex(Data.DATA10)))
      .createValuesBucket();
  }

  /**
   * Create postalAddress ValuesBucket object
   *
   * @returns ValuesBucket with postalAddress data
   */
  public createValuesBucket(): ValuesBucket {
    let values: ValuesBucket = super.createValuesBucket();
    values[Data.CUSTOM_DATA] = this.mCustomData;
    values[Data.STREET] = this.mStreet;
    values[Data.POBOX] = this.mPobox;
    values[Data.NEIGHBORHOOD] = this.mNeighborhood;
    values[Data.CITY] = this.mCity;
    values[Data.REGION] = this.mRegion;
    values[Data.POSTCODE] = this.mPostcode;
    values[Data.COUNTRY] = this.mCountry;
    values[Data.EXTEND7] = this.mExtend7;
    return values;
  }
}