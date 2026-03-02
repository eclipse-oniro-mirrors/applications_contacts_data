/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

import { Data } from './contract/Data';
import { Group } from './contract/Group';
import { DeleteRawContact } from './contract/DeleteRawContact';
import { Mimetype } from './contract/Mimetype';
import { RawContacts } from './contract/RawContacts';
import { Contacts } from './contract/Contacts';
import { SearchContact } from './contract/SearchContact';
import ContactBigPhotoConstant from './contract/ContactBigPhotoConstant';
import RawContactsColumns from './contract/RawContactsColumns';
import DataColumns from './contract/DataColumns';
import SqlConstants from './SqlConstants';

/**
 * Database constants
 */
export default class DbConstants {
  /**
   * New database name
   */
  public static readonly NEW_DATABASE_NAME = 'contacts.db';

  /**
   * clone database name
   */
  public static readonly CLONE_DATABASE_NAME = 'contact.db';

  /**
   * clone next2next database name
   */
  public static readonly CLONE_NEXT2NEXT_DATABASE_NAME = 'contacts.db';

  /**
   * Old database name
   */
  public static readonly OLD_DATABASE_NAME = 'contacts2_double.db';

  /**
   * New database blocklist name
   */
  public static readonly NEW_DATABASE_BLOCKLIST_NAME = 'blocklist.db';

  /**
   * New database next2next blocklist name
   */
  public static readonly CLONE_NEXT2NEXT_BLOCKLIST_NAME = 'blocklist_next2next.db';

  /**
   * New database profile name
   */
  public static readonly NEW_DATABASE_PROFILE_NAME = 'profile.db';

  /**
   * Old database profile name
   */
  public static readonly OLD_DATABASE_PROFILE_NAME = 'profile_double.db';

  /**
   * Backup path for upgrade restore
   */
  public static readonly BACK_DATABASE_PATH = '/data/storage/el2/backup/restore/com.ohos.providers.contacts/ce/databases/';

  /**
   * The table 'contacts' name in old database
   */
  public static readonly OLD_CONTACTS_TABLE_NAME = 'contacts';

  /**
   * The table 'contact' name in new database
   */
  public static readonly NEW_CONTACTS_TABLE_NAME = 'contact';

  /**
   * The table 'raw_contacts' name in old database
   */
  public static readonly OLD_RAW_CONTACTS_TABLE_NAME = 'raw_contacts';

  /**
   * The table 'raw_contact' name in new database
   */
  public static readonly NEW_RAW_CONTACT_TABLE_NAME = 'raw_contact';

  /**
   * The table 'raw_contact' name in new database
   */
  public static readonly CLOUD_RAW_CONTACT_TABLE_NAME = 'cloud_raw_contact';

  /**
   * The table 'cloud_groups' name in new database
   */
  public static readonly CLOUD_GROUPS_TABLE_NAME = 'cloud_groups';

  /**
   * The table 'cloud_contact_blocklist' name in new database
   */
  public static readonly CLOUD_CONTACT_BLOCKLIST_TABLE_NAME = 'cloud_contact_blocklist';

  /**
   * The table 'privacy_contacts_backup' name in new database
   */
  public static readonly NEW_PRIVACY_CONTACTS_BACKUP_TABLE_NAME = 'privacy_contacts_backup';

  /**
   * The table 'hw_account' name in new database
   */
  public static readonly NEW_HW_ACCOUNT_TABLE_NAME = 'hw_account';

  /**
   * The table 'raw_contact' name in new database
   */
  public static readonly CALLLOG_TABLE_NAME = 'calllog';

  /**
   * The table 'raw_contacts_tb' name in clone frame database
   */
  public static readonly CLONE_RAW_CONTACTS_TABLE_NAME = 'raw_contacts_tb';
  /**
   * The table 'raw_contacts_me_tb' name in clone frame database myCard
   */
  public static readonly CLONE_MY_CARD_TABLE_NAME = 'raw_contacts_me_tb';

  /**
   * The table 'data_me_tb' name in clone database
   */
  public static readonly CLONE_DATA_CARD_TABLE_NAME = 'data_me_tb';

  /**
   * The table 'contact' name in clone frame database
   */
  public static readonly CLONE_NEXT2NEXT_CONTACTS_TABLE_NAME = 'contact';

  /**
   * The table 'raw_contact' name in clone frame database
   */
  public static readonly CLONE_NEXT2NEXT_RAW_CONTACTS_TABLE_NAME = 'raw_contact';

