/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { BusinessError } from '@ohos.base';
import LogUtils from '../common/utils/LogUtils';
import dataShare from '@ohos.data.dataShare';
import common from '@ohos.app.ability.common';
import rdb from '@ohos.data.relationalStore';
import { ContactInfo } from '../common/data/ContactInfo';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import { Contacts } from '../common/data/Contacts';
import relationalStore from '@ohos.data.relationalStore';
import HandleUniqueKeyService, { MergeHandleCount } from './HandleUniqueKeyService';

import BirthdayManager from './BirthdayManager';
import ContactAggregator from '../aggregation/ContactAggregator';
import { contextConstant } from '@kit.AbilityKit';
import { image } from '@kit.ImageKit';
import DbConstants, { DIRTY_ENUM } from '../common/DbConstants';
import CallLogSingleConstant from '../common/contract/CallLogSingleConstant';
import GroupIdUtil from '../common/utils/GroupIdUtil';
import { contactRdbHelper } from '../common/helperUtils/ContactsRdbHelperForCloudSync';
import RawContactsColumns from '../common/data/RawContactsColumns';
import MergeRawContact from '../common/data/MergeRawContact';
import { DeleteRawContact } from '../common/contract/DeleteRawContact';
import ContactsColumns from '../common/contract/ContactsColumns';
import DataColumns from '../common/data/DataColumns';
import { getFormatNumber } from '../common/utils/NumberFormatUtil';
import { callLogRdbHelper } from '../common/helperUtils/CallLogRdbHelperForCloudSync';
import { Data } from '../common/data/Data';

const TAG = 'ContactsDataService';

const TIMEOUT_AGGREGATION_ALL_ONCE = 200;

const MIN_RESOLUTION = 409600;

export class DisPlayNameResult {
  public displayName: string = '';
  public updateFlag: number = 0;
}

interface UpdateInfo {
  updateBucket: ValuesBucket,
  predicates: rdb.RdbPredicates
}

export interface ExtraInfo {
  deletePhoneList: string[],
  addPhoneList: string[],
  deleteFormatPhoneList: string[],
  addFormatPhoneList: string[],
}

export interface PhotoFileInfo {
  filePath: string,
  isCloud?: boolean,
  blobData: Uint8Array,
  blobSource?: number,
  resolution?: number,
  fileSize?: number,
}

// 无名氏首字母 ...
export const ANONYMOUS_SORT_FIRST_LETTER = '...';
// 无名氏排序数字，无名氏信息需要排到字母的后边
// 如果是字母最大是z，对应90，升序排列，无名氏需要在90后边
// 排序字段存99
export const ANONYMOUS_SORT = '99';

export default class ContactsDataService {
  private static instance: ContactsDataService;

  private groupDir: string = '';

  private dataShareHelper: dataShare.DataShareHelper | undefined = undefined;

  private isMergeContactEnd: boolean = true;

  private checkingCursor: boolean = false;

  private mergeWorkerId: number = 0;

  public context: common.ServiceExtensionContext | undefined = undefined;

  public QUERY_RAW_CONTACT_PAGE_NUM: number = 50;

  public dataShareHelperContactSyncSwitchUri?: Promise<dataShare.DataShareHelper> | dataShare.DataShareHelper;

  public callLogDataShareHelper: dataShare.DataShareHelper | undefined = undefined;

  // 单次克隆，相同form_id仅删除一次
  private formIdSet: Set<string> = new Set();

  // 是否处理退账号删除联系人，退账号删除联系人操作不被功耗管控
  private isHandleLogoutDelContactFlag = false;

  public getIsHandleLogoutDelContactFlag() {
    return this.isHandleLogoutDelContactFlag;
  }

  private constructor() {
  }

  init(context: common.ServiceExtensionContext) {
    this.context = context;
    BirthdayManager.getInstance().init(this.context);
    this.getGroupDir(context);
  }

