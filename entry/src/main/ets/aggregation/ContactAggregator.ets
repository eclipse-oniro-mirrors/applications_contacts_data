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
import { AggregatorRawContact } from './AggregatorRawContact';
import { common } from '@kit.AbilityKit';
import LogUtils from '../common/utils/LogUtils';
import RawContactsColumns from '../common/data/RawContactsColumns';
import rdb from '@ohos.data.relationalStore';
import DataColumns from '../common/data/DataColumns';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import dataShare from '@ohos.data.dataShare';
import ContactsDataService from '../serivce/ContactsDataService';
import { Data } from '../common/data/Data';
import RawContact from '../common/data/vo/RawContact';
import relationalStore from '@ohos.data.relationalStore';
import { DataItemType } from '../common/data/vo/DataItemType';
import HandleUniqueKeyService from '../serivce/HandleUniqueKeyService';
import { getFormatNumber } from '../common/utils/NumberFormatUtil';
import { StringUtil } from '../common/utils/StringUtil';
import SimpleContact from '../common/data/SimpleContact';
import BirthdayManager from '../serivce/BirthdayManager';
import { contactRdbHelper } from '../common/helperUtils/ContactsRdbHelperForCloudSync';
import { callLogRdbHelper } from '../common/helperUtils/CallLogRdbHelperForCloudSync';
import DbConstants, { IsMyCard } from '../common/DbConstants';
import ContactsColumns from '../common/contract/ContactsColumns';
import { DeleteRawContact } from '../common/contract/DeleteRawContact';
import DatabaseUtils from '../common/utils/DatabaseUtils';
import { MigrateProgressInfo } from '../common/model/MigrateProgressInfo';
import CallLogSingleConstant from '../common/contract/CallLogSingleConstant';

const TAG = 'ContactAggregator';
const PAGE_SIZE_RAW: number = 500;
const NUM_50: number = 50;
const WAIT_QUERY: number = 200; // 构建view需等待，避免查询失败
const formIdSeparator = '//';
const EVENT_TYPE_ID: number = 11;
const BIRTHDAY_TYPE: string[] = ['1', '2', '1.0', '2.0'];
const BIRTHDAY_NOT_SYNC: number = 1;
const MAX_COMPOUND_SELECT: number = 500;
export default class ContactAggregator {
  public static sInstance: ContactAggregator;
  private isAggregationContact: boolean = false;

  public static getInstance() {
    if (ContactAggregator.sInstance == null) {
      ContactAggregator.sInstance = new ContactAggregator();
    }
    return ContactAggregator.sInstance;
  }

  // 执行全量智能合并，执行成功返回合并的联系人数量，失败返回-1
  public async aggregateAll(context: common.Context, needSendChangeNotify: boolean,
    migrateProgressInfo?: MigrateProgressInfo, displayNameList?: Array<string>): Promise<number> {
    LogUtils.w(TAG, `aggregateAll: start${displayNameList ? `, displayNameList length:${displayNameList.length}` : ''}`);
    if (this.isAggregationContact) {
      LogUtils.w(TAG, `aggregateAll: is aggregating!`);
      return -1;
    }
    this.isAggregationContact = true;
    let startTime = Date.now();
    const rdbStore = await contactRdbHelper.initDB(context);
    if (!rdbStore) {
      LogUtils.e(TAG, `aggregateAll: rdbStore init error!`);
      this.isAggregationContact = false;
      return -1;
    }
    if (migrateProgressInfo) {
      migrateProgressInfo.updateIsAggregatorAll(true);
    }
    // 查询view_contact_data，获取智能合并所需的信息
    let contactDataMap: Map<number, AggregatorRawContact> = await this.queryData(context, displayNameList);
    LogUtils.w(TAG, `aggregateAll: contactDataMap size:${contactDataMap.size}`);
    // 将查询结果按姓名、rawContact数组存放进map
    let aggregatorMap: Map<string, AggregatorRawContact[]> = new Map();
    for (let rawContact of contactDataMap.values()) {
      let displayName = rawContact.displayName;
      if (aggregatorMap.get(displayName) != null) {
        aggregatorMap.get(displayName)?.push(rawContact);
      } else {
        aggregatorMap.set(displayName, [rawContact]);
      }
    }
    //克隆后进行全量智能合并，可能会造成超时失败，将克隆进度进行计算返回
    if (migrateProgressInfo) {
      if (aggregatorMap.size > 0) {
        migrateProgressInfo.updateAggregatorContactCount(aggregatorMap.size);
      } else {
        // 联系人克隆进度智能合并count默认为1，如果查询联系人为0，更新智能合并进度为1
        migrateProgressInfo.updateAggregatorProcessCount(1);
      }
    }
    let aggregatorProcess = 0;
    let aggregationCount = 0;
    // 遍历上述map，针对每个displayName下的raw数据筛选出满足智能合并条件的联系人
    for (let similarArray of aggregatorMap.values()) {
      let needAggregateMap: Map<number, AggregatorRawContact[]> = new Map();
      // 只有存在两个及以上同名的联系人才可能存在相似联系人
      if (similarArray.length >= 2 && this.hasDifferentContactId(similarArray)) {
        needAggregateMap = this.filterNeedAggregate(similarArray);
        // 对同姓名的每组数据分别执行智能合并动作
        for (let aggregatorArray of needAggregateMap.values()) {
          aggregationCount += await this.executeAggregation(context, aggregatorArray, migrateProgressInfo);
          // 每智能合并 50 组联系人，休眠 100 ms，避免长期占用 CPU，进程被管控
          if (aggregationCount % NUM_50 == 0) {
            await DatabaseUtils.delay(NUM_50 * 2);
          }
        }
      }
      if (migrateProgressInfo) {
        aggregatorProcess++;
        migrateProgressInfo.updateAggregatorProcessCount(aggregatorProcess);
      }
    }

    // 如果有联系人被合并，则发送数据变更通知，触发界面刷新
    if (aggregationCount > 0) {
      if (needSendChangeNotify) {
        // 发送数据变更通知
        const dataShareHelper: dataShare.DataShareHelper | undefined =
          await HandleUniqueKeyService.getInstance().getMigrateDataAbilityHelper(context);
        await dataShareHelper?.notifyChange(Data.CONTENT_URI);
        await HandleUniqueKeyService.getInstance().closeMigrateDataAbilityHelper();
        await HandleUniqueKeyService.getInstance().notifyFormIdChange(context);

        // 发送通话记录变更通知
        const callLogDataShareHelper: dataShare.DataShareHelper | undefined = await ContactsDataService.getInstance()
            .getCallLogDataAbilityHelper(context);
        await callLogDataShareHelper?.notifyChange(CallLogSingleConstant.CALL_LOG_URI);
        await ContactsDataService.getInstance().closeCalllogDataAbilityHelper();
        LogUtils.w(TAG, `aggregateAll: send data change notify.`);
      }
    }
    this.isAggregationContact = false;
    LogUtils.w(TAG, `aggregateAll: end cost:${Date.now() - startTime}`);
    return aggregationCount;
  }