  /**
   * The table 'data' name in old database
   */
  public static readonly OLD_DATA_TABLE_NAME = 'data';

  /**
   * The table 'data' name in new database
   */
  public static readonly NEW_DATA_TABLE_NAME = 'contact_data';

  /**
   * The view 'view_contact_data' name in new database
   */
  public static readonly VIEW_CONTACT_DATA_NAME = 'view_contact_data';

  /**
   * The view 'view_raw_contact' name in new database
   */
  public static readonly VIEW_RAW_CONTACT_NAME = 'view_raw_contact';

  /**
   * The view 'view_contact' name in new database
   */
  public static readonly VIEW_CONTACT_NAME = 'view_contact';

  /**
   * The view 'view_contact_location' name in new database
   */
  public static readonly VIEW_CONTACT_LOCATION_NAME = 'view_contact_location';

  /**
   * The view 'view_groups' name in new database
   */
  public static readonly VIEW_GROUPS_NAME = 'view_groups';

  /**
   * The view 'view_deleted' name in new database
   */
  public static readonly VIEW_DELETED_NAME = 'view_deleted';

  /**
   * The table 'data' name in new database
   */
  public static readonly NEW_DELETE_RAW_CONTACT_TABLE_NAME = 'deleted_raw_contact';

  /**
   * The table 'data_tb' name in clone database
   */
  public static readonly CLONE_DATA_TABLE_NAME = 'data_tb';

  /**
   * The table 'contact_data' name in clone database
   */
  public static readonly CLONE_NEXT2NEXT_DATA_TABLE_NAME = 'contact_data';

  /**
   * Old database cache name
   */
  public static readonly OLD_DATABASE_SHM_NAME = 'contacts2.db-shm';

  /**
   * Old database cache name
   */
  public static readonly OLD_DATABASE_WAL_NAME = 'contacts2.db-wal';

  /**
   * The table 'mimetypes' name in old database
   */
  public static readonly OLD_MIMETYPE_TABLE_NAME = 'mimetypes';

  /**
   * The table 'accounts' name in old database
   */
  public static readonly OLD_ACCOUNT_TABLE_NAME = 'accounts';


  /**
   * The table 'account' name in new database
   */
  public static readonly NEW_ACCOUNT_TABLE_NAME = 'account';

  /**
   * The table 'hwsearch_contacts' name in new database
   */
  public static readonly OLD_SEARCH_TABLE_NAME = 'hwsearch_contacts';

  /**
   * The table 'search_contact' name in new database
   */
  public static readonly NEW_SEARCH_TABLE_NAME = 'search_contact';

  /**
   * The table 'groups' name in old database
   */
  public static readonly OLD_GROUPS_TABLE_NAME = 'groups';

  /**
   * The table 'deleted_raw_contacts' table name in old database
   */
  public static readonly OLD_DELETED_RAW_CONTACTS = 'deleted_raw_contacts';

  /**
   * The table 'deleted_raw_contact' table in new database
   */
  public static readonly NEW_DELETED_RAW_CONTACTS = 'deleted_raw_contact';

  /**
   * The table 'groups_tb' name in clone database
   */
  public static readonly CLONE_GROUPS_TABLE_NAME = 'groups_tb';

  /**
   * The table 'groups' name in clone database
   */
  public static readonly CLONE_NEXT2NEXT_GROUPS_TABLE_NAME = 'groups';

  /**
   * The table 'ContactBigPhoto' name in Clone database
   */
  public static readonly CLONE_CONTACTBIGPHOTO_TABLE_NAME = 'ContactBigPhoto';

  /**
   * The table 'ContactBigPhoto' name in Clone database
   */
  public static readonly CLONE_CONTACTBIGPHOTO_MYCARD_TABLE_NAME = 'ContactBigPhoto_me';

  /**
   * The table 'block_list' name in new database
   */
  public static readonly NEW_BLOCK_LIST_TABLE_NAME = 'contact_blocklist';

  /**
   * The table 'settings' name in new database
   */
  public static readonly NEW_SETTINGS_TABLE_NAME = 'settings';

  /**
   * The table 'local_lang' name in new database
   */
  public static readonly NEW_LOCAL_LANG_TABLE_NAME = 'local_lang';

  /**
   * The table 'photo_files' name in new database
   */
  public static readonly NEW_PHOTO_FILES_TABLE_NAME = 'photo_files';

  /**
   * The table 'contact_type' name in new database
   */
  public static readonly NEW_CONTACT_TYPE_TABLE_NAME = 'contact_type';

