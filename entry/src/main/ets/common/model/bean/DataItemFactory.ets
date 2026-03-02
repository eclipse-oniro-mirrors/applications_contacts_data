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

import { StringUtil } from '../../../common/utils/StringUtil';
import ContactEventDataItem from './ContactEventDataItem';
import DataItem from './DataItem';
import EmailDataItem from './EmailDataItem';
import GroupDataItem from './GroupDataItem';
import ImDataItem from './ImDataItem';
import NameDataItem from './NameDataItem';
import OrganizationDataItem from './OrganizationDataItem';
import PhoneDataItem from './PhoneDataItem';
import PhotoDataItem from './PhotoDataItem';
import PostalAddressDataItem from './PostalAddressDataItem';
import RelationDataItem from './RelationDataItem';
import LogUtils from '../../../common/utils/LogUtils';
import { Data } from '../../contract/Data';
import { Mimetype } from '../../contract/Mimetype';

/**
 * DataItemFactory to create data item obj
 */
export default class DataItemFactory {
  private readonly logTag: string = 'DataItemFactory';
  private static sInstance: DataItemFactory;

  /**
   * Get DataItemFactory instance
   *
   * @returns DataItemFactory obj
   */
  public static getInstance(): DataItemFactory {
    if (!DataItemFactory.sInstance) {
      DataItemFactory.sInstance = new DataItemFactory();
    }
    return DataItemFactory.sInstance;
  }

  /**
   * Build the DataItem info obj
   *
   * @param newRawId the raw_contact_id in table contact_data
   * @param type which mimetype in table contact_data
   * @param resultSet which to build
   * @param groupId the group_id in table contact_data
   * @return DataItem for the type
   */
  public buildDataItem(newRawId: number, type: string, resultSet: rdb.ResultSet, groupId: number): ValuesBucket | null {
    if (newRawId <= 0 || resultSet === null || resultSet === undefined || StringUtil.isEmpty(type)) {
      return null;
    }
    let detailInfo: string = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
    switch (type) {
      case Mimetype.SINGLE_EMAIL:
        return EmailDataItem.buildEmailData(newRawId, type, resultSet, detailInfo);
      case Mimetype.SINGLE_IM:
        return ImDataItem.buildImData(newRawId, type, resultSet, detailInfo);
      case Mimetype.SINGLE_ORGANIZATION:
        return OrganizationDataItem.buildOrganizationData(newRawId, type, resultSet, detailInfo);
      case Mimetype.SINGLE_PHONE:
        return PhoneDataItem.buildPhoneData(newRawId, type, resultSet, detailInfo);
      case Mimetype.SINGLE_NAME:
        return NameDataItem.buildNameData(newRawId, type, resultSet, detailInfo);
      case Mimetype.SINGLE_POSTAL_ADDRESS:
        return PostalAddressDataItem.buildPostalAddressData(newRawId, type, resultSet, detailInfo);
      case Mimetype.SINGLE_CONTACT_EVENT:
        return ContactEventDataItem.buildContactEventData(newRawId, type, resultSet, detailInfo);
      case Mimetype.SINGLE_PHOTO:
      case Mimetype.SINGLE_CAMCARD:
        return PhotoDataItem.buildPhotoData(newRawId, type, resultSet);
      case Mimetype.SINGLE_GROUP_MEMBERSHIP:
        return GroupDataItem.buildGroupData(newRawId, type, groupId);
      case Mimetype.SINGLE_RELATION:
        return RelationDataItem.buildRelationData(newRawId, type, resultSet, detailInfo);
      case Mimetype.SINGLE_NICKNAME:
      case Mimetype.SINGLE_SIP_ADDRESS:
      case Mimetype.SINGLE_NOTE:
      case Mimetype.SINGLE_WEBSITE:
        return this.buildSimpleData(newRawId, type, detailInfo).createValuesBucket();
      default:
        return null;
    }
  }

