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
import hilog from '@ohos.hilog';
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from '@ohos/hypium';
import PreferenceUtil from '../../../main/ets/common/utils/PreferenceUtil';
import TestApplication from '../application/TestApplication';

export default function PreferenceTest() {
  describe('PreferenceTest', () => {
    // Defines a test suite. Two parameters are supported: test suite name and test suite function.
    // m
    beforeAll(() => {
      // Presets an action, which is performed only once before all test cases of the test suite start.
      // This API supports only one parameter: preset action function.
      // m
      PreferenceUtil.getInstance().init(context);
    })
    beforeEach(() => {
      // Presets an action, which is performed before each unit test case starts.
      // The number of execution times is the same as the number of test cases defined by **it**.
      // This API supports only one parameter: preset action function.
    })
    afterEach(() => {
      // Presets a clear action, which is performed after each unit test case ends.
      // The number of execution times is the same as the number of test cases defined by **it**.
      // This API supports only one parameter: clear action function
    })
    afterAll(() => {
      // Presets a clear action, which is performed after all test cases of the test suite end.
      // This API supports only one parameter: clear action function.
    })
    it('savePreference', 0, async () => {
      // Defines a test case.
      // This API supports three parameters: test case name, filter parameter and test case function.
      hilog.info(0x0000, 'testTag', '%{public}s', 'it begin.');
      let testArray: string[] = ['1', '2', '3', '4', '5'];
      await PreferenceUtil.getInstance().saveToPreferences('TEST_ARRAY', testArray);
    })
    
    it('getPreference', 0, async () => {
      // Defines a test case.
      // This API supports three parameters: test case name, filter parameter and test case function.
      hilog.info(0x0000, 'testTag', '%{public}s', 'getPreference begin');
      let result: string[] = await PreferenceUtil.getInstance().getFromPreferences('TEST_ARRAY', []) as string[];
      expect('1,2,3,4,5').assertEqual(result.join(','));
    })
  })
}
