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

export default class CallLogArdConstant {
  public static readonly CALLS_CLONE_DB_NAME: string = 'calllog.db';
  public static readonly CALLS_CLONE_TABLE_NAME: string = 'calls_tb';
  public static readonly CALLS_SUBSCRIPTION_ID: string = 'subscription_id';
  public static readonly CALLS_NUMBER: string = 'number';
  public static readonly CALLS_TYPE: string = 'type';
  public static readonly CALLS_VOICEMAIL_URI: string = 'voicemail_uri';
  public static readonly CALLS_FEATURES: string = 'features';
  public static readonly CALLS_IS_READ: string = 'is_read';
  public static readonly CALLS_NEW: string = 'new';
  public static readonly CALLS_DURATION: string = 'duration';
  public static readonly CALLS_FORMATTED_NUMBER: string = 'formatted_number';
  public static readonly CALLS_DATE: string = 'date';
  public static readonly CALLS_GEOCODED_LOCATION: string = 'geocoded_location';
  public static readonly CALLS_PHOTO_ID: string = 'photo_id';
  public static readonly CALLS_COUNTRY_ISO: string = 'countryiso';
  public static readonly CALLS_RING_TIMES: string = 'ring_times';
  public static readonly CALLS_COUNTRYISO: string = 'countryiso';
  public static readonly QUERY_PAGE_NUM = 500;
  public static readonly CALLS_QUERY_CLONE_COLUMNS: Array<string> = [
    CallLogArdConstant.CALLS_SUBSCRIPTION_ID,
    CallLogArdConstant.CALLS_NUMBER,
    CallLogArdConstant.CALLS_TYPE,
    CallLogArdConstant.CALLS_VOICEMAIL_URI,
    CallLogArdConstant.CALLS_FEATURES,
    CallLogArdConstant.CALLS_IS_READ,
    CallLogArdConstant.CALLS_NEW,
    CallLogArdConstant.CALLS_DURATION,
    CallLogArdConstant.CALLS_FORMATTED_NUMBER,
    CallLogArdConstant.CALLS_DATE,
    CallLogArdConstant.CALLS_GEOCODED_LOCATION,
    CallLogArdConstant.CALLS_PHOTO_ID,
    CallLogArdConstant.CALLS_COUNTRY_ISO
  ];
  public static readonly TYPE_INCOMING = 1;
  // Call log type for incoming calls.
  public static readonly TYPE_OUTGOING = 2;
  // Call log type for outgoing calls.
  public static readonly TYPE_MISSED = 3;
  // Call log type for missed calls.
  public static readonly TYPE_VOICEMAIL = 4;
  // Call log type for voicemails.
  public static readonly TYPE_REJECTED = 5;
  // Call log type for calls rejected by direct user action.
  public static readonly TYPE_BLOCKED = 6;
  // Call log type for calls blocked automatically.
  /**
   * Call log type for a call which was answered on another device.  Used in situations where
   * a call rings on multiple devices simultaneously and it ended up being answered on a
   * device other than the current one.
   */
  public static readonly TYPE_ANSWERED_EXTERNALLY = 7;
  public static readonly FEATURES_VIDEO = 1;
  // Call had video.
  public static readonly FEATURES_PULLED_EXTERNALLY = 2;
  // Call was pulled externally.
  public static readonly FEATURES_HD_CALL = 4;
  // Call was HD.
  public static readonly FEATURES_WIFI = 8;
  // Call was WIFI call.
  public static readonly FEATURES_ASSISTED_DIALING_USED = 16;
  // Indicates the call underwent Assisted Dialing.
  public static readonly FEATURES_RTT = 32;
  // Call was on RTT at some point
  public static readonly FEATURES_VOLTE = 64;
  // Call was VoLTE
}
