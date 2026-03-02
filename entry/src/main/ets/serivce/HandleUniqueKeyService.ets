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
import rdb from '@ohos.data.relationalStore';
import MergeRawContact from '../common/data/MergeRawContact';
import SimpleRawContact from '../common/data/SimpleRawContact';
import SimpleContactData from '../common/data/SimpleContactData';
import FullContactData from '../common/data/FullContactData';
import MasterContact from '../common/data/MasterContact';
import DataColumns from '../common/data/DataColumns';
import { HashUtils } from '../common/utils/HashUtils';
import { getFormatNumber } from '../common/utils/NumberFormatUtil';
import { StringUtil } from '../common/utils/StringUtil';
import { Contact } from '../components/PlatformInterface/Contact';
import dataShare from '@ohos.data.dataShare';
import dataSharePredicates from '@ohos.data.dataSharePredicates';
import common from '@ohos.app.ability.common';
import RawContactsColumns from '../common/data/RawContactsColumns';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import { ContactInfo } from '../common/data/ContactInfo';
import ContactsDataService, { ExtraInfo, DisPlayNameResult, PhotoFileInfo } from './ContactsDataService';
import { Contacts } from '../common/data/Contacts';
import { DirtyDataClearServiceUpdate } from './DirtyDataClearServiceUpdate';
import { BusinessError } from '@ohos.base';
import BirthdayManager from './BirthdayManager';
import ContactAggregator from '../aggregation/ContactAggregator';
import fs from '@ohos.file.fs';
import { getPixelMapFromFromUint8Array } from '../common/utils/GroupIdUtil';
import { getImageInfoFromUint8Array } from '../common/utils/GroupIdUtil';
import { contactRdbHelper } from '../common/helperUtils/ContactsRdbHelperForCloudSync';
import DbConstants, { DIRTY_ENUM, EVENT_LABEL_ID, IsMyCard } from '../common/DbConstants';
import ContactsColumns from '../common/contract/ContactsColumns';
import CallLogSingleConstant from '../common/contract/CallLogSingleConstant';
import { DeleteRawContact } from '../common/contract/DeleteRawContact';
import { callLogRdbHelper } from '../common/helperUtils/CallLogRdbHelperForCloudSync';
import GroupIdUtil from '../common/utils/GroupIdUtil';
import { image } from '@kit.ImageKit';

/**
 * 合并处理的数量；回收站处理或列表处理
 */
export class MergeHandleCount {
  // 插入成功
  public insertSuccessCount = 0;
  // 硬删除
  public hardDelCount = 0;
  // 软删
  public softDelCount = 0;
  // 插入失败
  public insertFailCount = 0;
  // 更新数量（云下行，存在uniqueKey相同的数据，uuid没值的数据，将云下行的uuid更新到本地数据上）
  public updateUuidToLocal = 0;
}

const TAG = 'HandleUniqueKeyService';
const SPACE_OR_DASH_REG = /[\s-]/g;
const LABEL_ID_EMPTY_SLOT = 0;
const LABEL_NAME_EMPTY_SLOT = '';
const TEMP_UNIQUE_KEY_PREFIX = '&&&&&&';
const CONTACT_DB_PACKAGE = 'com.ohos.contactsdataability';
const formIdSeparator = '//';
const MERGE_TYPE_ARRAY: number[] = [
  DbConstants.FOUNDATION_TYPE_ID_MAP.EMAIL,
  DbConstants.FOUNDATION_TYPE_ID_MAP.IM,
  DbConstants.FOUNDATION_TYPE_ID_MAP.NICKNAME,
  DbConstants.FOUNDATION_TYPE_ID_MAP.ORGANIZATION,
  DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE,
  DbConstants.FOUNDATION_TYPE_ID_MAP.POSTAL_ADDRESS,
  DbConstants.FOUNDATION_TYPE_ID_MAP.PHOTO,
  DbConstants.FOUNDATION_TYPE_ID_MAP.GROUP_MEMBERSHIP,
  DbConstants.FOUNDATION_TYPE_ID_MAP.NOTE,
  DbConstants.FOUNDATION_TYPE_ID_MAP.CONTACT_EVENT,
  DbConstants.FOUNDATION_TYPE_ID_MAP.WEBSITE,
  DbConstants.FOUNDATION_TYPE_ID_MAP.RELATION,
  DbConstants.FOUNDATION_TYPE_ID_MAP.POSTER,

];
export const UNIQUE_KEY_TYPE_SET: Set<number> = new Set([
  DbConstants.FOUNDATION_TYPE_ID_MAP.EMAIL,
  DbConstants.FOUNDATION_TYPE_ID_MAP.IM,
  DbConstants.FOUNDATION_TYPE_ID_MAP.NICKNAME,
  DbConstants.FOUNDATION_TYPE_ID_MAP.ORGANIZATION,
  DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE,
  DbConstants.FOUNDATION_TYPE_ID_MAP.NAME,
  DbConstants.FOUNDATION_TYPE_ID_MAP.POSTAL_ADDRESS,
  DbConstants.FOUNDATION_TYPE_ID_MAP.NOTE,
  DbConstants.FOUNDATION_TYPE_ID_MAP.CONTACT_EVENT,
  DbConstants.FOUNDATION_TYPE_ID_MAP.WEBSITE,
  DbConstants.FOUNDATION_TYPE_ID_MAP.RELATION,
]);
const MERGE_TYPE_SET: Set<number> = new Set(MERGE_TYPE_ARRAY);
const WAIT_FOR_CALC_UNIQUE_KEY_IN_MS: number = 100;//每次等待100ms
const WAIT_FOR_CALC_UNIQUE_KEY_MAX_COUNT: number = 4500;//最多等待4500次，4500 * 100 = 450000 ms, 450s，也就是7.5分钟
const CALC_UNIQUEKEY_PAGE: number = 100;

export default class HandleUniqueKeyService {
  public static instance: HandleUniqueKeyService;
  public isCalcingUniqueKey: boolean = false;
  public hash: HashUtils = HashUtils.getInstance();
  private migrateDataShareHelper?: dataShare.DataShareHelper;
  private proxyDataShareHelper?: dataShare.DataShareHelper;
  public constructor() {
  }

  public static getInstance(): HandleUniqueKeyService {
    if (HandleUniqueKeyService.instance == null) {
      HandleUniqueKeyService.instance = new HandleUniqueKeyService();
    }
    return HandleUniqueKeyService.instance;
  }

  public isCalcUniqueKey(): boolean {
    return this.isCalcingUniqueKey;
  }

  /**
   * 等待计算uniqueKey
   * @param context
   * @returns
   */
  public async waitForCalcUniqueKeyEnd(context: common.Context): Promise<void> {
    if (this.isCalcingUniqueKey) {
      const startTs: number = Date.now();
      LogUtils.w(TAG, `isCalcingUniqueKey true, wait for isCalcingUniqueKey end,startTs:${startTs}`);
      let waitCount = WAIT_FOR_CALC_UNIQUE_KEY_MAX_COUNT;
      // 如果还在计算 uniqueKey，并且等待次数没到，则继续等待
      while (this.isCalcingUniqueKey && waitCount > 0) {
        waitCount--;
        await contactRdbHelper.delay(WAIT_FOR_CALC_UNIQUE_KEY_IN_MS * 2);
      }
      LogUtils.w(TAG, `wait for isCalcingUniqueKey end,isCalcingUniqueKey:${this.isCalcingUniqueKey},startTs:${
        startTs},used time:${Date.now() - startTs} ms,left waitCount:${waitCount}`);
    } else {
      LogUtils.w(TAG, `isCalcingUniqueKey false, trigger calcUniqueKey`);
      await this.calcUniqueKey(context);
    }
  }

  private async calcUniqueKey(context: common.Context): Promise<void> {
    if (this.isCalcingUniqueKey) {
      LogUtils.w(TAG, `isCalcingUniqueKey true, only one task can run, return`);
      return;
    }
    const rdbStore = await contactRdbHelper.initDB(context);
    if (!rdbStore) {
      LogUtils.w(TAG, `rdbStore is null, return`);
      return;
    }
    this.isCalcingUniqueKey = true;

    try {
      await this.calcUniqueKeyByPage();
      this.isCalcingUniqueKey = false;
    } catch (e) {
      LogUtils.e(TAG, `calcUniqueKey err:${e.message},stack:${e.stack}`);
      this.isCalcingUniqueKey = false;
    }
  }