  /**
   * The table 'merge_info' name in new database
   */
  public static readonly NEW_MERGE_INFO_TABLE_NAME = 'merge_info';

  /**
   * The table 'database_backup_task' name in new database
   */
  public static readonly NEW_DATABASE_BACKUP_TASK_TABLE_NAME = 'database_backup_task';

  /**
   * The table 'search_contact_view' name in new database
   */
  public static readonly NEW_SEARCH_CONTACT_VIEW_NAME = 'search_contact_view';
  
  /**
   * The table 'poster' name in new database
   */
  public static readonly NEW_POSTER_TABLE_NAME = 'poster';

  /**
   * The error code -1 by query account no data
   */
  public static readonly QUERY_ACCOUNT_NO_DATA = -1;

  /**
   * The error code -2 by query account error
   */
  public static readonly QUERY_ACCOUNT_FAILED = -2;

  /**
   * The query contact page number limit 2000
   */
  public static readonly QUERY_CONTACT_PAGE_NUM = 2000;

  /**
   * The query raw contact page number limit 200
   */
  public static readonly QUERY_RAW_CONTACT_PAGE_NUM = 200;

  /**
   * The query raw contact page number for migrate limit 2000
   */
  public static readonly QUERY_RAW_CONTACT_PAGE_NUM_MIGRATE = 2000;

  /**
   * The query data page number limit 50
   */
  public static readonly QUERY_DATA_PAGE_NUM = 50;

  /**
   * The query data page number limit 5
   */
  public static readonly QUERY_HMOS_DATA_PAGE_NUM = 5;

  /**
   * The query data page number for migrate limit 500
   */
  public static readonly QUERY_DATA_PAGE_NUM_MIGRATE = 500;

  /**
   * The query search page number limit 2000
   */
  public static readonly QUERY_SEARCH_PAGE_NUM = 2000;

  /**
   * The query deleted_raw_contacts page number limit 2000
   */
  public static readonly QUERY_DELETE_PAGE_NUM = 2000;

  /**
   * The query group page number limit 200
   */
  public static readonly QUERY_GROUP_PAGE_NUM = 200;

  /**
   * The query group page number for migrate limit 2000
   */
  public static readonly QUERY_GROUP_PAGE_NUM_MIGRATE = 2000;

  /**
   * The query raw contact page number limit 50
   */
  public static readonly QUERY_RAW_CONTACT_PAGE_NUM_RDB = 50;

  /**
   *Data Migration Type - Double liter single
   */
  public static readonly MIGRATE_TYPE_UPGRADE = '0';

  /**
   *Data Migration Type - Dual-to-single/local backup
   */
  public static readonly MIGRATE_TYPE_CLONE = '1';

  public static readonly CLONE_VERSION_NAME: string = '99.99.99.999';

  public static readonly LAN_STYLE_CJK = '2';

  public static readonly CLONE_ACCOUNT_TYPE = 'com.openharmony.phone';

  public static readonly CLONE_ACCOUNT_NAME = 'Phone';

  // 账号退出删除
  public static readonly CLOUD_DATA_STATUS_FIELD_LOGOUT_ACCOUNT_DELETION = 3;
  // 退出账号保留数据
  public static readonly CLOUD_DATA_STATUS_FIELD_LOGOUT_ACCOUNT_SAVE_DATA = 1;
  public static readonly CLOUD_DATA_STATUS_FIELD_NAME = 'data_status';

  public static readonly EL1_DATABASE = 'data/storage/el1/database/rdb/';

  public static readonly EL5_DATABASE = 'data/storage/el5/database/rdb/';

  public static readonly NEXT_DEFAULT: string = 'default';

  public static readonly IOS_NEXT: string = '99.99.99.997';

  /**
   * The query columns from table mimetypes
   */
  public static readonly QUERY_OLD_MIMETYPES: string[] = [
    Mimetype.OPENHARMONY_EMAIL,
    Mimetype.OPENHARMONY_IM,
    Mimetype.OPENHARMONY_NICKNAME,
    Mimetype.OPENHARMONY_ORGANIZATION,
    Mimetype.OPENHARMONY_PHONE,
    Mimetype.OPENHARMONY_SIP_ADDRESS,
    Mimetype.OPENHARMONY_NAME,
    Mimetype.OPENHARMONY_POSTAL_ADDRESS,
    Mimetype.OPENHARMONY_PHOTO,
    Mimetype.OPENHARMONY_GROUP_MEMBERSHIP,
    Mimetype.OPENHARMONY_NOTE,
    Mimetype.OPENHARMONY_CONTACT_EVENT,
    Mimetype.OPENHARMONY_WEBSITE,
    Mimetype.OPENHARMONY_RELATION,
    Mimetype.OPENHARMONY_CAMCARD
  ];