  public buildDataItemForClone(newRawId: number, 
                               type: string,
                               resultSet: rdb.ResultSet, groupId: number): ValuesBucket | null {
    if (newRawId <= 0 || resultSet === null || resultSet === undefined || StringUtil.isEmpty(type)) {
      LogUtils.e(this.logTag, 'buildDataItemForClone resultSet is empty');
      return null;
    }
    let detailInfo: string = '';
    let rawContactId = resultSet.getString(resultSet.getColumnIndex(Data.RAW_CONTACT_ID));
    switch (type) {
      case Mimetype.DOUBLE_EMAIL:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return EmailDataItem.buildEmailData(newRawId, Mimetype.SINGLE_EMAIL, resultSet, detailInfo);
      case Mimetype.DOUBLE_IM:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return ImDataItem.buildImData(newRawId, Mimetype.SINGLE_IM, resultSet, detailInfo);
      case Mimetype.DOUBLE_ORGANIZATION:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return OrganizationDataItem.buildOrganizationData(newRawId,
          Mimetype.SINGLE_ORGANIZATION,
          resultSet, detailInfo);
      case Mimetype.DOUBLE_PHONE:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return PhoneDataItem.buildPhoneData(newRawId, Mimetype.SINGLE_PHONE, resultSet, detailInfo);
      case Mimetype.DOUBLE_NAME:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        detailInfo = NameDataItem.getDisplayName(detailInfo, resultSet);
        return NameDataItem.buildNameData(newRawId, Mimetype.SINGLE_NAME, resultSet, detailInfo);
      case Mimetype.DOUBLE_POSTAL_ADDRESS:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return PostalAddressDataItem.buildPostalAddressData(newRawId, Mimetype.SINGLE_POSTAL_ADDRESS,
          resultSet, detailInfo);
      case Mimetype.DOUBLE_CONTACT_EVENT:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return ContactEventDataItem.buildContactEventData(newRawId, Mimetype.SINGLE_CONTACT_EVENT,
          resultSet, detailInfo);
      case Mimetype.DOUBLE_PHOTO:
        return PhotoDataItem.buildPhotoData(newRawId, Mimetype.SINGLE_PHOTO, resultSet);
      case Mimetype.DOUBLE_CAMCARD:
        return PhotoDataItem.buildPhotoData(newRawId, Mimetype.SINGLE_CAMCARD, resultSet);
      case Mimetype.DOUBLE_GROUP_MEMBERSHIP:
        return GroupDataItem.buildGroupData(newRawId, Mimetype.SINGLE_GROUP_MEMBERSHIP, groupId);
      case Mimetype.DOUBLE_RELATION:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return RelationDataItem.buildRelationData(newRawId, Mimetype.SINGLE_RELATION, resultSet, detailInfo);
      case Mimetype.DOUBLE_NICKNAME:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return this.buildSimpleData(newRawId, Mimetype.SINGLE_NICKNAME, detailInfo).createValuesBucket();
      case Mimetype.DOUBLE_SIP_ADDRESS:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return this.buildSimpleData(newRawId, Mimetype.SINGLE_SIP_ADDRESS, detailInfo).createValuesBucket();
      case Mimetype.DOUBLE_NOTE:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return this.buildSimpleData(newRawId, Mimetype.SINGLE_NOTE, detailInfo).createValuesBucket();
      case Mimetype.DOUBLE_WEBSITE:
        detailInfo = resultSet.getString(resultSet.getColumnIndex(Data.DATA1));
        return this.buildSimpleData(newRawId, Mimetype.SINGLE_WEBSITE, detailInfo).createValuesBucket();
      default:
        return null;
    }
  }

  public buildPhotoDataForClone(newRawId: number, type: string,
                                uintarr: Uint8Array): ValuesBucket | null {
    if (newRawId <= 0 || uintarr === null || StringUtil.isEmpty(type)) {
      return null;
    }
    return PhotoDataItem.buildPhotoDataForClone(newRawId, Mimetype.SINGLE_PHOTO, uintarr);
  }

  private buildSimpleData(newRawId: number, type: string, detailInfo: string): DataItem {
    let item: DataItem = new DataItem();
    return item.setRawContactId(newRawId)
      .setContentType(type)
      .setDetailInfo(detailInfo);
  }
}