  private async calcUniqueKeyByPage(): Promise<void> {
    let simpleRawContactArr: SimpleRawContact[] = await this.getNeedRecalcContactByPage();
    let dirtyNeedRecalcCount: number = 0;
    let isDeletedCount: number = 0;
    while (true) {
      if (simpleRawContactArr.length == 0) {
        LogUtils.w(TAG, `no more data need handle, calcUniqueKey end.`);
        break;
      }
      for (const simpleRawContact of simpleRawContactArr) {
        if (simpleRawContact.isDeleted == 1) {
          isDeletedCount++;
        }
        if (simpleRawContact.dirty == DIRTY_ENUM.NEED_RECALC) {
          dirtyNeedRecalcCount++;
        }
        const uniqueKey: string = await this.getUniqueKeyByRawData(simpleRawContact);
        await this.updateUniqueKey(simpleRawContact.rawId, uniqueKey, simpleRawContact.dirty);
      }
      // 如果本轮待处理的数量，少于一批处理量，说明没有更多数据需要处理
      if (simpleRawContactArr.length < CALC_UNIQUEKEY_PAGE) {
        LogUtils.w(TAG, `no more data need handle, calcUniqueKey end.`);
        break;
      }
      // 等待 100ms 再处理下一批，避免大数据量下CPU超基线被冻结
      await contactRdbHelper.delay(WAIT_FOR_CALC_UNIQUE_KEY_IN_MS);
      simpleRawContactArr = await this.getNeedRecalcContactByPage();
    }
    LogUtils.w(TAG, `calcUniqueKey end, handle data size is:${simpleRawContactArr.length},dirtyNeedRecalcCount:${
      dirtyNeedRecalcCount},isDeletedCount:${isDeletedCount}`);
  }

  public async updateUniqueKey(rawId: number, uniqueKey: string, dirty: number) {
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
    predicates.equalTo('id', rawId);
    const updateBucket: ValuesBucket = {};
    updateBucket[RawContactsColumns.UNIQUE_KEY] = uniqueKey;
    updateBucket[RawContactsColumns.DIRTY] =
      dirty === DIRTY_ENUM.REFRESH_UNIQUE_KEY ? DIRTY_ENUM.NOT_DIRTY : DIRTY_ENUM.NO_NEED_RECALC;
    const ret = await contactRdbHelper.updateData(updateBucket, predicates);
    if (ret < 0) {
      throw new Error(`updateUniqueKey err,ret:${ret}`);
    }
  }