  getGroupDir(context: Context) {
    let ctx = context;
    ctx.area = contextConstant.AreaMode.EL2;
    ctx.getGroupDir(GroupIdUtil.groupId).then((data) => {
      this.groupDir = data;
    }).catch((error: BusinessError) => {
      LogUtils.w(TAG, 'getGroupDir error:%s' + error?.message + ', stack: ' + error?.stack);
    });
  }

  public static getInstance(): ContactsDataService {
    if (ContactsDataService.instance == null) {
      ContactsDataService.instance = new ContactsDataService();
    }
    return ContactsDataService.instance;
  }

  public isMergingContact(): boolean {
    LogUtils.i(TAG, `mergeContact check,isMergeContactEnd:${this.isMergeContactEnd}, mergeWorkerId:${this.mergeWorkerId}`);
    return this.isMergeContactEnd == false || this.mergeWorkerId != 0;
  }

  public setMergingContact(isMerging: boolean) {
    LogUtils.i(TAG, `mergeContact setMergingContact:${isMerging}`);
    if (isMerging) {
      this.isMergeContactEnd = false;
      this.mergeWorkerId = Date.now();
    } else {
      this.isMergeContactEnd = true;
      this.mergeWorkerId = 0;
    }
  }

  /**
   * 本地表dirty为1，刷新至云表
   * Query the contact from the raw-cloud.
   * If the uuid not exist, it means that it is a insert addition.
   * If the uuid exists, it means that it is an update operation.
   * The insert request processing is to generate a vcard string,
   * insert it into the cloud table,and update the uuid and dirty of the raw-cloud table.
   * The update request is processed as follows: regenerate the vcard string,
   * update the data of the cloud table, and update the raw-cloud table dirty
   */
  public async contactsDataChange(context: common.Context): Promise<void> {
    LogUtils.i(TAG, 'contactsDataChange enter');
    // 修改数据上云前，需要检查空uniqueKey，空的重新生成
    await HandleUniqueKeyService.getInstance().waitForCalcUniqueKeyEnd(this.context!);
  }


  public async getCallLogDataAbilityHelper(context?: common.Context) {
    // 由于ArkTS类型推断，此处在Promise中调用callback获取dataShareHelper实例
    if (this.callLogDataShareHelper === undefined) {
      this.callLogDataShareHelper = await new Promise<dataShare.DataShareHelper>((resolve) => {
        dataShare.createDataShareHelper(context || this.context!, CallLogSingleConstant.CALL_LOG_URI,
          (err, callLogDataShareHelper) => {
            resolve(callLogDataShareHelper);
          });
      })
    }
    return this.callLogDataShareHelper;
  }

  public async next2nextDeleteContactFormId(formId: string) {
    const fromIdArr = formId.split('//');
    for (const oldFromId of fromIdArr) {
      const numberMatch = oldFromId.match(/\d+/);
      if (numberMatch && !this.formIdSet.has(numberMatch[0])) {
        await this.deleteContactFormId(this.extractNumberBeforeAsterisk(numberMatch[0]));
        this.formIdSet.add(numberMatch[0])
      }
    }
  }

  // 检查文件比例
  async checkFileScale(filePath: string, tag?: string) {
    const tagStr = TAG + ` bigPhotoFromFile checkFileScale tag: ${tag} `;
    // 默认使用海报显示大头像
    let blobSource = 3;
    let imageInfo: image.ImageInfo | null = null;
    try {
      // 根据filePath获取
      imageInfo = await this.getImageInfoFromFile(filePath);
      if (imageInfo) {
        let height = imageInfo.size.height;
        let width = imageInfo.size.width;
        if (height * width <= MIN_RESOLUTION) {
          blobSource = 1;
        }
      }
      return blobSource;
    } catch (e) {
      LogUtils.e(tagStr, `err,msg:${e.message}`);
      return blobSource;
    }
  }

