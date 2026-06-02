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
import UIAbility from '@ohos.app.ability.UIAbility';
import AbilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import hilog from '@ohos.hilog';
import { Hypium } from '@ohos/hypium';
import testsuite from '../test/List.test';
import window from '@ohos.window';
import type Want from '@ohos.app.ability.Want';
import AbilityConstant from '@ohos.app.ability.AbilityConstant';

const TAG: string = 'TestAbility';
const DOMAIN: number = 0x0000;

export default class TestAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'TestAbility onCreate');

    // 获取 AbilityDelegator 和 Arguments
    const abilityDelegator: AbilityDelegatorRegistry.AbilityDelegator = AbilityDelegatorRegistry.getAbilityDelegator();
    const abilityDelegatorArguments: AbilityDelegatorRegistry.AbilityDelegatorArgs = AbilityDelegatorRegistry.getArguments();

    hilog.info(DOMAIN, TAG, '%{public}s', 'start run test suite');

    // 执行测试套件
    if (abilityDelegator && abilityDelegatorArguments) {
      try {
        // 假设 Hypium 需要根据 delegator 或 arguments 进行初始化或运行
        // 具体调用方式取决于 Hypium 框架的具体 API 定义
        Hypium.start(abilityDelegator, testsuite);
      } catch (error) {
        hilog.error(DOMAIN, TAG, 'Failed to start test suite: %{public}s', String(error));
      }
    } else {
      hilog.warn(DOMAIN, TAG, 'AbilityDelegator or Arguments is null');
    }
  }

  onDestroy(): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'TestAbility onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'TestAbility onWindowStageCreate');
    
    // 如果是UI测试，可能需要加载特定的测试页面；如果是纯逻辑测试，可能无需加载UI
    // windowStage.loadContent('pages/TestPage').then(() => {
    //   hilog.info(DOMAIN, TAG, 'Load content success');
    // }).catch((err) => {
    //   hilog.error(DOMAIN, TAG, 'Load content failed: %{public}s', JSON.stringify(err));
    // });
  }

  onWindowStageDestroy(): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'TestAbility onWindowStageDestroy');
  }

  onForeground(): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'TestAbility onForeground');
  }

  onBackground(): void {
    hilog.info(DOMAIN, TAG, '%{public}s', 'TestAbility onBackground');
  }
}