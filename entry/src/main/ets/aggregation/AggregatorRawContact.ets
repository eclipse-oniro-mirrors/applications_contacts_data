/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import { Contact } from '../components/PlatformInterface/Contact';
import rdb from '@ohos.data.relationalStore';
import DataColumns from '../common/data/DataColumns';
import { ContactInfo } from '../common/data/ContactInfo';
import { ArrayUtil } from '../common/utils/ArrayUtil';
import RawContactsColumns from '../common/data/RawContactsColumns';
import { StringUtil } from '../common/utils/StringUtil';

export class AggregatorRawContact {
  public rawId: number;
  public contactId: number;
  public displayName: string = '';
  public isDeleted: number = 0;
  public aggregationStatus: number = 0;
  public favorite: number = 0;
  public favoriteOrder: number = -1;
  public phoneList: Contact.PhoneNumber[] = [];
  public static readonly AGGREGATOR_COLUMNS: string[] = [DataColumns.RAW_CONTACT_ID, RawContactsColumns.CONTACT_ID,
    RawContactsColumns.DISPLAY_NAME, RawContactsColumns.IS_DELETED, RawContactsColumns.AGGREGATION_STATUS,
    RawContactsColumns.FAVORITE, RawContactsColumns.FAVORITE_ORDER, DataColumns.TYPE_ID, DataColumns.DETAIL_INFO,
    DataColumns.CUSTOM_DATA, DataColumns.EXTEND7, DataColumns.FORMAT_PHONE_NUMBER];
  public static sDataClomns: string[] = [DataColumns.RAW_CONTACT_ID, DataColumns.TYPE_ID, DataColumns.DETAIL_INFO,
    DataColumns.CUSTOM_DATA, DataColumns.EXTEND7, DataColumns.FORMAT_PHONE_NUMBER];

  constructor(rawId: number, contactId: number, displayName: string, isDeleted: number,
    favorite: number | null, favoriteOrder: number | null, aggregationStatus?: number) {
    this.rawId = rawId;
    this.contactId = contactId;
    this.isDeleted = isDeleted;
    this.displayName = displayName;
    if (favorite) {
      this.favorite = favorite;
    }
    if (favoriteOrder) {
      this.favoriteOrder = favoriteOrder;
    }
    if (aggregationStatus) {
      this.aggregationStatus = aggregationStatus;
    }
  }

  public addPhone(phone: Contact.PhoneNumber) {
    if (phone) {
      this.phoneList.push(phone);
    }
  }

  public setPhoneList(phoneList: Contact.PhoneNumber[]) {
    if (phoneList && phoneList.length > 0) {
      this.phoneList = ArrayUtil.simpleCopy(phoneList);
    }
  }

  public static fromRawResultSet(resultSet: rdb.ResultSet): AggregatorRawContact {
    let rawId = resultSet.getLong(resultSet.getColumnIndex(DataColumns.RAW_CONTACT_ID));
    let contactId = resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.CONTACT_ID));
    let displayName = resultSet.getString(resultSet.getColumnIndex(RawContactsColumns.DISPLAY_NAME));
    let isDeleted = resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.IS_DELETED));
    let aggregationStatus = resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.AGGREGATION_STATUS));
    let favorite = resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.FAVORITE));
    let favoriteOrder = resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.FAVORITE_ORDER));
    let obj = new AggregatorRawContact(rawId, contactId, displayName, isDeleted, favorite, favoriteOrder,
      aggregationStatus);
    return obj;
  }

  public static fromContactInfo(contactInfo: ContactInfo): AggregatorRawContact {
    let aggregatorRaw = new AggregatorRawContact(contactInfo.id, contactInfo.contactId, contactInfo.displayName,
      contactInfo.isDeleted, contactInfo.favorite, contactInfo.favoriteOrder);
    aggregatorRaw.displayName = contactInfo.displayName;
    aggregatorRaw.phoneList = ArrayUtil.simpleCopy(contactInfo.phoneList);
    return aggregatorRaw;
  }

  public toString(): string {
    return `AggregatorRawContact rawId:` + this.rawId + `, contactId:` + this.contactId + `, displayName:` +
      StringUtil.maskSensitiveInfo(this.displayName) + `, favorite:` + this.favorite + `, favoriteOrder:` + this.favoriteOrder +
      `, aggregationStatus:` + this.aggregationStatus + `, phoneList:` +
      this.phoneList.map((phone => StringUtil.maskSensitiveInfo(phone.phoneNumber!))).join(',');
  }
}