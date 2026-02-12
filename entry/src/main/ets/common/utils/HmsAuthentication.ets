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
import { authentication } from '@kit.AccountKit';
import type common from '@ohos.app.ability.common';
import PreferenceUtil from './PreferenceUtil';
import LogUtils from './LogUtils';

const TAG = 'ContactRelationAbility';
const UID_REG = /^["|'](.*)["|']$/g;

export enum SCOPES {
  SCOPES_PROFILE = 'profile', /* 创建授权请求对应的scope_profile */
  SCOPES_MOBILE_NUMBER = 'phone' /* 创建授权请求对应的scope_mobile.number */
}
export enum PERMISSIONS {
  PERMISSIONS_AUTH_CODE = 'serviceauthcode', /* 创建授权请求对应的permission_AUTH_CODE */
  PERMISSIONS_ID_TOKEN = 'idtoken', /* 创建授权请求对应的permission_idtoken */
  PERMISSIONS_UID = 'uid' /* 创建授权请求对应的permission_uid */
}
export enum KEYWORDS {
  ACCOUNT_ID = 'Relation_Link_Keyword_ACCOUNT_ID',
  DEVICE_ID = 'Relation_Link_Keyword_DEVICE_ID',
  AUTH_CODE = 'Relation_Link_Keyword_AUTH_CODE'
}

export class HmsAuthManager {

  public hmsInfo: authentication.AuthorizationWithHuaweiIDCredential | null = null;
  public uid: string = '';
  public authCode: string = '';
  private static mInstance: HmsAuthManager;
  private context:common.Context | null = null;

  public static getInstance(): HmsAuthManager {
    if (HmsAuthManager.mInstance == undefined) {
      HmsAuthManager.mInstance = new HmsAuthManager();
    }
    return HmsAuthManager.mInstance;
  }

  public setContext(context:common.Context) {
    this.context = context;
  }

  private constructor() {
  }

  public generateRandomString(bits: number): string {
    const characters = '0123456789abcdefghijklmnopqrstuvwxyz';
    let result = '';
    for (let i = 0; i < bits; i++) {
      const randomIndex = Math.floor(Math.random() * characters.length);
      result += characters.charAt(randomIndex);
    }
    return result;
  }

  public async getDeviceId(): Promise<string> {
    let deviceId: string;
    PreferenceUtil.getInstance().init(this.context!);
    let isExist = await PreferenceUtil.getInstance().isExistKeyWord(KEYWORDS.DEVICE_ID);
    if (isExist) {
      deviceId = await PreferenceUtil.getInstance().getFromPreferences(KEYWORDS.DEVICE_ID, '') as string;
    } else {
      deviceId = this.generateRandomString(64).toUpperCase();
      await PreferenceUtil.getInstance().saveToPreferences(KEYWORDS.DEVICE_ID, deviceId);
    }
    return deviceId;
  }

  public async clearAccountIdCache(): Promise<void> {
    PreferenceUtil.getInstance().init(this.context!);
    PreferenceUtil.getInstance().saveToPreferences('myState', 0);
    await PreferenceUtil.getInstance().deleteFromPreferences(KEYWORDS.ACCOUNT_ID);
    await PreferenceUtil.getInstance().deleteFromPreferences('comToken');
    LogUtils.i(TAG, 'login out. clear cache');
  }

  public getSignInOptions(): authentication.AuthorizationWithHuaweiIDRequest {
    // 创建授权请求，并设置参数
    let authRequest = new authentication.HuaweiIDProvider().createAuthorizationWithHuaweiIDRequest();
    // 业务根据需要传对应的scope
    authRequest.scopes = ['phone', 'email'];
    // 业务根据需要传对应permission
    authRequest.permissions = [PERMISSIONS.PERMISSIONS_UID, PERMISSIONS.PERMISSIONS_AUTH_CODE];
    // forceAuthorization参数用来控制是否拉起授权页面
    authRequest.forceAuthorization = false;
    authRequest.state = 'state';
    return authRequest;
  }
}
