/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

/**
 * log tool utils
 */
export class LogUtils {
  d(TAG, msg): void {
    console.debug("[ContactsData:]" + TAG + ":" + msg);
  }

  i(TAG, msg): void {
    console.info("[ContactsData:]" + TAG + ":" + msg);
  }

  w(TAG, msg): void {
    console.warn("[ContactsData:]" + TAG + ":" + msg);
  }

  e(TAG, msg): void {
    console.error("[ContactsData:]" + TAG + ":" + msg);
  }
}

let mLogUtil = new LogUtils();

export default mLogUtil as LogUtils;

