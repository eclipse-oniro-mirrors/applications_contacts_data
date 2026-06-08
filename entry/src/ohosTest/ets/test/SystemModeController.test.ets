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
import { describe, it, expect } from '@ohos/hypium'
import { SystemModeController } from '../../../main/ets/common/utils/SystemModeController';

export default function SystemModeControllerTest() {
  describe('SystemModeControllerTest', () => {
    it('SystemModeControllerTest_getIsEnableContactCloudSync', 0, () => {
      SystemModeController.getInstance().getIsEnableContactCloudSync();
      expect(1).assertEqual(1);
    })
  })
}