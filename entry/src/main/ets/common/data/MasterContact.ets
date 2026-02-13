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
import { ValuesBucket } from '@ohos.data.ValuesBucket';

export default class MasterContact {
  public contactId: number = 0;
  public rawId: number = 0;
  public uuid?: string;
  // 昵称
  public nickname?: string;
  // 公司
  public company?: string;
  // 职位
  public position?: string;
  // 头像
  public blobData?: Uint8Array;
  // 收藏
  public favorite?: number;
  // 收藏排序
  public favoriteOrder?: number;
  // 备注
  public note?: string;
  // 卡片
  public formId: string | null = null;
  public focusModeList: string | null = null;
  public uniqueKey: string = '';
  public needUpdateRaw: boolean = false;
  public rawUpdateValuesBucket: ValuesBucket = {};
  public needUpdateContact: boolean = false;
  public contactUpdateValuesBucket: ValuesBucket = {};
  public insertValuesBucket: ValuesBucket[] = [];
  public contactDataKeySet: Set<string> = new Set<string>();
  // 联系人生日 和 农历生日 只支持各保存1个
  public hasBirthday: boolean = false;
  public hasLunarBirthday: boolean = false;
  public contactedCount: number = 0;
  public lastestContactedTime: number = 0;
  public personalRingtone: string = '';
  public personalNotificationRingtone: string = '';
  public ringtonePath: string = '';
  public poster: string = '';
}