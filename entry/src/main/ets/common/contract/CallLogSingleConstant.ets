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

export default class CallLogSingleConstant {
  // datashare uri
  public static readonly CONTENT_URI: string = 'datashare:///com.ohos.calllogability';
  public static readonly CALL_LOG_URI: string = CallLogSingleConstant.CONTENT_URI + '/calls/calllog';
  // databases columns
  public static readonly IS_BLOCK: string = 'is_block';
  public static readonly CALL_LOG_SLOT_ID: string = 'slot_id';
  public static readonly CALL_LOG_PHONE_NUMBER: string = 'phone_number';
  public static readonly CALL_LOG_FORMAT_PHONE_NUMBER: string = 'format_phone_number';
  public static readonly CALL_LOG_DISPLAY_NAME: string = 'display_name';
  public static readonly CALL_LOG_CALL_DIRECTION: string = 'call_direction';
  public static readonly CALL_LOG_VOICEMAIL_URI: string = 'voicemail_uri';
  public static readonly CALL_LOG_SIM_TYPE: string = 'sim_type';
  public static readonly CALL_LOG_IS_HD: string = 'is_hd';
  public static readonly CALL_LOG_IS_READ: string = 'is_read';
  public static readonly CALL_LOG_RING_DURATION: string = 'ring_duration';
  public static readonly CALL_LOG_TALK_DURATION: string = 'talk_duration';
  public static readonly CALL_LOG_FORMAT_NUMBER: string = 'format_number';
  public static readonly CALL_LOG_QUICK_SEARCH_KEY: string = 'quicksearch_key';
  public static readonly CALL_LOG_NUMBER_TYPE: string = 'number_type';
  public static readonly CALL_LOG_NUMBER_TYPE_NAME: string = 'number_type_name';
  public static readonly CALL_LOG_BEGIN_TIME: string = 'begin_time';
  public static readonly CALL_LOG_END_TIME: string = 'end_time';
  public static readonly CALL_LOG_ANSWER_STATE: string = 'answer_state';
  public static readonly CALL_LOG_CREATE_TIME: string = 'create_time';
  public static readonly CALL_LOG_NUMBER_LOCATION: string = 'number_location';
  public static readonly CALL_LOG_PHOTO_ID: string = 'photo_id';
  public static readonly CALL_LOG_PHOTO_URI: string = 'photo_uri';
  public static readonly CALL_LOG_COUNTRY_ISO_CODE: string = 'country_iso_code';
  public static readonly CALL_LOG_PRIVACY_TAG: string = 'privacy_tag';
  public static readonly CALL_LOG_EXTRA_1: string = 'extra1';
  public static readonly CALL_LOG_EXTRA_2: string = 'extra2';
  public static readonly CALL_LOG_EXTRA_3: string = 'extra3';
  public static readonly CALL_LOG_EXTRA_4: string = 'extra4';
  public static readonly CALL_LOG_EXTRA_5: string = 'extra5';
  public static readonly CALL_LOG_EXTRA_6: string = 'extra6';
  public static readonly NEW_CALLING: string = 'new_calling';
  public static readonly CALL_LOG_FEATURES: string = 'features';
  public static readonly CALL_LOG_SATELLITE: string = 'hw_is_satellite';
  public static readonly CALL_LOG_CELIA: string = 'celia_call_type';
  public static readonly MARK_TYPE: string = 'mark_type';
  public static readonly MARK_CONTENT: string = 'mark_content';
  public static readonly MARK_COUNT: string = 'mark_count';
  public static readonly IS_CLOUD_MARK: string = 'is_cloud_mark';
  public static readonly IS_CNAP: string = 'is_cnap';
  public static readonly NEWCALL: string = 'new_calling';
  public static readonly BLOCK_REASON: string = 'block_reason';
  public static readonly MARK_SOURCE: string = 'mark_source';
  public static readonly START_TIME: string = 'start_time';
  public static readonly ABSTRACT_TYPE: string = 'abstract_type';
  public static readonly ABSTRACT_PROFILE: string = 'abstract_profile';
  public static readonly ABS_STATUS: string = 'abs_status';
  public static readonly ABS_CODE: string = 'abs_code';
  public static readonly FAIL_ABS_RECORD_ID: string = 'fail_abs_record_id';
  public static readonly FAIL_ABS_RECORD_STATUS: string = 'fail_abs_record_status';
  public static readonly FAIL_ABS_RECORD_UPDATER: string = 'fail_abs_record_updater';
  public static readonly FAIL_ABS_RECORD_UPDATE_TIME: string = 'fail_abs_record_update_time';
  public static readonly ABS_FIN_TIMES_CODE: string = 'abs_fin_times_code';
  public static readonly NOTES_ID: string = 'notes_id';
  public static readonly NOTES_STATUS: string = 'notes_status';
  public static readonly NOTES_STATUS_UPDATER: string = 'notes_status_updater';
  public static readonly NOTES_STATUS_UPDATE_TIME: string = 'notes_status_update_time';
  // is_hd
  public static readonly IS_HD_FALSE = 0;
  public static readonly IS_HD_TRUE = 1;
  // slot_id
  public static readonly SLOT_ID_ONE = 0;
  public static readonly SLOT_ID_TWO = 1;
  // call_direction
  public static readonly CALL_DIRECTION_IN = 0;
  public static readonly CALL_DIRECTION_OUT = 1;
  // MeeTime call_direction
  public static readonly MEETIME_DIRECTION_IN = 1;
  public static readonly MEETIME_DIRECTION_OUT = 0;
  // answer_state
  public static readonly CALL_ANSWER_MISSED = 0;
  public static readonly CALL_ANSWER_ACTIVED = 1;
  public static readonly CALL_ANSWER_REJECT = 2;
  public static readonly CALL_ANSWER_ANSWERED_EXTERNALLY = 3;
  public static readonly MEETIME_REJECT_BY_OTHER = 5;
  public static readonly CALL_ANSWER_BLOCKED = 6;
  public static readonly CALL_ANSWER_REJECT_BY_OTHER = 7;
  public static readonly QUERY_PAGE_NUM = 200;
  // block_reason
  public static readonly BLACK_LIST = 1; // 黑名单
  public static readonly CALLS_QUERY_NEXT2NEXT_COLUMNS: Array<string> = [
    CallLogSingleConstant.CALL_LOG_SLOT_ID,
    CallLogSingleConstant.CALL_LOG_PHONE_NUMBER,
    CallLogSingleConstant.CALL_LOG_FORMAT_PHONE_NUMBER,
    CallLogSingleConstant.CALL_LOG_DISPLAY_NAME,
    CallLogSingleConstant.CALL_LOG_CALL_DIRECTION,
    CallLogSingleConstant.CALL_LOG_VOICEMAIL_URI,
    CallLogSingleConstant.CALL_LOG_SIM_TYPE,
    CallLogSingleConstant.CALL_LOG_IS_HD,
    CallLogSingleConstant.CALL_LOG_IS_READ,
    CallLogSingleConstant.CALL_LOG_RING_DURATION,
    CallLogSingleConstant.CALL_LOG_TALK_DURATION,
    CallLogSingleConstant.CALL_LOG_FORMAT_NUMBER,
    CallLogSingleConstant.CALL_LOG_QUICK_SEARCH_KEY,
    CallLogSingleConstant.CALL_LOG_NUMBER_TYPE,
    CallLogSingleConstant.CALL_LOG_NUMBER_TYPE_NAME,
    CallLogSingleConstant.CALL_LOG_BEGIN_TIME,
    CallLogSingleConstant.CALL_LOG_END_TIME,
    CallLogSingleConstant.CALL_LOG_ANSWER_STATE,
    CallLogSingleConstant.CALL_LOG_CREATE_TIME,
    CallLogSingleConstant.CALL_LOG_NUMBER_LOCATION,
    CallLogSingleConstant.CALL_LOG_PHOTO_ID,
    CallLogSingleConstant.CALL_LOG_PHOTO_URI,
    CallLogSingleConstant.CALL_LOG_COUNTRY_ISO_CODE,
    CallLogSingleConstant.CALL_LOG_EXTRA_1,
    CallLogSingleConstant.CALL_LOG_EXTRA_2,
    CallLogSingleConstant.CALL_LOG_EXTRA_3,
    CallLogSingleConstant.CALL_LOG_EXTRA_4,
    CallLogSingleConstant.CALL_LOG_EXTRA_5,
    CallLogSingleConstant.CALL_LOG_EXTRA_6,
    CallLogSingleConstant.CALL_LOG_FEATURES,
    CallLogSingleConstant.MARK_TYPE,
    CallLogSingleConstant.MARK_CONTENT,
    CallLogSingleConstant.MARK_COUNT,
    CallLogSingleConstant.IS_CLOUD_MARK,
    CallLogSingleConstant.IS_CNAP
  ];
}