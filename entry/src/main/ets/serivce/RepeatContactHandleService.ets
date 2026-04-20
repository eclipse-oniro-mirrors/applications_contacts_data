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
import LogUtils from '../common/utils/LogUtils';
import HashMap from '@ohos.util.HashMap';
import common from '@ohos.app.ability.common';
import { JSON } from '@kit.ArkTS';
import { StringUtil } from '../common/utils/StringUtil';
import RawContactsColumns from '../common/data/RawContactsColumns';
import rdb from '@ohos.data.relationalStore';
import MergeRawContact from '../common/data/MergeRawContact';
import HandleUniqueKeyService, { MergeHandleCount } from './HandleUniqueKeyService';
import ContactsDataService from './ContactsDataService';
import { dataShare } from '@kit.ArkData';
import { Data } from '../common/data/Data';
import ContactAggregator from '../aggregation/ContactAggregator';
import { contactRdbHelper } from '../common/helperUtils/ContactsRdbHelperForCloudSync';
import DbConstants, { IsMyCard } from '../common/DbConstants';
import CallLogSingleConstant from '../common/contract/CallLogSingleConstant';
import { AutoMergeHandleCountReport, ManualMergeHandleCountReport } from '../common/utils/ContactStatisticsCount';

/**
 * 自动合并
 * 合并重复联系人任务；检查重复联系人
 * @param context
 * @param autoMergeStatus
 */
export function checkContactTask(context: common.Context, isAutoMerge: boolean = false): Promise<boolean> {
  return new Promise(async (resolve, reject) => {
    let mergeHandleCountTotal = new AutoMergeHandleCountReport();
    LogUtils.i('TaskService', 'checkContactTask Start to checkContactTask context is :' + context);
    try {
      // 检查重复联系人，返回重复联系人集合
      let repeatContacts: MergeRawContact[][] = await checkRepeatContacts(context);
      LogUtils.w('TaskService', 'checkContactTask repeatContacts task success!')

      if (repeatContacts.length <= 0) {
        LogUtils.w('TaskService', 'checkContactTask no repeat contact');
        await ContactAggregator.getInstance().aggregateAll(context, true);
        resolve(true);
      } else {
        // 合并重复联系人
        for (let i = 0; i < repeatContacts.length; ++i) {
          let sameNameAndPhoneContacts: MergeRawContact[] = repeatContacts[i];
          if (sameNameAndPhoneContacts.length > 1) {
            // 合并
            let mergeHandleCount =
              await HandleUniqueKeyService.getInstance()
                .mergeToMasterContact(sameNameAndPhoneContacts, context, isAutoMerge);
            mergeHandleCountTotal.HARD_DEL_NUM += mergeHandleCount.hardDelCount;
            mergeHandleCountTotal.SOFT_DEL_COUNT += mergeHandleCount.softDelCount;
          }
          // 每合并完50个，通知页面刷新
          if (i > 0 && i % 50 === 0) {
            const dataShareHelper: dataShare.DataShareHelper | undefined = await ContactsDataService.getInstance()
              .getDataAbilityHelper();
            await dataShareHelper.notifyChange(Data.CONTENT_URI);
          }
        }
        // 智能合并
        await ContactAggregator.getInstance().aggregateAll(context, false);
        LogUtils.w('TaskService', 'checkContactTask autoMerge end');

        const dataShareHelper: dataShare.DataShareHelper | undefined = await ContactsDataService.getInstance()
          .getDataAbilityHelper();
        await dataShareHelper.notifyChange(Data.CONTENT_URI);
        await ContactsDataService.getInstance().closeDataAbilityHelper();

        const callLogDataShareHelper: dataShare.DataShareHelper | undefined = await ContactsDataService.getInstance()
          .getCallLogDataAbilityHelper();
        await callLogDataShareHelper?.notifyChange(CallLogSingleConstant.CALL_LOG_URI);
        await ContactsDataService.getInstance().closeCalllogDataAbilityHelper();

        await HandleUniqueKeyService.getInstance().notifyFormIdChange(context);
        resolve(true);
      }
    } catch (e) {
      LogUtils.e('TaskService', 'checkContactTask err: ' + e?.message + ', stack: ' + e?.stack());
      resolve(false);
    }
  });
}

/**
 * 手动合并重复联系人任务
 * @param context
 * @param mergeContactArgs 手动合并重复联系人，重复联系人id，json字符串，两维数组，每个元素为一组重复联系人集合
 */