  /**
   * The query columns from table data
   */
  public static readonly QUERY_OLD_DATA: string[] = [
    Data.ID,
    Data.RAW_CONTACT_ID,
    Data.MIMETYPE_ID,
    Data.IS_PRIMARY,
    Data.DATA1,
    Data.DATA2,
    Data.DATA3,
    Data.DATA4,
    Data.DATA5,
    Data.DATA6,
    Data.DATA7,
    Data.DATA8,
    Data.DATA9,
    Data.DATA10,
    Data.DATA14,
    Data.DATA15
  ];

  /**
   * The query columns from clone table data
   */
  public static readonly QUERY_CLONE_DATA: string[] = [
    Data.RAW_CONTACT_ID,
    Data.MIMETYPE,
    Data.IS_PRIMARY,
    Data.PHOTO_URI,
    Data.DATA1,
    Data.DATA2,
    Data.DATA3,
    Data.DATA4,
    Data.DATA5,
    Data.DATA6,
    Data.DATA7,
    Data.DATA8,
    Data.DATA9,
    Data.DATA10,
    Data.DATA14,
    Data.DATA15
  ];

  /**
   * The query columns from next2next clone table data
   */
  public static readonly QUERY_NEXT2NEXT_CLONE_DATA: string[] = [
    Data.TYPE_ID,
    Data.RAW_CONTACT_ID,
    Data.READ_ONLY,
    Data.VERSION,
    Data.IS_PREFERRED_NUMBER,
    Data.DETAIL_INFO,
    Data.FAMILY_NAME,
    Data.MIDDLE_NAME_PHONETIC,
    Data.GIVEN_NAME,
    Data.GIVEN_NAME_PHONETIC,
    Data.ALIAS_DETAIL_INTO,
    Data.PHONETIC_NAME,
    Data.POSITION,
    Data.EXTEND1,
    Data.EXTEND2,
    Data.EXTEND3,
    Data.EXTEND4,
    Data.CITY,
    Data.COUNTRY,
    Data.NEIGHBORHOOD,
    Data.POBOX,
    Data.POSTCODE,
    Data.REGION,
    Data.STREET,
    Data.ALPHA_NAME,
    Data.OTHER_LAN_LAST_NAME,
    Data.OTHER_LAN_FIRST_NAME,
    Data.EXTEND5,
    Data.LAN_STYLE,
    Data.CUSTOM_DATA,
    Data.EXTEND6,
    Data.EXTEND7,
    Data.BLOB_DATA,
    Data.SYN_1,
    Data.SYN_2,
    Data.SYN_3,
    Data.EXTEND8,
    Data.EXTEND9,
    Data.EXTEND10,
    Data.EXTEND11
  ];

  /**
   * The query foundation frame type from table mimetypes
   */
  public static readonly MIMETYPE_MAP: Map<string, string> = new Map<string, string>([
    [Mimetype.OPENHARMONY_EMAIL, Mimetype.FOUNDATION_EMAIL],
    [Mimetype.OPENHARMONY_IM, Mimetype.FOUNDATION_IM],
    [Mimetype.OPENHARMONY_NICKNAME, Mimetype.FOUNDATION_NICKNAME],
    [Mimetype.OPENHARMONY_ORGANIZATION, Mimetype.FOUNDATION_ORGANIZATION],
    [Mimetype.OPENHARMONY_PHONE, Mimetype.FOUNDATION_PHONE],
    [Mimetype.OPENHARMONY_SIP_ADDRESS, Mimetype.FOUNDATION_SIP_ADDRESS],
    [Mimetype.OPENHARMONY_NAME, Mimetype.FOUNDATION_NAME],
    [Mimetype.OPENHARMONY_POSTAL_ADDRESS, Mimetype.FOUNDATION_POSTAL_ADDRESS],
    [Mimetype.OPENHARMONY_PHOTO, Mimetype.FOUNDATION_PHOTO],
    [Mimetype.OPENHARMONY_GROUP_MEMBERSHIP, Mimetype.FOUNDATION_GROUP_MEMBERSHIP],
    [Mimetype.OPENHARMONY_NOTE, Mimetype.FOUNDATION_NOTE],
    [Mimetype.OPENHARMONY_CONTACT_EVENT, Mimetype.FOUNDATION_CONTACT_EVENT],
    [Mimetype.OPENHARMONY_WEBSITE, Mimetype.FOUNDATION_WEBSITE],
    [Mimetype.OPENHARMONY_RELATION, Mimetype.FOUNDATION_RELATION],
    [Mimetype.OPENHARMONY_HICALL_DEVICE, Mimetype.FOUNDATION_HICALL_DEVICE],
    [Mimetype.OPENHARMONY_CONTACT_MISC, Mimetype.FOUNDATION_CONTACT_MISC],
    [Mimetype.OPENHARMONY_CAMCARD, Mimetype.FOUNDATION_CAMCARD]
  ]);