  async getImageInfoFromFile(filePath: string): Promise<image.ImageInfo | null> {
    let imageSource: image.ImageSource | null = null;
    let imageInfo: image.ImageInfo | null = null;
    try {
      imageSource = image.createImageSource(filePath);
      if (imageSource !== undefined) {
        imageInfo = await imageSource.getImageInfo();
      } else {
        LogUtils.w(TAG, 'getImageInfoFromFile createImageSource filePath result empty');
      }
    } catch (error) {
      LogUtils.e(TAG, `getImageInfoFromFile err,msg:${error.message}`);
    } finally {
      imageSource?.release().catch((error: BusinessError) => {
        LogUtils.e(TAG, `getImageInfoFromFile release resource error: ${error?.message}`);
      });
    }
    return imageInfo;
  }

  public async getDataAbilityHelper() {
    // 由于ArkTS类型推断，此处在Promise中调用callback获取dataShareHelper实例
    if (this.dataShareHelper === undefined) {
      this.dataShareHelper = await new Promise<dataShare.DataShareHelper>((resolve) => {
        dataShare.createDataShareHelper(this.context!, Contacts.CONTENT_URI, (err, dataShareHelper) => {
          resolve(dataShareHelper);
        });
      })
    }
    return this.dataShareHelper;
  }

  public async closeDataAbilityHelper() {
    await this.dataShareHelper?.close();
    this.dataShareHelper = undefined;
  }

  public async closeCalllogDataAbilityHelper() {
    await this.callLogDataShareHelper?.close();
    this.callLogDataShareHelper = undefined;
  }

  public getName(contactInfo: ContactInfo): DisPlayNameResult {
    let result: DisPlayNameResult = new DisPlayNameResult();
    let displayName: string = '';
    let updateFlag: number = 0;

    if (contactInfo.displayName != undefined && contactInfo.displayName.length > 0) {
      displayName = contactInfo.displayName;
      updateFlag = 0;
    } else if (contactInfo.nicknameList != null && contactInfo.nicknameList.length > 0) {
      displayName = contactInfo.nicknameList[0]?.nickName as string;
      updateFlag = 1;
    } else if (contactInfo.company != null && contactInfo.company != '') {
      displayName = contactInfo.company;
      updateFlag = 1;
    } else if (contactInfo.position != null && contactInfo.position != '') {
      displayName = contactInfo.position;
      updateFlag = 1;
    } else if (contactInfo.phoneList != null && contactInfo.phoneList.length > 0) {
      displayName = contactInfo.phoneList[0].phoneNumber as string;
      updateFlag = 1;
    } else if (contactInfo.emails != null && contactInfo.emails.length > 0) {
      displayName = contactInfo.emails[0].email as string;
      updateFlag = 1;
    }

    result.displayName = displayName;
    result.updateFlag = updateFlag;
    return result;
  }

