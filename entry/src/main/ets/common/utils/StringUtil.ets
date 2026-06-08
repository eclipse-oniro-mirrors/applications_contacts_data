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
import buffer from '@ohos.buffer';
import { ContactInfo } from '../data/ContactInfo';
import LogUtils from './LogUtils';
import DatabaseUtils from './DatabaseUtils';

const SHORT_UNIQUE_KEY_LENGTH = 8;
const SHORT_UUID_LENGTH = 13;
const LONG_UUID_LENGTH = 20;
const spaceRex = new RegExp('\\s+', 'g');
const dashReg = new RegExp('[\-\]', 'g');

/**
 * Handle string util
 */
export class StringUtil {
  static paddingLeft(str: string, padCh: string, len: number): string {
    while (str.length < len) {
      str = padCh + str;
    }

    return str;
  }

  /**
   * If string is empty
   *
   * @param str which to handle
   * @returns if string is empty
   */
  static isEmpty(str: string | undefined | null): boolean {
    return str === undefined || str === null || str.length === 0;
  }

  /**
   * If string is equals ignore case
   *
   * @param str which to handle
   * @returns string removed space
   */
  static equalsIgnoreCase(str: string, str1: string): boolean {
    if (StringUtil.isEmpty(str) && StringUtil.isEmpty(str1)) {
      return true;
    }
    if (StringUtil.isEmpty(str) || StringUtil.isEmpty(str1)) {
      return false;
    }
    return str1.toUpperCase() === str.toUpperCase();
  }

  /**
   * Get data type from mime type
   *
   * @param str which to handle
   * @returns removed space str
   */
  static getType(mimeType: string | undefined): string {
    if (mimeType === undefined || StringUtil.isEmpty(mimeType)) {
      return '';
    }
    if (mimeType.indexOf('/') === -1) {
      return '';
    }
    let type = mimeType.split('/')[1];
    if (StringUtil.isEmpty(type)) {
      return '';
    }
    if (type.indexOf('_') === -1) {
      return type;
    }
    return type.split('_')[0];
  }

  /**
   * Remove str space
   *
   * @param str which to handle
   * @returns string removed space
   */
  static removeSpace(str: string): string {
    if (StringUtil.isEmpty(str)) {
      return '';
    }
    str = str.trim();
    while (str.indexOf(' ') != -1) {
      str = str.replace(' ', '');
    }
    return str;
  }

  static removeSpaceNew(str: string): string {
    if (StringUtil.isEmpty(str)) {
      return '';
    }
    return str.replace(spaceRex, '').replace(dashReg, '');
  }

  static maskSensitiveInfo(str: string | undefined | null): string {
    try {
      if (str === undefined || str === null || str.length === 0) {
        return '';
      }

      // 只有一个字符，大概率是姓名只有一个字，此时不打印
      if (str.length <= 1) {
        return `*:${str.length}`;
      }
      // 只有2个字符时，打印首字母跟长度2，与下面统一
      if (str.length <= 2) {
        return `${str.substring(0, 1)}:${str.length}`;
      }
      // 大概率是姓名、公司，打印首尾字符
      if (str.length <= 7) {
        return `${str.substring(0, 1)}:${str.substring(str.length - 1, str.length)}:${str.length}`;
      }
      // 号码或者座机，打印前三位，后四位
      if (str.length <= 11) {
        return `${str.substring(0, 3)}:${str.substring(str.length - 4, str.length)}:${str.length}`;
      }
      // 对于带有+86这样的手机，打印前六位
      if (str.length <= 14) {
        return `${str.substring(0, 6)}:${str.substring(str.length - 4, str.length)}:${str.length}`;
      }
      return `${str.substring(0, 3)}:${str.substring(str.length - 4, str.length)}:${str.length}`;
    } catch (e) {
      LogUtils.e('StringUtil', 'maskSensitiveInfo error:' + e.message);
      return `***`;
    }
  }

  static fromBase64(str: string): string {
    try {
      if (StringUtil.isEmpty(str)) {
        return '';
      }
      return buffer.from(str, 'base64').toString('utf8');
    } catch (e) {
      LogUtils.e('StringUtil', 'fromBase64 error:' + e.message);
      return str;
    }
  }

  static getVcardLogStr(vcard: string): string {
    if (!vcard) {
      return '';
    }
    const arr = vcard.split(/[\r?\n]/);
    const resultArr = arr.filter((line: string) => {
      if (!line?.trim()) {
        return false;
      }
      if (line.startsWith('PHOTO;ENCODING=BASE64')) {
        return false;
      }
      return true;
    });
    return StringUtil.maskSensitiveInfo(resultArr.join('\r\n'));
  }

  static getContactInfoLogStr(contactInfo: ContactInfo): string {
    try {
      const str = JSON.stringify(contactInfo, (key: string, value: Object) => {
        // 如果属性名是"photo"，则过滤掉该属性
        if (key === 'photo') {
          return undefined; // 从序列化结果中排除这个属性
        }
        return value; // 保留这个属性
      });
      return StringUtil.maskSensitiveInfo(str);
    } catch (e) {
      LogUtils.e('StringUtil', 'getContactInfoLogStr error:' + e.message);
      return '';
    }
  }

  static async printLogStr(tag: string, str: string) {
    // 实测2400多长度日志可能就被截断了，打印重复联系人列表，删除列表等场景都需要保证完整，不被截断丢弃
    let limit = 2200;
    let start = 0;
    let end = 2200;
    if (str.length <= limit) {
      LogUtils.w(tag, str);
      return;
    }
    while (start < str.length) {
      LogUtils.w(tag, str.slice(start, end));
      start += limit;
      end += limit;
      await DatabaseUtils.delay(120);
    }
  }

  static getShortUniqueKey(uniqueKey: string): string {
    if (!uniqueKey || !uniqueKey.length) {
      return '';
    }
    return uniqueKey.slice(0, SHORT_UNIQUE_KEY_LENGTH);
  }

  static getShortUuid(uuid: string | null, long = false): string {
    if (!uuid || !uuid.length) {
      return '';
    }
    // 双的 uuid 前面都一样，所以打印后面子串
    return uuid.slice(long ? -LONG_UUID_LENGTH : -SHORT_UUID_LENGTH);
  }
}