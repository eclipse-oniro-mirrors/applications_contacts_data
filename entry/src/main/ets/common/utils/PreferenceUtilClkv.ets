/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import data_preferences from '@ohos.data.preferences';
import { AbstractPreferenceUtil } from './PreferenceUtilAbstract';

const TAG = 'PreferenceUtilClkv';

const NAME: string = 'CONTACT_PREFERENCE_CLKV';

/**
 * clkv 类型的 preference
 * 用PreferenceUtil工具类；PreferenceUtil有判断如果支持clkv，会创建这种类型的工具类
 */
export default class PreferenceUtilClKv extends AbstractPreferenceUtil {

  private static sInstance: PreferenceUtilClKv;

  public static getInstance(): PreferenceUtilClKv {
    if (!PreferenceUtilClKv.sInstance) {
      PreferenceUtilClKv.sInstance = new PreferenceUtilClKv();
      PreferenceUtilClKv.sInstance.setTag(TAG).setName(NAME).setStorageType(data_preferences.StorageType.GSKV);
    }
    return PreferenceUtilClKv.sInstance;
  }

}