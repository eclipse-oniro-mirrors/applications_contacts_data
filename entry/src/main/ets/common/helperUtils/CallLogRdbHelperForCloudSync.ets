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
import common from '@ohos.app.ability.common';
import contextConstant from '@ohos.app.ability.contextConstant';
import rdb from '@ohos.data.relationalStore'
import { BusinessError } from '@ohos.base';
import { ValuesBucket, ValueType } from '@ohos.data.ValuesBucket';
import CallLogSingleConstant from '../contract/CallLogSingleConstant'
import securityLabel from '@ohos.file.securityLabel';
import DbConstants from '../DbConstants';
import RawContactsColumns from '../contract/RawContactsColumns';
import DataColumns from '../contract/DataColumns';
import { CallStatisticsCount } from '../../serivce/BoardHiSysEventUtil';
import { dataShare, dataSharePredicates } from '@kit.ArkData';
import DataShareResultSet from '@ohos.data.DataShareResultSet';
import LogUtils from '../utils/LogUtils';
import { StringUtil } from '../utils/StringUtil';
import { ExtraInfo } from '../../serivce/ContactsDataService';
import { contactRdbHelper } from './ContactsRdbHelperForCloudSync';

const QUERY_PAGE_500: number = 500;

const TAG: string = 'CallLogRdbHelperForCloudSync';

const UPDATE_CALL_LOG_WHEN_DELETE_CONTACT_SQL =
  `update calllog
      set display_name = null,
      quicksearch_key = null,
      extra1 = null
      where quicksearch_key = ?`;

const UPDATE_CALL_LOG_WHEN_CONTACT_ADD_PHONE_SQL =
  `update calllog
      set display_name = ?,
      quicksearch_key = ?,
      extra1 = ?
      where phone_number in ?
      and (quicksearch_key is NULL or quicksearch_key = '')`;

const UPDATE_CALL_LOG_WHEN_CONTACT_DELETE_PHONE_SQL =
  `update calllog
      set display_name = null,
      quicksearch_key = null,
      extra1 = null
      where quicksearch_key = ? and phone_number in ?`;

export const CALL_LOG_TABLE_NAME = 'calllog';

export class CallContact {
  public id: number = -1;
  public callDirection: number = -1;
  public answerState: number = -1;
  public privacyTag: number = -1;

  static fromRuleSet(resultSet: rdb.ResultSet): CallContact {
    const callContact: CallContact = new CallContact();
    let index: number = 0;
    callContact.id = resultSet.getLong(index++);
    callContact.callDirection = resultSet.getLong(index++);
    callContact.answerState = resultSet.getLong(index++);
    callContact.privacyTag = resultSet.getLong(index++);
    return callContact;
  }
}

/**
 * 新增联系人，更新通话记录；新增联系人的信息
 */
export class InsertContactBoUpdateCallLog {
  public contactId: number = -1;
  public displayName: string = '';
  public extraInfo: ExtraInfo = {
    deletePhoneList: [],
    deleteFormatPhoneList: [],
    addPhoneList: [],
    addFormatPhoneList: [],
  }
  constructor() {
  }
  static newInstance(contactId: number, displayName: string) {
    const instance = new InsertContactBoUpdateCallLog();
    instance.contactId = contactId;
    instance.displayName = displayName;
    return instance;
  }
}

export class CallLogRdbHelperForCloudSync {
  private rdbStore: rdb.RdbStore | null = null;
  private dbName: string = 'calls.db';

