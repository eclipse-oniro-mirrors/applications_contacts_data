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

export default class BlockListDoubleConstant {
  public static readonly BLOCK_DB_NAME: string = 'HarassmentInterception_double.db';
  public static readonly BLOCK_LIST_TABLE_NAME: string =
    'interception_blacklist';
  public static readonly ACCESS_LIST_TABLE_NAME: string =
    'tbWhitelist'
  public static readonly BLOCK_CLONE_DB_NAME: string = 'harassment.db';
  public static readonly BLOCK_LIST_CLONE_TABLE_NAME: string =
    'comhuaweisystemmanagerHarassmentInterceptionDBProviderinterception_blacklist_tb';
  public static readonly ACCESS_LIST_CLONE_TABLE_NAME: string =
    'comhuaweisystemmanagerHarassmentInterceptionDBProviderwhitelist_tb'
  public static readonly BLOCK_PHONE: string = 'phone';
  public static readonly BLOCK_NAME: string = 'name';
  public static readonly BLOCK_TYPE: string = 'type';
  public static readonly QUERY_PAGE_NUM = 500;
  public static readonly BLOCK = 0;
  public static readonly BLOCK_START_WITH = 1;
  public static readonly ACCESS = 0;
  public static readonly BLOCK_QUERY_CLONE_COLUMNS: Array<string> = [
    BlockListDoubleConstant.BLOCK_PHONE,
    BlockListDoubleConstant.BLOCK_NAME,
    BlockListDoubleConstant.BLOCK_TYPE
  ];
}