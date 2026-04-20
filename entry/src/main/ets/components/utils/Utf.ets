/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import util from '@ohos.util';

export function Utf8ArrayToStr(array: Uint8Array, encoding: string = 'utf8'): string {
  return innerUtf8ArrayToStr(array)
}

let innerUtf8ArrayToStr = (() => {
  let charCache : string[] = new Array(512); // Preallocate the cache for the common single byte chars
  let charFromCodePt = String.fromCodePoint || String.fromCharCode;
  let result: string[] = [];
  return (array:Uint8Array) => {
    let codePt : number = 0;
    let byte1 : number = 0;
    let buffLen = array.length;
    result.length = 0;
    for (let i = 0; i < buffLen;) {
      byte1 = array[i++];

      if (byte1 <= 0x7F) {
        codePt = byte1;
      } else if (byte1 <= 0xDF) {
        codePt = ((byte1 & 0x1F) << 6) | (array[i++] & 0x3F);
      } else if (byte1 <= 0xEF) {
        codePt = ((byte1 & 0x0F) << 12) | ((array[i++] & 0x3F) << 6) | (array[i++] & 0x3F);
      } else if (String.fromCodePoint(byte1)) {
        codePt = ((byte1 & 0x07) << 18) | ((array[i++] & 0x3F) << 12) |
          ((array[i++] & 0x3F) << 6) | (array[i++] & 0x3F);
      } else {
        codePt = 63; // Cannot convert four byte code points, so use '?' instead
        i += 3;
      }

      result.push(charCache[codePt] || (charCache[codePt] = charFromCodePt(codePt)));
    }
    let r = result.join('');
    return r;
  };
})();

export function str2Uint8Array(content: string) {
  for (let i = 0; i < content.length; i++) {
    content.charCodeAt(i)
  }
}
