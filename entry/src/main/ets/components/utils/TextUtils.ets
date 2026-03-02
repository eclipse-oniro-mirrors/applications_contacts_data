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

export namespace TextUtils {

  const MIN_HIGH_SURROGATE: number = 0xD800;

  /**
   * The maximum value of a
   * <a href='http://www.unicode.org/glossary/#high_surrogate_code_unit'>
   * Unicode high-surrogate code unit</a>
   * in the UTF-16 encoding, constant {@code '\u005CuDBFF'}.
   * A high-surrogate is also known as a <i>leading-surrogate</i>.
   *
   */
  const MAX_HIGH_SURROGATE = 0xDBFF;

  /**
   * The minimum value of a
   * <a href='http://www.unicode.org/glossary/#low_surrogate_code_unit'>
   * Unicode low-surrogate code unit</a>
   * in the UTF-16 encoding, constant {@code '\u005CuDC00'}.
   * A low-surrogate is also known as a <i>trailing-surrogate</i>.
   *
   */
  const MIN_LOW_SURROGATE = 0xDC00;

  /**
   * The maximum value of a
   * <a href='http://www.unicode.org/glossary/#low_surrogate_code_unit'>
   * Unicode low-surrogate code unit</a>
   * in the UTF-16 encoding, constant {@code '\u005CuDFFF'}.
   * A low-surrogate is also known as a <i>trailing-surrogate</i>.
   *
   */
  const MAX_LOW_SURROGATE = 0xDFFF;

  /**
   * The minimum value of a Unicode surrogate code unit in the
   * UTF-16 encoding, constant {@code '\u005CuD800'}.
   *
   */
  const MIN_SURROGATE = MIN_HIGH_SURROGATE;

  /**
   * The maximum value of a Unicode surrogate code unit in the
   * UTF-16 encoding, constant {@code '\u005CuDFFF'}.
   *
   */
  const MAX_SURROGATE = MAX_LOW_SURROGATE;

  /**
   *
   * @param params string
   * @returns true if params is nul or undefined or "". false otherwise
   */
  export function isEmpty(params: string | null | undefined): boolean {
    if (params) {
      return false
    }
    return true
  }

  /**
   *
   * @param params string
   * @returns true if params is nul or undefined or "". false otherwise
   */
  export function isBlank(params: string | null | undefined): boolean {
    if (params && params !== '') {
      return false
    }
    return true
  }

  export function equals(a: string, b: string) {
    return a === b
  }

  export function isSurrogate(ch: number): boolean {
    return ch >= MIN_SURROGATE && ch < (MAX_SURROGATE + 1);
  }


  export function isHighSurrogate(ch: number) {
    return ch >= MIN_HIGH_SURROGATE && ch < (MAX_HIGH_SURROGATE + 1);
    //return  c >= 0xd800 && c <= 0xdbff
  }

  export function isLowSurrogate(ch: number) {
    return ch >= MIN_LOW_SURROGATE && ch < (MAX_LOW_SURROGATE + 1);

    //return c >= 0xdc00 && c <= 0xdfff
  }

  /**
   * do not use this method
   * @throw ArrayIndexOutOfBoundsException
   */
  export function offsetByCodePointsAImpl(seq: string, index: number, codePointOffset: number): number {
    let len = seq.length
    if (index < 0 || index > len) {
      throw new Error('Index out of bounds');
    }

    let numToGo = codePointOffset;
    let offset = index;
    let adjust = 1;
    if (numToGo >= 0) {
      for (; numToGo > 0; offset++) {
        numToGo--;
        if (TextUtils.isHighSurrogate(seq.charCodeAt(offset)) && (offset + 1) < len &&
        TextUtils.isLowSurrogate(seq.charCodeAt(offset + 1))) {
          offset++;
        }
      }
      return offset;
    } else {
      numToGo *= -1;
      for (; numToGo > 0; ) {
        numToGo--;
        offset--;
        if (TextUtils.isLowSurrogate(seq.charCodeAt(offset)) && (offset - 1) >= 0 &&
        TextUtils.isHighSurrogate(seq.charCodeAt(offset - 1))) {
          offset--;
        }
      }
      return offset;
    }
  }

  /**
   * Returns the index within the given char sequence that is offset
   * from the given {@code index} by {@code codePointOffset}
   * code points. Unpaired surrogates within the text range given by
   * {@code index} and {@code codePointOffset} count as
   * one code point each.
   *
   * @param seq the char sequence
   * @param index the index to be offset
   * @param codePointOffset the offset in code points
   * @return the index within the char sequence
   * @exception NullPointerException if {@code seq} is null.
   * @exception IndexOutOfBoundsException if {@code index}
   *   is negative or larger then the length of the char sequence,
   *   or if {@code codePointOffset} is positive and the
   *   subsequence starting with {@code index} has fewer than
   *   {@code codePointOffset} code points, or if
   *   {@code codePointOffset} is negative and the subsequence
   *   before {@code index} has fewer than the absolute value
   *   of {@code codePointOffset} code points.
   */
  export function offsetByCodePoints(seq: string, index: number,
                                     codePointOffset: number): number {
    let length = seq.length;
    if (index < 0 || index > length) {
      throw new Error('IndexOutOfBoundsException');
    }

    let x = index;
    if (codePointOffset >= 0) {
      let i: number;
      for (i = 0; x < length && i < codePointOffset; i++) {
        if (isHighSurrogate(seq.charCodeAt(x++)) && x < length &&
        isLowSurrogate(seq.charCodeAt(x))) {
          x++;
        }
      }
      if (i < codePointOffset) {
        throw new Error('IndexOutOfBoundsException');
      }
    } else {
      let i: number;
      for (i = codePointOffset; x > 0 && i < 0; i++) {
        if (isLowSurrogate(seq.charCodeAt(--x)) && x > 0 &&
        isHighSurrogate(seq.charCodeAt(x - 1))) {
          x--;
        }
      }
      if (i < 0) {
        throw new Error('IndexOutOfBoundsException');
      }
    }
    return x;
  }
}