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
import data_preferences from '@ohos.data.preferences';
import { AbstractPreferenceUtil } from './PreferenceUtilAbstract';

const TAG = 'PreferenceUtilXml';

const NAME: string = 'CONTACT_PREFERENCE';

/**
 * xml 类型的 preference
 * xml文件存储，多进程并发访问有问题，不要用；用PreferenceUtil工具类
 */
export default class PreferenceUtilXml extends AbstractPreferenceUtil {
  private static sInstance: PreferenceUtilXml;

  public static getInstance(): PreferenceUtilXml {
    if (!PreferenceUtilXml.sInstance) {
      PreferenceUtilXml.sInstance = new PreferenceUtilXml();
      PreferenceUtilXml.sInstance.setTag(TAG).setName(NAME).setStorageType(data_preferences.StorageType.XML);
    }
    return PreferenceUtilXml.sInstance;
  }

}