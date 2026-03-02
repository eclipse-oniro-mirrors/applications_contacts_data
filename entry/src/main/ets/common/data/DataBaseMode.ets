/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
import DataColumns from './DataColumns';
import rdb from '@ohos.data.relationalStore';
import DataShareResultSet from '@ohos.data.DataShareResultSet'

export namespace DataBaseMode {

  export class VcardValue {
    public uuid: string;
    public data: string;
    public rawId: number;
    public type: string;
    public isDeleted: number = 0;
    public delTime: number | null = null;
    public attachments: rdb.Assets | undefined = undefined;

    constructor(uuid: string, data: string, rawId: number, type: string, isDeleted: number,
      delTime: number | null, attachments: rdb.Assets | undefined) {
      this.uuid = uuid;
      this.data = data;
      this.rawId = rawId;
      this.type = type;
      this.isDeleted = isDeleted;
      this.delTime = delTime;
      this.attachments = attachments;
    }
  }

  export class UpdateValue {
    public uuid: string;
    public data: string;
    public isDeleted: number = 0;

    constructor(uuid: string, data: string, isDeleted: number) {
      this.uuid = uuid;
      this.data = data;
      this.isDeleted = isDeleted;
    }
  }


  export class ContactDataItem {
    public id: number;
    public typeId: number;
    public typeName: string = '';
    public rawContactId: number;
    public detailInfo: string;
    public position: string;
    public city: string;
    public country: string;
    public neighborhood: string;
    public pobox: string;
    public postcode: string;
    public region: string;
    public street: string;
    public customData: string;
    public extend7: string;
    public blobData: Uint8Array | null = null;
    public extend5: string;
    public contactLastUpdatedTimestamp: number | undefined = 0;
    public dirty: number | undefined = -1;

    constructor(id: number, typeId: number, rawContactId: number, detailInfo: string, position: string, city: string, country: string,
                neighborhood: string,pobox: string,postcode: string,region: string,street: string,customData: string,extend7: string,blobData: Uint8Array | null,
                extend5: string, dirty?: number, contactLastUpdatedTimestamp?: number) {
      this.id = id;
      this.typeId = typeId;
      this.rawContactId = rawContactId;
      this.detailInfo =detailInfo;
      this.position =position;
      this.city = city;
      this.country = country;
      this.neighborhood = neighborhood;
      this.pobox = pobox;
      this.postcode =postcode;
      this.region =region;
      this.street = street;
      this.customData =customData;
      this.extend7 =extend7;
      this.blobData =blobData;
      this.extend5 = extend5;
      this.dirty = dirty;
      this.contactLastUpdatedTimestamp = contactLastUpdatedTimestamp;
    }

    static fromRuleSet(resultSet: DataShareResultSet | rdb.ResultSet) {
      const id = resultSet.getLong(resultSet.getColumnIndex(DataColumns.ID));
      const typeId = resultSet.getLong(resultSet.getColumnIndex(DataColumns.TYPE_ID));
      const rawContactId = resultSet.getLong(resultSet.getColumnIndex(DataColumns.RAW_CONTACT_ID));
      const detailInfo = resultSet.getString(resultSet.getColumnIndex(DataColumns.DETAIL_INFO));
      const position = resultSet.getString(resultSet.getColumnIndex(DataColumns.POSITION));
      const city = resultSet.getString(resultSet.getColumnIndex(DataColumns.CITY));
      const country = resultSet.getString(resultSet.getColumnIndex(DataColumns.COUNTRY));
      const neighborhood = resultSet.getString(resultSet.getColumnIndex(DataColumns.NEIGHBORHOOD));
      const pobox = resultSet.getString(resultSet.getColumnIndex(DataColumns.POBOX));
      const postcode = resultSet.getString(resultSet.getColumnIndex(DataColumns.POSTCODE));
      const region = resultSet.getString(resultSet.getColumnIndex(DataColumns.REGION));
      const street = resultSet.getString(resultSet.getColumnIndex(DataColumns.STREET));
      const customData = resultSet.getString(resultSet.getColumnIndex(DataColumns.CUSTOM_DATA));
      const extend7 = resultSet.getString(resultSet.getColumnIndex(DataColumns.EXTEND7));
      const blobData = resultSet.getBlob(resultSet.getColumnIndex(DataColumns.BLOB_DATA));
      const extend5 = resultSet.getString(resultSet.getColumnIndex(DataColumns.EXTEND5));
      const contactDataItem = new ContactDataItem(id, typeId, rawContactId, detailInfo, position, city, country,
        neighborhood, pobox, postcode, region, street, customData, extend7, blobData, extend5);
      return contactDataItem;
    }
  }

  export class NoteContact {
    public raw_contact_id: string;
    public detail_info: string;
    public type_id: number;

    constructor(rawContactId: string, detailInfo: string, typeId: number) {
      this.raw_contact_id = rawContactId
      this.detail_info = detailInfo
      this.type_id = typeId
    }
  }

  export class DetailContact {
    public raw_contact_id: number;
    public detail_info: string;
    public extend7: number;
    public custom_data: number;
    public type_id: number;

    constructor(rawContactId: number, detailInfo: string, extend7: number, customData: number, typeId: number) {
      this.raw_contact_id = rawContactId
      this.detail_info = detailInfo
      this.extend7 = extend7
      this.custom_data = customData
      this.type_id = typeId
    }
  }

  export class WebsiteContent {
    public raw_contact_id: string;
    public detail_info: string;
    public type_id: number;

    constructor(rawContactId: string, detailInfo: string, typeId: number) {
      this.raw_contact_id = rawContactId
      this.detail_info = detailInfo
      this.type_id = typeId
    }
  }
}



