/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import { BusinessError, systemParameterEnhance } from '@kit.BasicServicesKit';
import LogUtils from './LogUtils';

/**
 * 系统模式
 */
enum ModeType {
  /** 默认模式 */
  DEFAULT_MODE = 'default',
  /** 蓬莱模式 */
  PENG_LAI_MODE = 'penglai'
}

const TAG = 'SystemModeController';

export class SystemModeController {
  private static sInstance: SystemModeController | null;
  private static readonly BOOT_MODE_KEY = 'ohos.boot.minisys.mode';
  private static readonly SUPPORT_PENG_LAI_KEY = 'const.penglai.feature_enable';
  private static readonly SUPPORT_PENG_LAI_TRUE = 'true';
  private static readonly SUPPORT_PENG_LAI_FALSE = 'false';
  private currentMode: ModeType = ModeType.DEFAULT_MODE;
  /** 是否启用联系人云同步能力 */
  private isEnableContactCloudSync: boolean = true;

  constructor() {
    this.currentMode = SystemModeController.getModeFromSystemParameter();
    if (this.currentMode === ModeType.PENG_LAI_MODE && SystemModeController.getIsSupportPengLai()) {
      LogUtils.i(TAG, 'handle penglai mode');
      this.setFeaturesEnableInPengLaiMode();
    }
  }

  public static getInstance() {
    if (!SystemModeController.sInstance) {
      SystemModeController.sInstance = new SystemModeController();
    }
    return SystemModeController.sInstance;
  }

  /**
   * 获取当前设备是否支持蓬莱模式
   * @returns boolean
   */
  private static getIsSupportPengLai = (): boolean => {
    let isSupportPengLai: boolean = false;
    try { // 从配置项文件中读取
      let value: string = systemParameterEnhance.getSync(this.SUPPORT_PENG_LAI_KEY, this.SUPPORT_PENG_LAI_FALSE);
      isSupportPengLai = (value === this.SUPPORT_PENG_LAI_TRUE);
    } catch (e) {
      LogUtils.e(TAG, `getIsSupportPengLai: get system parameter region failed, error: ${e?.code} ${e?.message}`);
    }
    LogUtils.i(TAG, `getIsSupportPengLai: isSupportPengLai ${isSupportPengLai}`);
    return isSupportPengLai;
  }
  /**
   * 蓬莱模式下，设置相关特性的是否启用
   */
  private setFeaturesEnableInPengLaiMode = () => {
    // 联系人云同步能力
    this.isEnableContactCloudSync = false;
  }
  /** 获取是否启用联系人云同步能力 */
  public getIsEnableContactCloudSync = () => {
    return this.isEnableContactCloudSync;
  }
  /** 从系统配置中读取当前系统模式 */
  private static getModeFromSystemParameter = () => {
    let modeString: string = ModeType.DEFAULT_MODE.toString();
    try {
      modeString = systemParameterEnhance.getSync(this.BOOT_MODE_KEY, ModeType.DEFAULT_MODE.toString());
    } catch (e) {
      LogUtils.e(TAG, `getCurrentMode: get system parameter error: ${e?.code} ${e?.message}`);
    }
    LogUtils.i(TAG, `getCurrentMode: currentMode: ${modeString}`);
    let mode: ModeType;
    switch (modeString) {
      case ModeType.PENG_LAI_MODE.toString():
        mode = ModeType.PENG_LAI_MODE;
        break;
      default: //Modes without special logic are identified as default modes.
        mode = ModeType.DEFAULT_MODE;
        break;
    }
    return mode;
  }
}