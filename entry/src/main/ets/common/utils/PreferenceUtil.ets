/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import data_preferences from '@ohos.data.preferences';
import LogUtils from './LogUtils';
import PreferenceUtilClKv from './PreferenceUtilClkv';
import { AbstractPreferenceUtil } from './PreferenceUtilAbstract';
import PreferenceUtilXml from './PreferenceUtilXml';

export enum MigrationProgress {
  NOT_START = 'notStart',
  CONTACT_TYPE = 'contactTypeComplete',
  ACCOUNT = 'accountComplete',
  GROUP = 'groupComplete',
  CONTACT = 'contactComplete',
  DELETE = 'deleteComplete',
  RAW_CONTACT = 'rawContactComplete',
  DATA = 'dataComplete',
  SEARCH = 'searchComplete',
  COMPLETE = 'Complete',
}

export default class PreferenceUtil extends AbstractPreferenceUtil {

  /**
   * The key of retryBackup progress end
   */
  public static readonly BACKUP_SERVICE_PROGRESS_END = 'backup_service_progress_end';
  /**
   * The key of parse delete contacts backup data progress end
   */
  public static readonly DELETE_BACKUP_DATA_END = 'delete_backup_data_end';
  /**
   * The key prefix of contacts
   */
  public static readonly CONTACT_PREFIX = 'ContactModel';
  public static readonly CONTACT_PREFIX_CLONE = 'CloneContactModel';
  /**
   * The key prefix of profile
   */
  public static readonly PROFILE_PREFIX = 'ProfileModel';

