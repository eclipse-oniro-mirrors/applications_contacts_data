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
export default class CloneDbColumns {
    /**
    * Column 'starred' in table raw_contacts_tb
    */
   public static readonly STARRED: string = 'starred';
 
   /**
    * Column 'aggregation_mode' in table raw_contacts_tb
    */
   public static readonly AGGREGATION_MODE: string = 'aggregation_mode';
 
   /**
    * Column 'last_time_contacted' in table raw_contacts_tb
    */
   public static readonly LAST_TIME_CONTACTED: string = 'last_time_contacted';
 
   /**
    * Column '_id' in table raw_contacts_tb
    */
   public static readonly ID: string = '_id';
 
   /**
    * Column 'send_to_voicemail' in table raw_contacts_tb
    */
   public static readonly SEND_TO_VOICEMAIL: string = 'send_to_voicemail';
 
   /**
    * Column 'ext_account_name' in table raw_contacts_tb
    */
   public static readonly EXT_ACCOUNT_NAME: string = 'ext_account_name';
 
   /**
    * Column 'ext_account_type' in table raw_contacts_tb
    */
   public static readonly EXT_ACCOUNT_TYPE: string = 'ext_account_type';
 
   /**
    * Column 'account_name' in table raw_contacts_tb
    */
   public static readonly ACCOUNT_NAME: string = 'account_name';
 
   /**
    * Column 'account_type' in table raw_contacts_tb
    */
   public static readonly ACCOUNT_TYPE: string = 'account_type';
 
   /**
    * Column 'ext_contact_type' in table raw_contacts_tb
    */
   public static readonly EXT_CONTACT_TYPE: string = 'ext_contact_type';
 
   /**
    * Column 'is_private' in table raw_contacts_tb
    */
   public static readonly IS_PRIVATE: string = 'is_private';
 
   /**
    * Column 'times_contacted' in table raw_contacts_tb
    */
   public static readonly TIMES_CONTACTED: string = 'times_contacted';

   public static readonly GROUP_IS_READ_ONLY: string = 'group_is_read_only';
 }