  private hasDifferentContactId(similarArray: AggregatorRawContact[]): boolean {
    let contactId = similarArray[0].contactId;
    for (const rawContact of similarArray) {
      if (rawContact.contactId != contactId) {
        return true;
      }
    }
    return false;
  }

  // 第一步：查找contact一对多的rawId记录
  // 第二步：对每个contactId对应的rawId构造contact表数据，插入新的contact表记录
  // 第三步：将新产生的contactId更新到raw表对应的记录
  /*
   * 不传参数时对全量数据做合并拆分
   * @param needSplitContactIdList: 云同步下行删除raw记录时，需将该条raw记录从被合并状态拆分开
   * @param rawIdListStr：需要拆分的raw列表(1、用户手动删除智能合并联系人  2、删除交集号码)
   * @param isFromUserDel：用户手动删除智能合并联系人场景标识
   */
  public async doSplitAggregationContactAfterDel(rawIdListStr?: string | null) {
    let startTime = Date.now();
    LogUtils.w(TAG, `doSplitAggregationContactAfterDel start:${startTime}`);

    let rawIdList: number[] = [];
    if (rawIdListStr != null) {
      rawIdList = rawIdListStr.split(',').map((item: string) => Number(item));
    }

    if (!rawIdList || rawIdList.length <= 0) {
      LogUtils.w(TAG, `doSplitAggregationContactAfterDel end: id list null, no need split.`);
      return;
    }

    let contactIdSet: Set<number> = new Set();
    // 查找被删除的全部rawId记录
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
    predicates.in(RawContactsColumns.ID, rawIdList);

    let rawContactList: Map<number, RawContact[]> = new Map();
    let resultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      resultSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, predicates,
      [RawContactsColumns.ID, RawContactsColumns.CONTACT_ID, RawContactsColumns.DISPLAY_NAME,
      RawContactsColumns.PHOTO_FILE_ID, RawContactsColumns.READ_ONLY,
      RawContactsColumns.PHOTO_ID, RawContactsColumns.IS_TRANSFER_VOICEMAIL, RawContactsColumns.COMPANY,
      RawContactsColumns.POSITION]);
      if (!resultSet || !resultSet.goToFirstRow()) {
        LogUtils.e(TAG, 'doSplitAggregationContactAfterDel: no raw contact.');
        return;
      }
      do {
        let contactId = resultSet?.getLong(resultSet.getColumnIndex(RawContactsColumns.CONTACT_ID));
        let rawContact = RawContact.fromResultSet(resultSet!);
        contactIdSet.add(contactId!);
        if (rawContactList.get(contactId!) != null) {
          rawContactList.get(contactId!)!.push(rawContact);
        } else {
          rawContactList.set(contactId!, [rawContact]);
        }
      } while (resultSet.goToNextRow());
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    LogUtils.w(TAG, `doSplitAggregationContactAfterDel rawContactList size:${rawContactList.size}, contactIdSet size:${contactIdSet.size}`);

    let contactMap: Map<number, SimpleContact> = new Map();
    if (contactIdSet.size > 0) {
      let contactIdList = Array.from(contactIdSet);
      const contactPredicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
      contactPredicates.in(ContactsColumns.NEW_ID, contactIdList);

      let result: relationalStore.ResultSet | null | undefined = null;
      try {
        result = await contactRdbHelper.queryData(DbConstants.NEW_CONTACTS_TABLE_NAME, contactPredicates,
          SimpleContact.columns);
        if (!result || !result.goToFirstRow()) {
          LogUtils.e(TAG, 'doSplitAggregationContactAfterDel: no contact.');
          return;
        }
        do {
          const simpleContact = SimpleContact.fromResultSet(result);
          contactMap.set(simpleContact.id, simpleContact);
        } while (result.goToNextRow());
      } finally {
        if (result && !result.isClosed) {
          result.close();
        }
      }
    }

