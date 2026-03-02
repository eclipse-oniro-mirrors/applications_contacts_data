/**
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

import LogUtils from '../common/utils/LogUtils';
import Want from '@ohos.app.ability.Want';
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import { abilityAccessCtrl } from '@kit.AbilityKit';
import { common } from '@kit.AbilityKit';
import data_preferences from '@ohos.data.preferences';
import PreferenceUtil from '../common/utils/PreferenceUtil';
import { BusinessError } from '@ohos.base';

const TAG = 'PermissionExtAbility';

export default class PermissionExtAbility extends UIExtensionAbility {
  onCreate() {
    LogUtils.i(TAG, 'onCreate');
  }

  onSessionCreate(want: Want, session: UIExtensionContentSession) {
    LogUtils.i(TAG, 'onSessionCreate');
    let storageParamObj: Record<string, UIExtensionContentSession> = {
      'uiExtensionSession': session
    };
    let storage: LocalStorage = new LocalStorage(storageParamObj);
    session.loadContent('pages/PermissionDialog', storage);

    let context: Context = getContext(this) as common.UIAbilityContext;
    PreferenceUtil.getInstance().init(context!);
    PreferenceUtil.getInstance()
      .getFromPreferences('calendar_permission_dialog', 0)
      .then((value: data_preferences.ValueType) => {
        let flag: number = value as number;
        if (flag == 0) { // 首次权限检查
          // 记录为首次权限弹窗flag
          PreferenceUtil.getInstance().saveToPreferences('calendar_permission_dialog', 1);
          this.firstPermissionCheck(context, session);
        } else { // 非首次权限检查
          this.permissionCheck(context, session);
        }
      });
  }

  firstPermissionCheck(context: Context, session: UIExtensionContentSession) {
    abilityAccessCtrl.createAtManager().requestPermissionsFromUser(
      context,
      ['ohos.permission.READ_CALENDAR', 'ohos.permission.WRITE_CALENDAR']
    ).then(async (res) => {
      if (res) {
        LogUtils.i(TAG, 'requestPermissionsFromUser authResults: ' + res.authResults);
        if (res.authResults.length < 1) {
          this.sendResult(session, -1);
          return;
        }
        if (res.authResults[0] === 0) {
          this.sendResult(session, 1);
          return;
        }
      } else {
        LogUtils.w(TAG, 'requestPermissionsFromUser error');
      }
      this.sendResult(session, 0);
    }).catch((err: BusinessError) => {
      LogUtils.e(TAG, 'requestPermissionsFromUser error: ' + err?.message + ', stack: ' + err?.stack);
      this.sendResult(session, -1);
    });
  }

  permissionCheck(context: Context, session: UIExtensionContentSession) {
    abilityAccessCtrl.createAtManager().requestPermissionsFromUser(
      context,
      ['ohos.permission.READ_CALENDAR', 'ohos.permission.WRITE_CALENDAR']
    ).then(async (res) => {
      if (res) {
        LogUtils.i(TAG, 'requestPermissionsFromUser authResults: ' + res.authResults);
        // 未授权时弹窗从settting中进行权限设置
        if (res.authResults.length < 2) { // 2个权限结果
          this.sendResult(session, -1);
          return;
        }
        if (res.authResults[0] === -1 || res.authResults[1] === -1) {
          abilityAccessCtrl.createAtManager()
            .requestPermissionOnSetting(context, ['ohos.permission.READ_CALENDAR', 'ohos.permission.WRITE_CALENDAR'])
            .then((data: Array<abilityAccessCtrl.GrantStatus>) => {
              LogUtils.i(TAG, 'requestPermissionOnSetting res size: ' + data?.length);
              if (data.length < 1) {
                this.sendResult(session, -1);
                return;
              }
              if (data[0] === 0) {
                this.sendResult(session, 1);
                return;
              }
              this.sendResult(session, 0);
              return;
            })
            .catch((err: BusinessError) => {
              LogUtils.e(TAG, 'requestPermissionOnSetting res error' + err?.message + ', stack: ' + err?.stack);
              this.sendResult(session, -1);
              return;
            });
        } else {
          this.sendResult(session, 0);
        }
      } else {
        LogUtils.w(TAG, 'requestPermissionsFromUser res error');
        this.sendResult(session, -1);
      }
    }).catch((err: BusinessError) => {
      LogUtils.e(TAG, 'requestPermissionsFromUser res error: ' + err?.message + ', stack: ' + err?.stack);
      this.sendResult(session, -1);
    });
  }

  sendResult(session: UIExtensionContentSession, res: number) {
    session.sendData({
      'result': res
    });
  }


  onSessionDestroy(session: UIExtensionContentSession) {
    LogUtils.i(TAG, 'onSessionDestroy');
  }

  onForeground() {
    LogUtils.i(TAG, 'onForeground');
  }

  onBackground() {
    LogUtils.i(TAG, 'onBackground');
  }

  onDestroy() {
    LogUtils.i(TAG, 'onDestroy');
  }
}