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
import { BusinessError } from '@ohos.base';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import contextConstant from '@ohos.app.ability.contextConstant';
import securityLabel from '@ohos.file.securityLabel';
import LogUtils from '../utils/LogUtils';
import { ArrayUtil } from '../utils/ArrayUtil';
import { Account } from '../contract/Account';
import DbConstants from '../DbConstants';
import DatabaseUtils from '../utils/DatabaseUtils';
import { SystemModeController } from '../utils/SystemModeController';
import { Data } from '../contract/Data';
import { LATEST_CHANNEL_STORE_VERSION } from '../SqlConstants';

/**
 * 双升单，单框架insert处理dbHelper
 */
export class BaseSingleRdbInsertHelper {
  private readonly logTag: string = '';

  constructor(logTag: string) {
    this.logTag = `[${logTag}::BaseSingleRdbInsertHelper]`;
  }

  public async getRdbStoreHelper(context: common.Context, dbName: string,
    isSupportSearch: boolean): Promise<rdb.RdbStore | undefined> {
    LogUtils.i(this.logTag, 'getRdbStoreHelper start');
    if (context === undefined || context === null) {
      LogUtils.w(this.logTag, 'getRdbStoreHelper failed context is empty');
      return undefined;
    }

    //单框架开本地联系人库指定路径EL2
    context.area = contextConstant.AreaMode.EL2;
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    try {
      // 设置文件的数据等级为s3
      await securityLabel.setSecurityLabel('/data/storage/el2/database/rdb/contacts.db', 's3').then(() => {
        LogUtils.i(this.logTag, 'Succeeded in setSecurityLabeling contacts.db s3');
      }).catch((err: BusinessError) => {
        LogUtils.e(this.logTag, `Failed to setSecurityLabel. Code: ${err.code}, message: ${err.message}`);
      });
      const CONFIG: rdb.StoreConfig = {
        name: dbName,
        //双升单连接单框架db，改为正确等级S3
        securityLevel: rdb.SecurityLevel.S3,
        isSearchable: isSupportSearch,
        autoCleanDirtyData: false,
        customDir: '../../rdb',
        allowRebuild: true
      };
      rdbStoreHelper = await rdb.getRdbStore(context, CONFIG);
      if (rdbStoreHelper != null) { // 如果开库成功，
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
      const curVersion = rdbStoreHelper.version;
      if (curVersion === 0) {
        LogUtils.i(this.logTag, 'initDB end, contact db version is:0, start exec ddl');
        await this.createTable(context, rdbStoreHelper);
        await this.createViewTable(context, rdbStoreHelper);
        await this.createTableRelative(rdbStoreHelper);
        rdbStoreHelper.version = LATEST_CHANNEL_STORE_VERSION;
      } else {
        LogUtils.i(this.logTag, `initDB end, contact db version is:${curVersion}, skip exec ddl`);
      }
      if (rdbStoreHelper != null && dbName === DbConstants.NEW_DATABASE_NAME) {
        // 开库成功之后，设置 distributedTables
        let distributedConfig: rdb.DistributedConfig = {
          autoSync: false,
          asyncDownloadAsset: true,
          enableCloud: SystemModeController.getInstance().getIsEnableContactCloudSync()
        };
        let tablesToSync: string[] = ['cloud_raw_contact', 'cloud_groups'];
        await rdbStoreHelper.setDistributedTables(tablesToSync, rdb.DistributedType.DISTRIBUTED_CLOUD,
          distributedConfig);
        LogUtils.w(this.logTag, 'getRdbStoreHelper,setDistributedTables success.');
      }
    } catch (error) {
      LogUtils.w(this.logTag, `getRdbStoreHelper failed,code is ${error.code},message is ${error.message}`);
    }
    return Promise.resolve(rdbStoreHelper);
  }

  protected async initDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    LogUtils.i(this.logTag, 'BaseSingleRdbInsertHelper initDB start');
    return Promise.resolve(undefined);
  }