    let splitCount: number = 0;
    // 对每个contactId对应的rawId构造contact表数据，插入新的contact表记录
    for (let contactId of rawContactList.keys()) {
      let rawList = rawContactList.get(contactId);
      // 如果没有多个raw表记录对应这个contactid，则直接返回，不需要拆分
      if (rawList!.length > 1) {
        LogUtils.w(TAG, `doSplitAggregationContactAfterDel contactId:${contactId}, masterRawId:${contactMap.get(contactId)!.nameRawId}, rawList:${rawList?.map((raw => raw.id)).join(',')}`);
        splitCount += await this.doRawSplit(rawList!, contactMap.get(contactId)!, true, null);
      }
    }
    LogUtils.w(TAG, `doSplitAggregationContact end cost:${Date.now() - startTime}, splitCount:${splitCount}`);
  }

  // 云同步下行删除的raw记录需要拆分
  // 情况1：智能合并联系人下的所有raw都被删除，可以直接全部拆分掉
  // 情况2：智能合并联系人下的非name_raw被删除，可以直接拆分
  // 情况3：智能合并联系人下的name_raw被删除，拆分时，需要找到合并联系人的其他raw（小的），将rawID更新到contact记录的name_raw字段；再进行拆分
  public async doSplitDeletedRaw(needSplitRawIdList?: number[] | null) {
    let startTime = Date.now();
    LogUtils.w(TAG, `doSplitDeletedRaw start:${startTime}`);

    if (needSplitRawIdList == null || needSplitRawIdList.length <= 0) {
      LogUtils.w(TAG, `doSplitDeletedRaw end: id list null, no need split.`);
      return;
    }

    let contactIdSet: Set<number> = new Set();
    // 查找被删除的全部rawId记录；根据删除的rawId查询
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
    predicates.in(RawContactsColumns.ID, needSplitRawIdList);

    // {contactId, rawContact[]} 的map，没排序
    let splitRawContactList: Map<number, RawContact[]> = new Map();

    let resultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      resultSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, predicates,
        [RawContactsColumns.ID, RawContactsColumns.CONTACT_ID, RawContactsColumns.DISPLAY_NAME,
          RawContactsColumns.PHOTO_FILE_ID, RawContactsColumns.READ_ONLY,
          RawContactsColumns.PHOTO_ID, RawContactsColumns.IS_TRANSFER_VOICEMAIL, RawContactsColumns.COMPANY,
          RawContactsColumns.POSITION]);
      if (!resultSet || !resultSet.goToFirstRow()) {
        LogUtils.e(TAG, 'doSplitDeletedRaw: no raw contact.');
        return;
      }
      do {
        let contactId = resultSet?.getLong(resultSet.getColumnIndex(RawContactsColumns.CONTACT_ID));
        let rawContact = RawContact.fromResultSet(resultSet!);
        contactIdSet.add(contactId!);
        if (splitRawContactList.get(contactId!) != null) {
          splitRawContactList.get(contactId!)!.push(rawContact);
        } else {
          splitRawContactList.set(contactId!, [rawContact]);
        }
      } while (resultSet.goToNextRow());
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    LogUtils.w(TAG, `doSplitDeletedRaw rawContactList size:${splitRawContactList.size}, contactIdSet size:${contactIdSet.size}`);

    let contactMap: Map<number, SimpleContact> = new Map();
    if (contactIdSet.size > 0) {
      const contactPredicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
      contactPredicates.in(ContactsColumns.NEW_ID, Array.from(contactIdSet));

      let result: relationalStore.ResultSet | null | undefined = null;
      try {
        result = await contactRdbHelper.queryData(DbConstants.NEW_CONTACTS_TABLE_NAME, contactPredicates,
          SimpleContact.columns);
        if (!result || !result.goToFirstRow()) {
          LogUtils.e(TAG, 'doSplitDeletedRaw: no contact.');
          return;
        }
        do {
          const simpleContact = SimpleContact.fromResultSet(result);
          contactMap.set(simpleContact.id, simpleContact);
        } while (result.goToNextRow());
      } finally {
        if (result && !result.isClosed) {
          result.close();
        }
      }
    }

    // {contactId，rawIdSet}的map，根据删除的contactId查询
    let rawContactMap: Map<number, Set<number>> = new Map();
    // 如果需要切换name raw id，则选取一个新的rawId，更新到raw表
    const rawPredicate: relationalStore.RdbPredicates =
      new relationalStore.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
    rawPredicate.in(RawContactsColumns.CONTACT_ID, Array.from(contactIdSet));
    rawPredicate.orderByAsc(RawContactsColumns.ID);
    let result: relationalStore.ResultSet | null | undefined = null;
    try {
      result = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, rawPredicate,
        [RawContactsColumns.ID, RawContactsColumns.CONTACT_ID]);
      if (!result || !result.goToFirstRow()) {
        LogUtils.e(TAG, 'doSplitDeletedRaw: query raw contact fail.');
        return;
      }
      do {
        let rawId = result.getLong(result.getColumnIndex(RawContactsColumns.ID));
        let contactId = result.getLong(result.getColumnIndex(RawContactsColumns.CONTACT_ID));
        if (rawContactMap.get(contactId!) != null) {
          rawContactMap.get(contactId!)!.add(rawId);
        } else {
          let set: Set<number> = new Set();
          set.add(rawId);
          rawContactMap.set(contactId!, set);
        }
      } while (result.goToNextRow());
    } finally {
      if (result && !result.isClosed) {
        result.close();
      }
    }

    let splitCount: number = 0;
    // 对每个contactId对应的rawId构造contact表数据，插入新的contact表记录
    for (let contactId of splitRawContactList.keys()) {
      let splitRawList = splitRawContactList.get(contactId);
      // 如果没有多个raw表记录对应这个contactid，则直接返回，不需要拆分
      LogUtils.w(TAG, `doSplitDeletedRaw contactId:${contactId}, masterRawId:${contactMap.get(contactId)!.nameRawId}, rawList:${splitRawList?.map((raw => raw.id)).join(',')}`);
      splitCount += await this.doRawSplit(splitRawList!, contactMap.get(contactId)!, true,
      rawContactMap.get(contactId)!);
    }
    LogUtils.w(TAG, `doSplitDeletedRaw end cost:${Date.now() - startTime}, splitCount:${splitCount}`);
  }

  /**
   *
   * @param needSplitRaw 删除的raw对象集合
   * @param masterContact 原始contact记录
   * @param isFromDelete 是否删除
   * @param rawContactSet 删除的rawId集合
   * @returns
   */
  private async doRawSplit(needSplitRaw: RawContact[], masterContact: SimpleContact, isFromDelete: boolean,
    rawContactSet: Set<number> | null): Promise<number> {
    let splitCount = 0;
    for (let rawContact of needSplitRaw) {
      let needChangeNameRawId = false;
      if (rawContact.id == masterContact.nameRawId) {
        LogUtils.w(TAG, `doRawSplit master rawID:${masterContact.nameRawId}`);
        // 智能合并联系人下的name_raw被删除，拆分时，需要找到合并联系人的其他raw（小的），将rawID更新到contact记录的name_raw字段；再进行拆分
        if (rawContactSet) {
          needChangeNameRawId = true;
          rawContactSet?.delete(masterContact.nameRawId);
          if (rawContactSet.size == 0) {
            LogUtils.w(TAG, `doRawSplit no more raw need split.`);
            // 如果待拆分的raw已经是主raw，并且对应的contactid下已经没有了其他raw，则拆分完毕，更新raw表aggregation_status为0
            const predicate: relationalStore.RdbPredicates =
              new relationalStore.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
            predicate.equalTo(RawContactsColumns.ID, rawContact.id);
            const updateValue: ValuesBucket = {};
            updateValue[RawContactsColumns.AGGREGATION_STATUS] = 0;
            await contactRdbHelper.updateData(updateValue, predicate);
            return splitCount;
          }
        } else {
          continue;
        }
      }

      let hasPhoneNumber = false;
      let hasEmail = false;
      let hasGroup = false;
      let hasName = false;
      // handle 查询raw是否有号码、邮箱、群组、姓名
      const dataPredicates = new rdb.RdbPredicates(DbConstants.NEW_DATA_TABLE_NAME);
      dataPredicates.equalTo(DataColumns.RAW_CONTACT_ID, rawContact.id);
      dataPredicates.in(DataColumns.TYPE_ID, [DataItemType.PHONE, DataItemType.EMAIL, DataItemType.GROUP_MEMBERSHIP,
        DataItemType.NAME]);
      let result: relationalStore.ResultSet | null | undefined = null;
      try {
        result = await contactRdbHelper.queryData(DbConstants.NEW_DATA_TABLE_NAME, dataPredicates,
          [DataColumns.DETAIL_INFO, DataColumns.TYPE_ID]);
        if (!result || !result.goToFirstRow()) {
          continue;
        }
        do {
          let typeId = result.getLong(result.getColumnIndex(DataColumns.TYPE_ID));
          switch (typeId) {
            case DataItemType.PHONE:
              hasPhoneNumber = true;
              break;
            case DataItemType.EMAIL:
              hasEmail = true;
              break;
            case DataItemType.GROUP_MEMBERSHIP:
              hasGroup = true;
              break;
            case DataItemType.NAME:
              hasName = true;
              break;
            default:
              break;
          }
        } while (result.goToNextRow());
      } finally {
        if (result && !result.isClosed) {
          result.close();
        }
      }

      try {
        // 构造插入contact表的valueBucket
        let contactValues: ValuesBucket = this.buildContatctInsertValues(rawContact, hasPhoneNumber, hasEmail, hasGroup,
          hasName, masterContact);

        // 开启事务
        const beginTransSuc = await contactRdbHelper.beginTransaction();
        if (!beginTransSuc) {
          LogUtils.e(TAG, `doRawSplit beginTransaction fail, return`);
          return splitCount;
        }

        // 插入contact表
        const contactId = await contactRdbHelper.insertData(contactValues, DbConstants.NEW_CONTACTS_TABLE_NAME);
        if (contactId < 0) {
          throw new Error(`doRawSplit insert contact fail, ret:${contactId}`);
        }

        // 更新raw表，将raw的contactId更新为新插入的
        const predicate: relationalStore.RdbPredicates =
          new relationalStore.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
        predicate.equalTo(RawContactsColumns.ID, rawContact.id);
        const updateValue: ValuesBucket = {};
        updateValue[RawContactsColumns.CONTACT_ID] = contactId;
        updateValue[RawContactsColumns.AGGREGATION_STATUS] = 0;
        let rdbRet = await contactRdbHelper.updateData(updateValue, predicate);
        if (rdbRet < 0) {
          throw new Error(`doRawSplit update raw contactId fail, ret:${rdbRet}`);
        }

        // 如果需要切换name raw，则需要更新contact表的name_raw
        if (needChangeNameRawId && rawContactSet!.size >= 1) {
          let nameRawId = -1;
          // 切换contact表的nameRawContactId
          for (let rawId of rawContactSet?.values()!) {
            nameRawId = rawId;
            LogUtils.i(TAG, `nameRawId ${nameRawId}`);
            break;
          }
          if (masterContact.id && nameRawId != -1) {
            const nameRawUpatepredicate: relationalStore.RdbPredicates =
              new relationalStore.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
            nameRawUpatepredicate.equalTo(ContactsColumns.NEW_ID, masterContact.id);
            const updateValue: ValuesBucket = {};
            updateValue[ContactsColumns.NAME_RAW_CONTACT_ID] = nameRawId;
            let rdbRet = await contactRdbHelper.updateData(updateValue, nameRawUpatepredicate);
            if (rdbRet < 0) {
              throw new Error(`doRawSplit update name raw id fail, ret:${rdbRet}`);
            }
            // 赋值新的master
            masterContact.nameRawId = nameRawId;
          }
        }
        rawContactSet?.delete(rawContact.id);

        // 如果是删除后拆分，则需要更新delete表contactId
        if (isFromDelete) {
          const predicate: relationalStore.RdbPredicates =
            new relationalStore.RdbPredicates(DbConstants.NEW_DELETE_RAW_CONTACT_TABLE_NAME);
          predicate.equalTo(DeleteRawContact.RAW_CONTACT_ID, rawContact.id);
          const updateValue: ValuesBucket = {
            'contact_id': contactId
          }
          let rdbRet = await contactRdbHelper.updateData(updateValue, predicate);
          if (rdbRet < 0) {
            throw new Error(`doRawSplit update raw contactId fail, ret:${rdbRet}`);
          }
        }

        // 提交事务
        await contactRdbHelper.commit();
        splitCount++;
      } catch (error) {
        LogUtils.e(TAG, 'doRawSplit error:%s' + error?.message + ', stack: ' + error?.stack);
        await contactRdbHelper.rollBack();
      }
    }
    return splitCount;
  }

  /**
   * 构造插入contact表的信息
   * @param rawContact
   * @param hasPhoneNumber
   * @param hasEmail
   * @param hasGroup
   * @param hasName
   * @returns
   */
  private buildContatctInsertValues(rawContact: RawContact, hasPhoneNumber: boolean, hasEmail: boolean,
    hasGroup: boolean, hasName: boolean, masterContact: SimpleContact): ValuesBucket {
    let insertContactValues: ValuesBucket = {};
    insertContactValues[ContactsColumns.NAME_RAW_CONTACT_ID] = rawContact.id;
    let company = rawContact.values.get(RawContactsColumns.COMPANY);
    let position = rawContact.values.get(RawContactsColumns.POSITION);
    let photoFileId = rawContact.values.get(RawContactsColumns.PHOTO_FILE_ID);
    let photoId = rawContact.values.get(RawContactsColumns.PHOTO_ID);
    let readOnly = rawContact.values.get(RawContactsColumns.READ_ONLY);
    let isTransferVoiceMail = rawContact.values.get(RawContactsColumns.IS_TRANSFER_VOICEMAIL);
    let contactLastUpdatedTimestamp = Date.now();
    insertContactValues[ContactsColumns.FOCUS_MODE_LIST] = masterContact.focusModeList;
    insertContactValues[ContactsColumns.PERSONAL_RINGTONE] = masterContact.personalRingtone;
    insertContactValues[ContactsColumns.PERSONAL_NOTIFICATION_RINGTONE] = masterContact.personalNotificationRingtone;
    insertContactValues[ContactsColumns.RINGTONE_PATH] = masterContact.ringtonePath;
    if (company) {
      insertContactValues[ContactsColumns.COMPANY] = company;
    }
    if (position) {
      insertContactValues[ContactsColumns.POSITION] = position;
    }
    if (photoFileId) {
      insertContactValues[ContactsColumns.PHOTO_FILE_ID] = photoFileId;
    }
    if (photoId) {
      insertContactValues[ContactsColumns.PHOTO_ID] = photoId;
    }
    if (readOnly) {
      insertContactValues[ContactsColumns.READ_ONLY] = readOnly;
    }
    if (isTransferVoiceMail) {
      insertContactValues[ContactsColumns.IS_TRANSFER_VOICEMAIL] = isTransferVoiceMail;
    }
    if (hasPhoneNumber) {
      insertContactValues[ContactsColumns.HAS_PHONE_NUMBER] = 1;
    }
    if (hasEmail) {
      insertContactValues[ContactsColumns.HAS_EMAIL] = 1;
    }
    if (hasGroup) {
      insertContactValues[ContactsColumns.HAS_GROUP] = 1;
    }
    if (hasName) {
      insertContactValues[ContactsColumns.HAS_DISPLAY_NAME] = 1;
    }
    insertContactValues[ContactsColumns.CONTACT_LAST_UPDATED_TIMESTAMP] = contactLastUpdatedTimestamp;
    return insertContactValues;
  }

  private async queryData(context: common.Context, displayNameList?: Array<string>):
    Promise<Map<number, AggregatorRawContact>> {
    // 分页查询contact_data视图
    let offset = 0;
    let isEnd = false;
    let waited = false;
    let rawContactMap: Map<number, AggregatorRawContact> = new Map();
    try {
      const rdbStore = await contactRdbHelper.initDB(context);
      if (!rdbStore) {
        LogUtils.e(TAG, `queryData: rdbStore init error!`);
        return rawContactMap;
      }
      const needMergeNameArr = await this.queryNeedMergeNameArr(displayNameList);
      LogUtils.w(TAG, `queryData needMergeNameArr size:${needMergeNameArr.length}`);
      if (needMergeNameArr?.length == 0) {
        return rawContactMap;
      }

      while (!isEnd) {
        const queryCondition = new rdb.RdbPredicates('view_contact_data');
        queryCondition.equalTo(RawContactsColumns.IS_DELETED, 0)
          .and()
          .equalTo(DataColumns.TYPE_ID, DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE)
          .and()
          .notEqualTo(RawContactsColumns.PRIMARY_CONTACT, IsMyCard.YES)
        if (needMergeNameArr && needMergeNameArr.length > 1) {
          queryCondition.and();
          queryCondition.in(RawContactsColumns.DISPLAY_NAME, needMergeNameArr);
        } else if (needMergeNameArr && needMergeNameArr.length === 1) {
          queryCondition.and();
          queryCondition.equalTo(RawContactsColumns.DISPLAY_NAME, needMergeNameArr[0]);
        }
        queryCondition.limitAs(PAGE_SIZE_RAW)
          .offsetAs(offset);

        LogUtils.w(TAG, `queryData: PAGE_SIZE_RAW: ${PAGE_SIZE_RAW}, offset: ${offset}`);
        if (!waited) {
          await DatabaseUtils.delay(WAIT_QUERY);
          waited = true;
        }
        let resultSet: relationalStore.ResultSet | null | undefined = null;
        try {
          resultSet = await contactRdbHelper.queryData('view_contact_data', queryCondition,
            AggregatorRawContact.AGGREGATOR_COLUMNS);

          if (!resultSet || !resultSet.goToFirstRow()) {
            LogUtils.e(TAG, 'queryData: query view_contact_data fail.');
            return rawContactMap;
          }

          resultSet.goToFirstRow();
          do {
            let rawId = resultSet.getLong(resultSet.getColumnIndex(DataColumns.RAW_CONTACT_ID));
            if (rawContactMap.get(rawId) == null) {
              rawContactMap.set(rawId, AggregatorRawContact.fromRawResultSet(resultSet));
            }
            let rawContact = rawContactMap.get(rawId);
            let phone = new Contact.PhoneNumber();
            phone.phoneNumber = resultSet.getString(resultSet.getColumnIndex(DataColumns.DETAIL_INFO));
            phone.labelName = resultSet.getString(resultSet.getColumnIndex(DataColumns.EXTEND7));
            phone.labelId = resultSet.getLong(resultSet.getColumnIndex(DataColumns.CUSTOM_DATA));
            phone.formatPhoneNumber = resultSet.getString(resultSet.getColumnIndex(DataColumns.FORMAT_PHONE_NUMBER));
            rawContact?.addPhone(phone);
          } while (resultSet.goToNextRow());

          offset += PAGE_SIZE_RAW;
          if (resultSet.rowCount < PAGE_SIZE_RAW) {
            isEnd = true;
          }
        } finally {
          if (resultSet && !resultSet.isClosed) {
            resultSet.close();
          }
        }
      }
      return rawContactMap;
    } catch (e) {
      LogUtils.e(TAG, 'queryData: query fail, error message:' + e);
      return rawContactMap;
    }
  }

  private async queryNeedMergeNameArr(displayNameList?: Array<string>):
    Promise<string[]> {
    let offset = 0;
    let isEnd = false;
    let needMergeNameArr: string[] = [];
    try {
      while (!isEnd) {
        const sql = this.getQueryRawContactSql(offset, displayNameList);
        LogUtils.w(TAG, `queryNeedMergeNameArr: PAGE_SIZE_RAW: ${PAGE_SIZE_RAW}, offset: ${offset}`);

        let resultSet: relationalStore.ResultSet | null | undefined = null;
        try {
          resultSet = await contactRdbHelper.querySql(sql);
          if (!resultSet || !resultSet.goToFirstRow()) {
            LogUtils.w(TAG, 'queryNeedMergeNameArr get empty resultSet.');
            return needMergeNameArr;
          }

          do {
            needMergeNameArr.push(resultSet.getString(0));
          } while (resultSet.goToNextRow());
          offset += PAGE_SIZE_RAW;
          if (resultSet.rowCount < PAGE_SIZE_RAW) {
            isEnd = true;
          }
        } finally {
          if (resultSet && !resultSet.isClosed) {
            resultSet.close();
          }
        }
      }
      return needMergeNameArr;
    } catch (e) {
      LogUtils.e(TAG, 'queryNeedMergeNameArr: query fail, error message:' + e);
      return needMergeNameArr;
    }
  }

  private getQueryRawContactSql(offset: number, displayNameList?: Array<string>): string {
    return `SELECT
    display_name 
  FROM
    raw_contact 
  WHERE
    is_deleted = 0 
    AND primary_contact != 1
    AND display_name is not null
    ${displayNameList?.length ?
        `AND display_name IN ( ${displayNameList.map((name: string) => `'${name}'`).join(',')} ) ` : ''} 
  GROUP BY
    display_name 
  HAVING
    count( display_name ) >1
  LIMIT ${offset},${PAGE_SIZE_RAW};`
  }

  private async getAggregationListSql(aggregationList: AggregatorRawContact[]): Promise<string> {
    return aggregationList.length < 2 ? ' ' : `SELECT
      ids.id AS id,
      COALESCE(rct.is_deleted, -1) AS is_deleted,
      COALESCE(rct.display_name, 'NULL') AS display_name
    FROM (
      SELECT ${aggregationList[0].rawId} AS id ${aggregationList.slice(1).map(id => `UNION SELECT ${id.rawId}`).join(' ')}
    ) AS ids
    LEFT JOIN raw_contact rct ON ids.id = rct.id;`;
  }

  private async batchProcessingResult(aggregationPatchList: AggregatorRawContact[],
    nameIdMap: Map<string, number[]>, shouldDeleteRawId: Set<number>): Promise<boolean> {
    let resultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      const sql = await this.getAggregationListSql(aggregationPatchList);
      if (sql === ' ') {
        LogUtils.e(TAG, 'batchProcessingResult: getAggregationListSql is null.');
        return false;
      }
      resultSet = await contactRdbHelper.querySql(sql);
      if (!resultSet || !resultSet.goToFirstRow()) {
        LogUtils.e(TAG, 'batchProcessingResult: get empty resultSet.');
        return false;
      }
      do {
        let id: number = resultSet.getLong(0);
        let isDeleted: number = resultSet.getLong(1);
        let name: string = resultSet.getString(2);
        nameIdMap.set(name, [...(nameIdMap.get(name) ?? []), id]);
        if (isDeleted === -1 || name === 'NULL' || isDeleted === 1) {
          shouldDeleteRawId.add(id);
          LogUtils.w(TAG, `batchProcessingResult: reduce rawId: ${id}.`);
        }
      } while (resultSet.goToNextRow());
    } catch (e) {
      LogUtils.e(TAG, `batchProcessingResult: check fail, error message: ${e?.message}`);
      return false;
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    return true;
  }

  private async checkAggregationList(aggregationList: AggregatorRawContact[]) {
    LogUtils.w(TAG, 'checkAggregationList: start.');
    if (aggregationList.length < 2) {
      LogUtils.e(TAG, 'checkAggregationList: aggregationList did not meet the requirements.');
      return;
    }
    let mainName: string = aggregationList[0].displayName;
    let nameIdMap: Map<string, number[]> = new Map<string, number[]>();
    let shouldDeleteRawId: Set<number> = new Set();
    try {
      for (let start = 0; start < aggregationList.length; start += MAX_COMPOUND_SELECT) {
        const end = Math.min(start + MAX_COMPOUND_SELECT, aggregationList.length);
        let isProcessSuccess = await this.batchProcessingResult(aggregationList.slice(start, end),
          nameIdMap, shouldDeleteRawId);
        if (!isProcessSuccess) {
          LogUtils.e(TAG, 'checkAggregationList: sql query fialed.');
          return;
        }
      }
      let differentNameId: number[] = Array.from(nameIdMap.entries())
        .filter(entry => entry[0] !== mainName)
        .flatMap(entry => entry[1]);
      differentNameId.forEach(item => shouldDeleteRawId.add(item));
      if (shouldDeleteRawId.size === 0) {
        LogUtils.w(TAG, 'checkAggregationList: no elements need to be removed.');
        return;
      }
      aggregationList.length = aggregationList.reduce((index, value) => {
        if (!shouldDeleteRawId.has(value.rawId)) {
          aggregationList[index++] = value;
        }
        return index;
      }, 0);
    } catch (e) {
      LogUtils.e(TAG, `checkAggregationList: check fail, error message: ${e?.message}`);
    }
    LogUtils.w(TAG, 'checkAggregationList: end.');
  }

  /*
   * 执行智能合并
   *
   * 智能合并动作：
   * 1、刷新raw表contactId，并设置智能合并状态
   * 2、删除多余的contact记录
   * 3、更新通话记录
   * @param context：上下文
   * @param needAggregationList：需要智能合并的raw记录列表
   */
  private async executeAggregation(context: common.Context, aggregationList: AggregatorRawContact[],
    migrateProgressInfo?: MigrateProgressInfo): Promise<number> {
    if (aggregationList.length < 2) {
      LogUtils.w(TAG, `executeAggregation param length < 2`);
      return 0;
    }
    await this.checkAggregationList(aggregationList);
    
    // needAggregationList中可能存在重复数据，先过滤下
    let needAggregationList: AggregatorRawContact[] = [];
    let rawIdArray: Set<number> = new Set();
    for (let aggregatorRaw of aggregationList) {
      if (!rawIdArray.has(aggregatorRaw.rawId)) {
        needAggregationList.push(aggregatorRaw);
        rawIdArray.add(aggregatorRaw.rawId);
      }
    }

    if (needAggregationList.length < 2) {
      LogUtils.w(TAG, `executeAggregation length < 2 after reduce duplicate`);
      return 0;
    }
    LogUtils.w(TAG, `executeAggregation needAggregationList length:${needAggregationList.length}`);

    // 找到主contactId，优先用已合并联系人的，尽量少的更改raw记录对应的contactId
    // 情况1：如果其中存在已合并的联系人，则合并到已合并联系人最大的contactId上（可能有多组）；
    // 情况2：否则选择这些联系人中最大的contactId；
    let masterContactId = -1;
    let maxContactIdAggregator = -1;
    let maxContactId = -1;
    let rawIdList: number[] = [];
    let contactIdList: number[] = [];
    let contactIdSet: Set<number> = new Set();
    // 计算收藏
    let favorite = 0;
    let favoriteOrder = -1;
    let needUpdateStatus = false;
    let count = 0;
    for (let rawContact of needAggregationList) {
      count++;
      // 每处理 200 个联系人，休眠 50 ms（正常同名的都不会超过 200 个，此处为 规避大量脏数据）
      if (count % WAIT_QUERY == 0) {
        await DatabaseUtils.delay(NUM_50);
      }
      LogUtils.w(TAG, `executeAggregation rawContact:` + rawContact.toString());
      if (rawContact.aggregationStatus == 1) {
        maxContactIdAggregator = rawContact.contactId > maxContactIdAggregator ?
          rawContact.contactId : maxContactIdAggregator;
      } else {
        needUpdateStatus = true;
      }
      maxContactId = rawContact.contactId > maxContactId ? rawContact.contactId : maxContactId;
      rawIdList.push(rawContact.rawId);
      contactIdList.push(rawContact.contactId);
      contactIdSet.add(rawContact.contactId);
      favorite = favorite || rawContact.favorite;
      if (rawContact.favorite) {
        if (favoriteOrder == -1) {
          favoriteOrder = rawContact.favoriteOrder;
        } else {
          // 取收藏次序最小的
          favoriteOrder = rawContact.favoriteOrder < favoriteOrder ? rawContact.favoriteOrder : favoriteOrder;
        }
      }
    }
    LogUtils.w(TAG, `executeAggregation favorite:` + favorite + `, favoriteOrder:` + favoriteOrder);

    if (contactIdSet.size <= 1) {
      if (needUpdateStatus) {
        LogUtils.w(TAG, `executeAggregation only one contactId, just update aggregator status`);
        const rawPredicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
        rawPredicates.in(RawContactsColumns.ID, rawIdList);
        const rawUpdateBucket: ValuesBucket = {};
        rawUpdateBucket[RawContactsColumns.AGGREGATION_STATUS] = 1;
        await contactRdbHelper.updateData(rawUpdateBucket, rawPredicates);
      } else {
        LogUtils.w(TAG, `executeAggregation only one contactId, no need update aggregator status`);
      }
      return 0;
    }

    if (maxContactIdAggregator != -1) {
      masterContactId = maxContactIdAggregator;
    } else {
      masterContactId = maxContactId;
    }
    LogUtils.w(TAG, `executeAggregation masterContactId:` + masterContactId + `, maxContactId:` + maxContactId);

    // form_id/focus_mode_list迁到contact表后，需要在这里将form_id合并到目标contact记录上
    let masterFormId = '';
    let masterFocusMode = '';
    let masterRingtone = '';
    let masterNotificationRingtone = '';
    let masterRingtonePath = '';
    const masterContactPredicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
    masterContactPredicates.equalTo(ContactsColumns.NEW_ID, masterContactId);
    let resultSet1: relationalStore.ResultSet | null | undefined = null;
    try {
      resultSet1 = await contactRdbHelper.queryData('contact', masterContactPredicates,
        [ContactsColumns.FORM_ID, ContactsColumns.FOCUS_MODE_LIST, ContactsColumns.PERSONAL_RINGTONE,
          ContactsColumns.PERSONAL_NOTIFICATION_RINGTONE, ContactsColumns.RINGTONE_PATH]);
      if (!resultSet1 || !resultSet1.goToFirstRow()) {
        LogUtils.e(TAG,
          `executeAggregation query master contact fail,id:${masterContactId},skip this rawContact group`);
        return 0;
      }
      resultSet1.goToFirstRow();
      do {
        masterFormId = resultSet1.getString(0);
        masterFocusMode = resultSet1.getString(1);
        masterRingtone = resultSet1.getString(2);
        masterNotificationRingtone = resultSet1.getString(3);
        masterRingtonePath = resultSet1.getString(4);
      } while (resultSet1.goToNextRow());
    } finally {
      if (resultSet1 && !resultSet1.isClosed) {
        resultSet1.close();
      }
    }

    const contactPredicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
    contactPredicates.in(ContactsColumns.NAME_RAW_CONTACT_ID, rawIdList);
    contactPredicates.and();
    contactPredicates.notEqualTo(ContactsColumns.NEW_ID, masterContactId);

    let formIdList: string[] = [];
    let focusModeList: string[] = [];
    let otherRingtone = '';
    let otherNotificationRingtone = '';
    let otherRingtonePath = '';
    let resultSet2: relationalStore.ResultSet | null | undefined = null;
    try {
      resultSet2 = await contactRdbHelper.queryData('contact', contactPredicates,
      [ContactsColumns.FORM_ID, ContactsColumns.FOCUS_MODE_LIST, ContactsColumns.PERSONAL_RINGTONE,
      ContactsColumns.PERSONAL_NOTIFICATION_RINGTONE, ContactsColumns.RINGTONE_PATH]);

      if (resultSet2 && resultSet2.goToFirstRow()) {
        do {
          const formId = resultSet2.getString(0);
          const focusMode = resultSet2.getString(1);
          const ringtone = resultSet2.getString(2);
          const notificationRingtone = resultSet2.getString(3);
          const ringtonePath = resultSet2.getString(4);
          if (formId) {
            formIdList.push(formId);
          }
          if (focusMode) {
            focusModeList.push(focusMode);
          }
          if (ringtone && !otherRingtone) {
            otherRingtone = ringtone;
            otherNotificationRingtone = notificationRingtone;
            otherRingtonePath = ringtonePath;
          }
        } while (resultSet2.goToNextRow());
      }
    } finally {
      if (resultSet2 && !resultSet2.isClosed) {
        resultSet2.close();
      }
    }
    if (formIdList.length > 0) {
      const formIdSet: Set<string> =
        new Set((masterFormId + formIdSeparator + formIdList.join(formIdSeparator))
          .split(formIdSeparator).filter((formId: string) => !StringUtil.isEmpty(formId)));
      const newFormId = Array.from(formIdSet).join(formIdSeparator);
      if (newFormId != masterFormId) {
        LogUtils.w(TAG, `executeAggregation update masterContactId ${masterContactId} formId from ${masterFormId} to ${
          newFormId}`);
        masterFormId = newFormId;
      }
    }
    if ((masterFocusMode == null || masterFocusMode == '') && focusModeList.length > 0) {
      LogUtils.w(TAG, `executeAggregation update masterContactId ${masterContactId} focusMode from ${
        masterFocusMode} to ${focusModeList[0]}`);
      masterFocusMode = focusModeList[0];
    }
    if (!masterRingtone && otherRingtone) {
      LogUtils.w(TAG, `executeAggregation update masterContactId ${masterContactId} ringtone to ${
        StringUtil.maskSensitiveInfo(otherRingtone)}:${StringUtil.maskSensitiveInfo(otherNotificationRingtone)}:${
          StringUtil.maskSensitiveInfo(otherRingtonePath)}`);
      masterRingtone = otherRingtone;
      masterNotificationRingtone = otherNotificationRingtone;
      masterRingtonePath = otherRingtonePath;
    }

    // 查询要删除的日历里的生日
    let eventIds: number[] = [];
    let contactDataIds: number[] = [];
    const deleteBirthdayPredicates = new rdb.RdbPredicates('view_contact_data');
    deleteBirthdayPredicates.in(DataColumns.RAW_CONTACT_ID, rawIdList);
    deleteBirthdayPredicates.and();
    deleteBirthdayPredicates.notEqualTo(DataColumns.CONTACT_ID, masterContactId);
    deleteBirthdayPredicates.and();
    deleteBirthdayPredicates.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
    deleteBirthdayPredicates.and();
    deleteBirthdayPredicates.in(DataColumns.CUSTOM_DATA, BIRTHDAY_TYPE);
    let resultSetCalendarId: relationalStore.ResultSet | null | undefined = null;
    try {
      resultSetCalendarId = await contactRdbHelper.queryData('view_contact_data', deleteBirthdayPredicates,
        [DataColumns.CALENDAR_EVENT_ID, DataColumns.ID]);
      if (resultSetCalendarId && resultSetCalendarId.goToFirstRow()) {
        LogUtils.w(TAG, `resultSetCalendarId rowCount is :${resultSetCalendarId.rowCount}`);
        do {
          let calendarEventId = resultSetCalendarId.getString(0);
          let id = resultSetCalendarId.getLong(1);
          if (calendarEventId) {
            eventIds.push(Number(calendarEventId));
            contactDataIds.push(id);
          }
        } while (resultSetCalendarId.goToNextRow());
      }
    } finally {
      if (resultSetCalendarId && !resultSetCalendarId.isClosed) {
        resultSetCalendarId.close();
      }
    }
    try {
      const initDbSuc = await contactRdbHelper.initDB(context);
      if (!initDbSuc) {
        throw new Error(`executeAggregation initDb fail`);
      }

      // 开启事务
      const beginTransSuc = await contactRdbHelper.beginTransaction();
      if (!beginTransSuc) {
        throw new Error(`executeAggregation beginTrans fail`);
      }

      // 刷新raw表contactId，并设置智能合并状态
      let needUpload = false;
      const rawPredicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
      rawPredicates.in(RawContactsColumns.ID, rawIdList);
      const rawUpdateBucket: ValuesBucket = {};
      rawUpdateBucket[RawContactsColumns.CONTACT_ID] = masterContactId;
      rawUpdateBucket[RawContactsColumns.AGGREGATION_STATUS] = 1;
      if (favorite == 1) {
        rawUpdateBucket[RawContactsColumns.FAVORITE] = 1;
        if (favoriteOrder > -1) {
          rawUpdateBucket[RawContactsColumns.FAVORITE_ORDER] = favoriteOrder;
        }
        rawUpdateBucket[RawContactsColumns.DIRTY] = 1;
        needUpload = true;
      }
      let rdbRet = await contactRdbHelper.updateData(rawUpdateBucket, rawPredicates);
      if (rdbRet < 0) {
        throw new Error(`executeAggregation update raw contact fail, ret:${rdbRet}`);
      }

      // 需要刷新主联系人

      const contactUpdateBucket: ValuesBucket = {};
      contactUpdateBucket[ContactsColumns.CONTACT_LAST_UPDATED_TIMESTAMP] = Date.now();
      contactUpdateBucket[ContactsColumns.FORM_ID] = masterFormId;
      contactUpdateBucket[ContactsColumns.FOCUS_MODE_LIST] = masterFocusMode;
      contactUpdateBucket[ContactsColumns.PERSONAL_RINGTONE] = masterRingtone;
      contactUpdateBucket[ContactsColumns.PERSONAL_NOTIFICATION_RINGTONE] = masterNotificationRingtone;
      contactUpdateBucket[ContactsColumns.RINGTONE_PATH] = masterRingtonePath;

      rdbRet = await contactRdbHelper.updateData(contactUpdateBucket, masterContactPredicates);
      if (rdbRet < 0) {
        throw new Error(`executeAggregation update raw contact fail, ret:${rdbRet}`);
      }

      // 删除多余的contact记录
      const deleteContactPredicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
      deleteContactPredicates.in(ContactsColumns.NAME_RAW_CONTACT_ID, rawIdList);
      deleteContactPredicates.and();
      deleteContactPredicates.notEqualTo(ContactsColumns.NEW_ID, masterContactId);
      rdbRet = await contactRdbHelper.deleteData(deleteContactPredicates);
      if (rdbRet < 0) {
        throw new Error(`executeAggregation delete contact fail, ret:${rdbRet}`);
      }
      await contactRdbHelper.commit();
      LogUtils.w(TAG, `executeAggregation contactDataIds length is :${contactDataIds.length},
        delete count: ${rdbRet}`);

      // 克隆云备份场景，进程在备份框架，没有权限直接操作日历数据库，只能先标记为待删除状态，因为该数据不会被硬删除，所以可以这么搞
      if (contactDataIds.length > 0 && migrateProgressInfo) {
        await BirthdayManager.getInstance().markedBirthdayToDelFlag(contactDataIds);
      }
      if (eventIds.length > 0 && !migrateProgressInfo) {
        await BirthdayManager.getInstance().deleteBirthdayEvent(eventIds);
        await BirthdayManager.getInstance().handleBatchBirthday(rawIdList, false);
      }

      // 更新通话记录
      await this.updateCallLog(context, contactIdList, masterContactId);
      if (needUpload) {
        await ContactsDataService.getInstance().contactsDataChange(context);
      }
    } catch (e) {
      LogUtils.e(TAG, `executeAggregation err,msg:${e.message}`);
      await contactRdbHelper.rollBack();
    }

    LogUtils.w(TAG, `executeAggregation aggregateCount:` + needAggregationList.length);
    return needAggregationList.length;
  }

  private async updateCallLog(context: common.Context, contactIdList: number[], masterContactId: number) {
    await callLogRdbHelper.initDB(context);
    await callLogRdbHelper.updateCallLogAfterAggregation(contactIdList, masterContactId);
  }

  // 过滤相似联系人数组中满足智能合并条件的联系人，同一姓名的可能会智能合并成多个联系人
  private filterNeedAggregate(similar: AggregatorRawContact[]): Map<number, AggregatorRawContact[]> {
    let phoneIntersectionMap: Map<number, Set<AggregatorRawContact>> = new Map();
    let phoneGroup = 0;
    // 首次遍历得到号码存在交集的联系人组
    for (let i = 0; i < similar.length - 1; i++) {
      phoneIntersectionMap.set(phoneGroup, new Set<AggregatorRawContact>());
      for (let j = i + 1; j < similar.length; j++) {
        if (!this.checkPhoneIntersection(similar[i], similar[j])) {
          // 如果两个联系人号码不存在交集，继续比较下一个联系人
          LogUtils.i(TAG, `filterNeedAggregate no intersection`);
          continue;
        }
        // 如果i已经存在于某个group中，则需要将j加入到对应的group中；相反亦然
        let needSkip = false;
        for (let k = 0; k < phoneIntersectionMap.size; k++) {
          if (phoneIntersectionMap.get(k)?.has(similar[i])) {
            phoneIntersectionMap.get(k)?.add(similar[j]);
            needSkip = true;
            break;
          }
          if (phoneIntersectionMap.get(k)?.has(similar[j])) {
            phoneIntersectionMap.get(k)?.add(similar[i]);
            needSkip = true;
            break;
          }
        }
        // 如果已经添加到之前的group，则不需要继续处理，直接比对下一条
        if (needSkip) {
          continue;
        }
        // 存在号码交集时，将联系人归到同一组
        if (phoneIntersectionMap.get(phoneGroup) != null && phoneIntersectionMap.get(phoneGroup)?.size! > 0) {
          phoneIntersectionMap.get(phoneGroup)!.add(similar[j]);
        } else {
          let rawContactSet = new Set<AggregatorRawContact>();
          rawContactSet.add(similar[i]);
          rawContactSet.add(similar[j]);
          phoneIntersectionMap.set(phoneGroup, rawContactSet);
        }
      }
      phoneGroup++;
    }
    // 联系人有交集的组合并到一起
    for (let i = 0; i < phoneGroup; i++) {
      if (phoneIntersectionMap.get(i)?.size == 0) {
        continue;
      }
      for (let j = i + 1; j < phoneGroup; j++) {
        if (phoneIntersectionMap.get(j)?.size == 0) {
          continue;
        }
        let needMerge = false;
        for (let contactInfo of phoneIntersectionMap.get(j)!) {
          if (phoneIntersectionMap.get(i)?.has(contactInfo)) {
            needMerge = true;
            break;
          }
        }
        if (needMerge) {
          phoneIntersectionMap.get(j)?.forEach(contactInfo => phoneIntersectionMap.get(i)?.add(contactInfo));
          phoneIntersectionMap.get(j)?.clear();
        }
      }
    }
    let resultMap: Map<number, AggregatorRawContact[]> = new Map();

    for (let phoneGroup of phoneIntersectionMap.keys()) {
      resultMap.set(phoneGroup, Array.from(phoneIntersectionMap.get(phoneGroup)!));
      LogUtils.i(TAG, `aggregatorMap: phoneGroup: ${StringUtil.maskSensitiveInfo(phoneGroup.toString())}  ${
        resultMap.get(phoneGroup)?.map(contactInfo => contactInfo.toString()).join(', ')}`);
    }

    return resultMap;
  }

  // 检查两个联系人号码是否存在交集
  private checkPhoneIntersection(a: AggregatorRawContact, b: AggregatorRawContact): boolean {
    let aPhone = a.phoneList;
    let bPhone = b.phoneList;
    for (let i = 0; i < aPhone.length; i++) {
      for (let j = 0; j < bPhone.length; j++) {
        let aFormatPhoneNumber: string | null = aPhone[i].formatPhoneNumber!;
        let bFormatPhoneNumber: string | null = bPhone[j].formatPhoneNumber!;
        if (aFormatPhoneNumber == null) {
          aFormatPhoneNumber = getFormatNumber(aPhone[i].phoneNumber);
        }
        if (bFormatPhoneNumber == null) {
          bFormatPhoneNumber = getFormatNumber(bPhone[j].phoneNumber);
        }
        if (aFormatPhoneNumber != null && aFormatPhoneNumber != '' && aFormatPhoneNumber === bFormatPhoneNumber) {
          return true;
        } else if (aPhone[i].phoneNumber === bPhone[j].phoneNumber) {
          return true;
        }
      }
    }
    return false;
  }
}