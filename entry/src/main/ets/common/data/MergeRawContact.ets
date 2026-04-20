/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
import RawContactsColumns from './RawContactsColumns';
import rdb from '@ohos.data.relationalStore';
import { StringUtil } from '../utils/StringUtil';
import { ContactInfo } from './ContactInfo';
import ContactsDataService, { DisPlayNameResult } from '../../serivce/ContactsDataService';

export default class MergeRawContact {
  public rawId: number = -1;
  public uuid: string = '';
  public contactId: number = -1;
  public favorite: number = 0;
  public favoriteOrder: number = 0;
  public uniqueKey: string = '';
  public isDeleted: number = 0;
  public delTime: number | null = null;
  public formId: string = '';
  public displayName: string = '';
  public dirty: number = 0;
  public aggregationStatus: number = 0;
  public focusModeList: string = '';
  public contactedCount: number = 0;
  public lastestContactedTime: number = 0;
  public primaryContact: number = 0;
  public personalRingtone: string = '';
  public personalNotificationRingtone: string = '';
  public ringtonePath: string = '';
  public contactIdExist: boolean = false;
  public accountId: number = 0;
  public static columns: string[] = ['id', RawContactsColumns.UUID, RawContactsColumns.CONTACT_ID,
    RawContactsColumns.FAVORITE, RawContactsColumns.UNIQUE_KEY, RawContactsColumns.IS_DELETED,
    RawContactsColumns.DISPLAY_NAME, RawContactsColumns.DIRTY, RawContactsColumns.CONTACTED_COUNT,
    RawContactsColumns.LASTEST_CONTACTED_TIME, RawContactsColumns.AGGREGATION_STATUS,
    RawContactsColumns.PRIMARY_CONTACT, RawContactsColumns.FAVORITE_ORDER, RawContactsColumns.ACCOUNT_ID];

  // 注意：所有索引要与上面 columns 一一对应
  constructor() {

  }

  static fromRuleSet(resultSet: rdb.ResultSet): MergeRawContact {
    const mergeRawContact: MergeRawContact = new MergeRawContact();
    let index = 0;
    mergeRawContact.rawId = resultSet.getLong(index++);
    mergeRawContact.uuid = resultSet.getString(index++);
    mergeRawContact.contactId = resultSet.getLong(index++);
    mergeRawContact.favorite = resultSet.getLong(index++);
    mergeRawContact.uniqueKey = resultSet.getString(index++);
    mergeRawContact.isDeleted = resultSet.getLong(index++);
    mergeRawContact.displayName = resultSet.getString(index++);
    mergeRawContact.dirty = resultSet.getLong(index++);
    mergeRawContact.contactedCount = resultSet.getLong(index++);
    mergeRawContact.lastestContactedTime = resultSet.getLong(index++);
    mergeRawContact.aggregationStatus = resultSet.getLong(index++);
    mergeRawContact.primaryContact = resultSet.getLong(index++);
    mergeRawContact.favoriteOrder = resultSet.getLong(index++);
    mergeRawContact.accountId = resultSet.getLong(index++);
    return mergeRawContact;
  }

  static fromCloudDownlodContact(contactInfo: ContactInfo): MergeRawContact {
    const mergeRawContact: MergeRawContact = new MergeRawContact();
    mergeRawContact.uuid = contactInfo.uuid!;
    mergeRawContact.favorite = contactInfo.favorite || 0;
    mergeRawContact.uniqueKey = contactInfo.uniqueKey;
    mergeRawContact.isDeleted = contactInfo.isDeleted;
    const nameRes: DisPlayNameResult = ContactsDataService.getInstance().getName(contactInfo);
    mergeRawContact.displayName = nameRes.displayName;
    return mergeRawContact;
  }

  static getLogStr(arr: MergeRawContact[]): string {
    if (!arr?.length) {
      return '';
    }
    let rawContactArr: string[] = [`displayName,rawId,contactId,isDeleted,uuid,uniqueKey,aggregationStatus`];
    for (const rawContact of arr) {
      const infoArr = [StringUtil.maskSensitiveInfo(rawContact.displayName), rawContact.rawId, rawContact.contactId,
      rawContact.isDeleted, StringUtil.getShortUuid(rawContact.uuid),
      StringUtil.getShortUniqueKey(rawContact.uniqueKey), rawContact.aggregationStatus];
      rawContactArr.push(infoArr.join(','));
    }
    return rawContactArr.join('|');
  }
}
