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

import rdb from '@ohos.data.relationalStore';
import common from '@ohos.app.ability.common';
import DbConstants from '../DbConstants';
import { BaseSingleRdbInsertHelper } from '../helperBase/BaseSingleRdbInsertHelper';
import LogUtils from '../utils/LogUtils';
import DatabaseUtils from '../utils/DatabaseUtils';

const TAG = 'ContactsSingleInsertHelper';

// 双升单处理单框架库dbHelper
class ContactsSingleInsertHelper extends BaseSingleRdbInsertHelper {
  private rdbStoreHelper?: rdb.RdbStore;

  constructor() {
    LogUtils.i(TAG, 'constructor start');
    super(TAG);
  }

  public async initDB(context: common.Context): Promise<rdb.RdbStore | undefined> {
    if (this.rdbStoreHelper) {
      return Promise.resolve(this.rdbStoreHelper);
    }
    LogUtils.i(TAG, 'initSingleDB start');
    this.rdbStoreHelper = await super.getRdbStoreHelper(context, DbConstants.NEW_DATABASE_NAME, true);
    if (!this.rdbStoreHelper) {
      LogUtils.w(TAG, 'initSingleDB get getRdbStoreHelper error');
      this.rdbStoreHelper = await DatabaseUtils.retryGetSingleRdb(contactsSingleInsertHelper,
        context, DbConstants.NEW_DATABASE_NAME, true);
    }
    return Promise.resolve(this.rdbStoreHelper);
  }
}

// 双升单处理单框架库dbHelper
export const contactsSingleInsertHelper: ContactsSingleInsertHelper = new ContactsSingleInsertHelper();