export function manualMergeContactTask(context: common.Context, mergeContactArgs: string): Promise<boolean> {
  let mergeHandleCountTotal = new ManualMergeHandleCountReport();
  return new Promise(async (resolve, reject) => {
    LogUtils.w('TaskService', `manualMergeContactTask Start! context is :${context},mergeContactArgs is ${
      mergeContactArgs}`);
    try {
      const rdbStore = await contactRdbHelper.initDB(context);
      if (!rdbStore) {
        LogUtils.e('TaskService', `manualMergeContactTask rdbStore init error!`);
        resolve(false);
        return;
      }
      // 重复联系人的集合，作为参数传过来；每个元素为重复数据id
      const repeatContactIds: string[][] = JSON.parse(mergeContactArgs) as string[][];
      if (!repeatContactIds.length || !repeatContactIds[0].length) {
        LogUtils.e('TaskService', `manualMergeContactTask get invalid mergeContactArgs`);
        resolve(false);
        return;
      }

      for (let i = 0; i < repeatContactIds.length; ++i) {
        // 处理一组重复的联系人
        let contacts: MergeRawContact[] = []
        // 一组重复联系人的contactId集合
        let contactIds = repeatContactIds[i];
        if (contactIds.length <= 1) {
          continue;
        }
        LogUtils.i('TaskService', `manualMergeContactTask get MergeRawContact! index: ${i}, contactIds: ${contactIds}`);

        const rawCondition = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
        rawCondition.in(RawContactsColumns.CONTACT_ID, contactIds)
          .and()
          .equalTo(RawContactsColumns.IS_DELETED, 0)

        const contactIdArr: number[] = [];
        const idToMergeRawContact: Map<number, MergeRawContact[]> = new Map();
        let rawDataResultSet: rdb.ResultSet | null | undefined = null;
        try {
          rawDataResultSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, rawCondition,
          MergeRawContact.columns);

          if (!rawDataResultSet || !rawDataResultSet.goToFirstRow()) {
            LogUtils.e('TaskService', 'manualMergeContactTask getAllContactData not found.');
            continue;
          }
          LogUtils.i('TaskService', `manualMergeContactTask query count: ${rawDataResultSet.rowCount}`);

          rawDataResultSet.goToFirstRow();
          do {
            const mergeRawContact = MergeRawContact.fromRuleSet(rawDataResultSet);
            contacts.push(mergeRawContact);
            contactIdArr.push(mergeRawContact.contactId);
            if (idToMergeRawContact.has(mergeRawContact.contactId)) {
              idToMergeRawContact.get(mergeRawContact.contactId)?.push(mergeRawContact);
            } else {
              idToMergeRawContact.set(mergeRawContact.contactId, [mergeRawContact]);
            }
          } while (rawDataResultSet.goToNextRow());
        } finally {
          if (rawDataResultSet && !rawDataResultSet.isClosed) {
            rawDataResultSet.close();
          }
        }

        LogUtils.i('TaskService', `manualMergeContactTask getAutoList is succeed! index: ${i
          }, contacts length: ${contacts.length}, contacts: ${MergeRawContact.getLogStr(contacts)}`);
        if (contacts.length > 1) {
          await HandleUniqueKeyService.getInstance().addContactInfo(contactIdArr, idToMergeRawContact);
          // 合并重复联系人，没新增的，只有硬删，软删的
          let mergeHandleCount = await HandleUniqueKeyService.getInstance().mergeToMasterContact(contacts, context, false);
          mergeHandleCountTotal.HARD_DEL_NUM += mergeHandleCount.hardDelCount;
          mergeHandleCountTotal.SOFT_DEL_NUM += mergeHandleCount.softDelCount;
        }
      }

      LogUtils.w('TaskService', 'manualMergeContactTask autoMerge end');

      const dataShareHelper: dataShare.DataShareHelper | undefined = await ContactsDataService.getInstance()
        .getDataAbilityHelper();
      await dataShareHelper?.notifyChange(Data.CONTENT_URI);
      await ContactsDataService.getInstance().closeDataAbilityHelper();

      const callLogDataShareHelper: dataShare.DataShareHelper | undefined = await ContactsDataService.getInstance()
      .getCallLogDataAbilityHelper();
      await callLogDataShareHelper?.notifyChange(CallLogSingleConstant.CALL_LOG_URI);
      await ContactsDataService.getInstance().closeCalllogDataAbilityHelper();

      await HandleUniqueKeyService.getInstance().notifyFormIdChange(context);
      resolve(true);
    } catch (err) {
      LogUtils.e('TaskService', 'manualMergeContactTask err: ' + err?.message + ', stack: ' + err?.stack());
      resolve(false);
    }
  });
}

