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

import { LineReaderInterface } from './LineReaderInterface';
import { Utf8ArrayToStr } from '../utils/Utf';
import { Log } from '../Log';
import fileio from '@ohos.fileio';

export class HOSLineReader extends LineReaderInterface {
  private mLineIndex = 0;
  private mMaxIndex = 0;
  private mInternalBuffer: string[] = [];
  private mNextLineIsValid: boolean = false;
  private mNextLine: string | null = null;

  openFile(path: string): number {
    return fileio.openSync(path, 0o2);
  }

  constructor(pathOrFd: string | number, mode: string | null) {
    super(pathOrFd);
    let fstat = fileio.fstatSync(this.fd);
    let buffer = new ArrayBuffer(fstat.size);
    fileio.readSync(this.fd, buffer);
    let stringBuffer = new Uint8Array(buffer);
    let resultString = Utf8ArrayToStr(stringBuffer);
    if (mode == 'vcard') {
      let arr: string[] = [];
      let str: string = '';
      arr = resultString.split('\r\n');
      let num = arr.length;
      for (let line of arr) {
        // The last line does not have a "=" character.
        let lastChar = line.charAt(line.length - 1);
        if (--num != 0 && lastChar == '=') {
        // Remove Clone Frame Wrap Added Characters"="
          line = line.slice(0, -1);
        }
        str = str + line;
      }
      this.mInternalBuffer = str.split('\n');
    } else {
        this.mInternalBuffer = resultString.split('\r\n');
        if (this.mInternalBuffer.length <= 1) {
          this.mInternalBuffer = resultString.split('\r');
        }
        if (this.mInternalBuffer.length <= 1) {
          this.mInternalBuffer = resultString.split('\n');
        }
      }
      this.mMaxIndex = this.mInternalBuffer.length;
  }

  private readLineInternal(): string | null {
    if (this.hasLine()) {
      return this.mInternalBuffer[this.mLineIndex++];
    }
    return null;
  }

  nextLine(): string | null {
    if (this.mNextLineIsValid) {
      let ret = this.mNextLine;
      this.mNextLine = null;
      this.mNextLineIsValid = false;
      return ret;
    }
    let line = this.readLineInternal();
    return line;
  }

  hasLine(): boolean {
    return this.mLineIndex < this.mMaxIndex;
  }

  peekLine(): string | null {
    if (!this.mNextLineIsValid) {
      this.mNextLine = this.readLineInternal();
      this.mNextLineIsValid = true;
    }
    return this.mNextLine;
  }
}