/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import type { Base64Interface } from './Base64Interface';
import { LineReaderInterface } from './LineReaderInterface';
import { HOSBase64 } from './HOSBase64';
import { HOSLineReader } from './HOSLineReader'

enum PLATFORM {
  node,
  openHarmonyOs
}

export class PlatformFactory {
  private static platform: PLATFORM = PLATFORM.openHarmonyOs;

  static getBase64Impl(): Base64Interface {
    if (PlatformFactory.platform == PLATFORM.openHarmonyOs) {
      return new HOSBase64();
    }
    return new HOSBase64();
  }

  static getLineReaderImpl(pathOrFd: string | number, mode: string | null): LineReaderInterface {
    if (PlatformFactory.platform == PLATFORM.openHarmonyOs) {
      return new HOSLineReader(pathOrFd, mode);
    }
    return new HOSLineReader(pathOrFd, mode);
  }
}