  /**
   * The query columns from table raw_contacts
   */
  public static readonly QUERY_RAW_COLUMNS: string[] = [
    RawContacts.ID,
    RawContacts.CONTACT_ID,
    RawContacts.SORT_KEY,
    RawContacts.SEND_TO_VOICEMAIL,
    RawContacts.CUSTOM_RINGTONE,
    RawContacts.DELETED,
    RawContacts.NOTIFICATION_TONE,
    RawContacts.DISPLAY_NAME,
    RawContacts.COMPANY,
    RawContacts.TITLE,
    RawContacts.STARRED,
    RawContacts.STARRED_ORDER,
    RawContacts.PHONETIC_NAME,
    RawContacts.PHONETIC_NAME_STYLE,
    RawContacts.RAW_CONTACT_IS_READ_ONLY,
    RawContacts.PHONEBOOK_LABEL,
    RawContacts.VIBRATION_TYPE,
    RawContacts.DIRTY,
    RawContacts.SYNC1,
    RawContacts.SYNC3
  ];

  /**
   * The query columns from table raw_contacts
   */
  public static readonly QUERY_CONTACTS_COLUMNS: string[] = [
    Contacts.ID,
    Contacts.NAME_RAW_CONTACT_ID,
    Contacts.CUSTOM_RINGTONE,
    Contacts.SEND_TO_VOICEMAIL,
    Contacts.COMPANY,
    Contacts.TITLE,
    Contacts.HAS_PHONE_NUMBER,
    Contacts.HAS_NAME,
    Contacts.HAS_EMAIL,
  ];

  /**
   * The query columns from table hwsearch_contacts
   */
  public static readonly QUERY_SEARCH_COLUMNS: string[] = [
    SearchContact.ID,
    SearchContact.ACCOUNT_ID,
    SearchContact.CONTACT_ID,
    SearchContact.RAW_CONTACT_ID,
    SearchContact.NAME,
    SearchContact.DISPLAY_NAME,
    SearchContact.PHONETIC_NAME
  ];

  /**
   * The query columns from clone table raw_contacts_tb
   */
  public static readonly QUERY_CLONE_RAW_COLUMNS: string[] = [
    RawContacts.ID,
    RawContacts.STARRED,
    RawContacts.AGGREGATION_MODE,
    RawContacts.SEND_TO_VOICEMAIL
  ];

  /**
   * The query columns from clone table raw_contacts_tb
   */
  public static readonly QUERY_NEXT2NEXT_CLONE_RAW_COLUMNS: string[] = [
    RawContacts.SINGLE_ID,
    RawContacts.CONTACT_ID,
    RawContacts.PHOTO_ID,
    RawContacts.PHOTO_FILE_ID,
    RawContacts.IS_TRANSFER_VOICEMAIL,
    RawContacts.PERSONAL_RINGTONE,
    RawContacts.IS_DELETED,
    RawContacts.PERSONAL_NOTIFICATION_RINGTONE,
    RawContacts.PHOTO_FIRST_NAME,
    RawContacts.ACCOUNT_ID,
    RawContacts.VERSION,
    RawContacts.DISPLAY_NAME,
    RawContacts.SORT,
    RawContacts.SORT_KEY,
    RawContacts.CONTACTED_COUNT,
    RawContacts.LASTEST_CONTACTED_TIME,
    RawContacts.FAVORITE,
    RawContacts.FAVORITE_ORDER,
    RawContacts.PHONETIC_NAME,
    RawContacts.PHONETIC_NAME_TYPE,
    RawContacts.COMPANY,
    RawContacts.POSITION,
    RawContacts.READ_ONLY,
    RawContacts.SORT_FIRST_LETTER,
    RawContacts.MERGE_MODE,
    RawContacts.IS_NEED_MERGE,
    RawContacts.MERGE_STATUS,
    RawContacts.IS_MERGE_TARGET,
    RawContacts.VIBRATION_SETTING,
    RawContacts.SYNC_ID,
    RawContacts.SYN_1,
    RawContacts.SYN_2,
    RawContacts.SYN_3,
    RawContacts.PRIMARY_CONTACT,
    RawContacts.EXTRA1,
    RawContacts.EXTRA2,
    RawContacts.EXTRA3,
    RawContacts.EXTRA4,
    RawContacts.DIRTY,
    RawContacts.UUID,
  ];

