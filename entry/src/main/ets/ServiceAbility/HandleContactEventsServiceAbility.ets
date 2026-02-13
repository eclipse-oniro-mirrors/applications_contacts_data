/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import ServiceExtension from '@ohos.app.ability.ServiceExtensionAbility';
import BirthdayManager from '../serivce/BirthdayManager';
import LogUtils from '../common/utils/LogUtils';
import type Want from '@ohos.application.Want';
import { Stub } from '../WorkScheduler/Common';
 
const TAG = 'HandleContactEventsServiceAbility';
const CONTACTS_BIRTHDAY_INSERT: number = 1;
const CONTACTS_BIRTHDAY_UPDATE: number = 2;
const CONTACTS_BIRTHDAY_DELETE: number = 3;
const CONTACTS_BIRTHDAY_SYNC: number = 4;
const CONTACTS_BIRTHDAY_BATCH_INSERT:number = 5;
export default class HandleContactEventsServiceAbility extends ServiceExtension {
  /**
   * ServiceExtension 初始化时，初始化业务处理类 BirthdayManager 的单例，并设置其上下文
   * @param want
   */
  onCreate(want: Want): void {
    LogUtils.i(TAG, 'onCreate HandleContactEvents service');
    BirthdayManager.getInstance().init(this.context);
  }
 
  onConnect(want: Want): Stub {
    LogUtils.i(TAG, 'onConnect HandleContactEvents service');
    return new Stub('HandleContactEventsServiceAbility');
  }
 
  onDisconnect(): void {
    LogUtils.i(TAG, 'onDisconnect HandleContactEvents service');
  }
 
  onRequest(want: Want, startId: number): void {
    let operationType: number = want.parameters?.operationType;
    let values: string = want.parameters?.values;
    let syncBirthdays: string = want.parameters?.syncBirthdays;
    LogUtils.i(TAG, 'onRequest HandleContactEventsServiceAbility operationType ' + operationType +
      ', values = ' + values + ', syncBirthdays = ' + syncBirthdays);
    switch (operationType) {
      case CONTACTS_BIRTHDAY_INSERT:
        BirthdayManager.getInstance().handleSingleBirthday(values, false);
        break;
      // 删除日历信息
      case CONTACTS_BIRTHDAY_DELETE:
        if (values.length > 0) {
          let eventIds: number[] = values.split(',').map(Number);
          LogUtils.i(TAG, 'onRequest birthday delete eventIds ' + eventIds);
          if (eventIds.length > 0) {
            BirthdayManager.getInstance().deleteBirthdayEvent(eventIds);
          }
        }
        break;
      case CONTACTS_BIRTHDAY_SYNC:
        BirthdayManager.getInstance().syncAllBirthdayToCalendar();
        break;
      case CONTACTS_BIRTHDAY_BATCH_INSERT:
        let rawContctIds: number[] = values.split(',').map(Number);
        if (rawContctIds.length > 0) {
          BirthdayManager.getInstance().handleBatchBirthday(rawContctIds, false);
        }
        break;
      default:
        break;
    }
  }
  onDestroy(): void {
    LogUtils.i(TAG, 'onDestroy ContactsData service');
  }
}