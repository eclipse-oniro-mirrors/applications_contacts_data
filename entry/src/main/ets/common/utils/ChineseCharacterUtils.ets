/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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

import { StringUtil } from '../../common/utils/StringUtil';
import i18n from '@ohos.i18n';
import { ANONYMOUS_SORT, ANONYMOUS_SORT_FIRST_LETTER } from '../../serivce/ContactsDataService';
const TAG = 'ChineseCharacterUtils';

// 双框架无名氏首字母，与单框架不同，双框架三个点为一个字符
export const ANONYMOUS_SORT_FIRST_LETTER_DOUBLE = '…';

const PINYIN_MAP: Map<string, string> = new Map<string, string>([
  ['ā', 'a'],
  ['á', 'a'],
  ['ǎ', 'a'],
  ['à', 'a'],
  ['ō', 'o'],
  ['ó', 'o'],
  ['ǒ', 'o'],
  ['ò', 'o'],
  ['ē', 'e'],
  ['é', 'e'],
  ['ě', 'e'],
  ['è', 'e'],
  ['ī', 'i'],
  ['í', 'i'],
  ['ǐ', 'i'],
  ['ì', 'i'],
  ['ū', 'u'],
  ['ú', 'u'],
  ['ǔ', 'u'],
  ['ù', 'u'],
  ['ǖ', 'v'],
  ['ǘ', 'v'],
  ['ǚ', 'v'],
  ['ǜ', 'v']
]);

const MULTI_PINYIN: Map<string, string> = new Map<string, string>([
  ['\u8983', 'qin'], // qin tan
  ['\u6c88', 'shen'], // SHEN yang
  ['\u66fe', 'zeng'], // ZENG zu fu
  ['\u8d3e', 'jia'], // JIA
  ['\u4fde', 'yu'],
  ['\u513F', 'er'],
  ['\u5475', 'he'],
  ['\u957f', 'chang'],
  ['\u7565', 'lue'],
  ['\u63a0', 'lue'],
  ['\u4e7e', 'qian'], // qian
  ['\u79d8', 'bi'],
  ['\u8584', 'bo'],
  ['\u79cd', 'chong'],
  ['\u891a', 'chu'],
  ['\u555c', 'chuai'],
  ['\u53e5', 'gou'],
  ['\u839e', 'guan'],
  ['\u7094', 'gui'],
  ['\u85c9', 'ji'],
  ['\u5708', 'juan'],
  ['\u89d2', 'jue'],
  ['\u961a', 'kan'],
  ['\u9646', 'lu'],
  ['\u7f2a', 'miao'],
  ['\u4f74', 'nai'],
  ['\u5152', 'ni'],
  ['\u4e5c', 'nie'],
  ['\u533a', 'ou'],
  ['\u6734', 'piao'],
  ['\u7e41', 'po'],
  ['\u4ec7', 'qiu'],
  ['\u5355', 'shan'],
  ['\u76db', 'sheng'],
  ['\u6298', 'she'],
  ['\u5bbf', 'su'],
  ['\u6d17', 'xian'],
  ['\u89e3', 'xie'],
  ['\u5458', 'yun'],
  ['\u7b2e', 'ze'],
  ['\u7fdf', 'zhai'],
  ['\u796d', 'zhai'],
  ['\u963f', 'a'],
  ['\u5b93', 'fu'],
  ['\u90a3', 'na'],
  ['\u5c09', 'yu'],
  ['\u86fe', 'yi'],
  ['\u67e5', 'zha'],
  ['\u5200', 'dao']
]);

const LETTER_UP_A_CODE = 65;
const LETTER_UP_Z_CODE = 90;
const LETTER_LOW_A_CODE = 97;
const LETTER_LOW_Z_CODE = 122;
const MIN_CHINESE_CODE = 19968;
const MAX_CHINESE_CODE = 40869;

const MIN_CHINESE_CODE_16 = 0x4E00;
const MAX_CHINESE_CODE_16 = 0x9FCF;

