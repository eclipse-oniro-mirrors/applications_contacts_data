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
import { Account } from '../contract/Account';
import { Mimetype } from '../contract/Mimetype';
import DbConstants from '../DbConstants';
import LogUtils from '../utils/LogUtils';

export class BaseDbHelper {
  private readonly logTag: string = '';

  constructor(logTag: string) {
    this.logTag = `[${logTag}::BaseDbHelper]`;
  }

  public async initDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    LogUtils.i(this.logTag, 'BaseDbHelper initDB start');
    return Promise.resolve(undefined);
  }

  public async getRdbStoreHelper(context: common.Context, dbName: string): Promise<rdb.RdbStore | undefined> {
    LogUtils.i(this.logTag, 'getRdbStoreHelper start');
    if (context === undefined || context === null) {
      LogUtils.w(this.logTag, '[createDB] context is empty');
      return undefined;
    }
    let rdbStoreHelper: rdb.RdbStore | undefined = undefined;
    // 此方法只用于尝试重新获取双框架数据库，无需设置el1或el2目录
    try {
      const OLD_CONFIG: rdb.StoreConfig = {
        name: dbName,
        //双升单连接双框架db，仍然维持原来等级
        securityLevel: rdb.SecurityLevel.S1
      };
      rdbStoreHelper = await rdb.getRdbStore(context, OLD_CONFIG);
    } catch (error) {
      LogUtils.e(this.logTag, `[createDB] failed, code:${error.code}, msg:${error.message}`);
    }
    return Promise.resolve(rdbStoreHelper);
  }

  public async initMimeTypeId(context: common.Context, typeIdMap: Record<number, number>): Promise<boolean> {
    let rdbStore = await this.initDB(context);
    let resultSetType: rdb.ResultSet | null | undefined = null;
    try {
      let predicatesType = new rdb.RdbPredicates(DbConstants.OLD_MIMETYPE_TABLE_NAME);
      let queryTypeColumn = [Mimetype.ID, Mimetype.MIMETYPE];
      resultSetType = await rdbStore!.query(predicatesType, queryTypeColumn);
      if (resultSetType === undefined || !resultSetType.goToFirstRow()) {
        LogUtils.w(this.logTag, 'initMimeTypeId no data phone found.');
        return true;
      }
      do {
        let id = resultSetType.getLong(resultSetType.getColumnIndex(Mimetype.ID));
        let type = resultSetType.getString(resultSetType.getColumnIndex(Mimetype.MIMETYPE));
        let singleType = DbConstants.MIMETYPE_MAP.get(type);
        if (singleType) {
          typeIdMap[id] = DbConstants.SIMPLE_TYPE.indexOf(singleType) + 1;
        }
      } while (resultSetType.goToNextRow());
      return true;
    } catch (error) {
      LogUtils.w(this.logTag, `[initMimeTypeId] error : ${error?.message}, stack: ${error?.stack}`);
    } finally {
      if (resultSetType && !resultSetType.isClosed) {
        resultSetType.close();
      }
    }
    return false;
  }

  public async queryAccountId(context: common.Context): Promise<number> {
    let rdbStore = await this.initDB(context);
    let id = DbConstants.QUERY_ACCOUNT_FAILED;
    let resultSetType: rdb.ResultSet | null | undefined = null;
    try {
      let predicatesType = new rdb.RdbPredicates(DbConstants.OLD_ACCOUNT_TABLE_NAME);
      let queryTypeColumn = [Account.ID];
      predicatesType.equalTo(Account.ACCOUNT_NAME, 'Phone');
      resultSetType = await rdbStore!.query(predicatesType, queryTypeColumn);
      if (resultSetType === undefined || !resultSetType.goToFirstRow()) {
        LogUtils.w(this.logTag, 'get account no data phone found.');
        return Promise.resolve(DbConstants.QUERY_ACCOUNT_NO_DATA);
      }
      id = resultSetType.getLong(resultSetType.getColumnIndex(Account.ID));
    } catch (error) {
      LogUtils.w(this.logTag, `[queryAccountId] error : ${error?.message}, stack: ${error?.stack}`);
    } finally {
      if (resultSetType && !resultSetType.isClosed) {
        resultSetType.close();
      }
    }
    return Promise.resolve(id);
  }
}