  public static readonly QUERY_CLONE_CONTACTBIGPHOTO: string[] = [
    ContactBigPhotoConstant.FILE_DATA,
    ContactBigPhotoConstant.RAW_CONTACTID,
    ContactBigPhotoConstant.DATA_INDEX
  ];

  /**
   * Query foundation frame event from table data
   */
  public static readonly EVENT_MAP: Map<string, string> = new Map<string, string>([
    ['1', '3'], // contact event anniversary index OpenHarmony frame is 1, foundation frame is 3
    ['2', '0'], // contact event important date index OpenHarmony frame is 2, foundation frame is 0
    ['3', '1'], // contact event birthday index OpenHarmony frame is 3, foundation frame is 1
    ['4', '2'] // contact event lunar birthday index OpenHarmony frame is 4, foundation frame is 2
  ]);

  /**
   * The query columns from table deleted_raw_contacts
   */
  public static readonly QUERY_OLD_GROUPS: string[] = [
    Group.ID,
    Group.ACCOUNT_ID,
    Group.TITLE,
    Group.NOTES,
    Group.DELETED
  ];

  /**
   * The query columns from table deleted_raw_contacts
   */
  public static readonly QUERY_OLD_DELETE_TABLE: string[] = [
    DeleteRawContact.RAW_CONTACT_ID,
    DeleteRawContact.DELETE_PACKAGE,
    DeleteRawContact.DELETE_TIME,
    DeleteRawContact.DISPLAY_NAME,
    DeleteRawContact.DELETE_BACKUP,
    DeleteRawContact.SYNC1,
    DeleteRawContact.SYNC3,
    DeleteRawContact.DIRTY,
    DeleteRawContact.BACKUP_UNSTRUCT
  ];

  /**
   * The query columns from clone table groups
   */
  public static readonly QUERY_CLONE_GROUPS: string[] = [
    Group.ID,
    Group.TITLE,
  ];

  /**
   * The table which need to filter deleted data
   */
  public static readonly FILTER_DELETED_TABLES: string[] = [
    DbConstants.OLD_DELETED_RAW_CONTACTS,
    DbConstants.OLD_RAW_CONTACTS_TABLE_NAME,
    SqlConstants.TABLE_DATA_WITH_RAW_CONTACTS
  ];

  /**
   * Simple mimetype for data info
   */
  public static readonly SIMPLE_TYPE: string[] = [
    Mimetype.FOUNDATION_EMAIL,
    Mimetype.FOUNDATION_IM,
    Mimetype.FOUNDATION_NICKNAME,
    Mimetype.FOUNDATION_ORGANIZATION,
    Mimetype.FOUNDATION_PHONE,
    Mimetype.FOUNDATION_NAME,
    Mimetype.FOUNDATION_POSTAL_ADDRESS,
    Mimetype.FOUNDATION_PHOTO,
    Mimetype.FOUNDATION_GROUP_MEMBERSHIP,
    Mimetype.FOUNDATION_NOTE,
    Mimetype.FOUNDATION_CONTACT_EVENT,
    Mimetype.FOUNDATION_WEBSITE,
    Mimetype.FOUNDATION_RELATION,
    Mimetype.FOUNDATION_CONTACT_MISC,
    Mimetype.FOUNDATION_HICALL_DEVICE,
    Mimetype.FOUNDATION_CAMCARD,
    Mimetype.FOUNDATION_SIP_ADDRESS
  ];

  /**
   * 单框架类型字典
   */
  public static readonly FOUNDATION_TYPE_ID_MAP: Record<string, number> = {
    'EMAIL': 1,
    'IM': 2,
    'NICKNAME': 3,
    'ORGANIZATION': 4,
    'PHONE': 5,
    'NAME': 6,
    'POSTAL_ADDRESS': 7,
    'PHOTO': 8,
    'GROUP_MEMBERSHIP': 9,
    'NOTE': 10,
    'CONTACT_EVENT': 11,
    'WEBSITE': 12,
    'RELATION': 13,
    'CONTACT_MISC': 14,
    'HICALL_DEVICE': 15,
    'CAMCARD': 16,
    'SIP_ADDRESS': 17,
    'POSTER':18
  };