const RANGE_1_MAX_LATIN_CODE_16 = 0x250;
const RANGE_2_MIN_LATIN_CODE_16 = 0x1e00;
const RANGE_2_MAX_LATIN_CODE_16 = 0x1eff;
const LATIN = 1;
const UNKNOWN = 3;
const ASCII_MAX_NUM = 128;

const SPACE = ' ';
const DEFAULT_SORT_LETTER = '#';

export default class ChineseCharacterUtils {

  private static getTransHanToLatinLocal(): i18n.Transliterator {
    if (ChineseCharacterUtils.transHanToLatinLocal == null) {
      ChineseCharacterUtils.transHanToLatinLocal = i18n.Transliterator.getInstance('Han-Latin/Names');
    }
    return ChineseCharacterUtils.transHanToLatinLocal;
  }

  private static getTransLatinToAsciiLocal(): i18n.Transliterator {
    if (ChineseCharacterUtils.transLatinToAsciiLocal == null) {
      ChineseCharacterUtils.transLatinToAsciiLocal = i18n.Transliterator.getInstance('Latin-Ascii');
    }
    return ChineseCharacterUtils.transLatinToAsciiLocal;
  }

  /**
   * i18n.Transliterator 对象，用完需要置空
   */
  public static destoryTransliterator() {
    ChineseCharacterUtils.transHanToLatinLocal == null;
    ChineseCharacterUtils.transLatinToAsciiLocal == null;
  }

  private static transHanToLatinLocal: i18n.Transliterator | null = null;
  private static transLatinToAsciiLocal: i18n.Transliterator | null = null;

  public static getPhotoFirstName(name: string): string {
    if (StringUtil.isEmpty(name)) {
      return '';
    }
    for (let index = 0; index < name.length; index++) {
      const letter = name[index];
      let code = letter.charCodeAt(0);
      if (code >= MIN_CHINESE_CODE && code <= MAX_CHINESE_CODE) {
        return letter;
      }
    }
    const charCode = name.charCodeAt(0);
    if ((charCode >= LETTER_UP_A_CODE && charCode <= LETTER_UP_Z_CODE) ||
      (charCode >= LETTER_LOW_A_CODE && charCode <= LETTER_LOW_Z_CODE)) {
      return name.charAt(0);
    }
    return '';
  }

  /**
   * 根据sortKey 获取sort信息
   * @param name
   * @returns
   */
  public static getSort(name: string): string {
    // 如果名称为空，说明是无名氏，应该返回99
    if (StringUtil.isEmpty(name)) {
      return ANONYMOUS_SORT;
    }
    const code = name.charCodeAt(0);
    if ((code >= LETTER_UP_A_CODE && code <= LETTER_UP_Z_CODE) ||
      (code >= LETTER_LOW_A_CODE && code <= LETTER_LOW_Z_CODE)) {
      return code.toString();
    }
    return '-1';
  }

  // 根据排序首字母，获取sort信息
  public static getSortBySortFirstLetter(name: string): string {
    if (StringUtil.isEmpty(name)) {
      return '-1';
    }
    // 排序首字母为三个点，返回99
    if (name === ANONYMOUS_SORT_FIRST_LETTER_DOUBLE || name === ANONYMOUS_SORT_FIRST_LETTER) {
      return ANONYMOUS_SORT;
    }
    const code = name.charCodeAt(0);
    if ((code >= LETTER_UP_A_CODE && code <= LETTER_UP_Z_CODE) ||
      (code >= LETTER_LOW_A_CODE && code <= LETTER_LOW_Z_CODE)) {
      return code.toString();
    }
    return '-1';
  }

