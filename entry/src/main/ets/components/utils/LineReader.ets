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

import fileio from '@ohos.fileio'
import { Utf8ArrayToStr } from './Utf'

/**
 * 一行一行读出文本文件
 */
export class LineReader {
  private MAX_SIZE: number = 1024 * 1024; //100KB
  private fileContent: string;

  constructor(private fd: number) {
    let arrayBuffer: ArrayBuffer = new ArrayBuffer(this.MAX_SIZE);
    let num = fileio.readSync(fd, arrayBuffer);
    let view = new Uint8Array(arrayBuffer);
    this.fileContent = Utf8ArrayToStr(view);
  }

  printFileContent(): string {
    return this.fileContent;
  }
}