  //手机账户ID，单框架目前只有这一个账户
  public static readonly MOBILE_ACCOUNT_ID: number = 1;

  /**
   * 收藏 值字典
   */
  public static readonly FAVORITE_VALUE_MAP: Record<string, number> = {
    'NOT_FAVORITE' : 0,
    'FAVORITE' : 1,
  }
  /**
   * 云同步上行，查询的rawContact表中的字段
   */
  public static readonly CLOUD_SYNC_UPLOAD_RAW_CONTACT_COLUMNS: string[] = [
    RawContactsColumns.NEW_ID,
    RawContacts.CONTACT_ID,
    RawContactsColumns.DISPLAY_NAME,
    RawContactsColumns.COMPANY,
    RawContactsColumns.POSITION,
    RawContactsColumns.UUID,
    RawContactsColumns.FAVORITE,
    RawContactsColumns.FAVORITE_ORDER,
    RawContactsColumns.IS_DELETED,
    RawContactsColumns.UNIQUE_KEY
  ];
  /**
   * 云同步上行，查询contactData表中的字段
   */
  public static readonly CLOUD_SYNC_UPLOAD_CONTACT_DATA_COLUMNS: string[] = [
    DataColumns.NEW_ID,
    DataColumns.TYPE_ID,
    DataColumns.RAW_CONTACT_ID,
    DataColumns.DETAIL_INFO,
    DataColumns.POSITION,
    DataColumns.CITY,
    DataColumns.COUNTRY,
    DataColumns.NEIGHBORHOOD,
    DataColumns.POBOX,
    DataColumns.POSTCODE,
    DataColumns.REGION,
    DataColumns.STREET,
    DataColumns.CUSTOM_DATA,
    DataColumns.EXTEND7,
    DataColumns.BLOB_DATA,
    DataColumns.EXTEND5
  ];

  /**
   * 将标志映射为操作类型整数
   */
  public static readonly BI_OPERATE_MAP: Map<string, number> = new Map<string, number>([
    [DbConstants.NEXT_DEFAULT, 0], // 单到单&云备份恢复
    ['99.99.99.999', 1], // 双克隆到单
    ['99.99.99.997', 2], // IOS克隆到单
    ['99.99.99.998', 3], // 三方到单
    ['0.0.0.0', 4], // 双升单
    ['5', 5], // 隐私空间迁移
    ['6', 6], // 每日上报
    ['0', 7], // 其他
  ]);

  /**
   * All sql to create tables for contacts.db
   */
  public static readonly CREATE_CONTACT_TABLE_MAP: Map<string, string> = new Map<string, string>([
    [DbConstants.NEW_CONTACTS_TABLE_NAME, SqlConstants.CREATE_CONTACT],
    [DbConstants.NEW_RAW_CONTACT_TABLE_NAME, SqlConstants.CREATE_RAW_CONTACT],
    [DbConstants.NEW_DATA_TABLE_NAME, SqlConstants.CREATE_CONTACT_DATA],
    [DbConstants.NEW_LOCAL_LANG_TABLE_NAME, SqlConstants.CREATE_LOCAL_LANG],
    [DbConstants.NEW_ACCOUNT_TABLE_NAME, SqlConstants.CREATE_ACCOUNT],
    [DbConstants.NEW_BLOCK_LIST_TABLE_NAME, SqlConstants.CREATE_CONTACT_BLOCKLIST],
    [DbConstants.NEW_PHOTO_FILES_TABLE_NAME ,SqlConstants.CREATE_PHOTO_FILES],
    [DbConstants.NEW_CONTACT_TYPE_TABLE_NAME, SqlConstants.CREATE_CONTACT_TYPE],
    [DbConstants.OLD_GROUPS_TABLE_NAME, SqlConstants.CREATE_GROUPS],
    [DbConstants.NEW_DELETE_RAW_CONTACT_TABLE_NAME, SqlConstants.CREATE_DELETED_RAW_CONTACT],
    [DbConstants.NEW_SEARCH_TABLE_NAME, SqlConstants.CREATE_SEARCH_CONTACT],
    [DbConstants.NEW_MERGE_INFO_TABLE_NAME, SqlConstants.MERGE_INFO],
    [DbConstants.NEW_DATABASE_BACKUP_TASK_TABLE_NAME, SqlConstants.CREATE_DATABASE_BACKUP_TASK],
    [DbConstants.CLOUD_RAW_CONTACT_TABLE_NAME, SqlConstants.CREATE_CLOUD_RAW_CONTACT],
    [DbConstants.CLOUD_GROUPS_TABLE_NAME, SqlConstants.CREATE_CLOUD_GROUPS],
    [DbConstants.NEW_SETTINGS_TABLE_NAME, SqlConstants.CREATE_SETTINGS],
    [DbConstants.NEW_HW_ACCOUNT_TABLE_NAME, SqlConstants.CREATE_CONTACT_HW_ACCOUNT],
    [DbConstants.CLOUD_CONTACT_BLOCKLIST_TABLE_NAME, SqlConstants.CREATE_CLOUD_CONTACT_BLOCKLIST],
    [DbConstants.NEW_PRIVACY_CONTACTS_BACKUP_TABLE_NAME, SqlConstants.CREATE_PRIVACY_CONTACTS_BACKUP],
    [DbConstants.NEW_POSTER_TABLE_NAME, SqlConstants.CREATE_POSTER],
  ]);

