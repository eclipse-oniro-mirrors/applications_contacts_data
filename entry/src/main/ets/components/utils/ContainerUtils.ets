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

export namespace ContainerUtils {

  type containerArrayAndSet<T> = T[] | Set<T> | undefined;

  export function contains<T>(str: containerArrayAndSet<T>, content: T): boolean {
    if (str == null) {
      return false;
    }
    if (str instanceof Array) {
      let result = str.find((v, i, array) => {
        return v == content;
      })
      return result ? true : false;
    }
    if (str instanceof Set) {
      return str.has(content);
    }
    return false;
  }

  export function length<T>(str: containerArrayAndSet<T>): number {

    if (str == null) {
      return 0;
    }
    if (str instanceof Array) {
      return str.length;
    }

    if (str instanceof Set) {
      return str.size;
    }

    return 0;
  }
}