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

export default class BlockListSingleConstant {
  // datashare uri
  public static readonly CONTENT_URI: string = 'datashare:///com.ohos.contactsdataability';
  public static readonly BLOCK_LIST_URI: string = BlockListSingleConstant.CONTENT_URI + '/contacts/contact_blocklist';
  // databases columns
  public static readonly BLOCK_LIST_PHONE_NUMBER: string = 'phone_number';
  public static readonly BLOCK_LIST_FORMAT_PHONE_NUMBER: string = 'format_phone_number';
  public static readonly BLOCK_LIST_NAME: string = 'name';
  public static readonly BLOCK_LIST_TYPES: string = 'types';
  public static readonly BLOCK_LIST_CONTENT: string = 'content';
  public static readonly BLOCK_LIST_INTERCEPTION_CALL_COUNT: string = 'interception_call_count';
  public static readonly BLOCK_LIST_INTERCEPTION_MSG_COUNT: string = 'interception_msg_count';
  public static readonly TIME_STAMP: string = 'time_stamp';
  // block type
  public static readonly BLOCK = 0;
  public static readonly ACCESS = 1;
  public static readonly BLOCK_START_WITH = 2;
  public static readonly QUERY_PAGE_NUM = 200;
  public static readonly BLOCK_LIST_QUERY_NEXT2NEXT_COLUMNS: Array<string> = [
    BlockListSingleConstant.BLOCK_LIST_PHONE_NUMBER,
    BlockListSingleConstant.BLOCK_LIST_NAME,
    BlockListSingleConstant.BLOCK_LIST_TYPES,
    BlockListSingleConstant.BLOCK_LIST_INTERCEPTION_CALL_COUNT,
    BlockListSingleConstant.BLOCK_LIST_INTERCEPTION_MSG_COUNT,
    BlockListSingleConstant.BLOCK_LIST_CONTENT,
    BlockListSingleConstant.TIME_STAMP
  ];
}