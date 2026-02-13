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
import rdb from '@ohos.data.relationalStore';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import LogUtils from '../../common/utils/LogUtils';
import { StringUtil } from '../../common/utils/StringUtil';
import { BusinessError } from '@ohos.base';
import dataShare from '@ohos.data.dataShare';
import CommonEventManager from '@ohos.commonEventManager';
import { BaseSingleRdbInsertHelper } from '../helperBase/BaseSingleRdbInsertHelper';
import { BaseRdbHelperForCloudSync } from '../helperBase/BaseRdbHelperForCloudSync';
import { BaseDbHelper } from '../helperBase/BaseDbHelper';

const RETRY_COUNT = 3;
const DELAY_TIMES = 100;
const TAG = 'DatabaseUtils';
const DELAY_TIME_ARRAY = [50, 100, 200, 400, 800];
const NEED_RETRY_CODE = new Set([14800015, 14800024, 14800025, 14800026, 14800028, 14800029]);
const DND_DATA_RESTORE_FINISH: string = 'custom.event.DND_DATA_RESTORE_FINISH';

/**
 * Used to handle failed operations when querying or inserting into the database
 */
export default class DatabaseUtils {
  /**
   * Retry three times getRdbStoreHelper for get double rdb object when get failed
   *
   * @param baseHelper to getRdbStoreHelper
   * @param context to get rdb object
   * @param dbName the rdb table
   * @returns rdb.RdbStore object or undefined
   */
  public static async retryGetDoubleRdb(baseHelper: BaseDbHelper, context: common.Context,
    dbName: string): Promise<rdb.RdbStore | undefined> {
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(TAG, `retryGetDoubleRdb start: ${i}`);
      rdbStoreHelper = await baseHelper.getRdbStoreHelper(context, dbName);
      if (rdbStoreHelper) {
        LogUtils.i(TAG, `retryGetDoubleRdb ${i} success`);
        break;
      }
      await DatabaseUtils.delay(DELAY_TIMES);
    }
    return rdbStoreHelper;
  }

  /**
   * Retry three times getRdbStoreHelper for get single rdb object when get failed
   *
   * @param baseHelper to getRdbStoreHelper
   * @param context to get rdb object
   * @param dbName the rdb table
   * @returns rdb.RdbStore object or undefined
   */
  public static async retryGetSingleRdb(baseHelper: BaseSingleRdbInsertHelper, context: common.Context,
    dbName: string, isSupportSearch: boolean): Promise<rdb.RdbStore | undefined> {
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(TAG, `retryGetSingleRdb start: ${i}`);
      rdbStoreHelper = await baseHelper.getRdbStoreHelper(context, dbName, isSupportSearch);
      if (rdbStoreHelper) {
        LogUtils.i(TAG, `retryGetSingleRdb ${i} success`);
        break;
      }
      await DatabaseUtils.delay(DELAY_TIMES);
    }
    return rdbStoreHelper;
  }

  public static async retryGetSingleRdbForCloudSync(baseHelper: BaseRdbHelperForCloudSync, context: common.Context,
    dbName: string, isSupportSearch: boolean): Promise<rdb.RdbStore | undefined> {
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(TAG, `retryGetSingleRdb start: ${i}`);
      rdbStoreHelper = await baseHelper.getRdbStoreHelper(context, dbName, isSupportSearch);
      if (rdbStoreHelper) {
        LogUtils.i(TAG, `retryGetSingleRdb ${i} success`);
        break;
      }
      await DatabaseUtils.delay(DELAY_TIMES);
    }
    return rdbStoreHelper;
  }

  /**
   * Retry three times init double mimeType for get typeIdMap
   *
   * @param baseHelper to init mimeType
   * @param context to get rdb object
   * @param typeIdMap to get the relative of double mimeType and single type
   * @returns if retryInitType success
   */
  public static async retryInitType(baseHelper: BaseDbHelper, context: common.Context,
    typeIdMap: Record<number, number>): Promise<boolean> {
    let result: boolean = false;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(TAG, `retryInitType start: ${i}`);
      result = await baseHelper.initMimeTypeId(context, typeIdMap);
      if (result) {
        LogUtils.i(TAG, `retryInitType ${i} success`);
        break;
      }
      await DatabaseUtils.delay(DELAY_TIMES);
    }
    return result;
  }

  /**
   * Retry three times insert single type to table contact_type
   *
   * @param baseHelper to insert the type data
   * @param context to get rdb object
   * @returns the number of successful insertions
   */
  public static async retryInsertType(baseHelper: BaseSingleRdbInsertHelper, context: common.Context): Promise<number> {
    let ret: number = -1;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(TAG, `retryInsertType start: ${i}`);
      ret = await baseHelper.insertContactType(context);
      if (ret >= 0) {
        LogUtils.i(TAG, `retryInsertType ${i} success`);
        break;
      }
      await DatabaseUtils.delay(DELAY_TIMES);
    }
    return ret;
  }

  /**
   * Retry three times query double phone account id
   *
   * @param baseHelper to query phone account id
   * @param context to get rdb object
   * @returns double phone account id
   */
  public static async retryQueryAccountId(baseHelper: BaseDbHelper, context: common.Context): Promise<number> {
    let ret: number = -1;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(TAG, `retryQueryAccountId start: ${i}`);
      ret = await baseHelper.queryAccountId(context);
      if (ret >= 0) {
        LogUtils.i(TAG, `retryQueryAccountId ${i} success`);
        break;
      }
      await DatabaseUtils.delay(DELAY_TIMES);
    }
    return ret;
  }

  /**
   * Retry three times insert single phone account id to table account
   *
   * @param baseHelper to insert phone account id
   * @param context to get rdb object
   * @param accountId which to be inserted
   * @returns the number of successful insertions
   */
  public static async retryInsertPhoneAccount(baseHelper: BaseSingleRdbInsertHelper,
    context: common.Context, accountId: number): Promise<number> {
    let ret: number = -1;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(TAG, `retryInsertPhoneAccount start: ${i}`);
      ret = await baseHelper.insertPhoneAccount(context, accountId);
      if (ret >= 0) {
        LogUtils.i(TAG, `retryInsertPhoneAccount ${i} success`);
        break;
      }
      await DatabaseUtils.delay(DELAY_TIMES);
    }
    return ret;
  }

  /**
   * Retry three times batch insert single data
   *
   * @param baseHelper to insert data
   * @param context to get RdbStore
   * @param values which data to insert
   * @param tableName which table name to insert
   * @returns the number of successful insertions
   */
  public static async retryInsertBatchData(baseHelper: BaseSingleRdbInsertHelper,
    context: common.Context, values: ValuesBucket[], tableName: string): Promise<number> {
    let ret: number = -1;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(TAG, `retryInsertBatchData start: ${i}`);
      ret = await baseHelper.insertBatchData(context, values, tableName);
      if (ret >= 0) {
        LogUtils.i(TAG, `retryInsertBatchData ${i} success`);
        break;
      }
      await DatabaseUtils.delay(DELAY_TIMES);
    }
    return ret;
  }

  /**
   * Check the columnName whether exist in the tableName
   *
   * @param rdbStore to get RdbStore
   * @param tableName which table name to check
   * @param columnName which column name to check
   * @returns the boolean of existence
   */
  public static async checkColumn(rdbStore: rdb.RdbStore, tableName: string, columnName: string): Promise<boolean> {
    let index : number = -1;
    LogUtils.i(TAG, 'checkColumn start');
    if (rdbStore === undefined) {
      return Promise.resolve(false);
    }
    if (StringUtil.isEmpty(tableName) || StringUtil.isEmpty(columnName)) {
      return Promise.resolve(false);
    }
    let predicatesInfo = new rdb.RdbPredicates(tableName);
    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      predicatesInfo.limitAs(1);
      resultSet = await rdbStore.query(predicatesInfo);
      if (resultSet && resultSet.goToFirstRow()) {
        index = resultSet.getColumnIndex(columnName);
        LogUtils.w(TAG, `checkColumn index : ${index} columnName : ${columnName}`);
        return Promise.resolve(index >= 0);
      }
      LogUtils.i(TAG, 'checkColumn end');
    } catch (error) {
      LogUtils.w(TAG, `checkColumn error : ${error?.message}, stack: ${error?.stack}`);
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    return Promise.resolve(false);
  }

  /**
   * Check the columnName whether exist in the tableName
   *
   * @param rdbStore to get RdbStore
   * @param tableName which table name to check
   * @param columnName which column name to check
   * @returns the boolean of existence
   */
  public static async checkIsHasValue(rdbStore: rdb.RdbStore, tableName: string): Promise<boolean> {
    let index : number = -1;
    if (rdbStore === undefined) {
      return Promise.resolve(false);
    }
    if (StringUtil.isEmpty(tableName)) {
      return Promise.resolve(false);
    }
    let predicatesInfo = new rdb.RdbPredicates(tableName);
    let resultSet: rdb.ResultSet | null | undefined = null;
    try {
      predicatesInfo.limitAs(1);
      resultSet = await rdbStore.query(predicatesInfo);
      if (resultSet) {
        let size = resultSet.rowCount;
        LogUtils.w(TAG, `checkIsHasValue ${size}`);
        // 查询出来了值，就是有数据
        return Promise.resolve(size > 0);
      }
      LogUtils.i(TAG, 'checkColumn end');
    } catch (error) {
      LogUtils.w(TAG, `checkColumn error : ${error?.message}, stack: ${error?.stack}`);
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    return Promise.resolve(false);
  }

  public static async delay(time: number): Promise<number> {
    return new Promise(resolve => setTimeout(resolve, time));
  }

  public static async notifyChange(dataAbilityHelper: dataShare.DataShareHelper, uri: string) {
    LogUtils.i(TAG, 'notifyChange start. uri:' + uri);
    await dataAbilityHelper.notifyChange(uri).then(() => {
      LogUtils.i(TAG, 'notifyChange success')
    }).catch((error: BusinessError) => {
      LogUtils.e(TAG, 'notifyChange error:' + error.message);
    });
  }

  public static notifyIntelligentScene(): void {
    LogUtils.i(TAG, 'notifyIntelligentScene start');
    let options: CommonEventManager.CommonEventPublishData = {
      bundleName: 'com.ohos.intelligentscene',
      data: 'com.ohos.contactsdataability',
    };
    try {
      CommonEventManager.publish(DND_DATA_RESTORE_FINISH, options, (error) => {
        if (error) {
          LogUtils.i(TAG, `notifyIntelligentScene error: ${error.code}, message is ${error.message}`);
        } else {
          LogUtils.e(TAG, `notifyIntelligentScene end`);
        }
      });
    } catch (error) {
      LogUtils.e(TAG, `notifyIntelligentScene error: ${error.code}, message is ${error.message}`);
    }
  }

  public static VALUE_BUCKET_SLOT: string = 'VALUE_BUCKET_SLOT'

  public static async executeSqlFuncWithRetry(sqlFunc: Function): Promise<void | number> {
    let retryIdx = 0;
    let ret: void | number = -1;
    while (retryIdx <= DELAY_TIME_ARRAY.length - 1) {
      try {
        ret = await sqlFunc();
      } catch (e) {
        LogUtils.e(TAG, `executeSql err,retryIdx:${retryIdx},code:${e.code},msg:${e.message}`);
        if (NEED_RETRY_CODE.has(e.code)) {
          await DatabaseUtils.delay(DELAY_TIME_ARRAY[retryIdx]);
          retryIdx++;
          continue;
        }
      }
      return ret;
    }
    LogUtils.e(TAG, `after retried still failed`);
    return ret;
  }
}