  // 升级后触发一次全量智能合并
  public async checkAggregationContact() {
    if (this.checkingCursor) {
      setTimeout(async () => {
        LogUtils.w(TAG, 'checkAggregationContact: timeout retry.');
        await this.checkAggregationContact();
      }, TIMEOUT_AGGREGATION_ALL_ONCE);
    }
    if (!this.checkingCursor) {
      await ContactAggregator.getInstance().aggregateAll(this.context!, true);
    } else {
      LogUtils.w(TAG, 'checkAggregationContact: is syncing, dont do aggregation.');
    }
  }
  // 重复克隆删除卡片的formId
  public async deleteContactFormId(formID: string) {
    LogUtils.i(TAG, 'deleteContactFormId start!');
    let ruleSet: relationalStore.ResultSet | null | undefined = null;
    try {
      const initDbSuc = await contactRdbHelper.initDB(this.context!);
      if (!initDbSuc) {
        throw new Error(`deleteContactFormId initDb fail`);
      }
      const beginTransSuc = await contactRdbHelper.beginTransaction();
      if (!beginTransSuc) {
        throw new Error(`deleteContactFormId beginTrans fail`);
      }
      const delPredicates = new rdb.RdbPredicates(DbConstants.CLONE_NEXT2NEXT_CONTACTS_TABLE_NAME);
      delPredicates.contains('form_id', formID);
      ruleSet = await contactRdbHelper.queryData(DbConstants.CLONE_NEXT2NEXT_CONTACTS_TABLE_NAME,
        delPredicates, ['form_id', 'id']);
      if (!ruleSet || !ruleSet.goToFirstRow()) {
        LogUtils.e(TAG, 'deleteContactFormId getAllContactData not found.');
        contactRdbHelper.rollBack();
        return
      }
      do {
        let originFormID = ruleSet?.getString(ruleSet.getColumnIndex('form_id'));
        let delUpdateID = ruleSet?.getString(ruleSet.getColumnIndex('id'))
        let parts: string[] | undefined = originFormID?.split('//');
        let filteredParts: string[] | undefined = parts?.filter(part =>!part.startsWith(formID));
        let result: string | undefined = filteredParts?.join('//');
        LogUtils.i(TAG,
          'deleteContactFormId originFormID: ' + originFormID + ' delUpdateID: ' + delUpdateID + ' result: ' + result);
        let updateBucket: ValuesBucket = {
          'form_id': result + ''
        }
        const delUpdatePredicates = new rdb.RdbPredicates(DbConstants.CLONE_NEXT2NEXT_CONTACTS_TABLE_NAME);
        delUpdatePredicates.equalTo('id', delUpdateID);
        let delRet = await contactRdbHelper.updateData(updateBucket, delUpdatePredicates);
        LogUtils.i(TAG, 'deleteContactFormId delete success: ' + delRet);
        if (delRet < 0) {
          throw new Error(`deleteContactFormId fail, ret:${delRet}`);
        }
      } while (ruleSet.goToNextRow());
      await contactRdbHelper.commit();
    } catch (e) {
      LogUtils.e(TAG, `deleteContactFormId fail err,msg:${e.message},stack:${e.stack}`);
      await contactRdbHelper.rollBack();
    } finally {
      if (ruleSet && !ruleSet.isClosed) {
        ruleSet.close();
      }
    }
  }

  public extractNumberBeforeAsterisk(input: string): string {
    const asteriskIndex = input.indexOf('*');
    if (asteriskIndex !== -1) {
      return input.substring(0, asteriskIndex);
    } else {
      return input;
    }
  }

  /**
   * 本地恢复联系人
   * @param localRestoreRawIdStr 恢复联系人的id
   */
  public async localRestoreContact(localRestoreRawIdStr: string) {
    const localRestoreRawIds = localRestoreRawIdStr?.split(',').map((item: string) => Number(item));
    if (!localRestoreRawIds.length) {
      LogUtils.e(TAG, `localRestoreContact err args:${localRestoreRawIdStr}`);
      return;
    }

    const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
    predicates.in(RawContactsColumns.ID, localRestoreRawIds);
    // 查询恢复联系人raw信息
    const restoreRawDataList = await this.getRawContactArr(predicates);
    await this.restoreContact(restoreRawDataList, true, -1);
  }

