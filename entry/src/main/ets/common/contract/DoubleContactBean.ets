/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
import { RawContacts } from './RawContacts';
import rdb from '@ohos.data.relationalStore';
import { StringUtil } from '../utils/StringUtil';

export default class DoubleContactBean {
  public rawId: number = 0;
  public contactId: number = 0;
  public displayNameBase64: string = '';
  public accountId: number = 0;

  public static columns: string[] = [RawContacts.ID, RawContacts.CONTACT_ID,
  RawContacts.DISPLAY_NAME, RawContacts.ACCOUNT_ID];

  // 注意：所有索引要与上面 columns 一一对应
  constructor() {
  }

  static fromRuleSet(resultSet: rdb.ResultSet): DoubleContactBean {
    const doubleContactBean: DoubleContactBean = new DoubleContactBean();
    let index = 0;
    doubleContactBean.rawId = resultSet.getLong(index++);
    doubleContactBean.contactId = resultSet.getLong(index++);
    doubleContactBean.displayNameBase64 = StringUtil.maskSensitiveInfo(resultSet.getString(index++));
    doubleContactBean.accountId = resultSet.getLong(index++);
    return doubleContactBean;
  }
}
