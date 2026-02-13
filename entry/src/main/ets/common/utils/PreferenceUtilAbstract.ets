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
import LogUtils from './LogUtils';
import contextConstant from '@ohos.app.ability.contextConstant';
import { StringUtil } from './StringUtil';

/**
 * PreferenceUtil 抽象
 */
export abstract class AbstractPreferenceUtil {

  /**
   * The hyphen of preference key
   */
  public static readonly HYPHEN = '_';

  protected mPreferences: data_preferences.Preferences | undefined = undefined;
  protected mContext: Context | undefined = undefined;


  private TAG = '';

  private NAME = '';

  private storageType: data_preferences.StorageType = data_preferences.StorageType.XML;

  public init(context : Context): AbstractPreferenceUtil {
    this.mContext = context;
    return this;
  }

  // tag  日志打印信息
  public setTag(tag: string): AbstractPreferenceUtil {
    this.TAG = tag;
    return this;
  }

  // 名字
  public setName(name: string): AbstractPreferenceUtil {
    this.NAME = name;
    return this;
  }

  // preference类型
  public setStorageType(storageType: data_preferences.StorageType): AbstractPreferenceUtil {
    this.storageType = storageType;
    return this;
  }

  // removePreferencesFromCache为关库接口
  // removePreferencesFromCache和removePreferencesFromCacheSync会进行句柄关闭，调用之后preference对象不可用
  // 需要重新getPreferences或getPreferencesSync获取新的对象
  // 1、避免重复申请/释放preference对象，即避免重复的getPreferences/removePreferencesFromCache调用，推荐持有preference单例句柄。
  // 2、不建议频繁调用removePreferencesFromCache和removePreferencesFromCacheSync，重新getPreferences会有性能开销
  public async removePreferencesFromCache(): Promise<void> {
    if (this.mContext === undefined || this.mContext === null) {
      return;
    }
    const oldVal = this.mContext.area;
    this.mContext.area = contextConstant.AreaMode.EL2;
    LogUtils.w(this.TAG, `removePreferencesFromCache succeed,preferencesDir:${this.mContext.preferencesDir}`);
    data_preferences.removePreferencesFromCacheSync(this.mContext, this.NAME);
    this.mContext.area = oldVal;
    this.mPreferences = undefined;
  }

  /**
   * getFromPreferences
   *
   * @return the value get from Preferences
   */
  public async getFromPreferences(key: string,
    defValue: data_preferences.ValueType): Promise<data_preferences.ValueType> {
    let preferences: data_preferences.Preferences | undefined = await this.getPreferences();
    if (preferences === undefined) {
      LogUtils.e(this.TAG, `preferences is null,get default value ${defValue} for key ${key}`);
      return defValue;
    }
    const value: data_preferences.ValueType = await preferences.get(key, defValue);
    LogUtils.w(this.TAG, `preferences get value ${value} for key ${key}`);
    return value;
  }

  /**
   * 获取所有信息
   * @returns
   */
  public async getAllSync(): Promise<Object | undefined> {
    let preferences: data_preferences.Preferences | undefined = await this.getPreferences();
    if (preferences === undefined) {
      LogUtils.e(this.TAG, `getAllSync preferences is null,getAllSync`);
      return undefined;
    }
    const value: data_preferences.ValueType = await preferences.getAllSync();
    return value;
  }

  /**
   * saveToPreferences
   *
   * @param key save to Preferences
   * @param value save to Preferences
   */
  public async saveToPreferences(key: string, value: data_preferences.ValueType) {
    let preferences: data_preferences.Preferences | undefined = await this.getPreferences();
    if (preferences === undefined || StringUtil.isEmpty(key)) {
      LogUtils.e(this.TAG, `saveToPreferences fail, preferences is null:${preferences == undefined},key:${
      key},value:${value}`);
      return;
    }
    await preferences.put(key, value);
    await preferences.flush();
    LogUtils.w(this.TAG, `saveToPreferences succeed,key:${key},value:${value}`);
  }

  /**
   * Compose preference key
   *
   * @param prefix the Preferences key of prefix
   * @param key the Preferences key
   */
  public key(prefix: string, key: string): string {
    return prefix + AbstractPreferenceUtil.HYPHEN + key;
  }

  // getPreferences为开库接口
  protected async getPreferences(): Promise<data_preferences.Preferences | undefined> {
    if (this.mPreferences) {
      return this.mPreferences;
    }
    if (this.mContext === undefined) {
      LogUtils.w(this.TAG, 'getPreferences context undefined');
      return undefined;
    }
    const oldVal = this.mContext.area;
    for (let retryTimes = 0; retryTimes < 3; retryTimes++) {
      try {
        const options : data_preferences.Options = {
          name: this.NAME,
          storageType: this.storageType
        };
        // 创建preference对象，创建到EL2
        this.mContext.area = contextConstant.AreaMode.EL2;
        LogUtils.w(this.TAG, `getPreferences preferencesDir:${this.mContext.preferencesDir}`);
        const sp = data_preferences.getPreferencesSync(this.mContext, options);
        this.mPreferences = sp;
        this.mContext.area = oldVal;
        return sp;
      } catch (error) {
        LogUtils.w(this.TAG, 'getPreferences err' + '--' + error.message + '--' + error.code);
        this.mContext.area = oldVal;
      }
    }

    return undefined;
  }

  /**
   * Detect if keyword exist
   *
   * @param key the Preferences key
   * @returns Promise
   */
  public async isExistKeyWord(key: string) : Promise<Boolean> {
    let preferences: data_preferences.Preferences | undefined = await this.getPreferences();
    if (preferences === undefined) {
      LogUtils.w(this.TAG, 'isExist get preferences undefined');
      return false;
    }
    return preferences.has(key);
  }

  public async deleteFromPreferences(key: string) : Promise<void> {
    let preferences: data_preferences.Preferences | undefined = await this.getPreferences();
    if (preferences === undefined) {
      LogUtils.w(this.TAG, 'delete get preferences undefined');
      return;
    }
    await preferences.delete(key);
    await preferences.flush();
    await this.removePreferencesFromCache();
  }

  public async clear(): Promise<void> {
    let preferences: data_preferences.Preferences | undefined = await this.getPreferences();
    if (preferences === undefined) {
      LogUtils.w(this.TAG, 'clear get preferences undefined');
      return;
    }
    await preferences.clear();
    await preferences.flush();
    await this.removePreferencesFromCache();
  }

  /**
   * 删除xml sp和xml文件
   * @returns
   */
  public async deletePreferences(): Promise<void> {
    await data_preferences.deletePreferences(this.mContext, this.NAME);
    return;
  }
}