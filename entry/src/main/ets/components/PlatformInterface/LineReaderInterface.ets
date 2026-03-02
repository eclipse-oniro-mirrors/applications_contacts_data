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

/**
 * 不同平台读取文件的方式不同，使用这个通用的文件读取器
 */
export abstract class LineReaderInterface {
  protected fd: number;

  constructor(pathOrFd: string | number) {
    if (typeof pathOrFd === 'string') {
      this.fd = this.openFile(pathOrFd);
    } else {
      this.fd = pathOrFd;
    }
  }

  abstract openFile(path: string): number;

  /**
   * @returns result string or null if eof
   */
  abstract nextLine(): string | null;

  abstract hasLine(): boolean;

  abstract peekLine(): string | null;
}