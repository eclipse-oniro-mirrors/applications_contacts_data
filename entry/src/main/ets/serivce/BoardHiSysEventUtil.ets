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

import { SpaceType } from "../common/utils/ContactStatisticsCount";

//
export class BaseStatisticsCount {
  // 应用包名
  public PNAMEID: string = 'com.ohos.contactsdataability';
  // 应用版本
  public PVERSIONID: string = '1000071';
}

/**
 * 通话记录数量: call—log数量信息
 */
export class CallStatisticsCount extends BaseStatisticsCount {
  // 操作类型 0-单到单或云备份,1-是双到单,2-是ios到单,3-是三方到单,4-是双升单,5-隐私空间迁移,6-其他
  public TYPE: number = 0;
  // 本次操作通话数量
  public FRONT_CALL_NUM: number = 0;
  // 本次实际操作通话数量
  public LATER_CALL_NUM: number = 0;
  // 本次操作来电通话数量
  public INCOMING_CALL_NUM: number = 0;
  // 本次操作去电通话数量
  public OUTGOING_CALL_NUM: number = 0;
  // 本次操作私密通话数量
  public PRIVATE_CALL_NUM: number = 0;
  // 本次操作拦截通话数量
  public INTERCEPT_CALL_NUM: number = 0;
  // 主空间还是隐私空间
  public USERID: number = SpaceType.MAIN;
  // 操作前通话记录总数量
  public BEFORE_TOTAL: number = 0;
  // 操作后通话记录总数量
  public AFTER_TOTAL: number = 0;
  // 操作后来电总数量
  public INCOMING_CALL_TOTAL: number = 0;
  // 操作后去电总数量
  public OUTGOING_CALL_TOTAL: number = 0;
  // 操作后私密总数量
  public PRIVATE_CALL_TOTAL: number = 0;
  // 操作后拦截总数量
  public INTERCEPT_CALL_TOTAL: number = 0;
}

/**
 * Call 迁移
 */
export class Migration extends BaseStatisticsCount {
  // 迁移类型 恢复时EL1迁移到EL5:0,单备份时从EL5迁移到EL1:1
  public TYPE: number = 0;
  // 执行结果 0是成功 其他异常
  public RESULT: number = -1;
  // 迁移成功数量
  public COUNT: number = 0;
  // 场景或失败原因
  public REASON: string = '';
}