  public async getRdbStore(context: common.Context): Promise<rdb.RdbStore | null> {
    if (!context) {
      LogUtils.e(TAG, 'getRdbStore context is null');
      return null;
    }
    let applicationContext: common.Context = context.getApplicationContext();
    applicationContext.area = contextConstant.AreaMode.EL5;
    // 云同步在获取rdb数据库句柄前，先让Cpp侧做一次完整升级，开库等流程。
    let isAccessSuccess = await this.accessCalllog(context);
    if (!isAccessSuccess) {
      LogUtils.e(TAG, `failed to access calllog.`);
    }
    try {
      await securityLabel.setSecurityLabel('/data/storage/el5/database/rdb/calls.db', 's4').then(() => {
        LogUtils.i(TAG, 'Succeeded in setSecurityLabeling calls.db s4');
      }).catch((err: BusinessError) => {
        LogUtils.e(TAG, `Failed to setSecurityLabel. Code: ${err.code}, message: ${err.message}`);
      });
      LogUtils.w(TAG, `getRdbStore start, context dir:${applicationContext.databaseDir}, context filesDir:${
        applicationContext.filesDir},dbName:${this.dbName}`);
      this.rdbStore = await rdb.getRdbStore(applicationContext,
        {
          name: this.dbName,
          //云同步连接单框架db，改为正确等级S4
          securityLevel: rdb.SecurityLevel.S4,
          isSearchable: true,
          autoCleanDirtyData: false,
          allowRebuild: true
        });
      if (this.rdbStore != null) { // 如果开库成功，
        let rebuilt: number = this.rdbStore.rebuilt;
        LogUtils.i(TAG, `getRdbStoreCalls, rebuilt:${rebuilt}`);
        if (rebuilt == rdb.RebuildType.REBUILT ) { // 如果进行了重建
          await this.rdbStore.restore('calls.db.bak', (err) => {
            if (err) {
              LogUtils.e(TAG, `getRdbStoreCalls, Restore failed, code is ${err.code},message is ${err.message}`);
              return;
            }
            LogUtils.i(TAG, `getRdbStoreCalls, Restore success.`);
          }); // 则进行恢复
        } else {
          // 没有重建，什么也不做
          LogUtils.i(TAG, `getRdbStoreCalls, No rebuilt`);
        }
      } else {
        //开库失败 进行恢复
        LogUtils.e(TAG, `getRdbStoreCalls, GetRdbStore err, rdbStore is null, skip rebuilt. `);
        await (this.rdbStore as rdb.RdbStore).restore('calls.db.bak', (err) => {
          if (err) {
            LogUtils.e(TAG, `Restore failed,code is ${err.code},message is ${err.message}`);
            return;
          }
          LogUtils.i(TAG, `Restore success.`);
        })
      }
    } catch (e) {
      LogUtils.e(TAG, `getRdbStore error ${e?.message || e?.code} dBName is ${this.dbName}`);
      return null;
    }
    if (!this.rdbStore) {
      LogUtils.e(TAG, 'getRdbStore failed');
      return null;
    }
    return this.rdbStore;
  }

