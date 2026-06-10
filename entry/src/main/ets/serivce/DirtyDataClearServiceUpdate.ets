/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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
import { common } from '@kit.AbilityKit';
import LogUtils from '../common/utils/LogUtils';
import rdb from '@ohos.data.relationalStore';
import { contactRdbHelper } from '../common/helperUtils/ContactsRdbHelperForCloudSync';
const TAG = 'DirtyDataClearService';

export class DirtyDataClearServiceUpdate {
  private static instance: DirtyDataClearServiceUpdate | null = null;
  public context: common.Context | undefined = undefined;

  /**
   * 初始化context
   * @param context
   */
  public init(context: common.Context) {
    this.context = context;
  }

  /**
   * 获取单实例
   * @returns
   */
  public static getInstance(): DirtyDataClearServiceUpdate {
    if (DirtyDataClearServiceUpdate.instance === null) {
      DirtyDataClearServiceUpdate.instance = new DirtyDataClearServiceUpdate();
    }
    return DirtyDataClearServiceUpdate.instance;
  }

  public async deleteContactInfoHardByContactAndRawIds(contactIdList: number[],
    rawContactIdList: number[]): Promise<boolean> {
    const rawIdStr: string = rawContactIdList.join(',');
    LogUtils.w(TAG, `deleteContactInfoHardByContactAndRawIds contactIdList: ${
      contactIdList}, rawContactIdList: ${rawIdStr}`);
    if (!contactIdList.length || !rawContactIdList.length) {
      return true;
    }
    try {
      // 更新contact表的nameRawContactId为空
      let updateRawContactSql = 'UPDATE raw_contact SET contact_id = NULL WHERE id in (' + rawIdStr + ')';
      await contactRdbHelper.executeSql(updateRawContactSql);

      let deleteDeleteRawContactSql = 'DELETE FROM deleted_raw_contact WHERE raw_contact_id in (' + rawIdStr + ')';
      await contactRdbHelper.executeSql(deleteDeleteRawContactSql);

      // 删除SearchContact表
      let deleteSearchContactSql = 'DELETE FROM search_contact WHERE raw_contact_id in (' + rawIdStr + ')';
      await contactRdbHelper.executeSql(deleteSearchContactSql);

      // 删除contact_data信息
      let delContactDataSql = 'DELETE FROM contact_data WHERE raw_contact_id in (' + rawIdStr + ')';
      await contactRdbHelper.executeSql(delContactDataSql);

      // 删除contact
      let delContactSql = 'delete from contact WHERE name_raw_contact_id in (' + rawIdStr + ')';
      await contactRdbHelper.executeSql(delContactSql);

      // 删除rawContact
      let delRawContactSql = 'delete from raw_contact WHERE id in (' + rawIdStr + ')';
      await contactRdbHelper.executeSql(delRawContactSql);

      return true;
    } catch (e) {
      LogUtils.e(TAG, 'deleteContactInfoHardByContactAndRawIds error：' + e.message + ' ' + rawContactIdList + ' ' + contactIdList);
      return false;
    }
  }
}
 
export class ContactInfo {
 
  public id: number;
 
  public contactId: number;
 
  public company: string;
 
  public position: string;
 
  constructor(id: number, contactId: number, company: string, position: string) {
    this.id = id;
    this.contactId = contactId;
    this.company = company;
    this.position = position;
  }
 
  public static fromResultSet(resultSet: rdb.ResultSet) : ContactInfo {
    const contactInfo: ContactInfo = new ContactInfo(resultSet.getLong(0),
    resultSet.getLong(1), resultSet.getString(2), resultSet.getString(3));
    return contactInfo;
  }
}