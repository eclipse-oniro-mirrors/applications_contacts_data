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
import contextConstant from '@ohos.app.ability.contextConstant';
import relationalStore from '@ohos.data.relationalStore';
import LogUtils from '../utils/LogUtils';
import { ArrayUtil } from '../utils/ArrayUtil';
import { SystemModeController } from '../utils/SystemModeController';

const RETRY_COUNT = 3;
const DELAY_TIMES = 100;
const DELAY_TIME_ARRAY = [50, 100, 200, 400, 800];
const NEED_RETRY_CODE = new Set([14800015, 14800024, 14800025, 14800026, 14800028, 14800029]);

/**
 * 单独给云同步用的 rdb helper，因为克隆、双升单的help涉及建库场景，云同步不涉及
 */
export class BaseRdbHelperForCloudSync {
  private readonly logTag: string = '';
  protected rdbStoreHelper?: rdb.RdbStore;

  constructor(logTag: string) {
    this.logTag = `[${logTag}::BaseRdbHelperForCloudSync]`;
  }

  public async getRdbStoreHelper(context: common.Context, dbName: string,
      isSupportSearch: boolean): Promise<rdb.RdbStore | undefined> {
    LogUtils.w(this.logTag, `getRdbStoreHelper start,dbName:${dbName}`);
    if (context === undefined || context === null) {
      LogUtils.w(this.logTag, 'getRdbStoreHelper failed context is empty');
      return undefined;
    }
    context.area = contextConstant.AreaMode.EL2;
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    try {
      const CONFIG: rdb.StoreConfig = {
        name: dbName,
        //云同步连接单框架db，改为正确等级S3
        securityLevel: rdb.SecurityLevel.S3,
        isSearchable: isSupportSearch,
        autoCleanDirtyData: false,
        customDir: '../../rdb',
        allowRebuild: true
      };
      rdbStoreHelper = await rdb.getRdbStore(context, CONFIG);
      if (rdbStoreHelper != null) { // 如果开库成功，
        const schemaVersion = await rdbStoreHelper.execute('pragma schema_version;');
        LogUtils.w(this.logTag, `initDB end, contact db version is:${rdbStoreHelper.version},schemaVersion:${schemaVersion}`);
        let rebuilt: number = rdbStoreHelper.rebuilt;
        LogUtils.i(this.logTag, `getRdbStoreContacts, rebuilt:${rebuilt}`);
        if (rebuilt == rdb.RebuildType.REBUILT ) { // 如果进行了重建
          await rdbStoreHelper.restore('contacts.db.bak', (err) => {
            if (err) {
              LogUtils.e(this.logTag, `getRdbStoreContacts, Restore failed, code is ${err.code},message is ${err.message}`);
              return;
            }
            LogUtils.i(this.logTag, `getRdbStoreContacts, Restore success.`);
          }); // 则进行恢复
        } else {
          // 没有重建，什么也不做
          LogUtils.i(this.logTag, `getRdbStoreContacts, No rebuilt`);
        }
      } else {
        //开库失败 进行恢复
        LogUtils.e(this.logTag, `getRdbStoreContacts, GetRdbStore err, rdbStoreHelper is null, skip rebuilt. `);
        await (rdbStoreHelper as rdb.RdbStore).restore('contacts.db.bak', (err) => {
          if (err) {
            LogUtils.e(this.logTag, `Restore failed,code is ${err.code},message is ${err.message}`);
            return;
          }
          LogUtils.i(this.logTag, `Restore success.`);
        })
      }
      if (rdbStoreHelper != null) {
        // 开库成功之后，设置 distributedTables
        let distributedConfig: relationalStore.DistributedConfig = {
          autoSync: false,
          asyncDownloadAsset: true,
          enableCloud: SystemModeController.getInstance().getIsEnableContactCloudSync()
        };
        let tablesToSync: string[] = ['cloud_raw_contact', 'cloud_groups'];
        await rdbStoreHelper.setDistributedTables(tablesToSync, relationalStore.DistributedType.DISTRIBUTED_CLOUD,
          distributedConfig);
        LogUtils.w(this.logTag, 'getRdbStoreHelper,setDistributedTables success.');
      }
    } catch (error) {
      LogUtils.w(this.logTag, `getRdbStoreHelper failed,code is ${error.code},message is ${error.message}`);
    }
    return Promise.resolve(rdbStoreHelper);
  }