  public static getSearchInfo(name: string): string {
    if (StringUtil.isEmpty(name)) {
      return '';
    }
    let pinyin = '';
    let initials = '';
    for (let index = 0; index < name.length; index++) {
      const letter = name[index];
      let code = letter.charCodeAt(0);
      if (code < MIN_CHINESE_CODE || code > MAX_CHINESE_CODE) {
        initials += letter;
        pinyin += letter;
        continue;
      }
      let pyLetter: string | undefined = undefined;
      if (index === 0 && MULTI_PINYIN.has(letter)) {
        pyLetter = MULTI_PINYIN.get(letter);
      }
      if (!pyLetter) {
        pyLetter = ChineseCharacterUtils.removePinyinTones(
          ChineseCharacterUtils.getTransLatinToAsciiLocal().transform(
            ChineseCharacterUtils.getTransHanToLatinLocal().transform(letter)));
      }
      initials += pyLetter.charAt(0);
      pinyin += pyLetter;
    }
    return `${name}||${pinyin}||${initials}`;
  }

  public static removePinyinTones(pinyin: string): string {
    if (StringUtil.isEmpty(pinyin)) {
      return '';
    }
    let result = '';
    for (let index = 0; index < pinyin.length; index++) {
      let letter = pinyin[index];
      let temp = PINYIN_MAP.get(letter);
      result += temp ? temp : letter;
    }
    return result;
  }

  private static appendSortKey(sortKey: string, pinYin: string, hanZi: string) {
    if (sortKey.length > 0) {
      sortKey = (sortKey + SPACE);
    }
    sortKey = (sortKey + pinYin.toUpperCase() + SPACE + hanZi);
    return sortKey;
  }

  private static appendSameTypeStr(sortKey: string, sameTypeStr: string) {
    if (sameTypeStr.length > 0) {
      if (sortKey.length > 0) {
        sortKey = (sortKey + SPACE);
      }
      sortKey = (sortKey + sameTypeStr.toUpperCase());
      return sortKey;
    }
    return sortKey;
  }

  public static getSortKey(name: string) {
    let sortKey = '';
    if (StringUtil.isEmpty(name)) {
      return sortKey;
    }

    try {
      const trimName = name.trimStart();
      if (trimName.length === 0) {
        return sortKey;
      }

      // 相同类型的字符串，如 “aaa啊”格式，“aaa”为相同类型字符串，需要转为“aaa a 啊”形式
      let curHandleSameTypeStr = '';
      let curHandleType = LATIN;
      let index: number = -1;
      for (const letter of trimName) {
        // 处理的字符的编码值
        let code = letter.charCodeAt(0);
        index += 1;
        // 中文处理
        if (code >= MIN_CHINESE_CODE_16 && code <= MAX_CHINESE_CODE_16) {
          // 中文处理前的同类型字符串加入到sortkey
          sortKey = ChineseCharacterUtils.appendSameTypeStr(sortKey, curHandleSameTypeStr);
          curHandleSameTypeStr = '';
          let pinYin = '';
          if (index == 0) {
            // 处理姓氏多音字
            pinYin = ChineseCharacterUtils.getPinYinByMulti(letter);
          } else {
            pinYin = ChineseCharacterUtils.getPinYinByHanZi(letter);
          }
          sortKey = ChineseCharacterUtils.appendSortKey(sortKey, pinYin, letter);
          continue;
        }

        // 如果是ASCII，转换后字符和原始相同；
        if (code < ASCII_MAX_NUM) {
          // 不是LATIN类型的加入转换
          if (curHandleType != LATIN) {
            sortKey = ChineseCharacterUtils.appendSameTypeStr(sortKey, curHandleSameTypeStr);
            curHandleSameTypeStr = '';
          }
          // sameTypeStr 当前处理为为LATIN类型字符串，字符加入到字符串
          curHandleType = LATIN;
          curHandleSameTypeStr = (curHandleSameTypeStr + letter);
          continue;
        }
        // 如果是latin，也作为ascii，转换为对应的ascii
        // Latin-Ascii
        if (code < RANGE_1_MAX_LATIN_CODE_16 ||
          (RANGE_2_MIN_LATIN_CODE_16 <= code && code < RANGE_2_MAX_LATIN_CODE_16)) {
          // 不是LATIN类型的加入转换
          if (curHandleType != LATIN) {
            sortKey = ChineseCharacterUtils.appendSameTypeStr(sortKey, curHandleSameTypeStr);
            curHandleSameTypeStr = '';
          }

          // sameTypeStr 为LATIN类型字符串
          // Extended Latin. Transcode these to ASCII equivalents.
          curHandleType = LATIN;
          curHandleSameTypeStr = (curHandleSameTypeStr +
            ChineseCharacterUtils.getTransLatinToAsciiLocal().transform(letter));
          continue;
        }

        // 可能是中文字符，中文字符作为中文处理的，转为 targetstr+“ ”+sourcestr
        // 。，“ ？ 等等
        let asciiStr = ChineseCharacterUtils.getTransLatinToAsciiLocal().transform(letter);
        // 字符串不相同，作为中文处理
        if (letter != asciiStr) {
          sortKey = ChineseCharacterUtils.appendSameTypeStr(sortKey, curHandleSameTypeStr);
          curHandleSameTypeStr = '';
          sortKey = ChineseCharacterUtils.appendSortKey(sortKey, asciiStr, letter);
          continue;
        }

        // 其他类型
        if (curHandleType != UNKNOWN) {
          sortKey = ChineseCharacterUtils.appendSameTypeStr(sortKey, curHandleSameTypeStr);
          curHandleSameTypeStr = '';
        }

        curHandleType = UNKNOWN;
        curHandleSameTypeStr = (curHandleSameTypeStr + letter);
      }
      sortKey = ChineseCharacterUtils.appendSameTypeStr(sortKey, curHandleSameTypeStr);
    } catch (e) {
      console.error(`${TAG} getSortKey err,name:${name},msg:${e.message}`)
    }
    return sortKey;
  }