  private async getRawContactArr(predicates: rdb.RdbPredicates): Promise<MergeRawContact[]> {
    const rawContactArr: MergeRawContact[] = [];

    let ruleSet: relationalStore.ResultSet | null | undefined = null;
    try {
      ruleSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, predicates,
        MergeRawContact.columns);
      if (!ruleSet || !ruleSet.goToFirstRow()) {
        return rawContactArr;
      }
      do {
        const mergeRawContact: MergeRawContact = MergeRawContact.fromRuleSet(ruleSet);
        rawContactArr.push(mergeRawContact);
      } while (ruleSet.goToNextRow());
    } finally {
      if (ruleSet && !ruleSet.isClosed) {
        ruleSet.close();
      }
    }
    return rawContactArr;
  }

  /**
   * 恢复联系人：（1）应用执行从回收站恢复，c++测删除回收站表数据，后续处理执行到这里(2)云同步下行恢复联系人
   * @param restoreRawDataList
   * @param isLocalRestore 是否本地恢复联系人
   * @param maxCursor
   */
  public async restoreContact(restoreRawDataList: MergeRawContact[], isLocalRestore: boolean, maxCursor: number) {
    const restoreRawIds: number[] = [];
    const restoreContactIds: number[] = [];
    const restoreContacts: MergeRawContact[] = [];
    const displayNameList: string[] = [];
    const logTag = isLocalRestore ? 'localRestoreContact' : 'cloudRestoreContact';
    for (const rawContact of restoreRawDataList) {
      // 否则进行恢复
      restoreRawIds.push(rawContact.rawId);
      restoreContactIds.push(rawContact.contactId);
      restoreContacts.push(rawContact);
      displayNameList.push(rawContact.displayName);
    }
    let restoreSuccessFlag = false;
    try {
      const beginTransSuc = await contactRdbHelper.beginTransaction();
      if (!beginTransSuc) {
        LogUtils.e(logTag, `beginTransaction fail, return`);
        return;
      }
      if (restoreRawIds.length) {
        // 更新raw表is_deleted字段
        const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
        predicates.in(RawContactsColumns.ID, restoreRawIds);
        const updateBucket: ValuesBucket = {};
        updateBucket[RawContactsColumns.IS_DELETED] = 0;

        const updateRet = await contactRdbHelper.updateData(updateBucket, predicates);
        LogUtils.w(logTag, `update raw table is_deleted Ret: ${updateRet}, restoreRawIds:${restoreRawIds}`);

        // 删除最近删除表中对应数据
        // TODO 本地删除， c++侧也删除了delete raw contact表，可能c++侧删除了，js删除操作失败，联系人没了
        const delPredicates = new rdb.RdbPredicates(DbConstants.NEW_DELETE_RAW_CONTACT_TABLE_NAME);
        delPredicates.in(DeleteRawContact.RAW_CONTACT_ID, restoreRawIds);
        const delRet = await contactRdbHelper.deleteData(delPredicates);
        LogUtils.w(logTag, `del delete table Ret: ${delRet}, restoreRawIdList:${restoreRawIds}`);
      }
      // 恢复的联系人，更新时间戳信息
      if (restoreContactIds.length > 0) {
        const predicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
        predicates.in(ContactsColumns.NEW_ID, restoreContactIds);
        const updateBucket: ValuesBucket = {};
        updateBucket[ContactsColumns.CONTACT_LAST_UPDATED_TIMESTAMP] = Date.now();
        const updateRet = await contactRdbHelper.updateData(updateBucket, predicates);
        LogUtils.w(logTag, `restoreUpdateTimestamp Ret: ${updateRet}, restoreNum:${restoreContactIds.length}`);
      }

      restoreSuccessFlag = true;
      await contactRdbHelper.commit();

      if (restoreContacts.length) {
        await this.refreshCalllogWhenRestoreContacts(restoreContacts, isLocalRestore);
      }

      // 添加生日到日历
      if (restoreRawIds.length) {
        for (let index = 0; index < restoreRawIds.length; index++) {
          await BirthdayManager.getInstance().handleSingleBirthday(restoreRawIds[index].toString(), true);
        }
      }

    } catch (e) {
      await contactRdbHelper.rollBack();
      LogUtils.e(logTag, `catch err:${e.message},stack:${e.messstackage}`);
    }

    // 触发智能合并
    if (displayNameList && displayNameList.length > 0) {
      if (isLocalRestore) {
        await ContactAggregator.getInstance().aggregateAll(this.context!, true, undefined, displayNameList);
      }
    }
  }

  public async refreshCalllogWhenRestoreContacts(restoreContacts: MergeRawContact[], needNotify: boolean) {
    try {
      for (const rawContact of restoreContacts) {
        const predicates = new rdb.RdbPredicates(DbConstants.NEW_DATA_TABLE_NAME);
        predicates.equalTo(DataColumns.RAW_CONTACT_ID, rawContact.rawId);
        predicates.and();
        predicates.equalTo(DataColumns.TYPE_ID, DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE);

        const extraInfo: ExtraInfo = {
          deletePhoneList: [],
          deleteFormatPhoneList: [],
          addPhoneList: [],
          addFormatPhoneList: [],
        };
        let ruleSet: relationalStore.ResultSet | null | undefined = null;
        try {
          ruleSet = await contactRdbHelper.queryData(DbConstants.NEW_DATA_TABLE_NAME, predicates,
            [DataColumns.DETAIL_INFO, DataColumns.FORMAT_PHONE_NUMBER]);
          if (!ruleSet || !ruleSet.goToFirstRow()) {
            continue;
          }
          do {
            const detailInfo = ruleSet.getString(0);
            const formatPhoneNumber = ruleSet.getString(1);
            if (formatPhoneNumber) {
              extraInfo.addFormatPhoneList.push(formatPhoneNumber);
            } else if (detailInfo) {
              const formatPhoneNumber = getFormatNumber(detailInfo);
              if (formatPhoneNumber) {
                extraInfo.addFormatPhoneList.push(formatPhoneNumber);
              } else {
                extraInfo.addPhoneList.push(detailInfo);
              }
            }
          } while (ruleSet.goToNextRow());
        } finally {
          if (ruleSet && !ruleSet.isClosed) {
            ruleSet.close();
          }
        }
        await this.refreshCallLog(rawContact.contactId, rawContact.displayName, extraInfo, false);
      }
      if (needNotify) {
        const dataShareHelper: dataShare.DataShareHelper | undefined = await this.getDataAbilityHelper();
        await dataShareHelper?.notifyChange(Data.CONTENT_URI);

        const callLogDataShareHelper: dataShare.DataShareHelper | undefined = await this
          .getCallLogDataAbilityHelper();
        await callLogDataShareHelper?.notifyChange(CallLogSingleConstant.CALL_LOG_URI);
      }
    } catch (e) {
      LogUtils.e(TAG, `refreshCalllogWhenRestoreContacts err:${e.message},stack:${e.messstackage}`);
    }
  }

  /**
   * 对于软删除的，
   * 对于恢复的联系人
   * 对于新增、更新的联系人
   * 为啥不能通过变动的号码来定向刷新（用户可能有2个相同号码，删了一个还有一个）
   */
  public async refreshCallLog(contactId: number, displayName: string,
    extraInfo: ExtraInfo, isDelete: boolean, context?: common.Context) {
    try {
      await callLogRdbHelper.initDB(context ? context : this.context!);
      if (isDelete) {
        await callLogRdbHelper.updateCallLogWhenDeleteContact(contactId);
        return;
      }
      if (extraInfo.deletePhoneList?.length) {
        await callLogRdbHelper.updateCallLogWhenContactDeletePhone(contactId, displayName,
          extraInfo.deletePhoneList, false);
      }
      if (extraInfo.deleteFormatPhoneList?.length) {
        await callLogRdbHelper.updateCallLogWhenContactDeletePhone(contactId, displayName,
          extraInfo.deleteFormatPhoneList, true);
      }
      // 新增电话号码，处理通话记录
      if (extraInfo.addPhoneList?.length) {
        await callLogRdbHelper.updateCallLogWhenContactAddPhone(contactId, displayName,
          extraInfo.addPhoneList, false);
      }
      if (extraInfo.addFormatPhoneList?.length) {
        await callLogRdbHelper.updateCallLogWhenContactAddPhone(contactId, displayName,
          extraInfo.addFormatPhoneList, true);
      }
    } catch (e) {
      LogUtils.e(TAG, `refreshCallLog err,msg:${e.message || e.code}`);
    }
  }
}