  public async initDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    if (this.rdbStore) {
      return this.rdbStore;
    }
    LogUtils.i(TAG, 'init callLog SingleDB start');
    this.rdbStore = await this.getRdbStore(context);
    return this.rdbStore!;
  }

  private async accessCalllog(context: common.Context): Promise<boolean> {
    LogUtils.w(TAG, `accessCalllog start.`);
    let resultSet: DataShareResultSet | undefined;
    let dataShareHelper = await dataShare.createDataShareHelper(context, CallLogSingleConstant.CALL_LOG_URI);
    if (!dataShareHelper) {
      LogUtils.e(TAG, `accessCalllog dataShareHelper is null, and accessCalllog end.`);
      return false;
    }
    try {
      let predicate: dataSharePredicates.DataSharePredicates = new dataSharePredicates.DataSharePredicates();
      let columns: string[] = ['id'];
      resultSet = await dataShareHelper.query(CallLogSingleConstant.CALL_LOG_URI, predicate, columns);
      if (resultSet === undefined || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, 'accessCalllog, no call log found.');
      } else {
        LogUtils.w(TAG, 'accessCalllog, call log found.');
      }
    } catch (error) {
      LogUtils.e(TAG, `accessCalllog error : ${error.message}`);
      return false;
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
        LogUtils.w(TAG, 'accessCalllog resultSet close success');
      }
      if (dataShareHelper) {
        dataShareHelper.close();
        LogUtils.w(TAG, 'accessCalllog release dataShareHelper success');
      }
      LogUtils.w(TAG, `accessCalllog end.`);
    }
    return true;
  }

  /**
   * Update data into contact
   *
   * @param data which data to update
   * @param tableName which table name to update
   * @returns the number of successful insertions
   */
  public async updateData(data: ValuesBucket, predicates: rdb.RdbPredicates): Promise<number> {
    if (data == undefined || predicates == undefined) {
      LogUtils.e(TAG, 'updateData ValuesBucket or predicates is NULL');
      return -1;
    }
    if (this.rdbStore === undefined) {
      LogUtils.e(TAG, 'rdbStore is NULL');
      return -1;
    }
    let ret = -1;
    try {
      ret = await this.rdbStore!.update(data, predicates);
    } catch (error) {
      LogUtils.e(TAG, `updateData err, code: ${error.code}, msg:${error.message}`);
    }
    return ret;
  }

  public async updateCallLogWhenDeleteContact(contactId: number) {
    try {
      const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
      predicates.equalTo(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, contactId.toString());
      const updateBucket: ValuesBucket = {};
      updateBucket[CallLogSingleConstant.CALL_LOG_DISPLAY_NAME] = null;
      updateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = null;
      updateBucket[CallLogSingleConstant.CALL_LOG_EXTRA_1] = null;
      const ret = await this.updateData(updateBucket, predicates);
      if (ret < 0) {
        LogUtils.e(TAG, `updateCallLogWhenDeleteContact err ,contactId:${contactId}, ret: ` + ret);
      }
      return ret;
    } catch (err) {
      LogUtils.e(TAG, `updateCallLogWhenDeleteContact err ,contactId:${contactId}, code: ` + err?.code +
        ', err message: ' + err?.message);
    }
    return -1;
  }

  public async updateCallLogWhenBatchDelContact(contactIdArr: string[]) {
    try {
      const detailInfoSet = new Set<string>();
      const detailInfos = await this.queryDeletedPhoneData(contactIdArr, detailInfoSet);
      if (detailInfos.length == 0) {
        LogUtils.w(TAG, `updateCallLogWhenBatchDelContact detailInfos is empty`);
        return;
      }
      const phoneToNewContactId = await this.checkPhoneInOtherContact(contactIdArr, detailInfos, detailInfoSet);
      LogUtils.w(TAG, `updateCallLogWhenBatchDelContact detailInfoSet size:${
        detailInfoSet.size},phoneToNewContactId size:${phoneToNewContactId.size}`);
      if (phoneToNewContactId.size > 0) {
        await this.updateCallLogToOtherContact(phoneToNewContactId);
      }
      if (detailInfoSet.size > 0) {
        const needDelDetailInfos = Array.from(detailInfoSet);
        const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
        predicates.in(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, contactIdArr);
        predicates.and();
        predicates.beginWrap();
        predicates.in(CallLogSingleConstant.CALL_LOG_PHONE_NUMBER, needDelDetailInfos);
        predicates.or();
        predicates.in(CallLogSingleConstant.CALL_LOG_FORMAT_PHONE_NUMBER, needDelDetailInfos);
        predicates.endWrap();
        const updateBucket: ValuesBucket = {};
        updateBucket[CallLogSingleConstant.CALL_LOG_DISPLAY_NAME] = null;
        updateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = null;
        updateBucket[CallLogSingleConstant.CALL_LOG_EXTRA_1] = null;
        const ret = await this.updateData(updateBucket, predicates);
        if (ret < 0) {
          LogUtils.e(TAG, `updateCallLogWhenBatchDelContact err ,contactIdArr:${contactIdArr}, ret: ` + ret);
        }
      }
    } catch (err) {
      LogUtils.e(TAG, `updateCallLogWhenDeleteContact err ,contactIdArr:${contactIdArr}, code: ` + err?.code +
        ', err message: ' + err?.message);
    }
  }

  public async queryDeletedPhoneData(contactIdArr: string[],
    detailInfoSet: Set<string>) {
    const detailInfos: string[] = [];
    const predicates = new rdb.RdbPredicates(DbConstants.VIEW_CONTACT_DATA_NAME);
    predicates.in(RawContactsColumns.CONTACT_ID, contactIdArr);
    predicates.and();
    predicates.equalTo(DataColumns.TYPE_ID, DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE);
    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      resultSet = await contactRdbHelper.queryData(DbConstants.VIEW_CONTACT_DATA_NAME, predicates,
        [DataColumns.DETAIL_INFO, DataColumns.FORMAT_PHONE_NUMBER]);
      if (!resultSet || !resultSet.goToFirstRow()) {
        return detailInfos;
      }
      do {
        const detailInfo = resultSet.getString(0);
        const formatPhoneNumber = resultSet.getString(1);
        if (formatPhoneNumber && formatPhoneNumber.length) {
          detailInfos.push(formatPhoneNumber);
          detailInfoSet.add(formatPhoneNumber);
        } else {
          detailInfos.push(detailInfo);
          detailInfoSet.add(detailInfo);
        }
      } while (resultSet.goToNextRow())
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    return detailInfos;
  }
 
  public async checkPhoneInOtherContact(
    contactIdArr: string[], detailInfos: string[], detailInfoSet: Set<string>) {
    const phoneToNewContactId = new Map<string, InsertContactBoUpdateCallLog>();
    const predicates = new rdb.RdbPredicates(DbConstants.VIEW_CONTACT_DATA_NAME);
    predicates.notIn(RawContactsColumns.CONTACT_ID, contactIdArr);
    predicates.and();
    predicates.beginWrap();
    predicates.in(DataColumns.FORMAT_PHONE_NUMBER, detailInfos);
    predicates.or();
    predicates.in(DataColumns.DETAIL_INFO, detailInfos);
    predicates.endWrap();
    predicates.and();
    predicates.equalTo(RawContactsColumns.IS_DELETED, 0);
    predicates.and();
    predicates.equalTo(DataColumns.TYPE_ID, DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE);

    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      resultSet = await contactRdbHelper.queryData(DbConstants.VIEW_CONTACT_DATA_NAME, predicates,
        [DataColumns.DETAIL_INFO, DataColumns.FORMAT_PHONE_NUMBER, RawContactsColumns.CONTACT_ID,
          RawContactsColumns.DISPLAY_NAME]);
      if (!resultSet || !resultSet.goToFirstRow()) {
        return phoneToNewContactId;
      }
      do {
        const formatPhoneNumber = resultSet.getString(1);
        const contactId = resultSet.getLong(2);
        const displayName = resultSet.getString(3);
        if (formatPhoneNumber && formatPhoneNumber.length) {
          detailInfoSet.delete(formatPhoneNumber);
          phoneToNewContactId.set(formatPhoneNumber, InsertContactBoUpdateCallLog.newInstance(contactId, displayName));
        } else {
          const detailInfo = resultSet.getString(0);
          detailInfoSet.delete(detailInfo);
          phoneToNewContactId.set(detailInfo, InsertContactBoUpdateCallLog.newInstance(contactId, displayName));
        }
      } while (resultSet.goToNextRow())
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    return phoneToNewContactId;
  }
 
  public async updateCallLogToOtherContact(phoneToNewContactId = new Map<string, InsertContactBoUpdateCallLog>()) {
    for (const detailInfo of phoneToNewContactId.keys()) {
      const insertContactBoUpdateCallLog: InsertContactBoUpdateCallLog = phoneToNewContactId.get(detailInfo)!;
      const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
      predicates.equalTo(CallLogSingleConstant.CALL_LOG_FORMAT_PHONE_NUMBER, detailInfo);
      predicates.or();
      predicates.equalTo(CallLogSingleConstant.CALL_LOG_PHONE_NUMBER, detailInfo);
      const updateBucket: ValuesBucket = {};
      const contactId = insertContactBoUpdateCallLog.contactId.toString();
      updateBucket[CallLogSingleConstant.CALL_LOG_DISPLAY_NAME] = insertContactBoUpdateCallLog.displayName;
      updateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = contactId;
      updateBucket[CallLogSingleConstant.CALL_LOG_EXTRA_1] = contactId;
      const ret = await this.updateData(updateBucket, predicates);
      if (ret < 0) {
        LogUtils.e(TAG, `updateCallLogToOtherContact err, ret: ` + ret);
      }
    }
  }

  public async updateCallLogWhenDeleteContactBatch(contactIdList: number[]) {
    try {
      if (contactIdList.length === 0) {
        return;
      }
      const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
      predicates.in(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, contactIdList.map(String));
      const updateBucket: ValuesBucket = {};
      updateBucket[CallLogSingleConstant.CALL_LOG_DISPLAY_NAME] = null;
      updateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = null;
      updateBucket[CallLogSingleConstant.CALL_LOG_EXTRA_1] = null;
      const ret = await this.updateData(updateBucket, predicates);
      if (ret < 0) {
        LogUtils.e(TAG, `updateCallLogWhenDeleteContactBatch err ,contactIdLen:${contactIdList.length}, ret: ` + ret);
      }
      return ret;
    } catch (err) {
      LogUtils.e(TAG, `updateCallLogWhenDeleteContactBatch err ,contactIdLen:${contactIdList.length}, code: ` + err?.code +
        ', err message: ' + err?.message);
    }
    return -1;
  }

  /**
   * 新增电话，处理通话记录
   * @param contactId
   * @param displayName
   * @param phoneNumberArr
   * @param isFormatNumber 是否格式化的电话号码
   * @returns
   */
  public async updateCallLogWhenContactAddPhone(contactId: number,
    displayName: string, phoneNumberArr: string[], isFormatNumber: boolean) {
    const updateLog = `updateCallLogWhenContactAddPhone start,contactId:${contactId},displayName:${
    StringUtil.maskSensitiveInfo(displayName)},phoneNumberArr:${StringUtil.maskSensitiveInfo(phoneNumberArr?.join())}`;
    try {
      const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
      // 通话记录号码匹配，且没关联到联系人
      predicates.in(isFormatNumber ? CallLogSingleConstant.CALL_LOG_FORMAT_PHONE_NUMBER :
        CallLogSingleConstant.CALL_LOG_PHONE_NUMBER, phoneNumberArr)
        .beginWrap()
        .isNull(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY)
        .or()
        .equalTo(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, '')
        .or()
        .equalTo(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, '0')
        .endWrap();
      const updateBucket: ValuesBucket = {};
      updateBucket[CallLogSingleConstant.CALL_LOG_DISPLAY_NAME] = displayName;
      updateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = contactId.toString();
      updateBucket[CallLogSingleConstant.CALL_LOG_EXTRA_1] = contactId.toString();
      updateBucket[CallLogSingleConstant.IS_CNAP] = 0;
      updateBucket[CallLogSingleConstant.NEWCALL] = 0;
      const ret = await this.updateData(updateBucket, predicates);
      if (ret < 0) {
        LogUtils.e(TAG, `updateCallLogWhenContactAddPhone err ,updateLog:${updateLog}, ret: ` + ret);
      }
      if (contactId === 0) {
        LogUtils.w(TAG, `updateCallLogWhenContactAddPhone abnormal contactId, updateLog:${updateLog}`);
      }
      return ret;
    } catch (err) {
      LogUtils.e(TAG, `updateCallLogWhenContactAddPhone err ,updateLog:${updateLog}, code: ` + err?.code +
        ', err message: ' + err?.message);
    }
    return -1;
  }

  public async updateCallLogWhenContactDeletePhone(contactId: number,
    displayName: string, phoneNumberArr: string[], isFormatNumber: boolean) {
    const updateLog = `updateCallLogWhenContactDeletePhone start,contactId:${contactId},displayName:${
    StringUtil.maskSensitiveInfo(displayName)},phoneNumberArr:${StringUtil.maskSensitiveInfo(phoneNumberArr?.join())}`;
    try {
      const detailInfoSet = new Set<string>(phoneNumberArr);
      const phoneToNewContactId = await this.checkPhoneInOtherContact([contactId.toString()], phoneNumberArr,
        detailInfoSet);
      if (phoneToNewContactId.size > 0) {
        await this.updateCallLogToOtherContact(phoneToNewContactId);
      }
      if (detailInfoSet.size > 0) {
        const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
        predicates.in(isFormatNumber ? CallLogSingleConstant.CALL_LOG_FORMAT_PHONE_NUMBER :
          CallLogSingleConstant.CALL_LOG_PHONE_NUMBER, Array.from(detailInfoSet))
          .and()
          .equalTo(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, contactId.toString());
        const updateBucket: ValuesBucket = {};
        updateBucket[CallLogSingleConstant.CALL_LOG_DISPLAY_NAME] = null;
        updateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = null;
        updateBucket[CallLogSingleConstant.CALL_LOG_EXTRA_1] = null;
        const ret = await this.updateData(updateBucket, predicates);
        if (ret < 0) {
          LogUtils.e(TAG, `updateCallLogWhenContactDeletePhone err ,updateLog:${updateLog}, ret: ` + ret);
        }
        return ret;
      }
      return 0;
    } catch (err) {
      LogUtils.e(TAG, `updateCallLogWhenContactDeletePhone err ,updateLog:${updateLog}, code: ` + err?.code +
        ', err message: ' + err?.message);
    }
    return -1;
  }

  /**
   * 更新关联到联系人的通话记录  的联系人信息为空
   * @returns
   */
  public async updateCallLogContactInfoToNull() {
    LogUtils.w(TAG, 'updateCallLogContactInfoToNull start');
    try {
      const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
      // 将关联到联系人的通话记录的联系人信息置空（quickSearchKey不为空，且不为空串）
      // 如果没关联到联系人，可能displayName存了黄页信息，quickSearchKey为空
      predicates.beginWrap()
        .isNotNull(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY)
        .and()
        .notEqualTo(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, '')
        .endWrap();
      const updateBucket: ValuesBucket = {};
      updateBucket[CallLogSingleConstant.CALL_LOG_DISPLAY_NAME] = null;
      updateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = null;
      updateBucket[CallLogSingleConstant.CALL_LOG_EXTRA_1] = null;
      const ret = await this.updateData(updateBucket, predicates);
      if (ret < 0) {
        LogUtils.e(TAG, `updateCallLogContactInfoToNull size < 0, ret:` + ret);
      }
      return ret;
    } catch (err) {
      LogUtils.e(TAG, `updateCallLogContactInfoToNull err, code: ` + err?.code +
        ', err message: ' + err?.message);
    }
    LogUtils.w(TAG, 'updateCallLogContactInfoToNull end');
    return -1;
  }

  public async updateCallLogAfterAggregation(aggregationContactIdList: number[], masterContactId: number) {
    try {
      const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
      predicates.in(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, aggregationContactIdList.map(String));
      const updateBucket: ValuesBucket = {};
      updateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = masterContactId.toString();
      updateBucket[CallLogSingleConstant.CALL_LOG_EXTRA_1] = masterContactId.toString();
      const ret = await this.updateData(updateBucket, predicates);
      if (ret < 0) {
        LogUtils.e(TAG, `updateCallLogAfterAggregation err, aggregationContactIdList:${aggregationContactIdList},
          masterContactId:${masterContactId}, ret: ` + ret);
      }
      return ret;
    } catch (err) {
      LogUtils.e(TAG, `updateCallLogAfterAggregation err, aggregationContactIdList:${aggregationContactIdList},
          masterContactId:${masterContactId}, code: ` + err?.code + ', err message: ' + err?.message);
    }
    return -1;
  }

  /**
   * 根据电话号码查询未关联到联系人的通话记录，存在的话，需要根据电话号码更新对应的通话记录
   * @param contactMap
   * @returns
   */
  public async queryHasCallLogPhoneNumber(contactMap: Map<number, InsertContactBoUpdateCallLog>):
    Promise<Set<string>> {
    let hasCallLogPhoneNumberSet = new Set<string>();
    if (this.rdbStore === undefined) {
      LogUtils.e(TAG, `rdbStore is undefined`);
      return hasCallLogPhoneNumberSet;
    }
    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      let phoneFormatNumberSet = new Set<string>();
      let phoneNumberSet = new Set<string>();
      // 查询一次通话记录中是否有对应的号码，有的话需要根据电话号码和联系人id更新，否则不用更新
      for (let insertContactBoUpdateCallLog of contactMap.values()) {
        insertContactBoUpdateCallLog.extraInfo.addFormatPhoneList.forEach(formatPhoneNumber => {
          phoneFormatNumberSet.add(formatPhoneNumber);
        });
        insertContactBoUpdateCallLog.extraInfo.addPhoneList.forEach(phoneNumber => {
          phoneNumberSet.add(phoneNumber);
        });
      }
      if (phoneFormatNumberSet.size > 0) {
        const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
        predicates.in(CallLogSingleConstant.CALL_LOG_FORMAT_PHONE_NUMBER, Array.from(phoneFormatNumberSet))
        .beginWrap()
          .isNull(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY)
          .or()
          .equalTo(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, '')
          .endWrap();
        resultSet =
          await this.rdbStore?.query(predicates, ['distinct ' + CallLogSingleConstant.CALL_LOG_FORMAT_PHONE_NUMBER]);
        if (resultSet && resultSet.goToFirstRow()) {
          do {
            let formatPhoneNumber = resultSet.getString(0);
            hasCallLogPhoneNumberSet.add(formatPhoneNumber);
          } while (resultSet.goToNextRow());
        }
      }
      if (phoneNumberSet.size > 0) {
        const predicates = new rdb.RdbPredicates(CALL_LOG_TABLE_NAME);
        predicates.in(CallLogSingleConstant.CALL_LOG_PHONE_NUMBER, Array.from(phoneNumberSet)).beginWrap()
          .isNull(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY)
          .or()
          .equalTo(CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY, '')
          .endWrap();
        let resultSet = await this.rdbStore?.query(predicates, ['distinct ' + CallLogSingleConstant.CALL_LOG_PHONE_NUMBER]);
        if (resultSet && resultSet.goToFirstRow()) {
          do {
            let phoneNumber = resultSet.getString(0);
            hasCallLogPhoneNumberSet.add(phoneNumber);
          } while (resultSet.goToNextRow());
        }
      }
    } catch (e) {
      LogUtils.e(TAG, 'query phoneNumber is has calllog error:' + e.code + '--' + e.message)
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }

    return hasCallLogPhoneNumberSet;
  }

  /**
   * 查询calllog表相关统计信息信息
   * @param rdbStoreHelper
   * @param callCount
   * @returns
   */
  public async queryCallContactInfo(rdbStoreHelper: rdb.RdbStore, callCount: CallStatisticsCount): Promise<void> {
    let selectSql = 'select count(1), count(case when call_direction = ' + CallLogSingleConstant.CALL_DIRECTION_IN +
      ' then 1 end), count(case when call_direction  = ' + CallLogSingleConstant.CALL_DIRECTION_OUT +
      ' then 1 end), count(case when answer_state = ' + CallLogSingleConstant.CALL_ANSWER_BLOCKED +
      ' then 1 end), count(case when privacy_tag != -1 then 1 end) from calllog';
    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      resultSet = await rdbStoreHelper.querySql(selectSql);
      if (resultSet?.goToFirstRow()) {
        let index: number = 0;
        callCount.AFTER_TOTAL = resultSet.getLong(index++); // 总数量
        callCount.INCOMING_CALL_TOTAL = resultSet.getLong(index++); // 来电总数量
        callCount.OUTGOING_CALL_TOTAL = resultSet.getLong(index++); // 去电总数量
        callCount.INTERCEPT_CALL_TOTAL = resultSet.getLong(index++); // 拦截总数量
        callCount.PRIVATE_CALL_TOTAL = resultSet.getLong(index++); // 私密总数量
      }
    } catch (e) {
      LogUtils.e(TAG, 'queryCallContactInfo error:' + e?.code + '--' + e?.message);
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
  }
}

export const callLogRdbHelper: CallLogRdbHelperForCloudSync = new CallLogRdbHelperForCloudSync();