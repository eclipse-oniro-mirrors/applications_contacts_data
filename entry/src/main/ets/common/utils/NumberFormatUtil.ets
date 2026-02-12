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

import i18n from '@ohos.i18n';
import { BusinessError } from '@ohos.base';
import LogUtils from './LogUtils';

const TAG = 'NumberFormatUtil';

export function getFormatNumber(phoneNumber: string | null): string | null {
  if (!phoneNumber || phoneNumber.length === 0) {
    return null;
  }
  try {
    let systemRegion: string = i18n.System.getSystemRegion(); // 获取系统当前地区设置
    let option: i18n.PhoneNumberFormatOptions = {
      type: 'E164'
    }
    let numberFormat: i18n.PhoneNumberFormat = new i18n.PhoneNumberFormat(systemRegion, option);
    if (!numberFormat.isValidNumber(phoneNumber)) {
      LogUtils.w(TAG, `getFormatNumber i18n format result is empty`);
      return null;
    }
    const formatResult = numberFormat.format(phoneNumber);
    if (!formatResult || formatResult.length === 0) {
      LogUtils.w(TAG, `getFormatNumber format result is empty`);
      return null;
    }
    return formatResult;
  } catch (error) {
    LogUtils.e(TAG, `getFormatNumber error code: ${error?.code}, message: ${error?.message}.`);
    return null;
  }
}