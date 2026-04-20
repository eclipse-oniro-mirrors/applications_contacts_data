/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
  d(tag: string, msg: string): void {
    console.debug('[ContactsData:]' + tag + ':' + msg);
  }

  i(tag: string, msg: string): void {
    console.info('[ContactsData:]' + tag + ':' + msg);
  }

  empty(tag: string, msg: string): void {
    
  }

  w(tag: string, msg: string): void {
    console.warn('[ContactsData:]' + tag + ':' + msg);
  }

  e(tag: string, msg: string): void {
    console.error('[ContactsData:]' + tag + ':' + msg);
  }

  logWithoutPhone(tag: string, msg: string): void {
    if (msg && msg.toString) {
      try {
        console.warn('[ContactsData:]' + tag + ':' +
            msg.toString().replace(/[\r\n]/g, '')
                .replace(/([\d|\s]{7,20})/g,
                    (phone) => new Array(phone.length).fill('*').join('')));
      } catch (e) {
        console.error('[ContactsData:] logWithoutPhone err:' + e.message);
      }
    }
  }
}

let mLogUtil = new LogUtils();

export default mLogUtil as LogUtils;
