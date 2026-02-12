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

export class PhoneNumberUtils {
  /*
   * The BCD extended type used to determine the extended char for the digit which is greater than
   * 9.
   *
   * see TS 51.011 section 10.5.1 EF_ADN(Abbreviated dialling numbers)
   */
  public static readonly BCD_EXTENDED_TYPE_EF_ADN = 1;
  /*
   * The BCD extended type used to determine the extended char for the digit which is greater than
   * 9.
   *
   * see TS 24.008 section 10.5.4.7 Called party BCD number
   */
  public static readonly BCD_EXTENDED_TYPE_CALLED_PARTY = 2;
  /*
   * Special characters
   *
   * (See 'What is a phone number?' doc)
   * 'p' --- GSM pause character, same as comma
   * 'n' --- GSM wild character
   * 'w' --- GSM wait character
   */
  public static readonly PAUSE = ',';
  public static readonly WAIT = ';';
  public static readonly WILD = 'N';
  /*
   * Calling Line Identification Restriction (CLIR)
   */
  private static readonly CLIR_ON = '*31#';
  private static readonly CLIR_OFF = '#31#';
  /*
   * TOA = TON + NPI
   * See TS 24.008 section 10.5.4.7 for details.
   * These are the only really useful TOA values
   */
  public static readonly toaInternational = 0x91;
  public static readonly toaUnknown = 0x81;
  public static readonly LOG_TAG = 'PhoneNumberUtils';
  private static readonly DBG = false;
  private static readonly BCD_EF_ADN_EXTENDED = '*#,N;';
  private static readonly BCD_CALLED_PARTY_EXTENDED = '*#abc';
  /*
   * global-phone-number = ['+'] 1*( DIGIT / written-sep )
   * written-sep         = ('-'/'.')
   */
  private static readonly GLOBAL_PHONE_NUMBER_PATTERN =
    new RegExp('[\\+]?[0-9.-]+');

  /** True if c is ISO-LATIN characters 0-9 */
  public static isISODigit(c: string): boolean {
    return c >= '0' && c <= '9';
  }

  /** True if c is ISO-LATIN characters 0-9, *, # */
  public static is12Key(c: string): boolean {
    return (c >= '0' && c <= '9') || c == '*' || c == '#' || c == ',' || c == ';';
  }

  /** True if c is ISO-LATIN characters 0-9, *, # , +, WILD  */
  public static isDialable(c: string): boolean {
    return (c >= '0' && c <= '9') || c == '*' || c == '#' || c == '+' || c == PhoneNumberUtils.WILD;
  }

  /** True if c is ISO-LATIN characters 0-9, *, # , + (no WILD)  */
  public static isReallyDialable(c: string): boolean {
    return (c >= '0' && c <= '9') || c == '*' || c == '#' || c == '+';
  }

  /** True if c is ISO-LATIN characters 0-9, *, # , +, WILD, WAIT, PAUSE   */
  public static isNonSeparator(c: string): boolean {
    return (c >= '0' && c <= '9') || c == '*' || c == '#' || c == '+' ||
      c == PhoneNumberUtils.WILD || c == PhoneNumberUtils.WAIT || c == PhoneNumberUtils.PAUSE;
  }

  /** This any anything to the right of this char is part of the
   *  post-dial string (eg this is PAUSE or WAIT)
   */
  public static isStartsPostDial(c: string): boolean {
    return c == PhoneNumberUtils.PAUSE || c == PhoneNumberUtils.WAIT;
  }

  private static isPause(c: string): boolean {
    return c == 'p' || c == 'P';
  }

  private static isToneWait(c: string): boolean {
    return c == 'w' || c == 'W';
  }

  /** The current locale is unknown, look for a country code or don't format */
  public static readonly FORMAT_UNKNOWN = 0;
  /** NANP formatting */
  public static readonly FORMAT_NANP = 1;
  /** Japanese formatting */
  public static readonly FORMAT_JAPAN = 2;
  private static readonly NANP_STATE_DIGIT = 1;
  private static readonly NANP_STATE_PLUS = 2;
  private static readonly NANP_STATE_ONE = 3;
  private static readonly NANP_STATE_DASH = 4;

  public static formatNumber(source: string, defaultFormattingType: number): string {
    return source;
  }
}