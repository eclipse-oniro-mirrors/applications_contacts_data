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
import PreferenceUtilXml from './PreferenceUtilXml';
import PreferenceUtilClkv from './PreferenceUtilClkv';
import data_preferences from '@ohos.data.preferences';
import LogUtils from './LogUtils';

const TAG = 'PreferenceMigrateDataToClkvUtil';
export class PreferenceMigrateDataToClkvUtil {
  public static PREFERENCE_MIGRATE_XML_TO_CLKV_FAIL_KEY = 'PREFERENCE_MIGRATE_XML_TO_CLKV_FAIL_KEY';
  /**
   * 迁移xml的sp数据到clkv
   * @param context
   */
  public static async migrateXmlDataToClkv(context : Context) {
    try {
      let isSupportClkv = data_preferences.isStorageTypeSupported(data_preferences.StorageType.GSKV);
      if (!isSupportClkv) {
        LogUtils.w(TAG, 'not support clkv, return');
        return;
      }

      // 获得sp操作工具对象
      let preferenceUtilClkv = await PreferenceUtilClkv.getInstance();
      preferenceUtilClkv.init(context);
      let preferenceUtilXml = await PreferenceUtilXml.getInstance();
      preferenceUtilXml.init(context);

      LogUtils.w(TAG, 'migrate begin');
      let dataXml = await preferenceUtilXml.getAllSync();
      // xml 无数据，直接返回
      if (!dataXml) {
        LogUtils.w(TAG, 'preferenceXml not has data, return;');
        await preferenceUtilXml.removePreferencesFromCache();
        return;
      }

      // 迁移数据
      let keys = Object.keys(dataXml);
      LogUtils.w(TAG, 'migrate size: ' + keys.length);
      let values: data_preferences.ValueType[] = Object.values(dataXml);
      let migrateFailKeysArr: string[] = [];
      for (let index = 0; index < keys.length; index++) {
        const key = keys[index];
        try {
          await preferenceUtilClkv.saveToPreferences(key, values[index]);
        } catch (e) {
          migrateFailKeysArr.push(key);
          LogUtils.e(TAG, 'migrate ' + key + ' error ' + e.code + '--' + e.message);
        }
      }
      // 关闭xml 类型 preference
      await preferenceUtilXml.removePreferencesFromCache();
      // 都迁移成功了，删除xml文件
      if (migrateFailKeysArr.length === 0) {
        // 删除xml 类型 preference 文件
        await preferenceUtilXml.deletePreferences();
      }

      // 有迁移失败的，在clkv里记录下迁移失败的key
      if (migrateFailKeysArr.length > 0) {
        LogUtils.e(TAG, 'migrate fail key: ' + migrateFailKeysArr);
        try {
          await preferenceUtilClkv.saveToPreferences(
            PreferenceMigrateDataToClkvUtil.PREFERENCE_MIGRATE_XML_TO_CLKV_FAIL_KEY, migrateFailKeysArr);
        } catch (e) {
          LogUtils.e(TAG, 'save migrate fail key error: ' + e.code + '--' + e.message);
        }
      }

      LogUtils.w(TAG, 'migrate end');

    } catch (e) {
      LogUtils.e(TAG, 'migrate error ' + e.code + '--' + e.message);
    }
  }

}