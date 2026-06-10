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

import util from '@ohos.util';
import { Log } from '../Log'

export class StringBuilder {
  private mBuffer: string[] = [];
  private mIndex = 0;
  private mCharsetStr: string = 'utf8'

  constructor() {
  }

  public append(content: string): StringBuilder {
    this.mIndex += this.mBuffer.push(content);
    return this;
  }

  public getUint8Bytes(): Uint8Array | null {
    let str = this.toString();
    let back = new Uint8Array(str.length * 4);
    let len: number = str.length * 4;
    let index = this.stringToUTF8Array(str, back, 0, len);
    return back.slice(0, index);
  }

  private getUint8BytesInternalOne(): Uint8Array | null {
    let str = this.toString();
    let back = new Uint8Array(str.length * 4);
    let byteSize = 0;
    for (let i = 0; i < str.length; i++) {
      let code = str.charCodeAt(i);
      if (0x00 <= code && code <= 0x7f) {
        back[byteSize++] = code;
      } else if (0x80 <= code && code <= 0x7ff) {
        back[byteSize++] = (192 | (31 & (code >> 6)));
        back[byteSize++] = (128 | (63 & code));
      } else if ((0x800 <= code && code <= 0xd7ff) ||
        (0xe000 <= code && code <= 0xffff)) {
        back[byteSize++] = (224 | (15 & (code >> 12)));
        back[byteSize++] = (128 | (63 & (code >> 6)));
        back[byteSize++] = (128 | (63 & code));
      }
    }
    for (let i = 0; i < back.length; i++) {
      back[i] &= 0xff;
    }
    return back.slice(0, byteSize);
  }

  private stringToUTF8Array(str: string, outU8Array: Uint8Array, outIdx: number, maxBytesToWrite: number) {
    if (!(maxBytesToWrite > 0)) {
      return 0;
    }
    let startIdx = outIdx;
    let endIdx = outIdx + maxBytesToWrite - 1;
    for (let i = 0; i < str.length; ++i) {
      let u = str.charCodeAt(i);
      if (u >= 55296 && u <= 57343) {
        let u1 = str.charCodeAt(++i);
        u = 65536 + ((u & 1023) << 10) | u1 & 1023
      }
      if (u <= 127) {
        if (outIdx >= endIdx) {
          break;
        }
        outU8Array[outIdx++] = u
      } else if (u <= 2047) {
        if (outIdx + 1 >= endIdx) {
          break;
        }
        outU8Array[outIdx++] = 192 | u >> 6;
        outU8Array[outIdx++] = 128 | u & 63
      } else if (u <= 65535) {
        if (outIdx + 2 >= endIdx) {
          break;
        }
        outU8Array[outIdx++] = 224 | u >> 12;
        outU8Array[outIdx++] = 128 | u >> 6 & 63;
        outU8Array[outIdx++] = 128 | u & 63
      } else {
        if (outIdx + 3 >= endIdx) {
          break;
        }
        outU8Array[outIdx++] = 240 | u >> 18;
        outU8Array[outIdx++] = 128 | u >> 12 & 63;
        outU8Array[outIdx++] = 128 | u >> 6 & 63;
        outU8Array[outIdx++] = 128 | u & 63
      }
    }
    outU8Array[outIdx] = 0;
    return outIdx - startIdx;
  }

  public clear(): void {
    this.mIndex = 0;
  }

  public size(): number {
    return this.mIndex;
  }

  public get length(): number {
    return this.mIndex;
  }

  public toString(): string {
    return this.mBuffer.slice(0, this.mIndex).join('');
  }
}