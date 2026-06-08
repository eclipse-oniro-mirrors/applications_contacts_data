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
import { BaseRdbHelperForCloudSync } from '../helperBase/BaseRdbHelperForCloudSync';
import LogUtils from '../utils/LogUtils';

const TAG = 'ContactsRdbHelperForCloudSync';
/**
 * 单独给云同步用的 rdb helper，因为克隆、双升单的help涉及建库场景，云同步不涉及
 */
class ContactsRdbHelperForCloudSync extends BaseRdbHelperForCloudSync {

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
      this.rdbStoreHelper = await this.retryGetSingleRdbForCloudSync(contactRdbHelper,
        context, DbConstants.NEW_DATABASE_NAME, true);
    }
    return Promise.resolve(this.rdbStoreHelper);
  }
}

// 创建对象，创建后没有初始化rdbStoreHelper，需要调用initDB才初始化了
export const contactRdbHelper: ContactsRdbHelperForCloudSync = new ContactsRdbHelperForCloudSync();