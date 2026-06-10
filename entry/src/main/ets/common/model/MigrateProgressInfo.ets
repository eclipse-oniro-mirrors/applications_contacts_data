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

import LogUtils from '../../common/utils/LogUtils';

@Sendable
export class MigrateProgressInfo {
  private logTag: string = 'MigrateProgressInfo';
  private migrateProgress: string = '';

  private rawContactCount: number = 0;
  private succContactCount: number = 0;
  private processContactCount: number = 0;
  private callLogCount: number = 1;
  private repeatCallLogCount: number = 0;
  private succCallLogCount: number = 0;

  private cloneProgressCoefficients: number = 0.6;
  private aggregatorProgressCoefficients: number = 0.2;
  private aggregatorQueryCoefficients: number = 0.2;
  private aggregatorContactCount: number = 1;
  private aggregatorProcessCount: number = 0;
  private isAggregatorAll: boolean = false;
  private aggregatorAllQueryCount: number = 2000;
  private aggregatorAllQueryProcess: number = 0;
  private expandCapacity: number = 100;

  private errorCode: string = '0';
  private errorInfo: string = '';

  // 操作前通话记录总数量
  public beforeTotal: number = 0;
  // 操作前来电总数量
  public incomingCallTotal: number = 0;
  // 操作前去电总数量
  public outgoingCallTotal: number = 0;
  // 操作前私密总数量
  public privateCallTotal: number = 0;
  // 操作前拦截总数量
  public interceptCallTotal: number = 0;

  updateErrorCode(errorCode: string) {
    this.errorCode = errorCode;
  }

  updateErrorinfo(errorInfo: string) {
    this.errorInfo = errorInfo;
  }

  public getSuccContactCount(): number {
    return this.succContactCount;
  }

  // 旧表开始拿到的需要迁移的通话记录数量
  public getCallLogCount(): number {
    return this.callLogCount;
  }

  // 新表拿到的操作前通话记录总数量
  public getBeforeTotal(): number {
    return this.beforeTotal;
  }

  // 更新操作前通话记录总数量
  public updateBeforeTotal(count: number) {
    this.beforeTotal = count;
  }

  // 新表拿到的操作前来电总数量
  public getIncomingCallTotal(): number {
    return this.incomingCallTotal;
  }

  // 更新操作前来电总数量
  public updateIncomingCallTotal(count: number) {
    this.incomingCallTotal = count;
  }

  // 新表拿到的操作前去电总数量
  public getOutgoingCallTotal(): number {
    return this.outgoingCallTotal;
  }

  // 更新操作前去电总数量
  public updateOutgoingCallTotal(count: number) {
    this.outgoingCallTotal = count;
  }

  // 新表拿到的操作前私密总数量
  public getPrivateCallTotal(): number {
    return this.privateCallTotal;
  }

  // 更新操作前私密总数量
  public updatePrivateCallTotal(count: number) {
    this.privateCallTotal = count;
  }

  // 新表拿到的操作前拦截总数量
  public getInterceptCallTotal(): number {
    return this.interceptCallTotal;
  }

  // 更新操作前拦截总数量
  public updateInterceptCallTotal(count: number) {
    this.interceptCallTotal = count;
  }

  getErrorCode(): string {
    return this.errorCode;
  }
  getErrorInfo(): string {
    return this.errorInfo;
  }

  getMigrateProgress(): string {
    let rawContactProcess: number = this.rawContactCount * this.expandCapacity;
    this.migrateProgress = `{"progressInfo":[{"name":"contact","processed": ${
      this.getContactMigrateProgress(rawContactProcess)}, "total": ${rawContactProcess}},{"name":"calllog","processed": ${
      this.succCallLogCount + this.repeatCallLogCount},"total": ${this.callLogCount}}]}`
    return this.migrateProgress;
  }

  getContactMigrateProgress(rawContactProcess: number): string {
    let cloneProgress: number = this.processContactCount * this.expandCapacity * this.cloneProgressCoefficients;
    let aggregatorQueryProgress: number = 0;
    // 如果在克隆完成后执行全量智能合并时，进度请求过来，智能合并总数未更新，增加查询进度，总进度2000，每次自增1
    if (this.isAggregatorAll) {
      if (this.aggregatorContactCount == 1 && this.aggregatorProcessCount == 0) {
        this.aggregatorAllQueryProcess = this.aggregatorAllQueryProcess + 1;
        aggregatorQueryProgress = this.aggregatorAllQueryProcess / this.aggregatorAllQueryCount *
          rawContactProcess * this.aggregatorQueryCoefficients;
      } else {
        // 如果智能合并已经查到数据，查询进度直接为1
        aggregatorQueryProgress = 1 * rawContactProcess * this.aggregatorQueryCoefficients;
      }
    }
    let aggregatorProgress: number = this.aggregatorProcessCount / this.aggregatorContactCount *
      rawContactProcess * this.aggregatorProgressCoefficients;
    LogUtils.i(this.logTag, `processContactCount: ${this.processContactCount}, aggregatorProcessCount: ${
      this.aggregatorProcessCount}, aggregatorContactCount: ${this.aggregatorContactCount},cloneProgress: ${
      cloneProgress},aggregatorProgress: ${aggregatorProgress}, isAggregatorAll: ${
      this.isAggregatorAll}, aggregatorQueryProgress: ${aggregatorQueryProgress}`);
    let progress: number = Math.round(cloneProgress + aggregatorQueryProgress + aggregatorProgress);
    if (progress > rawContactProcess) {
      progress = rawContactProcess;
    }
    return `${progress}`;
  }

  updateRawContactCount(rawContactCount: number) {
    this.rawContactCount = rawContactCount;
  }

  updateSuccContactCount(succContactCount: number) {
    this.succContactCount = succContactCount;
  }

  updateProcessContactCount(processContactCount: number) {
    this.processContactCount = processContactCount;
  }

  updateCallLogCount(callLogCount: number) {
    this.callLogCount = callLogCount;
  }

  updateRepeatCallLogCount(repeatCallLogCount: number) {
    this.repeatCallLogCount = repeatCallLogCount;
  }

  updateSuccCallLogCount(succCallLogCount: number) {
    this.succCallLogCount = succCallLogCount;
  }
  
  updateAggregatorContactCount(aggregatorContactCount: number) {
    this.aggregatorContactCount = aggregatorContactCount;
  }

  updateAggregatorProcessCount(aggregatorProcessCount: number) {
    this.aggregatorProcessCount = aggregatorProcessCount;
  }

  updateIsAggregatorAll(isAggregatorAll: boolean) {
    this.isAggregatorAll = isAggregatorAll;
  }
}