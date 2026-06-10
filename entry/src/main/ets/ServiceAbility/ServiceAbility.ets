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
import type Want from '@ohos.application.Want';
import ServiceExtension from '@ohos.app.ability.ServiceExtensionAbility';
import ContactsDataService from './../serivce/ContactsDataService';
import LogUtils from '../common/utils/LogUtils';
import hiTraceMeter from '@ohos.hiTraceMeter';
import { checkContactTask, manualMergeContactTask } from './../serivce/RepeatContactHandleService';
import HandleUniqueKeyService from '../serivce/HandleUniqueKeyService';
import { Stub } from '../WorkScheduler/Common';
import ContactAggregator from '../aggregation/ContactAggregator';
import { StringUtil } from '../common/utils/StringUtil';

const TAG = 'ServiceAbility';

export default class ServiceAbility extends ServiceExtension {
  public contactsDataService: ContactsDataService | undefined = undefined;
  public timestamp: number = 0;
  public workerId: number = 0;
  public syncTimestamp: number = 0;
  public syncWorkerId: number = 0;
  public traceServiceCreate: number = 1001;
  public athousandMs = 1000;
  public oneHundredMs: number = 5000;
  public fiveThousandMs: number = 5000;
  /**
   * ServiceExtension 初始化时，初始化业务处理类 ContactsDataService 的单例，并设置其上下文
   * @param want
   */
  onCreate(want: Want): void {
    LogUtils.i(TAG, 'onCreate ContactsData service');
    hiTraceMeter.startTrace('serviceonCreate', this.traceServiceCreate);
    this.contactsDataService = ContactsDataService.getInstance();
    this.contactsDataService.init(this.context);
    hiTraceMeter.finishTrace('serviceonCreate', this.traceServiceCreate);
  }

  onConnect(want: Want): Stub {
    LogUtils.i(TAG, 'onConnect ContactsData service');
    return new Stub('ServiceAbility');
  }

  onDisconnect(): void {
    LogUtils.i(TAG, 'onDisconnect ContactsData service');
  }

  onRequest(want: Want, startId: number): void {
    let insertParam: string = want.parameters?.insert;
    let updateParam: string = want.parameters?.update;
    let deleteParam: string = want.parameters?.delete;
    let compareCloudAndRawContact: string = want.parameters?.compareCloudAndRawContact;
    let refreshContact: string = want.parameters?.refreshContact;
    let mergeContactArgs: string = want.parameters?.mergeContactArgs;

    LogUtils.w(TAG, 'onRequest ContactsData service insertParam length' + insertParam?.length + ' updateParam length' +
      updateParam?.length +
      ' deleteParam length' + deleteParam?.length + ' compareCloudAndRawContact ' + compareCloudAndRawContact +
      ' refreshContact ' + refreshContact +
      ' mergeContactArgs ' + want.parameters?.mergeContactArgs);
    if (refreshContact === 'mergeContact') {
      // 应用触发全量去重，会通过 datashare 调到这里，克隆 云同步是过程中去重，不会调这里
      LogUtils.i(TAG, 'start mergeContact task');
      if (!ContactsDataService.getInstance().isMergingContact()) { //如果当前没在去重，才去调用去重方法，保证去重任务不会重复执行
        ContactsDataService.getInstance().setMergingContact(true);
        let task: Promise<boolean> = Promise.resolve(true);
        // 合并联系人
        if (want.parameters?.mergeContactArgs && want.parameters?.mergeContactArgs !== 'manual') {
          task = manualMergeContactTask(this.context!, want.parameters?.mergeContactArgs);
        } else {
          let isAutoMerge: boolean = StringUtil.isEmpty(want.parameters?.mergeContactArgs);
          task = HandleUniqueKeyService.getInstance().waitForCalcUniqueKeyEnd(this.context)
            .then(() => {
              // 检查重复联系人
              return checkContactTask(this.context!, isAutoMerge);
            });
        }
        task.then(() => {//调用去重方法
          ContactsDataService.getInstance().setMergingContact(false);
        }).catch((error: Error) => {
          ContactsDataService.getInstance().setMergingContact(false);
          LogUtils.e(TAG, 'checkContactTask error:' + error.message);
        });
      }
     } else if (refreshContact === 'splitRawContactAfterDel') {
      LogUtils.i(TAG, 'start splitRawContactAfterDel task');
      ContactAggregator.getInstance().doSplitAggregationContactAfterDel(want.parameters?.mergeContactArgs)
        .catch((error: Error) => {
          LogUtils.e(TAG, 'splitRawContactAfterDel error:%s' + error?.message + ', stack: ' + error?.stack);
        });
    } else if (refreshContact === 'localRestoreContact') {
      LogUtils.i(TAG, 'start localRestoreContact task');
      // 本地恢复联系人
      ContactsDataService.getInstance().localRestoreContact(want.parameters?.mergeContactArgs)
        .catch((error: Error) => {
          LogUtils.e(TAG, 'localRestoreContact error:%s' + error?.message + ', stack: ' + error?.stack);
        });
    } else {
      let nowTimeStamp = new Date().getTime();
      if (nowTimeStamp - this.timestamp < this.fiveThousandMs) {
        clearTimeout(this.workerId)
      }
      this.timestamp = nowTimeStamp;
      this.workerId = setTimeout(() => {
        ContactsDataService.getInstance().contactsDataChange(this.context).catch((error: Error) => {
          LogUtils.e(TAG, 'local contacts change contactsDataChange error:%s' + error?.message + ', stack: ' + error?.stack);
        })
      }, this.fiveThousandMs); // 如果短期内数据变化频繁，顺延定时器，让这段逻辑不会并行或者多次执行
    }
  }

  onDestroy(): void {
    LogUtils.i(TAG, 'onDestroy ContactsData service');
  }
}