  private static getPinYinByHanZi(text: string) {
    let pinYin = PINYIN_MAP.get(text);
    if (pinYin) {
      return pinYin;
    }

    return ChineseCharacterUtils.getTransLatinToAsciiLocal().transform(
      ChineseCharacterUtils.getTransHanToLatinLocal().transform(text));
  }

  private static getPinYinByMulti(letter: string): string {
    if (MULTI_PINYIN.has(letter)) {
      const pinyin = MULTI_PINYIN.get(letter);
      if (pinyin !== undefined) {
        return pinyin;
      }
    }
    return ChineseCharacterUtils.getPinYinByHanZi(letter);
}

  public static getSortKeyOld(name: string): string {
    if (StringUtil.isEmpty(name)) {
      return '';
    }
    let currentStr = '';
    let sortKey = '';
    try {
      const trimName = name.trimStart();
      if (trimName.length === 0) {
        return '';
      }

      for (let index = 0; index < trimName.length; index++) {
        const letter = trimName[index];
        let code = letter.charCodeAt(0);
        //非中文不需要转拼音
        if (code < MIN_CHINESE_CODE || code > MAX_CHINESE_CODE) {
          currentStr += letter.toUpperCase();
          continue;
        }
        if (currentStr) {
          sortKey += currentStr + SPACE;
          currentStr = '';
        }
        let pyLetter: string | undefined = undefined;
        if (index === 0 && MULTI_PINYIN.has(letter)) {
          pyLetter = MULTI_PINYIN.get(letter);
        }
        if (!pyLetter) {
          pyLetter = ChineseCharacterUtils.removePinyinTones(
            ChineseCharacterUtils.getTransLatinToAsciiLocal().transform(
              ChineseCharacterUtils.getTransHanToLatinLocal().transform(letter)));
        }
        sortKey += pyLetter.toUpperCase() + SPACE + letter + SPACE;
      }
      if (currentStr && currentStr.length) {
        sortKey += currentStr;
      }
    } catch (e) {
      console.error(`${TAG} getSortKey err,name:${name},msg:${e.message}`)
    }
    return sortKey;
  }

  public static getSortFirstLetter(name: string): string {
    if (StringUtil.isEmpty(name)) {
      return DEFAULT_SORT_LETTER;
    }
    if (name[0] >= 'a' && name[0] <= 'z') {
      return name[0].toUpperCase();
    }
    if (name[0] >= 'A' && name[0] <= 'Z') {
      return name[0];
    }
    return DEFAULT_SORT_LETTER;
  }
}