  /**
   * The migrateStatus
   * -1: not start backup contacts
   * 0: start backup, not success
   * 1: backup success
   */
  public static readonly MIGRATE_STATUS = 'migrateStatus';
  public static readonly CLONE_MIGRATE_STATUS = 'cloneMigrateStatus';
  /**
   * The migrated rawIds
   */
  public static readonly MIGRATED_RAW_ID = 'migratedRawIds';
  public static readonly CLONE_MIGRATED_RAW_ID = 'cloneMigratedRawIds';
  /**
   * Is insert table data success
   */
  public static readonly INSERT_DATA_SUCCESS = 'insertDataSuccess';
  public static readonly CLONE_INSERT_DATA_SUCCESS = 'cloneInsertDataSuccess';
  /**
   * Is insert table raw contact success
   */
  public static readonly INSERT_RAW_SUCCESS = 'insertRawSuccess';
  public static readonly CLONE_INSERT_RAW_SUCCESS = 'cloneInsertRawSuccess';
  /**
   * The groups id maps
   */
  public static readonly GROUPS_ID_MAP = 'groupsIdMaps';
  public static readonly CLONE_GROUPS_ID_MAP = 'cloneGroupsIdMaps';
  /**
   * The raw contact id maps
   */
  public static readonly RAW_ID_MAP = 'rawIdMap';
  public static readonly CLONE_RAW_ID_MAP = 'cloneRawIdMap';
  /**
   * The raw contact migrated page
   */
  public static readonly RAW_MIGRATE_PAGE = 'rawMigratedPage';
  public static readonly CLONE_RAW_MIGRATE_PAGE = 'cloneRawMigratedPage';
  /**
   * The data migrated page
   */
  public static readonly DATA_MIGRATE_PAGE = 'dataMigratedPage';
  public static readonly CLONE_DATA_MIGRATE_PAGE = 'cloneDataMigratedPage';
  /**
   * The group migrated page
   */
  public static readonly GROUP_MIGRATE_PAGE = 'groupsMigratedPage';
  public static readonly CLONE_GROUP_MIGRATE_PAGE = 'cloneGroupsMigratedPage';
  /**
   * The contact migrate status key
   */
  public static readonly NEXT2NEXT_CONTACT_MIGRATE_STATUS = 'next2next_contact_migrate_status';
  /**
   * The call log migrate status key
   */
  public static readonly CALL_LOG_MIGRATE_STATUS = 'call_log_migrate_status';
  public static readonly CLONE_CALL_LOG_MIGRATE_STATUS = 'clone_call_log_migrate_status';
  public static readonly ARD2NEXT_CALL_LOG_MIGRATE_STATUS = 'ard2next_call_log_migrate_status';
  public static readonly NEXT2NEXT_CALL_LOG_MIGRATE_STATUS = 'next2next_call_log_migrate_status';
  /**
   * The call log copy info key from el5 to el1
   */
  public static readonly CALL_LOG_COPY_EL5_TO_EL1_TIMESTAMP = 'call_log_copy_el5_to_el1_timestamp';
  /**
   * The block list migrate status key
   */
  public static readonly BLOCK_LIST_MIGRATE_STATUS = 'block_list_migrate_status';
  /**
   * The key of mimeTypeId maps
   */
  public static readonly TYPE_ID_MAP = 'typeIdMap';
  /**
   * The key of migration progress
   */
  public static readonly PROGRESS = 'MigrationProgress';
  /**
   * The key of the phone account's id
   */
  public static readonly ACCOUNT_ID = 'accountId';
  /**
   * The key of the migrate complete page index
   */
  public static readonly GROUP_MIGRATED_PAGE = 'groupMigratedPage';
  /**
   * The key of the migrate complete page index
   */
  public static readonly DELETE_MIGRATED_PAGE = 'deleteMigratedPage';
  /**
   * The key of the migrate complete page index
   */
  public static readonly CONTACT_MIGRATED_PAGE = 'contactMigratedPage';
  /**
   * The key of the migrate complete page index
   */
  public static readonly RAW_CONTACT_MIGRATED_PAGE = 'rawContactMigratedPage';
  /**
   * The key of the migrate complete page index
   */
  public static readonly DATA_MIGRATED_PAGE = 'dataMigratedPage';
  /**
   * The key of the migrate complete page index
   */
  public static readonly SEARCH_MIGRATED_PAGE = 'searchMigratedPage';
  /**
   * The key of the migrate group error pages
   */
  public static readonly GROUP_FAILED = 'groupFailedList';
  /**
   * The key of the migrate delete_raw_contact error pages
   */
  public static readonly DELETE_FAILED = 'deleteFailedList';
  /**
   * The key of the migrate contact error pages
   */
  public static readonly CONTACT_FAILED = 'contactFailedList';
  /**
   * The key of the migrate raw_contact error pages
   */
  public static readonly RAW_CONTACT_FAILED = 'rawContactFailedList';
  /**
   * The key of the migrate contact_data error pages
   */
  public static readonly DATA_FAILED = 'contactDataFailedList';
  /**
   * The key of the migrate search error pages
   */
  public static readonly SEARCH_FAILED = 'searchFailedList';

  // 云同步联系人数量，功耗管控使用
  public static readonly CLOUD_SYNC_COUNT = 'cloudSyncCount';
  // 云同步联系人数量统计日期，功耗管控使用
  public static readonly CLOUD_SYNC_COUNT_STATISTICS_DATE = 'cloudSyncCountStatisticsDate';

  private static sInstance: AbstractPreferenceUtil;

  public static getInstance(): AbstractPreferenceUtil {
    if (PreferenceUtil.sInstance) {
      return PreferenceUtil.sInstance;
    }

    if (data_preferences.isStorageTypeSupported(data_preferences.StorageType.GSKV)) {
      PreferenceUtil.sInstance = PreferenceUtilClKv.getInstance();
      LogUtils.w('PreferenceUtil', 'support ClKv, use ClKv');
    } else {
      PreferenceUtil.sInstance = PreferenceUtilXml.getInstance();
      LogUtils.w('PreferenceUtil', 'not support ClKv, use xml');
    }

    return PreferenceUtil.sInstance;
  }

}