/**
 *
 * @param context
 * @returns 数组每个元素，为一组重复的联系人
 */
async function checkRepeatContacts(context: common.Context): Promise<MergeRawContact[][]> {
  LogUtils.i('TaskService', 'Start to query all contacts with PhoneNumbers context is :' + context);
  // 单次最多查询 500条数据
  let offset = 0;
  let isEnd = false;
  const MAX_DATA_LIMIT = 500;
  let repeatContacts: MergeRawContact[][] = []
  // {uniqueKey，重复的联系人}
  let uniqueContactIdMap: HashMap<string, MergeRawContact[]> = new HashMap();
  try {
    const rdbStore = await contactRdbHelper.initDB(context);
    if (!rdbStore) {
      LogUtils.e('TaskService', `rdbStore init error!`);
      return [];
    }

    while (!isEnd) {
      const rawCondition = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
      // uniqyeKey存在值，未删除
      rawCondition.isNotNull(RawContactsColumns.UNIQUE_KEY)
        .and()
        .notEqualTo(RawContactsColumns.UNIQUE_KEY, '')
        .and()
        .equalTo(RawContactsColumns.IS_DELETED, 0)
        .and()
        .notEqualTo(RawContactsColumns.PRIMARY_CONTACT, IsMyCard.YES)// 过滤我的名片
        .limitAs(MAX_DATA_LIMIT) // 一批处理500条
        .offsetAs(offset);

      LogUtils.i('TaskService', `checkRepeatContacts MAX_DATA_LIMIT: ${MAX_DATA_LIMIT}, offset: ${offset}`);

      let resultSet: rdb.ResultSet | null | undefined = null;
      try {
        resultSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, rawCondition,
          MergeRawContact.columns);

        if (!resultSet) {
          LogUtils.e('TaskService', 'checkRepeatContacts getAllContact UNIQUE_KEY not found.');
          break;
        }
        LogUtils.i('TaskService', `checkRepeatContacts query count: ${resultSet.rowCount}`);
        offset += MAX_DATA_LIMIT;

        // 处理结束
        if (resultSet.rowCount < MAX_DATA_LIMIT) {
          isEnd = true;
        }
        if (!resultSet.goToFirstRow()) {
          break;
        }
        do {
          const mergeRawContact = MergeRawContact.fromRuleSet(resultSet);
          if (!StringUtil.isEmpty(mergeRawContact.uniqueKey)) {
            if (uniqueContactIdMap.hasKey(mergeRawContact.uniqueKey)) {
              uniqueContactIdMap.get(mergeRawContact.uniqueKey).push(mergeRawContact);
            } else {
              uniqueContactIdMap.set(mergeRawContact.uniqueKey, [mergeRawContact]);
            }
          }
        } while (resultSet.goToNextRow());
      } finally {
        if (resultSet && !resultSet.isClosed) {
          resultSet.close();
        }
      }
    }

    let repeatNum = 0;
    const contactIdArr: number[] = [];
    const idToMergeRawContact: Map<number, MergeRawContact[]> = new Map();
    uniqueContactIdMap.forEach((mergeRawContacts: MergeRawContact[]) => {
      if (mergeRawContacts.length > 1) {
        repeatNum += mergeRawContacts.length;
        repeatContacts.push(mergeRawContacts);
        mergeRawContacts.forEach((mergeRawContact: MergeRawContact) => {
          contactIdArr.push(mergeRawContact.contactId);
          if (idToMergeRawContact.has(mergeRawContact.contactId)) {
            idToMergeRawContact.get(mergeRawContact.contactId)?.push(mergeRawContact);
          } else {
            idToMergeRawContact.set(mergeRawContact.contactId, [mergeRawContact]);
          }
        })
      }
    })
    await HandleUniqueKeyService.getInstance().addContactInfo(contactIdArr, idToMergeRawContact);
    LogUtils.w('TaskService', `getAutoList is succeed! repeatNum: ${repeatNum
    }, repeatContactIds length: ${repeatContacts.length}, repeatContactIds: ${MergeRawContact.getLogStr(repeatContacts
      .flat())}`);
    return repeatContacts;
  } catch (err) {
    LogUtils.e('TaskService', `getAutoList is false! err: ${err.message}, code: ${err.code}`);
    return repeatContacts;
  }
}