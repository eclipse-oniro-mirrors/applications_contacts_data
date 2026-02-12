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

export default class CallLogDoubleConstant {
  public static readonly CALLS_DB_NAME: string = 'calls_double.db';
  public static readonly CALLS_TABLE_NAME: string = 'calls';
  public static readonly CALLS_CLONE_DB_NAME: string = 'calllog.db';
  public static readonly CALLS_CLONE_TABLE_NAME: string = 'calls_tb';
  public static readonly CALLS_ID: string = '_id';
  public static readonly CALLS_SUBSCRIPTION_ID: string = 'subscription_id';
  public static readonly CALLS_NUMBER: string = 'number';
  public static readonly CALLS_NAME: string = 'name';
  public static readonly CALLS_TYPE: string = 'type';
  public static readonly CALLS_VOICEMAIL_URI: string = 'voicemail_uri';
  public static readonly CALLS_FEATURES: string = 'features';
  public static readonly CALLS_IS_READ: string = 'is_read';
  public static readonly CALLS_NEW: string = 'new';
  public static readonly CALLS_RING_TIMES: string = 'ring_times';
  public static readonly CALLS_DURATION: string = 'duration';
  public static readonly CALLS_FORMATTED_NUMBER: string = 'formatted_number';
  public static readonly CALLS_DATE: string = 'date';
  public static readonly CALLS_GEOCODED_LOCATION: string = 'geocoded_location';
  public static readonly CALLS_PHOTO_ID: string = 'photo_id';
  public static readonly CALLS_PHOTO_URI: string = 'photo_uri';
  public static readonly CALLS_COUNTRY_ISO: string = 'countryiso';
  public static readonly CALLS_HICALL_DEVICE_COM_ID: string = 'hicall_device_com_id';
  public static readonly CALLS_CALL_TYPE: string = 'call_type';
  public static readonly CALLS_HW_SATELLITE: string = 'hw_is_satellite';
  public static readonly MARK_TYPE: string = 'mark_type';
  public static readonly MARK_CONTENT: string = 'mark_content';
  public static readonly MARK_COUNT: string = 'mark_count';
  public static readonly IS_CLOUD_MARK: string = 'is_cloud_mark';
  public static readonly DATA1: string = 'data1';
  public static readonly QUERY_PAGE_NUM = 500;
  public static readonly CALLS_QUERY_COLUMNS: Array<string> = [
    CallLogDoubleConstant.CALLS_ID,
    CallLogDoubleConstant.CALLS_SUBSCRIPTION_ID,
    CallLogDoubleConstant.CALLS_NUMBER,
    CallLogDoubleConstant.CALLS_NAME,
    CallLogDoubleConstant.CALLS_TYPE,
    CallLogDoubleConstant.CALLS_VOICEMAIL_URI,
    CallLogDoubleConstant.CALLS_FEATURES,
    CallLogDoubleConstant.CALLS_IS_READ,
    CallLogDoubleConstant.CALLS_NEW,
    CallLogDoubleConstant.CALLS_RING_TIMES,
    CallLogDoubleConstant.CALLS_DURATION,
    CallLogDoubleConstant.CALLS_FORMATTED_NUMBER,
    CallLogDoubleConstant.CALLS_DATE,
    CallLogDoubleConstant.CALLS_GEOCODED_LOCATION,
    CallLogDoubleConstant.CALLS_PHOTO_ID,
    CallLogDoubleConstant.CALLS_PHOTO_URI,
    CallLogDoubleConstant.CALLS_COUNTRY_ISO,
    CallLogDoubleConstant.CALLS_HICALL_DEVICE_COM_ID,
    CallLogDoubleConstant.CALLS_CALL_TYPE,
    CallLogDoubleConstant.CALLS_HW_SATELLITE,
    CallLogDoubleConstant.MARK_TYPE,
    CallLogDoubleConstant.MARK_CONTENT,
    CallLogDoubleConstant.MARK_COUNT,
    CallLogDoubleConstant.DATA1,
    CallLogDoubleConstant.IS_CLOUD_MARK
  ];
  public static readonly CALLS_QUERY_CLONE_COLUMNS: Array<string> = [
    CallLogDoubleConstant.CALLS_SUBSCRIPTION_ID,
    CallLogDoubleConstant.CALLS_NUMBER,
    CallLogDoubleConstant.CALLS_TYPE,
    CallLogDoubleConstant.CALLS_VOICEMAIL_URI,
    CallLogDoubleConstant.CALLS_FEATURES,
    CallLogDoubleConstant.CALLS_IS_READ,
    CallLogDoubleConstant.CALLS_RING_TIMES,
    CallLogDoubleConstant.CALLS_DURATION,
    CallLogDoubleConstant.CALLS_FORMATTED_NUMBER,
    CallLogDoubleConstant.CALLS_DATE,
    CallLogDoubleConstant.CALLS_GEOCODED_LOCATION,
    CallLogDoubleConstant.CALLS_PHOTO_ID,
    CallLogDoubleConstant.CALLS_COUNTRY_ISO,
    CallLogDoubleConstant.CALLS_HICALL_DEVICE_COM_ID,
    CallLogDoubleConstant.CALLS_NEW,
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
  //畅联通话其他设备拒接状态
  public static readonly REJECTED_REMOTE = 304;

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