  /**
   * All sql to create index or trigger for contacts.db
   */
  public static readonly CREATE_CONTACT_TABLE_RELATIVE: string[] = [
    SqlConstants.CREATE_RAW_CONTACT_INDEX_UNIQUE_KEY,
    SqlConstants.CREATE_SEARCH_CONTACT_INDEX1,
    SqlConstants.CREATE_SEARCH_CONTACT_INDEX2,
    SqlConstants.CREATE_CONTACT_INDEX_DATA1,
    SqlConstants.CREATE_CONTACT_INDEX,
    SqlConstants.CREATE_CONTACT_INDEX_DATA2,
    SqlConstants.CREATE_RAW_CONTACT_INDEX,
    SqlConstants.CREATE_RAW_CONTACT_INDEX_SORT,
    SqlConstants.UPDATE_RAW_CONTACT_VERSION,
    SqlConstants.INSERT_CONTACT_QUICK_SEARCH,
    SqlConstants.CREATE_INSERT_BACKUP_TIME,
    SqlConstants.UPDATE_CONTACT_BY_INSERT_CONTACT_DATA,
    SqlConstants.UPDATE_CONTACT_BY_DELETE_CONTACT_DATA,
    SqlConstants.UPDATE_CONTACT_BY_UPDATE_CONTACT_DATA,
    SqlConstants.MERGE_INFO_INDEX,
    SqlConstants.INIT_CHANGE_TIME,
    SqlConstants.CREATE_CONTACT_BLOCKLIST_INDEX_PHONE
  ];

  /**
   * All sql to create view tables for contacts.db
   */
  public static readonly CREATE_CONTACT_VIEW_MAP: Map<string, string> = new Map<string, string>([
    [DbConstants.NEW_SEARCH_CONTACT_VIEW_NAME, SqlConstants.CREATE_SEARCH_CONTACT_VIEW],
    [DbConstants.VIEW_CONTACT_DATA_NAME, SqlConstants.CREATE_VIEW_CONTACT_DATA],
    [DbConstants.VIEW_RAW_CONTACT_NAME, SqlConstants.CREATE_VIEW_RAW_CONTACT],
    [DbConstants.VIEW_CONTACT_NAME, SqlConstants.CREATE_VIEW_CONTACT],
    [DbConstants.VIEW_CONTACT_LOCATION_NAME, SqlConstants.CREATE_VIEW_CONTACT_LOCATION],
    [DbConstants.VIEW_GROUPS_NAME, SqlConstants.CREATE_VIEW_GROUPS],
    [DbConstants.VIEW_DELETED_NAME, SqlConstants.CREATE_VIEW_DELETED]
  ]);
}

export enum DIRTY_ENUM {
  // 不需要上云的联系人
  NOT_DIRTY = 0,
  // 有变更，需要上云，但是不需要重新计算 uniqueKey 的联系人，上云结束后设置为 NOT_DIRTY：0
  NO_NEED_RECALC = 1,
  // 有变更，需要上云，且需要重新计算 uniqueKey 的联系人，计算完 uniqueKey 以后，会设置为 NO_NEED_RECALC
  NEED_RECALC = 2,
  // 有变更，需要重新计算 uniqueKey
  REFRESH_UNIQUE_KEY = 3,
}

export enum IsMyCard {
  // 表示该联系人是我的名片
  YES = 1,
}

export enum EVENT_LABEL_ID {
  // 生日
  BIRTHDAY = 1,
  // 农历生日
  LUNAR_BIRTHDAY = 2,
  // 周年纪念
  ANNIVERSARIES = 3,
  // 其他重要日期
  OTHER = 0
}