  protected async initDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    LogUtils.i(this.logTag, 'BaseRdbHelperForCloudSync initDB start');
    return Promise.resolve(undefined);
  }

  /**
   * query data
   *
   * @param predicates 查询条件
   * @param columns 查询列
   * @returns ResultSet
   */
  public async queryData(table: string, predicates: rdb.RdbPredicates, columns?: Array<string>)
    : Promise<rdb.ResultSet | undefined> {
    try {
      const ruleSet = await this.rdbStoreHelper!.query(predicates, columns);
      return ruleSet;
    } catch (e) {
      LogUtils.e(this.logTag, `queryData table:${table}, err code:${e.code}, msg:${e.message}`);
    }
    return undefined;
  }

  /**
   * Insert data into contact
   *
   * @param data which data to insert
   * @param tableName which table name to insert
   * @returns the number of successful insertions
   */
  public async insertData(data: ValuesBucket, tableName: string): Promise<number> {
    if (data == undefined) {
      LogUtils.e(this.logTag, 'insertData ValuesBucket is NULL');
      return Promise.resolve(-1);
    }
    if (this.rdbStoreHelper === undefined) {
      LogUtils.e(this.logTag, 'rdbStore is NULL');
      return Promise.resolve(-1);
    }
    return this.executeSqlFuncWithRetry(() => this.rdbStoreHelper!.insert(tableName, data)) as Promise<number>;
  }

  /**
   * 开启rdb事务
   *
   */
  public async beginTransaction() {
    let delayTime = [100, 250, 500, 1000, 2000, 4000];
    let retryIdx = 0;
    while (retryIdx <= delayTime.length - 1) {
      try {
        if (this.rdbStoreHelper === undefined) {
          LogUtils.e(this.logTag, `beginTransaction err,this.rdbStoreHelper is undefined`);
          return false;
        }
        await this.rdbStoreHelper.beginTransaction();
      } catch (e) {
        LogUtils.e(this.logTag, `beginTransaction err,retryIdx:${retryIdx},msg:${e.message}`);
        //未能获取事务，延迟对应时间尝试再次获取
        await this.delay(delayTime[retryIdx]);
        retryIdx ++;
        continue;
      }
      return true;
    }
    LogUtils.e(this.logTag, `after retried, beginTransaction err`);
    return false;
  }

  /**
   * 提交rdb事务
   *
   */
  public async commit() {
    try {
      if (this.rdbStoreHelper === undefined) {
        LogUtils.e(this.logTag, `commit err,this.rdbStoreHelper is undefined`);
        return -1;
      }
      await this.rdbStoreHelper.commit();
      return 0;
    } catch (e) {
      LogUtils.e(this.logTag, `commit err,msg:${e.message}`);
      return -1;
    }
  }

  /**
   * 回滚rdb事务
   *
   */
  public async rollBack() {
    try {
      if (this.rdbStoreHelper === undefined) {
        LogUtils.e(this.logTag, `rollBack err,this.rdbStoreHelper is undefined`);
        return -1;
      }
      await this.rdbStoreHelper.rollBack();
      return 0;
    } catch (e) {
      LogUtils.e(this.logTag, `rollBack err,msg:${e.message}`);
      return -1;
    }
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
      LogUtils.e(this.logTag, 'updateData ValuesBucket or predicates is NULL');
      return -1;
    }
    if (this.rdbStoreHelper === undefined) {
      LogUtils.e(this.logTag, 'rdbStore is NULL');
      return -1;
    }
    return this.executeSqlFuncWithRetry(() => this.rdbStoreHelper!.update(data, predicates)) as Promise<number>;
  }

  /**
   * delete contact data
   *
   * @param data which data to contact
   * @param tableName which table name to contact
   * @returns the number of successful insertions
   */
  public async deleteData(predicates: rdb.RdbPredicates): Promise<number> {
    if (predicates == undefined) {
      LogUtils.e(this.logTag, 'deleteData predicates is NULL');
      return -1;
    }
    if (this.rdbStoreHelper === undefined) {
      LogUtils.e(this.logTag, 'rdbStore is NULL');
      return -1;
    }
    let ret = -1;
    try {
      ret = await this.rdbStoreHelper.delete(predicates);
    } catch (error) {
      LogUtils.e(this.logTag, `deleteData err,msg:${error.message}`);
    }
    return ret;
  }

  /**
   * Batch insert data into contact
   *
   * @param contactsDataList which data to insert
   * @param tableName which table name to insert
   * @returns the number of successful insertions
   */
  public async insertBatchData(contactsDataList: ValuesBucket[], tableName: string): Promise<number> {
    if (ArrayUtil.isEmpty(contactsDataList)) {
      LogUtils.e(this.logTag, 'contactsDataList is NULL');
      return -1;
    }
    if (this.rdbStoreHelper === undefined) {
      LogUtils.e(this.logTag, 'rdbStore is NULL');
      return -1;
    }
    let ret = -1;
    try {
      ret = await this.rdbStoreHelper.batchInsert(tableName, contactsDataList);
    } catch (error) {
      LogUtils.e(this.logTag, `insertBatchData err,msg:${error.message}`);
    }
    return ret;
  }

  /**
   * executesql
   *
   * @param sql SQL string
   * @param logSuccess exec result
   * @param tableName which table name to insert
   * @returns the number of successful insertions
   */
  public async querySql(sql: string, bindArgs?: Array<rdb.ValueType>) : Promise<relationalStore.ResultSet | undefined> {
    if (bindArgs === undefined) {
      bindArgs = []
    }
    if (this.rdbStoreHelper === undefined) {
      LogUtils.e('ContactsRelation', 'current rdbStoreHelper undefined');
      return undefined
    } else {
      let resultSet = await this.rdbStoreHelper.querySql(sql, bindArgs);
      if (resultSet === undefined ) {
        LogUtils.e('ContactsRelation', 'query result undefined');
        return undefined;
      }
      return resultSet;
    }
  }

  /**
   * executesql，可能抛出异常，使用放在try catch中；如果在事务中使用，执行抛出异常后，可以处理回滚
   *
   * @param sql SQL string
   * @param logSuccess exec result
   * @param tableName which table name to insert
   * @returns the number of successful insertions
   */
  public async executeSql(sql: string, logSuccess?: string, bindArgs?: Array<rdb.ValueType>) {
    if (bindArgs === undefined) {
      bindArgs = []
    }
    try {
      await this.rdbStoreHelper?.executeSql(sql, bindArgs);
      if (logSuccess != undefined && logSuccess != null) {
        LogUtils.i('ContactsRelation', logSuccess);
      }
    } catch (err) {
      LogUtils.e('ContactsRelation', `sql failed, err code: ${err?.code}, err message: ${err?.message}`);
      throw new Error('baseRdbHelperForCloudSync executeSql error');
    }
  }

  public async delay(time: number): Promise<number> {
    return new Promise(resolve => setTimeout(resolve, time));
  }

  public async executeSqlFuncWithRetry(sqlFunc: Function): Promise<void | number> {
    let retryIdx = 0;
    let ret: void | number = -1;
    while (retryIdx <= DELAY_TIME_ARRAY.length - 1) {
      try {
        ret = await sqlFunc();
      } catch (e) {
        LogUtils.e(this.logTag, `executeSql err,retryIdx:${retryIdx},code:${e?.code},msg:${e?.message}`);
        if (NEED_RETRY_CODE.has(e?.code)) {
          await this.delay(DELAY_TIME_ARRAY[retryIdx]);
          retryIdx++;
          continue;
        }
      }
      return ret;
    }
    LogUtils.e(this.logTag, `after retried still failed`);
    return ret;
  }

  public async retryGetSingleRdbForCloudSync(baseHelper: BaseRdbHelperForCloudSync, context: common.Context,
    dbName: string, isSupportSearch: boolean): Promise<rdb.RdbStore | undefined> {
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    for (let i = 1; i <= RETRY_COUNT; i++) {
      LogUtils.i(this.logTag, `retryGetSingleRdb start: ${i}`);
      rdbStoreHelper = await baseHelper.getRdbStoreHelper(context, dbName, isSupportSearch);
      if (rdbStoreHelper) {
        LogUtils.i(this.logTag, `retryGetSingleRdb ${i} success`);
        break;
      }
      await this.delay(DELAY_TIMES);
    }
    return rdbStoreHelper;
  }
}