  /**
   * Batch insert data into table contact_type
   *
   * @param context to get RdbStore
   * @param contactsDataList which data to insert
   * @returns the number of successful insertions
   */
  public async insertContactType(context: common.Context): Promise<number> {
    let rdbStoreHelper: rdb.RdbStore | undefined = await this.initDB(context);
    if (rdbStoreHelper === undefined) {
      return Promise.resolve(-1);
    }
    let typeDataArray: ValuesBucket[] = [];
    DbConstants.SIMPLE_TYPE.forEach((type, index) => {
      let pushBody : Record<string, string | number> = {};
      pushBody[Data.NEW_ID] = index + 1;
      pushBody[Data.CONTENT_TYPE] = type;
      typeDataArray.push(pushBody);
    });
    let ret = -1;
    LogUtils.i(this.logTag, 'insertType start');
    try {
      rdbStoreHelper.beginTransaction();
      ret = await rdbStoreHelper.batchInsert('contact_type', typeDataArray);
      rdbStoreHelper.commit();
    } catch (error) {
      rdbStoreHelper.rollBack();
      LogUtils.e(this.logTag, 'insertType error');
    }
    return ret;
  }

  /**
   * Insert data into table account
   *
   * @param context to get RdbStore
   * @param id which data to insert
   * @returns the number of successful insertions
   */
  public async insertPhoneAccount(context: common.Context, id: number): Promise<number> {
    let rdbStoreHelper: rdb.RdbStore | undefined = await this.initDB(context);
    if (rdbStoreHelper === undefined) {
      return Promise.resolve(-1);
    }
    let ret = -1;
    const values: ValuesBucket = {};
    values[Account.NEW_ID] = id;
    values[Account.ACCOUNT_NAME] = 'phone';
    values[Account.ACCOUNT_TYPE] = 'com.ohos.contacts';
    try {
      ret = await rdbStoreHelper.insert(DbConstants.NEW_ACCOUNT_TABLE_NAME, values);
    } catch (error) {
      LogUtils.e(this.logTag, 'insertPhoneAccount error');
    }
    return Promise.resolve(ret);
  }

  /**
   * Batch insert data into contact
   *
   * @param context to get RdbStore
   * @param contactsDataList which data to insert
   * @param tableName which table name to insert
   * @returns the number of successful insertions
   */
  public async insertBatchData(context: common.Context,
    contactsDataList: ValuesBucket[], tableName: string): Promise<number> {
    if (ArrayUtil.isEmpty(contactsDataList)) {
      LogUtils.e(this.logTag, 'contactsDataList is NULL');
      return Promise.resolve(-1);
    }
    let rdbStoreHelper: rdb.RdbStore | undefined = await this.initDB(context);
    if (rdbStoreHelper === undefined) {
      return Promise.resolve(-1);
    }
    let ret = -1;
    LogUtils.i(this.logTag, 'insertBatchData start');
    try {
      rdbStoreHelper.beginTransaction();
      ret = await rdbStoreHelper.batchInsert(tableName, contactsDataList);
      rdbStoreHelper.commit();
    } catch (error) {
      rdbStoreHelper.rollBack();
      LogUtils.e(this.logTag, `insertBatchData err,msg:${error.message}`);
    }
    return ret;
  }

  private async createTable(context: common.Context, rdbStore: rdb.RdbStore) {
    for (const tableName of DbConstants.CREATE_CONTACT_TABLE_MAP.keys()) {
      const createTableSql = DbConstants.CREATE_CONTACT_TABLE_MAP.get(tableName);
      try {
        await DatabaseUtils.executeSqlFuncWithRetry(() => rdbStore.executeSql(createTableSql!));
      } catch (error) {
        LogUtils.e(this.logTag, `Create table error: code: ${error.code}, message: ${error.message}`);
      }
    }
    LogUtils.w(this.logTag, `Create table end`);
  }

  private async createViewTable(context: common.Context, rdbStore: rdb.RdbStore) {
    for (const viewName of DbConstants.CREATE_CONTACT_VIEW_MAP.keys()) {
      const createViewSql = DbConstants.CREATE_CONTACT_VIEW_MAP.get(viewName);
      try {
        await DatabaseUtils.executeSqlFuncWithRetry(() => rdbStore.executeSql(createViewSql!));
      } catch (error) {
        LogUtils.e(this.logTag, `Create view table error: code: ${error.code}, message: ${error.message}`);
      }
    }
    LogUtils.w(this.logTag, `Create view table end`);
  }

  /**
   * 创建索引或触发器
   * @param rdbStore
   */
  private async createTableRelative(rdbStore: rdb.RdbStore) {
    for (const sql of DbConstants.CREATE_CONTACT_TABLE_RELATIVE) {
      try {
        await DatabaseUtils.executeSqlFuncWithRetry(() => rdbStore.executeSql(sql));
      } catch (error) {
        LogUtils.e(this.logTag, `Create table relative error: code: ${error.code}, message: ${error.message}`);
      }
    }
    LogUtils.w(this.logTag, `Create table relative end`);
  }
}