  public async getNeedRecalcContactByPage() {
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
    predicates.isNull(RawContactsColumns.UNIQUE_KEY)
      .or()
      .equalTo(RawContactsColumns.UNIQUE_KEY, '')
      .or()
      .equalTo(RawContactsColumns.DIRTY, DIRTY_ENUM.NEED_RECALC)
      .limitAs(CALC_UNIQUEKEY_PAGE);

    const simpleRawContactArr: SimpleRawContact[] = [];
    let ruleSet: rdb.ResultSet | null | undefined = null;
    try {
      ruleSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, predicates,
      SimpleRawContact.columns);
      if (!ruleSet || !ruleSet.goToFirstRow()) {
        return simpleRawContactArr;
      }
      do {
        simpleRawContactArr.push(SimpleRawContact.fromRuleSet(ruleSet));
      } while (ruleSet.goToNextRow());
    } finally {
      if (ruleSet && !ruleSet.isClosed) {
        ruleSet.close();
      }
    }
    return simpleRawContactArr;
  }

  public async getUniqueKeyByRawData(simpleRawContact: SimpleRawContact) {
    const rawId: number = simpleRawContact.rawId;
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_DATA_TABLE_NAME);
    predicates.equalTo(DataColumns.RAW_CONTACT_ID, rawId);

    const nameAndPhoneArr: SimpleContactData[] = [];
    const otherDataArr: SimpleContactData[] = [];

    let ruleSet: rdb.ResultSet | null | undefined = null;
    try {
      ruleSet = await contactRdbHelper.queryData(DbConstants.NEW_DATA_TABLE_NAME, predicates,
      SimpleContactData.columns);
      if (!ruleSet || !ruleSet.goToFirstRow()) {
        // 没有详情数据，存临时key
        const tempUniqueKey = `${TEMP_UNIQUE_KEY_PREFIX}|${rawId}`
        LogUtils.w(TAG, `get no data when generate unique key, set unique key to :${tempUniqueKey}`);
        return tempUniqueKey;
      }
      do {
        const data: SimpleContactData = SimpleContactData.fromResultSet(ruleSet);
        if (!UNIQUE_KEY_TYPE_SET.has(data.typeId)) {
          continue;
        }
        if (data.typeId === DbConstants.FOUNDATION_TYPE_ID_MAP.NAME ||
          data.typeId === DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE) {
          nameAndPhoneArr.push(data);
        } else {
          otherDataArr.push(data);
        }
      } while (ruleSet.goToNextRow());
    } finally {
      if (ruleSet && !ruleSet.isClosed) {
        ruleSet.close();
      }
    }
    return this.getUniqueKeyBySimpleContactDataArr(rawId, nameAndPhoneArr, otherDataArr, simpleRawContact.isDeleted,
      simpleRawContact.dirty);
  }

  private getFormatNumberFromDetailInfo(detailInfo: string) {
    let formatPhoneNumber = getFormatNumber(detailInfo);
    if (formatPhoneNumber) {
      return formatPhoneNumber;
    }
    formatPhoneNumber = detailInfo.replace(SPACE_OR_DASH_REG, '');
    return formatPhoneNumber;
  }

  public getUniqueKeyBySimpleContactDataArr(rawId: number, nameAndPhoneArr: SimpleContactData[],
    otherDataArr: SimpleContactData[], isDeleted?: number, dirty?: number): string {
    if (nameAndPhoneArr.length > 0) {
      return this.getUniqueKeyByNameAndPhoneArr(rawId, nameAndPhoneArr, isDeleted, dirty);
    }
    return this.getUniqueKeyByOtherDataArr(rawId, otherDataArr, isDeleted, dirty);
  }

  public getUniqueKeyByNameAndPhoneArr(rawId: number, simpleContactDataArr: SimpleContactData[],
    isDeleted?: number, dirty?: number): string {
    let uniqueKey = '';
    const phoneDataArr: SimpleContactData[] = [];
    for (const simpleContactData of simpleContactDataArr) {
      if (simpleContactData.typeId == DbConstants.FOUNDATION_TYPE_ID_MAP.NAME) {
        if (simpleContactData.detailInfo && !uniqueKey) {
          // 姓名去掉收尾空格，放到 uniqueKey 第一位，假如有多个名称的脏数据，只取第一个名称
          uniqueKey = simpleContactData.detailInfo.trim();
        }
        continue;
      }
      // 上面只查了姓名号码，剩下的都是号码，优先取 format_phone_number，否则用原值
      if (simpleContactData.formatPhoneNumber) {
        phoneDataArr.push(simpleContactData);
        continue;
      }
      // 对于无格式化号码，再次尝试格式化
      const formatPhoneNumber = this.getFormatNumberFromDetailInfo(simpleContactData.detailInfo);
      simpleContactData.formatPhoneNumber = formatPhoneNumber;
      phoneDataArr.push(simpleContactData);
    }
    phoneDataArr.sort((a, b) => {
      // 首先根据 labelId 手机类型排序
      if (a.labelId !== b.labelId) {
        return a.labelId < b.labelId ? -1 : 1;
      }
      // 假如有自定义名称，则根据 labelName 自定义名称排序
      if (a.labelName !== b.labelName) {
        return a.labelName < b.labelName ? -1 : 1;
      }
      // 最后根据格式化号码排序
      if (a.formatPhoneNumber !== b.formatPhoneNumber) {
        return a.formatPhoneNumber < b.formatPhoneNumber ? -1 : 1;
      }
      // 如果所有比较的属性都相同，返回0，表示顺序不变
      return 0;
    });
    for (const simpleContactData of phoneDataArr) {
      uniqueKey +=
        `_${simpleContactData.labelId || LABEL_ID_EMPTY_SLOT}_${simpleContactData.labelName ||
        LABEL_NAME_EMPTY_SLOT}_${simpleContactData.formatPhoneNumber}`;
    }
    let hashValue: string = '';
    if (uniqueKey) {
      hashValue = this.hash.sha256DigestByMd(uniqueKey);
    } else {
      LogUtils.w(TAG, `get empty uniqueKey, rawId: ${rawId}`);
      hashValue = `${TEMP_UNIQUE_KEY_PREFIX}|${rawId}`;
    }
    LogUtils.i(TAG, `rawId:${rawId}${isDeleted !== undefined ? `,isDeleted:${isDeleted}` : ''}${dirty !== undefined ?
      `,dirty:${dirty}` : ''},uniqueKey:${StringUtil.getShortUniqueKey(hashValue)}`);
    return hashValue;
  }

  public getUniqueKeyByOtherDataArr(rawId: number, otherDataArr: SimpleContactData[],
    isDeleted?: number, dirty?: number): string {
    if (otherDataArr.length === 0) {
      LogUtils.w(TAG, `get no other data when calc uniqueKey, rawId: ${rawId}`);
      return `${TEMP_UNIQUE_KEY_PREFIX}|${rawId}`;
    }
    let uniqueKey = '';
    otherDataArr.sort((a, b) => {
      // 首先根据 typeId 类型排序
      if (a.typeId !== b.typeId) {
        return a.typeId < b.typeId ? -1 : 1;
      }
      // 其次根据 labelId 子类型排序
      if (a.labelId !== b.labelId) {
        return a.labelId < b.labelId ? -1 : 1;
      }
      // 假如有自定义名称，则根据 labelName 自定义名称排序
      if (a.labelName !== b.labelName) {
        return a.labelName < b.labelName ? -1 : 1;
      }
      // 根据格详情数据排序
      if (a.detailInfo !== b.detailInfo) {
        return a.detailInfo < b.detailInfo ? -1 : 1;
      }
      // 对于公司信息，还需要比较职位
      if (a.position !== b.position) {
        return a.position < b.position ? -1 : 1;
      }
      // 如果所有比较的属性都相同，返回0，表示顺序不变
      return 0;
    });
    for (const simpleContactData of otherDataArr) {
      uniqueKey +=
        `_${simpleContactData.typeId}_${simpleContactData.labelId || LABEL_ID_EMPTY_SLOT}_${
          simpleContactData.labelName || LABEL_NAME_EMPTY_SLOT}_${simpleContactData.detailInfo}${
            simpleContactData.position ? `_${simpleContactData.detailInfo}` : ''}`;
    }
    const hashValue = this.hash.sha256DigestByMd(uniqueKey);
    LogUtils.i(TAG, `rawId:${rawId}${isDeleted !== undefined ? `,isDeleted:${isDeleted}` : ''}${dirty !== undefined ?
      `,dirty:${dirty}` : ''},uniqueKey:${StringUtil.getShortUniqueKey(hashValue)}`);
    return hashValue;
  }

  public async getMergeDataByRawIds(rawIds: number[]) {
    if (!rawIds?.length) {
      return [];
    }
    LogUtils.i(TAG, `getMergeDataByRawIds rawIds:${rawIds}`);
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_DATA_TABLE_NAME);
    //所有data都查出来，避免数据库每行都做in判断
    //所有重复rawContact的data一次查出来，避免多次做全表扫描读取
    predicates.in(DataColumns.RAW_CONTACT_ID, rawIds);

    const fullContactDataArr: FullContactData[] = [];
    let ruleSet: rdb.ResultSet | null | undefined = null;
    try {
      ruleSet = await contactRdbHelper.queryData(DbConstants.NEW_DATA_TABLE_NAME, predicates, FullContactData.columns);
      if (!ruleSet || !ruleSet.goToFirstRow()) {
        LogUtils.w(TAG, `get no merge data for rawIds:${rawIds}`);
        return [];
      }
      do {
        const fullContactData = new FullContactData(ruleSet);
        if (MERGE_TYPE_SET.has(fullContactData.type_id)) {
          fullContactDataArr.push(fullContactData);
        }
      } while (ruleSet.goToNextRow());
    } finally {
      if (ruleSet && !ruleSet.isClosed) {
        ruleSet.close();
      }
    }
    return fullContactDataArr;
  }

  public async getMergeDataByRawId(rawId: number) {
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_DATA_TABLE_NAME);
    predicates.equalTo(DataColumns.RAW_CONTACT_ID, rawId);
    predicates.and();
    predicates.in(DataColumns.TYPE_ID, MERGE_TYPE_ARRAY);

    const fullContactDataArr: FullContactData[] = [];
    let ruleSet: rdb.ResultSet | null | undefined = null;
    try {
      ruleSet = await contactRdbHelper.queryData(DbConstants.NEW_DATA_TABLE_NAME, predicates,
        FullContactData.columns);
      if (!ruleSet || !ruleSet.goToFirstRow()) {
        LogUtils.w(TAG, `get no merge data for rawId:${rawId}`);
        return fullContactDataArr;
      }
      do {
        const fullContactData = new FullContactData(ruleSet);
        fullContactDataArr.push(fullContactData);
      } while (ruleSet.goToNextRow());
    } finally {
      if (ruleSet && !ruleSet.isClosed) {
        ruleSet.close();
      }
    }
    return fullContactDataArr;
  }

  public async getMergeRawContactArrByUniqueKey(contactInfo: ContactInfo, isDeleted: number) {

    const mergeRawContactArr: MergeRawContact[] = [];
    const contactIdArr: number[] = [];
    const idToMergeRawContact: Map<number, MergeRawContact[]> = new Map();
    await this.getMergeUuidByUniqueKey(contactInfo, isDeleted, mergeRawContactArr,
      contactIdArr, idToMergeRawContact);

    await this.getMergeNotUuidByUniqueKey(contactInfo, isDeleted, mergeRawContactArr,
      contactIdArr, idToMergeRawContact);
    await this.addContactInfo(contactIdArr, idToMergeRawContact);
    return mergeRawContactArr;
  }

  private async getMergeUuidByUniqueKey(contactInfo: ContactInfo, isDeleted: number,
    mergeRawContactArr: MergeRawContact[], contactIdArr: number[],
    idToMergeRawContact: Map<number, MergeRawContact[]>) {
      const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
      predicates.equalTo(RawContactsColumns.UNIQUE_KEY, contactInfo.uniqueKey);
      predicates.and();
      predicates.equalTo(RawContactsColumns.IS_DELETED, isDeleted);
      predicates.and();
      predicates.isNull(RawContactsColumns.UUID);
      predicates.and();
      predicates.notEqualTo(RawContactsColumns.PRIMARY_CONTACT, IsMyCard.YES);// 过滤我的名片
      predicates.orderByAsc(RawContactsColumns.ID);

    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      resultSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, predicates,
        MergeRawContact.columns);
      if (!resultSet || !resultSet.goToFirstRow()) {
        return mergeRawContactArr;
      }
      do {
        const mergeRawContact: MergeRawContact = MergeRawContact.fromRuleSet(resultSet);
        mergeRawContactArr.push(mergeRawContact);
        contactIdArr.push(mergeRawContact.contactId);
        if (idToMergeRawContact.has(mergeRawContact.contactId)) {
          idToMergeRawContact.get(mergeRawContact.contactId)?.push(mergeRawContact);
        } else {
          idToMergeRawContact.set(mergeRawContact.contactId, [mergeRawContact]);
        }
      } while (resultSet.goToNextRow());
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
      return mergeRawContactArr;
    }

  private async getMergeNotUuidByUniqueKey(contactInfo: ContactInfo, isDeleted: number,
    mergeRawContactArr: MergeRawContact[], contactIdArr: number[],
    idToMergeRawContact: Map<number, MergeRawContact[]>) {
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
    predicates.equalTo(RawContactsColumns.UNIQUE_KEY, contactInfo.uniqueKey);
    predicates.and();
    predicates.equalTo(RawContactsColumns.IS_DELETED, isDeleted);
    predicates.and();
    predicates.isNotNull(RawContactsColumns.UUID);
    predicates.and();
    predicates.notEqualTo(RawContactsColumns.PRIMARY_CONTACT, IsMyCard.YES);// 过滤我的名片
    predicates.orderByAsc(RawContactsColumns.ID);
    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      resultSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, predicates,
        MergeRawContact.columns);
      if (!resultSet || !resultSet.goToFirstRow()) {
        return mergeRawContactArr;
      }
      do {
        const mergeRawContact: MergeRawContact = MergeRawContact.fromRuleSet(resultSet);
        mergeRawContactArr.push(mergeRawContact);
        contactIdArr.push(mergeRawContact.contactId);
        if (idToMergeRawContact.has(mergeRawContact.contactId)) {
          idToMergeRawContact.get(mergeRawContact.contactId)?.push(mergeRawContact);
        } else {
          idToMergeRawContact.set(mergeRawContact.contactId, [mergeRawContact]);
        }

        if (contactInfo.updateFlag == 1 && (!contactInfo.phoneList || contactInfo.phoneList.length == 0)) {
          LogUtils.w(TAG, `getMergeNotUuidByUniqueKey`);
          break;
        }
      } while (resultSet.goToNextRow());
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    return mergeRawContactArr;
  }

  public async addContactInfo(contactIdArr: number[], idToMergeRawContact: Map<number, MergeRawContact[]>) {
    LogUtils.w(TAG, `addContactInfo contactIdArr:${contactIdArr}`);
    if (!contactIdArr?.length) {
      return;
    }
    const predicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
    // 如果只有一个元素，用 equalTo 效率更高（不依赖底层引擎进行优化）
    if (contactIdArr.length === 1) {
      predicates.equalTo(ContactsColumns.NEW_ID, contactIdArr[0]);
    } else {
      predicates.in(ContactsColumns.NEW_ID, contactIdArr);
    }
    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      resultSet = await contactRdbHelper.queryData(DbConstants.NEW_CONTACTS_TABLE_NAME, predicates,
        [ContactsColumns.NEW_ID, ContactsColumns.FORM_ID, ContactsColumns.FOCUS_MODE_LIST,
          ContactsColumns.PERSONAL_RINGTONE, ContactsColumns.PERSONAL_NOTIFICATION_RINGTONE,
            ContactsColumns.RINGTONE_PATH]);
      if (!resultSet || !resultSet.goToFirstRow()) {
        return;
      }
      do {
        const contactId = resultSet.getLong(0);
        const formId = resultSet.getString(1);
        const focusModeList = resultSet.getString(2);
        const personalRingtone = resultSet.getString(3);
        const personalNotificationRingtone = resultSet.getString(4);
        const ringtonePath = resultSet.getString(5);
        const mergeRawContactArr = idToMergeRawContact.get(contactId)!;
        for (const mergeRawContact of mergeRawContactArr) {
          mergeRawContact.contactIdExist = true;
          if (formId) {
            mergeRawContact.formId = formId;
          }
          if (focusModeList) {
            mergeRawContact.focusModeList = focusModeList;
          }
          if (personalRingtone) {
            mergeRawContact.personalRingtone = personalRingtone;
            mergeRawContact.personalNotificationRingtone = personalNotificationRingtone;
            mergeRawContact.ringtonePath = ringtonePath;
          }
        }
      } while (resultSet.goToNextRow());
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    return;
  }

  /**
   * 获取所有待合并联系人的大头像信息
   * @param mergeRawContactArr
   */
  async getPhotoFileInfoList(mergeRawContactArr: MergeRawContact[], context: common.Context) {
    const tagStr = TAG + ` bigPhotoFromFile getPhotoFileInfoList `;
    let photoFileInfoArr: PhotoFileInfo[] = [];
    try {
      let groupDir = await GroupIdUtil.getInstance().getGroupDir(context);
      const fullContactDataArr = await this.getMergeDataByRawIds(mergeRawContactArr.map(
        (item: MergeRawContact) => item.rawId));
      if (fullContactDataArr && fullContactDataArr.length) {
        for (const fullContactData of fullContactDataArr) {
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.PHOTO && fullContactData.blob_data &&
          fullContactData.detail_info) {
            let filePath = groupDir + '/' + fullContactData.detail_info;
            let fileFlag = await fs.access(filePath);
            if (fileFlag) {
              let photoFileInfo: PhotoFileInfo = {
                filePath: filePath,
                blobData: fullContactData.blob_data,
              }
              photoFileInfoArr.push(photoFileInfo);
            }
          }
        }
      }
      return photoFileInfoArr;
    } catch (e) {
      LogUtils.e(tagStr, `err,msg:${e.message}`);
      return photoFileInfoArr;
    }
  }

  /**
   * 保存合并之后的联系人大头像信息
   * @param masterContact
   * @param photoFileInfo
   */
  async saveMasterContactPhotoInfo(masterContact: MasterContact, photoFileInfoArrNew: PhotoFileInfo[],
    photoFileInfo: PhotoFileInfo, context: common.Context) {
    const tagStr = TAG + ` bigPhotoFromFile saveMasterContactPhotoInfo `;
    let queryResult: rdb.ResultSet | null | undefined = null;
    try {
      // 将大头像文件信息写入合并之后的联系人数据表中
      let groupDir = await GroupIdUtil.getInstance().getGroupDir(context);
      let fileDir = groupDir + '/photo';
      let blobSource = 0;
      let isOldFilePathAccess = await fs.access(photoFileInfo.filePath);
      if (isOldFilePathAccess) {
        blobSource = await ContactsDataService.getInstance().checkFileScale(photoFileInfo.filePath, 'saveMasterContactPhotoInfo');
      } else {
        blobSource = await GroupIdUtil.getInstance().getBlobSourceByBlobData(photoFileInfo.blobData);
      }
      // 首先判断master联系人是否存在type_id为8的数据列
      const predicates = new rdb.RdbPredicates(DbConstants.NEW_DATA_TABLE_NAME);
      predicates.equalTo(DataColumns.RAW_CONTACT_ID, masterContact.rawId)
        .and()
        .equalTo(DataColumns.TYPE_ID, 8);
      queryResult = await contactRdbHelper.queryData(DbConstants.NEW_DATA_TABLE_NAME, predicates,
        [DataColumns.DETAIL_INFO, DataColumns.TYPE_ID]);
      if (!queryResult || !queryResult.goToFirstRow()) {
        // 表示不存在type_id为8的数据列，需要重新插入
        let insertContactDataValues: ValuesBucket = {};
        insertContactDataValues[DataColumns.TYPE_ID] = 8;
        insertContactDataValues[DataColumns.RAW_CONTACT_ID] = masterContact.rawId;
        insertContactDataValues[DataColumns.DETAIL_INFO] =
          'photo/' + masterContact.contactId + '_' + masterContact.rawId;
        insertContactDataValues[DataColumns.BLOB_DATA] = photoFileInfo.blobData;
        insertContactDataValues[DataColumns.BLOB_SOURCE] = blobSource;
        const contactDataId =
          await contactRdbHelper.insertData(insertContactDataValues, DbConstants.NEW_DATA_TABLE_NAME);
        if (contactDataId < 0) {
          LogUtils.w(tagStr, `insert masterContact contact_data fail`);
        } else {
          LogUtils.w(tagStr, `insert masterContact contact_data success`);
          // 数据表插入成功后保存大头像文件,复制头像文件至群组沙箱下
          let filePath = fileDir + '/' + masterContact.contactId + '_' + masterContact.rawId;
          let dirFlag = await fs.access(fileDir);
          if (!dirFlag) {
            await fs.mkdir(fileDir);
          }
          await fs.copyFile(photoFileInfo.filePath, filePath);
          LogUtils.w(tagStr, 'copy bigPhotoFile success!!!');
        }
      } else {
        // 表示存在type_id为8的数据列，直接更新
        const updateBucket: ValuesBucket = {};
        updateBucket[DataColumns.DETAIL_INFO] = 'photo/' + masterContact.contactId + '_' + masterContact.rawId;
        updateBucket[DataColumns.BLOB_DATA] = photoFileInfo.blobData;
        updateBucket[DataColumns.BLOB_SOURCE] = blobSource;
        let rdbRet = -1;
        rdbRet = await contactRdbHelper.updateData(updateBucket, predicates);
        if (rdbRet <= 0) {
          LogUtils.e(tagStr, `update masterContact fail, ret:${rdbRet}, raw_contact_id: ${masterContact.rawId}`);
        } else {
          LogUtils.w(tagStr, `update masterContact success, ret:${rdbRet}, raw_contact_id: ${masterContact.rawId}`);
          // 当数据库更新成功，证明contact_data表有type_id为8的数据，代表联系人确实需要大头像文件，那么才复制头像文件至群组沙箱下
          let filePath = fileDir + '/' + masterContact.contactId + '_' + masterContact.rawId;
          let dirFlag = await fs.access(fileDir);
          if (!dirFlag) {
            await fs.mkdir(fileDir);
          }
          // 如果高清头像取得是masterContact自己的，就不做copy操作
          if (photoFileInfo.filePath === filePath) {
            LogUtils.w(tagStr, 'file is masterContact self, not need copy file');
          } else {
            let fileFlag = await fs.access(filePath);
            if (fileFlag) {
              await fs.unlink(filePath);
              LogUtils.w(tagStr, 'file is exists, delete it before');
            }
            if (isOldFilePathAccess) {
              await fs.copyFile(photoFileInfo.filePath, filePath);
              LogUtils.w(tagStr, 'copy bigPhotoFile success!!!');
            } else {
              let pixelMap: PixelMap | undefined | null = undefined;
              try {
                pixelMap = await getPixelMapFromFromUint8Array(photoFileInfo.blobData);
                if (pixelMap) {
                  await GroupIdUtil.getInstance().getImagePackingData(pixelMap, filePath);
                }
              } finally {
                if (pixelMap) {
                  pixelMap.release().catch((error: BusinessError) => {
                    LogUtils.e(tagStr, `pixelMap release err msg:${error?.message}`);
                  });
                }
              }
            }
          }
        }
      }
    } catch (e) {
      LogUtils.e(tagStr, `err,msg:${e.message}`);
      return;
    } finally {
      if (queryResult && !queryResult.isClosed) {
        queryResult.close();
      }
    }
  }

  /**
   * 对比头像，规则：优先保留分辨率大的，如果分辨率一致，则保留文件大小更大的
   * @param photoFileInfoArr 大头像信息集合
   */
  async compareFileScale(photoFileInfoArr: PhotoFileInfo[]) {
    const tagStr = TAG + ` bigPhotoFromFile compareFileScale `;
    // 给一个默认的结果
    let photoFileInfo: PhotoFileInfo = {
      filePath: photoFileInfoArr[0].filePath,
      blobData: photoFileInfoArr[0].blobData,
    }
    try {
      // 首先计算每个大头像的分辨率和大小
      let photoFileInfoArrNew: PhotoFileInfo[] = [];
      for (let photoFileInfo of photoFileInfoArr) {
        let resolution = 0;
        let imageInfo: image.ImageInfo | null = null;
        try {
          let isOldFilePathAccess = await fs.access(photoFileInfo.filePath);
          if (isOldFilePathAccess) {
            imageInfo = await ContactsDataService.getInstance().getImageInfoFromFile(photoFileInfo.filePath);
          } else if (photoFileInfo.blobData !== null && photoFileInfo.blobData !== undefined) {
            imageInfo = await getImageInfoFromUint8Array(photoFileInfo.blobData) as image.ImageInfo | null;
          }

          if (imageInfo) {
            let height = imageInfo.size.height;
            let width = imageInfo.size.width;
            resolution = height * width;
          }
        } catch (error) {
          LogUtils.e(tagStr, `get resolution eroor: ${error?.message}`);
        }
        photoFileInfo.resolution = resolution;
        let stat = await fs.stat(photoFileInfo.filePath);
        photoFileInfo.fileSize = stat.size;
        photoFileInfoArrNew.push(photoFileInfo);
      }
      photoFileInfoArrNew.sort((a, b) => {
        // 先按照分辨率排序
        if (a.resolution && b.resolution) {
          if (a.resolution > b.resolution) {
            return -1;
          } else if (a.resolution < b.resolution) {
            return 1;
          }
        }
        if (a.fileSize && b.fileSize) {
          if (a.fileSize > b.fileSize) {
            return -1;
          } else if (a.fileSize < b.fileSize) {
            return 1;
          }
        }
        // 如果分辨率和文件大小都相同，则保持原顺序
        return 0;
      })
      LogUtils.w(tagStr, `photoFileInfoArrNew[0] result: filePath:
        ${StringUtil.maskSensitiveInfo(photoFileInfoArrNew[0].filePath)}`);
      return photoFileInfoArrNew[0];
    } catch (e) {
      LogUtils.e(tagStr, `err,msg:${e.message}`);
      LogUtils.w(tagStr, `default photoFileInfo result: filePath: ${StringUtil.maskSensitiveInfo(photoFileInfo.filePath)}`);
      return photoFileInfo;
    }
  }

  public getAddressFromPostalAddress(element: Contact.PostalAddress) {
    //todo 解析完直接设置 address 值，避免后续多次拼接
    if (!element) {
      return '';
    }
    let address: string = '';
    if (element.country != null) {
      address = address + element.country;
    }
    if (element.region != null) {
      address = address + element.region;
    }
    if (element.neighborhood != null) {
      address = address + element.neighborhood;
    }
    if (element.street != null) {
      address = address + element.street;
    }
    if (element.postalAddress != null) {
      address = address + element.postalAddress;
    }
    if (element.postcode != null) {
      address = address + element.postcode;
    }
    return address;
  }

  /**
   * 排序后，第一个元素为master
   * @param mergeArr
   */
  public sortMasterContact(mergeArr: MergeRawContact[]) {
    mergeArr.sort((a, b) => {
      // 如果有一个 raw 关联的 contact_id 不存在，则选择其他 raw
      if (!a.contactIdExist && b.contactIdExist) {
        return 1;
      }
      if (a.contactIdExist && !b.contactIdExist) {
        return -1;
      }
      // 如果都有uuid
      if (a.uuid && b.uuid) {
        // 优先保留短的双框架uuid
        if (a.uuid.length !== b.uuid.length) {
          return a.uuid.length < b.uuid.length ? -1 : 1;
        }
        // 长度相等，则保留字典序小的 uuid，保证单框架各端决策一致
        return a.uuid < b.uuid ? -1 : 1;
      }
      // 如果只有一个有 uuid，则保留这个
      if (a.uuid || b.uuid) {
        return a.uuid ? -1 : 1;
      }
      // 都没有uuid，则保留小的 rawId
      return a.rawId < b.rawId ? -1 : 1;
    });
  }

  public async queryMaxFavoriteOrder(): Promise<number> {
    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
      resultSet = await contactRdbHelper.queryData(DbConstants.NEW_RAW_CONTACT_TABLE_NAME, predicates,
        ['max(favorite_order)']);
      if (!resultSet || !resultSet.goToFirstRow()) {
        LogUtils.w(TAG, `queryMaxFavoriteOrder from raw_contact table done, MaxFavoriteOrder is null.`);
        return 0;
      }
      let maxFavoriteOrder = resultSet.getLong(0);
      LogUtils.w(TAG, `queryMaxFavoriteOrder from raw_contact table done, MaxFavoriteOrder is ${maxFavoriteOrder}.`);
      return maxFavoriteOrder;
    } catch (error) {
      LogUtils.e(TAG, 'queryMaxFavoriteOrder error:' + error.message);
      return 0;
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
  }

  /**
   * 合并到master；如果有contactInfo，会被吸收，不会新增
   * @param mergeArr 重复联系人的数组
   * @param context
   * @param skipUuidDupContact
   * @param contactInfo contactInfo被吸收，不会有新增的联系人
   * @returns
   */
  public async mergeToMasterContact(mergeArr: MergeRawContact[], context: common.Context,
    skipUuidDupContact: boolean = true, contactInfo?: ContactInfo): Promise<MergeHandleCount> {
    let mergeHandleCount: MergeHandleCount = new MergeHandleCount();
    try {
      const rdbStore = await contactRdbHelper.initDB(context);
      if (!rdbStore) {
        LogUtils.e(TAG, `mergeToMasterContact rdbStore init error!`);
        return mergeHandleCount;
      }
      if (mergeArr.length <= 1 && !contactInfo) {
        LogUtils.e(TAG, `mergeToMasterContact mergeArr size:${mergeArr.length},contactInfo exist:${
          contactInfo != undefined}`);
        return mergeHandleCount;
      }
      // 获取所有待合并的联系人大头像，合并联系人之后，取最高清的一张作为最后的大头像
      const photoFileInfoArr: PhotoFileInfo[] = await this.getPhotoFileInfoList(mergeArr, context);

      const masterContact: MasterContact = new MasterContact();

      // 被合并的联系人有生日，主联系人没有生日时，合并之后删除被合并的联系人同步的生日，等生日合并到主联系人，需要把主联系人的生日同步到日历
      let isNeedSyncMasterContactBirRetry: boolean = false;

      // 排序并取出主联系人
      // 非云同步新增场景，需要先排序寻找本地主联系人，再记录 id 与收藏信息
      this.sortMasterContact(mergeArr);
      masterContact.contactId = mergeArr[0].contactId;
      masterContact.rawId = mergeArr[0].rawId;
      masterContact.uuid = mergeArr[0].uuid;
      masterContact.personalRingtone = mergeArr[0].personalRingtone;
      masterContact.personalNotificationRingtone = mergeArr[0].personalNotificationRingtone;
      masterContact.ringtonePath = mergeArr[0].ringtonePath;
      masterContact.favorite = mergeArr[0].favorite || 0;
      masterContact.favoriteOrder = mergeArr[0].favoriteOrder;
      // 克隆联系人有formId，清楚当前数据库相同fromId数据后，合并到主联系人
      if (contactInfo?.formId) {
        let formId = contactInfo?.formId;
        // 删除本机相同fromId 将迁移过来fromId合并到主联系人
        await ContactsDataService.getInstance().next2nextDeleteContactFormId(formId);
        const fromIdArr = formId.split('//');
        for (const oldFromId of fromIdArr) {
          this.updateFromIdForMasterContact(oldFromId, mergeArr[0]);
        }
      }
      if (mergeArr[0].formId) {
        masterContact.formId = mergeArr[0].formId;
        masterContact.needUpdateContact = true;
        masterContact.contactUpdateValuesBucket[ContactsColumns.FORM_ID] = masterContact.formId;
      }
      masterContact.focusModeList = mergeArr[0].focusModeList || null;
      if (contactInfo?.focusModeList && !masterContact.focusModeList) {
        masterContact.focusModeList = contactInfo?.focusModeList;
        masterContact.contactUpdateValuesBucket[ContactsColumns.FOCUS_MODE_LIST] = masterContact.focusModeList;
        masterContact.needUpdateContact = true;
      }
      masterContact.contactedCount = mergeArr[0].contactedCount || 0;
      masterContact.lastestContactedTime = mergeArr[0].lastestContactedTime || 0;
      if (!masterContact.contactedCount && contactInfo?.contactedCount) {
        masterContact.contactedCount = contactInfo?.contactedCount;
        masterContact.rawUpdateValuesBucket[RawContactsColumns.CONTACTED_COUNT] = masterContact.contactedCount;
        masterContact.needUpdateRaw = true;
      }
      if (!masterContact.lastestContactedTime && contactInfo?.lastestContactedTime) {
        masterContact.lastestContactedTime = contactInfo?.lastestContactedTime;
        masterContact.rawUpdateValuesBucket[RawContactsColumns.LASTEST_CONTACTED_TIME] =
          masterContact.lastestContactedTime;
        masterContact.needUpdateRaw = true;
      }
      LogUtils.w(TAG, `mergeContact goto masterContact mergeRawContactArr:${MergeRawContact.getLogStr(mergeArr)
        },skipUuidDupContact:${skipUuidDupContact
          },contactInfo exist:${contactInfo != undefined}`);

      let noUuidContacts: MergeRawContact[] = [];
      let noUuidContactRawIds: number[] = [];
      let noUuidContactContactIds: number[] = [];
      // 需要删除的联系人：存在uuid
      let hasUuidContacts: MergeRawContact[] = [];
      let hasUuidContactRawIds: number[] = [];
      let hasUuidContactContactIds: number[] = [];
      // 需要删除的联系人（不存在uuid+存在uuid）
      let needDeleteRawIds: number[] = [];
      let needNotifyFormIdChange: boolean = false;

      // master后边的联系人遍历处理
      for (let i = 1; i < mergeArr.length; i++) {
        let contact = mergeArr[i];
        if (skipUuidDupContact && !StringUtil.isEmpty(contact.uuid)) {
          continue;
        }
        if (contact.contactId == masterContact.contactId) {
          masterContact.contactUpdateValuesBucket[ContactsColumns.NAME_RAW_CONTACT_ID] = masterContact.rawId;
          masterContact.needUpdateContact = true;
        }
        // 继承被删除联系人的 收藏信息
        if ((masterContact.favorite !== 1 && contact.favorite === 1) || (contact.favorite === 1 &&
          masterContact.favorite === 1 && contact.favoriteOrder < masterContact.favoriteOrder)) {
          masterContact.favorite = 1;
          masterContact.favoriteOrder = contact.favoriteOrder;
          masterContact.rawUpdateValuesBucket[RawContactsColumns.FAVORITE] = 1;
          masterContact.rawUpdateValuesBucket[RawContactsColumns.FAVORITE_ORDER] = contact.favoriteOrder;
          masterContact.needUpdateRaw = true;
        }
        if (!masterContact.focusModeList && contact.focusModeList) {
          masterContact.focusModeList = contact.focusModeList;
          masterContact.contactUpdateValuesBucket[ContactsColumns.FOCUS_MODE_LIST] = masterContact.focusModeList;
          masterContact.needUpdateContact = true;
        }
        if (!masterContact.contactedCount && contact.contactedCount) {
          masterContact.contactedCount = contact.contactedCount;
          masterContact.rawUpdateValuesBucket[RawContactsColumns.CONTACTED_COUNT] = masterContact.contactedCount;
          masterContact.needUpdateRaw = true;
        }
        if (!masterContact.lastestContactedTime && contact.lastestContactedTime) {
          masterContact.lastestContactedTime = contact.lastestContactedTime;
          masterContact.rawUpdateValuesBucket[RawContactsColumns.LASTEST_CONTACTED_TIME] =
            masterContact.lastestContactedTime;
          masterContact.needUpdateRaw = true;
        }
        // 继承被删除联系人的 卡片信息

        if (!StringUtil.isEmpty(contact.formId) && contact.formId != masterContact.formId) {
          let newFormId: string = '';
          if (!StringUtil.isEmpty(masterContact.formId)) {
            // 通过set做一次去重
            const formIdSet: Set<string> = new Set((masterContact.formId + formIdSeparator + contact.formId)
              .split(formIdSeparator).filter((formId: string) => !StringUtil.isEmpty(formId)));
            newFormId = Array.from(formIdSet).join(formIdSeparator);
          } else {
            newFormId = contact.formId;
          }
          LogUtils.w(TAG, `merge contactId ${contact.contactId} formId:${contact.formId
            } to master contact ${masterContact.contactId},update from ${masterContact.formId} to ${newFormId}`);
          masterContact.formId = newFormId;
          masterContact.contactUpdateValuesBucket[ContactsColumns.FORM_ID] = masterContact.formId;
          masterContact.needUpdateContact = true;
          needNotifyFormIdChange = true;
        }

        // 继承被删除联系人的铃声信息
        if (!masterContact.personalRingtone && contact.personalRingtone) {
          LogUtils.w(TAG, `merge ringtone from contactId ${contact.contactId} to masterContactId:${masterContact.
            contactId}, update to ${StringUtil.maskSensitiveInfo(contact.personalRingtone)}:${
                StringUtil.maskSensitiveInfo(contact.personalNotificationRingtone)}:${
                  StringUtil.maskSensitiveInfo(contact.ringtonePath)}`);
          masterContact.personalRingtone = contact.personalRingtone;
          masterContact.personalNotificationRingtone = contact.personalNotificationRingtone;
          masterContact.ringtonePath = contact.ringtonePath;
          masterContact.contactUpdateValuesBucket[ContactsColumns.PERSONAL_RINGTONE] = contact.personalRingtone;
          masterContact.contactUpdateValuesBucket[ContactsColumns.PERSONAL_NOTIFICATION_RINGTONE] =
            contact.personalNotificationRingtone;
          masterContact.contactUpdateValuesBucket[ContactsColumns.RINGTONE_PATH] = contact.ringtonePath;
          masterContact.needUpdateContact = true;
        }
        needDeleteRawIds.push(contact.rawId);
        if (StringUtil.isEmpty(contact.uuid)) {
          noUuidContacts.push(contact);
          noUuidContactRawIds.push(contact.rawId);
          noUuidContactContactIds.push(contact.contactId);
        } else {
          hasUuidContacts.push(contact);
          hasUuidContactRawIds.push(contact.rawId);
          hasUuidContactContactIds.push(contact.contactId);
        }
      }

      if (masterContact.favorite != 1 && contactInfo?.favorite == 1) {
        masterContact.rawUpdateValuesBucket[RawContactsColumns.FAVORITE] = 1;
        masterContact.rawUpdateValuesBucket[RawContactsColumns.FAVORITE_ORDER] = await this.queryMaxFavoriteOrder() + 1;
        masterContact.needUpdateRaw = true;
      }

      // 获取主联系人的所有数据
      const masterNameAndPhoneData: SimpleContactData[] = [];
      const masterContactDataArr = await this.getMergeDataByRawId(masterContact.rawId);
      if (masterContactDataArr && masterContactDataArr.length) {
        for (const fullContactData of masterContactDataArr) {
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.NICKNAME) {
            masterContact.nickname = fullContactData.detail_info;
            continue;
          }
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.ORGANIZATION) {
            masterContact.company = fullContactData.detail_info;
            masterContact.position = fullContactData.position;
            continue;
          }
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.PHOTO) {
            masterContact.blobData = fullContactData.blob_data;
            continue;
          }
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.NOTE) {
            masterContact.note = fullContactData.detail_info;
            continue;
          }
          // 非云同步合并场景，需要处理号码，因为手动合并也复用了这段逻辑
          const contactDataKey = this.createContactDataKey(fullContactData);
          masterContact.contactDataKeySet.add(contactDataKey);
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE || 
            fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.NAME) {
            masterNameAndPhoneData.push(SimpleContactData.fromFullContactData(fullContactData));
          }
          // 联系人生日 和 农历生日 只支持各保存1个
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.CONTACT_EVENT) {
            if (fullContactData.custom_data === EVENT_LABEL_ID.BIRTHDAY) {
              masterContact.hasBirthday = true;
            } else if (fullContactData.custom_data === EVENT_LABEL_ID.LUNAR_BIRTHDAY) {
              masterContact.hasLunarBirthday = true;
            }
          }
          // 合并联系人是否有海报判断
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.POSTER) {
            masterContact.poster = fullContactData.detail_info;
            masterContact.contactUpdateValuesBucket[ContactsColumns.PREFER_AVATAR] = 0;
          }
        }
      }

      // 要被合并删除的联系人如果有大头像，也需要处理
      if (contactInfo?.photo !== null && contactInfo?.photo != undefined && contactInfo?.photo.uri !== null &&
        contactInfo?.photo.uri !== undefined) {
        let fileResPath = context.databaseDir + '/photo/' + contactInfo.oldContactId;
        if (!fs.accessSync(fileResPath)) {
          fileResPath = context.databaseDir + '/photo/' + contactInfo.oldContactId + '_' + contactInfo.oldRawId;
        }
        let photoFileInfo: PhotoFileInfo = {
          filePath: fileResPath,
          blobData: contactInfo.photo.uri as Uint8Array
        }
        photoFileInfoArr.push(photoFileInfo);
      }

      // 获取所有待删除的联系人数据
      let needDeleteContactDataArr: FullContactData[] = [];
      if (needDeleteRawIds.length) {
        needDeleteContactDataArr = await this.getMergeDataByRawIds(needDeleteRawIds);
      }
      const extraInfo: ExtraInfo = {
        deletePhoneList: [],
        deleteFormatPhoneList: [],
        addPhoneList: [],
        addFormatPhoneList: [],
      };
      let valuesBucketArr: ValuesBucket[] = [];

      // 将数据都合并到主联系人上
      let isPhoneChange = false;
      if (needDeleteContactDataArr && needDeleteContactDataArr.length) {
        for (const fullContactData of needDeleteContactDataArr) {
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.NICKNAME &&
            !masterContact.nickname) {
            LogUtils.w(TAG, `add nickname ${StringUtil.maskSensitiveInfo(fullContactData.detail_info)} to masterContact uuid:${
              StringUtil.getShortUuid(masterContact.uuid)},rawId:${masterContact.rawId}`);
            masterContact.nickname = fullContactData.detail_info;
            fullContactData.raw_contact_id = masterContact.rawId;
            masterContact.insertValuesBucket.push(fullContactData.toValuesBucket());
            continue;
          }
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.ORGANIZATION &&
            !masterContact.company
            && !masterContact.position) {
            LogUtils.w(TAG,
              `add company ${StringUtil.maskSensitiveInfo(fullContactData.detail_info)} ${StringUtil.maskSensitiveInfo(
                fullContactData.position)} to masterContact uuid:${StringUtil.getShortUuid(masterContact.uuid)},rawId:${
                  masterContact.rawId}`);
            masterContact.company = fullContactData.detail_info;
            masterContact.position = fullContactData.position;
            fullContactData.raw_contact_id = masterContact.rawId;
            masterContact.insertValuesBucket.push(fullContactData.toValuesBucket());
            masterContact.rawUpdateValuesBucket[RawContactsColumns.COMPANY] = fullContactData.detail_info;
            masterContact.rawUpdateValuesBucket[RawContactsColumns.POSITION] = fullContactData.position;
            masterContact.needUpdateRaw = true;
            masterContact.contactUpdateValuesBucket[ContactsColumns.COMPANY] = fullContactData.detail_info;
            masterContact.contactUpdateValuesBucket[ContactsColumns.POSITION] = fullContactData.position;
            masterContact.needUpdateContact = true;
            continue;
          }
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.PHOTO && !masterContact.blobData) {
            LogUtils.w(TAG, `add photo to masterContact uuid:${StringUtil.getShortUuid(masterContact.uuid)},rawId:${
              masterContact.rawId}`);
            masterContact.blobData = fullContactData.blob_data;
            fullContactData.raw_contact_id = masterContact.rawId;
            masterContact.insertValuesBucket.push(fullContactData.toValuesBucket());
            continue;
          }
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.NOTE &&
            !masterContact.note) {
            LogUtils.w(TAG, `add note ${StringUtil.maskSensitiveInfo(fullContactData.detail_info)} to masterContact uuid:${
              StringUtil.getShortUuid(masterContact.uuid)},rawId:${masterContact.rawId}`);
            masterContact.note = fullContactData.detail_info;
            fullContactData.raw_contact_id = masterContact.rawId;
            masterContact.insertValuesBucket.push(fullContactData.toValuesBucket());
            continue;
          }
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.NICKNAME ||
            fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.ORGANIZATION ||
            fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.PHOTO ||
            fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.NOTE
          ) {
            // 这些类型上面已经处理过
            continue;
          }
          // 非云同步合并场景，需要处理号码，因为手动合并也复用了这段逻辑
          const contactDataKey = this.createContactDataKey(fullContactData);
          if (!masterContact.contactDataKeySet.has(contactDataKey)) {
            LogUtils.w(TAG, `add ${StringUtil.maskSensitiveInfo(contactDataKey)} to masterContact uuid:${StringUtil.getShortUuid(
              masterContact.uuid)},rawId:${masterContact.rawId}`);
            // 联系人生日 和 农历生日 只支持各保存1个
            if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.CONTACT_EVENT) {
              if (fullContactData.custom_data === EVENT_LABEL_ID.BIRTHDAY) {
                if (masterContact.hasBirthday) {
                  continue;
                } else {
                  masterContact.hasBirthday = true;
                  isNeedSyncMasterContactBirRetry = true;
                }
              } else if (fullContactData.custom_data === EVENT_LABEL_ID.LUNAR_BIRTHDAY) {
                if (masterContact.hasLunarBirthday) {
                  continue;
                } else {
                  masterContact.hasLunarBirthday = true;
                  isNeedSyncMasterContactBirRetry = true;
                }
              }
            }
            masterContact.contactDataKeySet.add(contactDataKey);
            fullContactData.raw_contact_id = masterContact.rawId;
            masterContact.insertValuesBucket.push(fullContactData.toValuesBucket());
            if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE) {
              isPhoneChange = true;
              masterNameAndPhoneData.push(SimpleContactData.fromFullContactData(fullContactData));
            }
          }
          if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.POSTER &&
            !masterContact.poster) {
            LogUtils.w(TAG, `add poster ${StringUtil.maskSensitiveInfo(fullContactData.detail_info)} to masterContact uuid:${
              StringUtil.getShortUuid(masterContact.uuid)},rawId:${masterContact.rawId}`);
            masterContact.poster = fullContactData.detail_info;
            fullContactData.raw_contact_id = masterContact.rawId;
            masterContact.insertValuesBucket.push(fullContactData.toValuesBucket());
            masterContact.contactUpdateValuesBucket[ContactsColumns.PREFER_AVATAR] = 0;
            continue;
          }
        }
      }
      // 手动合并过程中，如果号码有变更，则需要重新计算 uniqueKey
      if (isPhoneChange) {
        const uniqueKey = this.getUniqueKeyByNameAndPhoneArr(masterContact.rawId, masterNameAndPhoneData,
          mergeArr[0].isDeleted, 1);
        masterContact.needUpdateRaw = true;
        masterContact.rawUpdateValuesBucket[RawContactsColumns.UNIQUE_KEY] = uniqueKey;
        masterContact.needUpdateContact = true;
        masterContact.contactUpdateValuesBucket[ContactsColumns.CONTACT_LAST_UPDATED_TIMESTAMP] = Date.now();
      }

      // 更新主联系人
      const beginTransSuc = await contactRdbHelper.beginTransaction();
      if (!beginTransSuc) {
        LogUtils.e(TAG, `BeginTransaction before execute your sql error.`);
        return mergeHandleCount;
      }
      try {
        if (masterContact.needUpdateRaw || masterContact.needUpdateContact ||
          masterContact.insertValuesBucket.length > 0) {
          if (masterContact.needUpdateRaw || masterContact.insertValuesBucket.length > 0) {
            masterContact.rawUpdateValuesBucket[RawContactsColumns.DIRTY] = DIRTY_ENUM.NO_NEED_RECALC;
            masterContact.needUpdateRaw = true;
          }
          const insertSuc = await this.updateMasterContact(masterContact);
          // 更新失败，事务回滚
          if (!insertSuc) {
            LogUtils.e(TAG, `updateMasterContact err, need rollBack.`);
            await contactRdbHelper.rollBack();
            return mergeHandleCount;
          }
        }

        // 查询要删除的生日的calendar_event_id
        let eventIds: number[] = [];
        const delBirthdayRawIdArr: number[] = [];
        delBirthdayRawIdArr.push(...noUuidContactRawIds);
        delBirthdayRawIdArr.push(...hasUuidContactRawIds);
        if (delBirthdayRawIdArr.length > 0) {
          eventIds = await BirthdayManager.getInstance().queryEventId(delBirthdayRawIdArr);
        }

        // 删除不带 UUID 的重复联系人（硬删除）
        if (noUuidContactContactIds.length) {
          const hardDelRet = await DirtyDataClearServiceUpdate.getInstance()
            .deleteContactInfoHardByContactAndRawIds(noUuidContactContactIds, noUuidContactRawIds);
        }
        // 硬删数量
        mergeHandleCount.hardDelCount = noUuidContactContactIds.length;

        // 有对应的UUID 软删除
        if (hasUuidContacts.length) {
          await this.softDelContact(masterContact, hasUuidContactRawIds, hasUuidContacts, 'updateMasterContact');
          await ContactAggregator.getInstance().doSplitDeletedRaw(hasUuidContactRawIds);
        }
        // 软删数量
        mergeHandleCount.softDelCount = hasUuidContacts.length;
        await contactRdbHelper.commit();
        if (eventIds.length > 0) {
          await BirthdayManager.getInstance().deleteBirthdayEvent(eventIds);
        }
        LogUtils.w(TAG, `mergeToMasterContact isNeedSyncMasterContactBirRetry = ` + isNeedSyncMasterContactBirRetry +
          ', masterContact.rawId = ' + masterContact.rawId);
        if (isNeedSyncMasterContactBirRetry) {
          await BirthdayManager.getInstance().handleSingleBirthday(String(masterContact.rawId), false);
        }
      } catch (e) {
        if (contactInfo) {
          LogUtils.e(TAG, `updateMasterContact err! contactInfo: ${StringUtil.getContactInfoLogStr(contactInfo)}`);
        }
        LogUtils.e(TAG, `updateMasterContact err: ${e.message}, stack:${e.stack}`);
        await contactRdbHelper.rollBack();
        return mergeHandleCount;
      }

      if (noUuidContactContactIds.length || hasUuidContactContactIds.length) {
        await this.updateCalllogWhenMergeContact([...noUuidContactContactIds,
        ...hasUuidContactContactIds], masterContact.contactId, context);
      }

      if (needNotifyFormIdChange) {
        LogUtils.i(TAG, `needNotifyFormIdChange: ${needNotifyFormIdChange}`);
        await this.notifyFormIdChange(context);
      }
      // 下云合并联系人时需要判断使用云上的联系人头像资源还是本地的联系人头像文件作为大头像
      // 判断标准：优先保留分辨率大的，如果分辨率一致，则保留文件大小更大的
      // await this.dealMasterContactBigPhoto(masterContact, photoFileInfoArr, context, contactInfo);
    } catch (e) {
      LogUtils.e(TAG, `mergeToMasterContact err: ${e.message}, stack:${e.stack}`);
    }
    return mergeHandleCount;
  }

  private updateFromIdForMasterContact(formId: string, mergeRawContact: MergeRawContact) {
    if (mergeRawContact.formId && mergeRawContact.formId != '') {
      let formIdArr = mergeRawContact.formId.split('//');
      for (let i = 0; i < formIdArr.length; i++) {
        if (formIdArr[i] === formId) {
          return;
        }
      }
      mergeRawContact.formId = mergeRawContact.formId + '//' + formId;
    } else {
      mergeRawContact.formId = formId;
    }
  }

  private async softDelContactPutFormIdNull(
      masterContact: MasterContact, softDelContacts: MergeRawContact[], tag: string) {
    const contactIdArr: number[] = [];
    let needPutFormIdNull: boolean = false;
    for (const softDelContact of softDelContacts) {
      if (masterContact.contactId != softDelContact.contactId) {
        needPutFormIdNull = true;
        contactIdArr.push(softDelContact.contactId);
      }
    }
    if (!needPutFormIdNull) {
      return true;
    }

    const contactValues: ValuesBucket = {};
    contactValues[ContactsColumns.FORM_ID] = null;
    LogUtils.w(TAG, `${tag} putFormIdNull contactIdArr:${contactIdArr}`);

    let predicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
    if (contactIdArr.length == 1) {
      predicates.equalTo(ContactsColumns.NEW_ID, contactIdArr[0]);
    } else {
      predicates.in(ContactsColumns.NEW_ID, contactIdArr);
    }
    let rdbRet = await contactRdbHelper.updateData(contactValues, predicates);
    if (rdbRet < 0) {
      LogUtils.e(TAG, `${tag} updateData err: ${rdbRet}`);
      return false;
    }
    return true;
  }

  private async softDelContact(masterContact: MasterContact,
    rawIds: number[], softDelContacts: MergeRawContact[], tag: string) {
    if (softDelContacts?.length == 0 || rawIds?.length == 0 ) {
      return true;
    }
    const delInsertArr: ValuesBucket[] = [];
    for (const softDelContact of softDelContacts) {
      const delInsertValues: ValuesBucket = {};
      delInsertValues[DeleteRawContact.CONTACT_ID] = softDelContact.contactId;
      delInsertValues[DeleteRawContact.RAW_CONTACT_ID] = softDelContact.rawId;
      delInsertValues[DeleteRawContact.IS_DELETED] = 0;
      delInsertValues[DeleteRawContact.DISPLAY_NAME] = softDelContact.displayName;
      delInsertValues[DeleteRawContact.DELETE_TIME] = Date.now();
      delInsertValues[DeleteRawContact.DELETE_SOURCE] = CONTACT_DB_PACKAGE;
      delInsertArr.push(delInsertValues);
    }
    const rawValues: ValuesBucket = {};
    // 软删除数据肯定有 uuid，需要设置 dirty 上云
    rawValues[RawContactsColumns.DIRTY] = DIRTY_ENUM.NO_NEED_RECALC;
    rawValues[RawContactsColumns.IS_DELETED] = 1;

    let predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
    if (rawIds.length == 1) {
      predicates.equalTo('id', rawIds[0]);
    } else {
      predicates.in('id', rawIds);
    }
    let rdbRet = await contactRdbHelper.updateData(rawValues, predicates);
    if (rdbRet < 0) {
      LogUtils.e(TAG, `${tag} softDelContact updateData err: ${rdbRet}`);
      return false;
    }
    let putFromIdNullRet = await this.softDelContactPutFormIdNull(masterContact, softDelContacts, tag);
    if (!putFromIdNullRet) {
      return false;
    }
    rdbRet = await contactRdbHelper.insertBatchData(delInsertArr, DbConstants.NEW_DELETE_RAW_CONTACT_TABLE_NAME);
    LogUtils.w(TAG, `${tag} softDelContact rdbRet: ${rdbRet},size:${rawIds.length}.rawIds:${rawIds}`);
    if (rdbRet < 0) {
      return false;
    }
    return true;
  }

  private async updateCalllogWhenMergeContact(delContactArr: number[], newContactId: number, context: common.Context) {
    const callLogRdbStore = await callLogRdbHelper.initDB(context);
    if (!callLogRdbStore) {
      LogUtils.e(TAG, `updateCalllogWhenMergeContact callLogRdbStore init error!`);
      return;
    }
    const predicates = new rdb.RdbPredicates(DbConstants.CALLLOG_TABLE_NAME);
    predicates.in('quicksearch_key', delContactArr.map((id: number) => id.toString()));
    const callLogUpdateBucket: ValuesBucket = {
      'extra1': newContactId.toString()
    };
    callLogUpdateBucket[CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY] = newContactId.toString();
    await callLogRdbHelper.updateData(callLogUpdateBucket, predicates);
  }

  // update master contact
  public async updateMasterContact(masterContact: MasterContact) {
    LogUtils.w(TAG, `updateMasterContact needUpdateRaw:${masterContact.needUpdateRaw
      },needUpdateContact:${masterContact.needUpdateContact
      }, insertValuesBucket size:${masterContact.insertValuesBucket.length}`);
    if (masterContact.needUpdateRaw) {
      const predicates = new rdb.RdbPredicates(DbConstants.NEW_RAW_CONTACT_TABLE_NAME);
      predicates.equalTo('id', masterContact.rawId);
      let rdbRet = await contactRdbHelper.updateData(masterContact.rawUpdateValuesBucket, predicates);
      if (rdbRet < 0) {
        LogUtils.e(TAG, `update raw contact err: ${rdbRet}`);
        return false;
      }
    }

    if (masterContact.needUpdateContact) {
      const predicates = new rdb.RdbPredicates(DbConstants.NEW_CONTACTS_TABLE_NAME);
      predicates.equalTo('id', masterContact.contactId);
      let rdbRet = await contactRdbHelper.updateData(masterContact.contactUpdateValuesBucket, predicates);
      if (rdbRet < 0) {
        LogUtils.e(TAG, `update contact err: ${rdbRet}`);
        return false;
      }
    }

    if (masterContact.insertValuesBucket.length) {
      const rdbRet = await contactRdbHelper.insertBatchData(masterContact.insertValuesBucket,
        DbConstants.NEW_DATA_TABLE_NAME);
      if (rdbRet < 0) {
        LogUtils.e(TAG, `update contact data err: ${rdbRet}`);
        return false;
      }
    }
    return true;
  }

  // use data to calc key
  public createContactDataKey(fullContactData: FullContactData) {
    // 对于号码要用格式化号码
    let detailInfo = fullContactData.detail_info;
    if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.PHONE) {
      detailInfo = fullContactData.formatPhoneNumber || this.getFormatNumberFromDetailInfo(detailInfo);
    }
    // 生日没有自定义类型，但是应用之前存了脏值
    if (fullContactData.type_id === DbConstants.FOUNDATION_TYPE_ID_MAP.CONTACT_EVENT) {
      return `${fullContactData.type_id}_${fullContactData.custom_data || LABEL_ID_EMPTY_SLOT}_${
        LABEL_NAME_EMPTY_SLOT}_${detailInfo}`;
    }
    return `${fullContactData.type_id}_${fullContactData.custom_data || LABEL_ID_EMPTY_SLOT}_${
      fullContactData.extend7 || LABEL_NAME_EMPTY_SLOT}_${detailInfo}`;
  }

  public async getMigrateDataAbilityHelper(context: common.Context) {
    // 由于ArkTS类型推断，此处在Promise中调用callback获取dataShareHelper实例
    if (this.migrateDataShareHelper === undefined) {
      this.migrateDataShareHelper = await new Promise<dataShare.DataShareHelper>((resolve) => {
        dataShare.createDataShareHelper(context, Contacts.CONTENT_URI, (err, dataShareHelper) => {
          resolve(dataShareHelper);
        });
      })
    }
    return this.migrateDataShareHelper;
  }

  public async getProxyDataAbilityHelper(context: common.Context) {
    // 由于ArkTS类型推断，此处在Promise中调用callback获取dataShareHelper实例
    if (this.proxyDataShareHelper === undefined) {
      this.proxyDataShareHelper = await new Promise<dataShare.DataShareHelper>((resolve) => {
        dataShare.createDataShareHelper(context, Contacts.SETTING_PROXY_URI,
          { isProxy: true }, (err, dataShareHelper) => {
            resolve(dataShareHelper);
          });
      })
    }
    return this.proxyDataShareHelper;
  }

  public async closeMigrateDataAbilityHelper() {
    await this.migrateDataShareHelper?.close();
    this.migrateDataShareHelper = undefined;
  }

  // notify contact card to refresh
  public async notifyFormIdChange(context: common.Context) {
    const proxyDataAbilityHelper = await this.getProxyDataAbilityHelper(context);
    let predicates = new dataSharePredicates.DataSharePredicates();
    predicates.equalTo('id', 1);
    const valuesBucket: ValuesBucket = {};
    valuesBucket['contact_change_time'] = Date.now();
    LogUtils.i(TAG, `start notifyFormIdChange`);

    proxyDataAbilityHelper?.update(Contacts.SETTING_PROXY_URI, predicates, valuesBucket).then((data: number) => {
      LogUtils.i(TAG, 'notifyFormIdChange update succeed, data : ' + data);
    }).catch((err: BusinessError) => {
      LogUtils.i(TAG, `notifyFormIdChange error: code: ${err.code}, message: ${err.message} `);
    });
  }
}

//没有状态依赖的方法，单独抽出来export或者 static，方便复用（依赖全局变量helper之类的 改为入参）