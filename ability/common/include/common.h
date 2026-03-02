/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <set>

#include "contacts_path.h"

namespace OHOS {
namespace Contacts {
enum class CallLogType {
    /**
     * The APP_KEY2 is destroyed due to screen lock. The database directory is moved to class C.
     */
    C_CallLogType,
    /**
     * Screen unlock causes APP_KEY2 creation, move database directory to class E
     */
    E_CallLogType
};

enum class ExecuteResult {
   FAIL = -1,
   SUCCESS = 0,
};

enum class ContactDbInfo {
   DB_UPGRADE_BEFORE = 1,
   DB_UPGRADE_AFTER = 2,
   DB_OPEN = 3,
   DB_CREATE = 4,
   DB_TABLES_CHECK = 5,
   DB_COLUMNS_CHECK = 6,
};

// error code
constexpr int RDB_EXECUTE_OK = 0;
constexpr int RDB_EXECUTE_FAIL = -1;
constexpr int RDB_PERMISSION_ERROR = -2;
constexpr int OPERATION_ERROR = -1;
constexpr int OPEN_FILE_FAILED = -1;
constexpr int OPERATION_OK = 0;
constexpr int RDB_OBJECT_EMPTY = -1;
constexpr int PARAMETER_EMPTY = -1;
constexpr int DELETE_MARK = 1;
constexpr int NOT_DELETE_MARK = 0;
constexpr int ID_EMPTY = 0;
constexpr int BATCH_INSERT_COUNT = 40;

// ResultSet get Num
constexpr int RESULT_GET_ONE = 1;
constexpr int RESULT_GET_TWO = 2;
constexpr int RESULT_GET_THERR = 3;
constexpr int RESULT_GET_FOUR = 4;

// contact table has Judge
constexpr int HAS_NAME = 1;
constexpr int HAS_PHONE = 2;
constexpr int HAS_EMAIL = 3;
constexpr int HAS_GROUP = 4;

// result index
constexpr int INDEX_ZERO = 0;
constexpr int INDEX_ONE = 1;
constexpr int INDEX_TWO = 2;
constexpr int INDEX_THREE = 3;
constexpr int INDEX_FOUR = 4;

// MAIN_SPACE_SYNC_FLAG 一个数字，转为二进制后：11，第一位表示需要同步，第二位表示是否有生日信息，第三位是否同步大头像
// 隐私空间操作联系人迁移，联系人从主空间迁出或迁入，无法处理日历信息
// 如果迁移联系人有生日信息，需要做一次全量同步
constexpr int MAIN_SPACE_SYNC_FLAG_HAS_BIRTH = 3;
// 隐私空间操作联系人迁移，联系人从主空间迁出或迁入，无法触发云同步，此标记表示需要触发次云同步
constexpr int MAIN_SPACE_SYNC_FLAG_NOT_HAS_BIRTH = 1;
// 同步标记默认值，为0
constexpr int MAIN_SPACE_SYNC_DEFAULT = 0;
constexpr int MAIN_SPACE_NEED_SYNC_BIT = 0;
constexpr int MAIN_SPACE_NEED_SYNC_BIRTH_BIT = 1;
constexpr int MAIN_SPACE_NEED_SYNC_BIG_PHOTO_BIT = 2;

// displayName从其他信息生成的标记
constexpr int DISPLAY_NAME_GENERATE_FROM_OTHER_INFO_FLAG = 1;

constexpr int POSTER_BATCH_DOWNLOAD_CODE = 3004; // 海报下载接口——批量下载

// Contacts DataBase Code
constexpr int CONTACTS_CONTACT = 10000;
constexpr int CONTACTS_RAW_CONTACT = 10001;
constexpr int CONTACTS_CONTACT_DATA = 10002;
constexpr int CONTACTS_CONTACT_TYPE = 10003;
constexpr int CONTACTS_DELETED_RAW_CONTACT = 10004;
constexpr int CONTACTS_GROUPS = 10005;
constexpr int CONTACTS_ACCOUNT = 10006;
constexpr int CONTACTS_BLOCKLIST = 10007;
constexpr int CONTACTS_SEARCH_CONTACT = 10008;
constexpr int CONTACTS_SYNC_STATUS = 10009;
constexpr int CONTACTS_PHOTO_FILES = 100010;
constexpr int CONTACTS_LOCAL_LANG = 10011;
constexpr int CONTACTS_DATABASE_BACKUP_TASK = 10012;
constexpr int CONTACTS_DELETE = 10013;
constexpr int QUERY_MERGE_LIST = 10014;
constexpr int SPLIT_CONTACT = 10015;
constexpr int MANUAL_MERGE = 10016;
constexpr int AUTO_MERGE = 10017;
constexpr int CONTACTS_DELETE_RECORD = 10018;
constexpr int CONTACT_TYPE = 10019;
constexpr int CONTACT_BACKUP = 10020;
constexpr int CONTACT_RECOVER = 10021;
constexpr int CONTACTS_SETTINGS = 10022;
constexpr int ACCOUNT = 10020;
constexpr int CONTACTS_COMPANY_CLASSIFY = 10023; // 查询有单位信息联系人分组
constexpr int QUERY_COUNT_WITHOUT_COMPANY = 10024; // 查询无单位信息联系人分组
constexpr int CONTACTS_LOCATION_CLASSIFY = 10025; // 查询有归属地信息联系人分组
constexpr int QUERY_COUNT_WITHOUT_LOCATION = 10026; // 查询无归属地信息联系人分组
constexpr int CONTACTS_FREQUENT_CLASSIFY = 10027; // 查询最近联系人分组
constexpr int CONTACTS_CONTACT_LOCATION = 10028; // 根据归属地查询联系人
constexpr int CONTACTS_DETECT_REPAIR = 10029; // 提供给外部检测工具用于查询15天内的回收站表
constexpr int CONTACTS_POSTER = 10030; // poster table
constexpr int CONTACTS_DOWNLOAD_POSTERS = 10031; // 下载联系人海报中转用
constexpr int CONTACTS_ADD_FAILED_DELETE = 10032;
constexpr int CALLLOG = 20000;
constexpr int VOICEMAIL = 20001;
constexpr int REPLAYING = 20002;
constexpr int RECYCLE_RESTORE = 20003;
constexpr int CONTACTS_DELETE_MERGE = 20004;
constexpr int CONTACTS_PLACE_HOLDER = 20005;
// 双升单数据恢复
constexpr int RESTORE_CONTACT_BY_DOUBLE_TO_SINGLE_UPGRADE_DATA = 20006;

// 联系人一键恢复时，（1）在drop了本地表后，全搜需要清空搜索库的对应表（2）在新建了本地表后，需要重新建立触发器
constexpr int CLEAR_AND_RECREATE_TRIGGER_SEARCH_TABLE = 20007;

constexpr int SPLIT_AGGREGATION_CONTACT = 20008;

//黑名单迁移状态
constexpr int BLOCKLIST_MIGRATE_FAILED = 0;
constexpr int BLOCKLIST_MIGRATE_SUCCESS = 1;

// PROFILE DATABASE CODE
constexpr int PROFILE_CONTACT = 30000;
constexpr int PROFILE_RAW_CONTACT = 30001;
constexpr int PROFILE_CONTACT_DATA = 30002;
constexpr int PROFILE_CONTACT_TYPE = 30003;
constexpr int PROFILE_DELETED_RAW_CONTACT = 30004;
constexpr int PROFILE_GROUPS = 30005;
constexpr int PROFILE_ACCOUNT = 30006;
constexpr int PROFILE_BLOCKLIST = 30007;
constexpr int PROFILE_SEARCH_CONTACT = 30008;
constexpr int PROFILE_SYNC_STATUS = 30009;
constexpr int PROFILE_PHOTO_FILES = 300010;
constexpr int PROFILE_LOCAL_LANG = 30011;
constexpr int PROFILE_DATABASE_BACKUP_TASK = 30012;
constexpr int PROFILE_DELETE_RECORD = 30013;
constexpr int PROFILE_TYPE = 30014;
constexpr int PROFILE_DELETE = 30015;
constexpr int PROFILE_BACKUP = 30016;
constexpr int PROFILE_RECOVER = 30017;

// Cloud DATABASE CODE
constexpr int CLOUD_DATA = 40000;

// Pull down to synchronize contacts
constexpr int PULLDOWN_SYNC_CONTACT_DATA = 40001;

// Queries UUIDs only in the cloud table.
constexpr int QUERY_UUID_NOT_IN_RAW_CONTACT = 40002;

// Queries setting table is refresh contact.
constexpr int QUERY_IS_REFRESH_CONTACT = 40003;

// Upload data to cloud.40017
constexpr int UPLOAD_DATA_TO_CLOUD = 40004;

// Queries to start backup restore
constexpr int BACKUP_RESTORE = 40005;

// Update contact from cloud
constexpr int TRIGGER_UPLOAD_CLOUD = 40006;
constexpr int QUERY_BIG_LENGTH_NAME = 40008;
constexpr int HW_ACCOUNT = 40009;
constexpr int MERGE_CONTACT = 40010;
constexpr int SET_FULL_SEARCH_SWITCH_OPEN = 40011;
constexpr int SET_FULL_SEARCH_SWITCH_CLOSE = 40012;
constexpr int SYNC_BIRTHDAY_TO_CALENDAR = 40013;
constexpr int REBUILD_CONTACT_SERACH_INDEX = 40014;
constexpr int QUERY_CONTACT_COUNT_DOUBLE_DB = 40015;
// 隐私空间迁移数据，批量插入接口
constexpr int ADD_CONTACT_INFO_BATCH = 40016;
// 隐私空间迁移数据，硬删除联系人接口
constexpr int CONTACTS_HARD_DELETE = 40017;
// 克隆完成之后更新通话记录黄页信息
constexpr int UPDATE_YELLOW_PAGE_INFO_FOR_CALLLOG = 40018;
constexpr int PRIVACY_CONTACTS_BACKUP = 40019;
constexpr int RBD_STORE_RETRY_REQUEST_WITH_SLEEP_TIMES = 100;
const std::set<int> RBD_STORE_RETRY_REQUEST_WITH_SLEEP = {
    27394082,  // 数据库繁忙，重试可能恢复
    27394110,  // 内存申请失败，重试可能恢复
    27394112,  // IO访问失败，重试可能恢复
    27394108,  // 数据库正在被其他进程写，阻塞本进程写入，延时重试可恢复
    27394109,  // 数据库正在被其他线程写，阻塞本线程写入，延时重试可恢复
    27394100   // 磁盘满，延时重试可能恢复
};

// DATABASE VERSION 2
constexpr int DATABASE_VERSION_2 = 2;

// DATABASE VERSION 3
constexpr int DATABASE_VERSION_3 = 3;

// DATABASE VERSION 4
constexpr int DATABASE_VERSION_4 = 4;

// DATABASE VERSION 5
constexpr int DATABASE_VERSION_5 = 5;

// DATABASE VERSION 6
constexpr int DATABASE_VERSION_6 = 6;

// DATABASE VERSION 7
constexpr int DATABASE_VERSION_7 = 7;

// DATABASE VERSION 8
constexpr int DATABASE_VERSION_8 = 8;

// DATABASE VERSION 9
constexpr int DATABASE_VERSION_9 = 9;

// DATABASE VERSION 10
constexpr int DATABASE_VERSION_10 = 10;

// DATABASE VERSION 11
constexpr int DATABASE_VERSION_11 = 11;

// DATABASE VERSION 12
constexpr int DATABASE_VERSION_12 = 12;

// DATABASE VERSION 13
constexpr int DATABASE_VERSION_13 = 13;

// DATABASE VERSION 14
constexpr int DATABASE_VERSION_14 = 14;

// DATABASE VERSION 15
constexpr int DATABASE_VERSION_15 = 15;

// DATABASE VERSION 16
constexpr int DATABASE_VERSION_16 = 16;

// DATABASE VERSION 17
constexpr int DATABASE_VERSION_17 = 17;

// DATABASE VERSION 18
constexpr int DATABASE_VERSION_18 = 18;

// DATABASE VERSION 19
constexpr int DATABASE_VERSION_19 = 19;

// DATABASE VERSION 20
constexpr int DATABASE_VERSION_20 = 20;

// DATABASE VERSION 21
constexpr int DATABASE_VERSION_21 = 21;

// DATABASE VERSION 22
constexpr int DATABASE_VERSION_22 = 22;
 
// DATABASE VERSION 23
constexpr int DATABASE_VERSION_23 = 23;

// DATABASE VERSION 24
constexpr int DATABASE_VERSION_24 = 24;

// DATABASE VERSION 25
constexpr int DATABASE_VERSION_25 = 25;

// DATABASE VERSION 26
constexpr int DATABASE_VERSION_26 = 26;

// DATABASE VERSION 27
constexpr int DATABASE_VERSION_27 = 27;

// DATABASE VERSION 28
constexpr int DATABASE_VERSION_28 = 28;

// DATABASE VERSION 29
constexpr int DATABASE_VERSION_29 = 29;

// DATABASE VERSION 30
constexpr int DATABASE_VERSION_30 = 30;

// DATABASE VERSION 31
constexpr int DATABASE_VERSION_31 = 31;

// DATABASE VERSION 32
constexpr int DATABASE_VERSION_32 = 32;

// DATABASE VERSION 33
constexpr int DATABASE_VERSION_33 = 33;

// DATABASE VERSION 34, 迁移隐私空间联系人需求，更新contactdata视图
constexpr int DATABASE_VERSION_34 = 34;

// DATABASE VERSION 35
constexpr int DATABASE_VERSION_35 = 35;

// DATABASE VERSION 36
constexpr int DATABASE_VERSION_36 = 36;

// DATABASE VERSION 37
constexpr int DATABASE_VERSION_37 = 37;

// DATABASE VERSION 38
constexpr int DATABASE_VERSION_38 = 38;

// DATABASE VERSION 39
constexpr int DATABASE_VERSION_39 = 39;

// DATABASE VERSION 40
constexpr int DATABASE_VERSION_40 = 40;

// DATABASE VERSION 41
constexpr int DATABASE_VERSION_41 = 41;

// DATABASE VERSION 42
constexpr int DATABASE_VERSION_42 = 42;

// DATABASE OPEN VERSION CONTACTS
constexpr int DATABASE_CONTACTS_OPEN_VERSION = 42;

// DATABASE OPEN VERSION CallLog
constexpr int DATABASE_CALL_LOG_OPEN_VERSION = 27;

// DATABASE OPEN VERSION Blocklist
constexpr int DATABASE_BLOCKLIST_OPEN_VERSION = 1;

// DATABASE NEW VERSION
constexpr int DATABASE_NEW_VERSION = 2;

constexpr int TYPE_ID_PHOTO = 8;

constexpr int TYPE_ID_GROUP = 9;

constexpr int TYPE_ID_POSTER = 18;

constexpr int PHOTO_TYPE_BIG = 3;

constexpr int PHOTO_TYPE_SMALL = 2;

// REQUEST PARAMS ARGS NUM
constexpr int REQUEST_PARAMS_NUM = 2;
constexpr const char *PROFILE_DATABASE_NAME = "profile";
constexpr const char *CONTACT_DATABASE_NAME = "contacts";

// 增删改操作
constexpr const char *OPERATE_TYPE_INSERT = "insert";
constexpr const char *OPERATE_TYPE_UPDATE = "update";
constexpr const char *OPERATE_TYPE_DELETE = "delete";
constexpr const char *CHANGE_ID_MSG_COLUMN_NAME = "idsStr";
// 通知类型，未更改，可能根据条件更新或删除操作，更新数量为0
constexpr const char *NOTIFY_MODIFY_TYPE_NOT_MODIFY = "NOT_MODIFY";

// Rebase
constexpr int REBASE_SETTING = 0;
constexpr int REBASE_COUNT = 5;

// MergeMode Number
constexpr int MERGE_MODE_DEFAULT = 0;
constexpr int MERGE_MODE_MANUAL = 1;
constexpr int MERGE_MODE_AUTO = 2;

constexpr const char *CONTACT_URI = "datashare:///com.ohos.contactsdataability";
constexpr const char *VOICEMAIL_URI = "datashare:///com.ohos.voicemailability";
constexpr const char *CALL_LOG_URI = "datashare:///com.ohos.calllogability";
constexpr const char *CONTACT_CHANGE_URI = "datashare:///com.ohos.contactsdataability/contacts/contact_data";
constexpr const char *ADD_CONTACT_INFO_BATCH_URI =
    "datashare:///com.ohos.contactsdataability/contacts/add_contact_batch";
constexpr const char *CONTACTS_HARD_DELETE_URI =
    "datashare:///com.ohos.contactsdataability/contacts/contacts_hard_delete";

// 无名氏首字母 ...
constexpr const char *ANONYMOUS_SORT_FIRST_LETTER = "...";
// 无名氏排序数字，无名氏信息需要排到字母的后边
// 如果是字母最大是z，对应90，升序排列，无名氏需要在90后边
// 排序字段存99
constexpr const char *ANONYMOUS_SORT = "99";
constexpr const char *LATIN_TO_ASCII_ICU_TRANSLITE_ID = "Latin-Ascii";
constexpr const char *HANZI_TO_PINYIN_ICU_TRANSLITE_ID = "Han-Latin/Names; Latin-Ascii";

constexpr const char *ACCOUNT_LOGOUT_DEL_CONTACT_NOTIFICATION = "DELETE#ALL_CLOUDDATA";

constexpr const char *CREATE_SEARCH_CONTACT_VIEW =
    "CREATE VIEW IF NOT EXISTS [search_contact_view] AS SELECT "
    "[search_contact].[id] AS [id], "
    "[search_contact].[account_id] AS [account_id], "
    "[search_contact].[contact_id] AS [contact_id], "
    "[search_contact].[raw_contact_id] AS [raw_contact_id], "
    "[search_contact].[search_name] AS [search_name], "
    "[search_contact].[photo_id] AS [photo_id], "
    "[search_contact].[photo_file_id] AS [photo_file_id], "
    "[contact_type].[content_type] AS [content_type], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type], "
    "[account].[data_info] AS [data_info], "
    "[groups].[group_name] AS [group_name], "
    "[groups].[group_notes] AS [group_notes], "
    "[groups].[is_deleted] AS [group_is_deleted], "
    "[groups].[group_ringtone] AS [group_ringtone], "
    "[groups].[ringtone_modify_time] AS [ringtone_modify_time], "
    "[groups].[lastest_modify_time] AS [lastest_modify_time], "
    "[photo_files].[file_size] AS [file_size], "
    "[photo_files].[file_height] AS [file_height], "
    "[photo_files].[file_width] AS [file_width], "
    "[contact].[personal_ringtone] AS [personal_ringtone], "
    "[contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[contact].[personal_notification_ringtone] AS "
    "[personal_notification_ringtone], "
    "[contact].[has_phone_number] AS [has_phone_number], "
    "[contact].[has_display_name] AS [has_display_name], "
    "[contact].[has_email] AS [has_email], "
    "[contact].[has_group] AS [has_group], "
    "[raw_contact].[contacted_count] AS [contacted_count], "
    "[raw_contact].[favorite] AS [favorite], "
    "[raw_contact].[favorite_order] AS [favorite_order], "
    "[raw_contact].[display_name] AS [display_name], "
    "[raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], "
    "[raw_contact].[sort_first_letter] AS [sort_first_letter], "
    "[raw_contact].[sort] AS [sort], "
    "[raw_contact].[photo_first_name] AS [photo_first_name], "
    "[raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], "
    "[raw_contact].[is_deleted] AS [is_deleted], "
    "[raw_contact].[primary_contact] AS [primary_contact], "
    "[raw_contact].[extra1] AS [extra1], "
    "[raw_contact].[extra2] AS [extra2], "
    "[raw_contact].[extra3] AS [extra3], "
    "[raw_contact].[extra4] AS [extra4], "
    "[contact_data].[type_id] AS [type_id], "
    "[contact_data].[phonetic_name] AS [phonetic_name], "
    "[contact_data].[raw_contact_id] AS [raw_contact_id], "
    "[contact_data].[read_only] AS [read_only], "
    "[contact_data].[version] AS [version], "
    "[contact_data].[alias_detail_info] AS [alias_detail_info], "
    "[contact_data].[is_preferred_number] AS [is_preferred_number], "
    "[contact_data].[detail_info] AS [detail_info], "
    "[contact_data].[city] AS [city], "
    "[contact_data].[position] AS [position], "
    "[contact_data].[middle_name_phonetic] AS [middle_name_phonetic], "
    "[contact_data].[given_name] AS [given_name], "
    "[contact_data].[family_name] AS [family_name], "
    "[contact_data].[given_name_phonetic] AS [given_name_phonetic], "
    "[contact_data].[country] AS [country], "
    "[contact_data].[neighborhood] AS [neighborhood], "
    "[contact_data].[pobox] AS [pobox], "
    "[contact_data].[postcode] AS [postcode], "
    "[contact_data].[region] AS [region], "
    "[contact_data].[street] AS [street], "
    "[contact_data].[extend1] AS [extend1], "
    "[contact_data].[extend2] AS [extend2], "
    "[contact_data].[extend3] AS [extend3], "
    "[contact_data].[extend4] AS [extend4], "
    "[contact_data].[alpha_name] AS [alpha_name], "
    "[contact_data].[other_lan_last_name] AS [other_lan_last_name], "
    "[contact_data].[other_lan_first_name] AS [other_lan_first_name], "
    "[contact_data].[extend5] AS [extend5], "
    "[contact_data].[lan_style] AS [lan_style], "
    "[contact_data].[custom_data] AS [custom_data], "
    "[contact_data].[extend6] AS [extend6], "
    "[contact_data].[extend7] AS [extend7], "
    "[contact_data].[blob_data] AS [blob_data], "
    "[contact_data].[syn_1] AS [syn_1], "
    "[contact_data].[syn_2] AS [syn_2], "
    "[contact_data].[syn_3] AS [syn_3], "
    "[contact_data].[extend8] AS [extend8], "
    "[contact_data].[extend9] AS [extend9], "
    "[contact_data].[extend10] AS [extend10], "
    "[contact_data].[extend11] AS [extend11] "
    "FROM [search_contact] "
    "JOIN [raw_contact] ON "
    "([search_contact].[raw_contact_id] = [raw_contact].[id]) "
    "JOIN [contact] ON "
    "([contact].[id] = [raw_contact].[contact_id]) "
    "JOIN [raw_contact] AS [name_raw_contact] ON "
    "([name_raw_contact_id] = [name_raw_contact].[id]) "
    "LEFT JOIN [account] ON "
    "([search_contact].[account_id] = [account].[id]) "
    "LEFT JOIN [contact_data] ON "
    "([contact_data].[raw_contact_id] = [raw_contact].[id]) "
    "LEFT JOIN [contact_type] ON "
    "([contact_data].[type_id] = [contact_type].[id]) "
    "LEFT JOIN [groups] ON "
    "([contact_type].[content_type] = 'group_membership' "
    "AND [groups].[id] = [contact_data].[detail_info]) "
    "LEFT JOIN [photo_files] ON "
    "([contact_type].[content_type] = 'photo' "
    "AND [photo_files].[id] = [search_contact].[photo_file_id])";

constexpr const char *CREATE_VIEW_DELETED =
    "CREATE VIEW IF NOT EXISTS [view_deleted] "
    "AS  SELECT "
    "[deleted_raw_contact].[id] AS [id], "
    "[contact].[id] AS [contact_id], "
    "[contact].[photo_id] AS [photo_id], "
    "[contact].[photo_file_id] AS [photo_file_id], "
    "[contact].[quick_search_key] AS [quick_search_key], "
    "[contact].[has_phone_number] AS [has_phone_number], "
    "[contact].[has_display_name] AS [has_display_name], "
    "[contact].[has_email] AS [has_email], "
    "[contact].[has_group] AS [has_group], "
    "[name_raw_contact].[account_id] AS [account_id], "
    "[name_raw_contact].[id] AS [raw_contact_id], "
    "[name_raw_contact].[is_transfer_voicemail] AS [is_transfer_voicemail], "
    "[name_raw_contact].[personal_ringtone] AS [personal_ringtone], "
    "[name_raw_contact].[is_deleted] AS [is_deleted], "
    "[name_raw_contact].[version] AS [version], "
    "[name_raw_contact].[display_name] AS [display_name], "
    "[name_raw_contact].[sort] AS [sort], "
    "[name_raw_contact].[contacted_count] AS [contacted_count], "
    "[name_raw_contact].[lastest_contacted_time] AS [lastest_contacted_time], "
    "[name_raw_contact].[favorite] AS [favorite], "
    "[name_raw_contact].[favorite_order] AS [favorite_order], "
    "[name_raw_contact].[personal_notification_ringtone] AS [personal_notification_ringtone], "
    "[name_raw_contact].[phonetic_name] AS [phonetic_name], "
    "[name_raw_contact].[phonetic_name_type] AS [phonetic_name_type], "
    "[name_raw_contact].[company] AS [company], "
    "[name_raw_contact].[position] AS [position], "
    "[name_raw_contact].[read_only] AS [read_only], "
    "[name_raw_contact].[sort_first_letter] AS [sort_first_letter], "
    "[name_raw_contact].[merge_mode] AS [merge_mode], "
    "[name_raw_contact].[is_need_merge] AS [is_need_merge], "
    "[name_raw_contact].[merge_status] AS [merge_status], "
    "[name_raw_contact].[is_merge_target] AS [is_merge_target], "
    "[name_raw_contact].[vibration_setting] AS [vibration_setting], "
    "[name_raw_contact].[photo_first_name] AS [photo_first_name], "
    "[name_raw_contact].[sync_id] AS [sync_id], "
    "[name_raw_contact].[syn_1] AS [syn_1], "
    "[name_raw_contact].[syn_2] AS [syn_2], "
    "[name_raw_contact].[syn_3] AS [syn_3], "
    "[raw_contact].[primary_contact] AS [primary_contact], "
    "[raw_contact].[extra1] AS [extra1], "
    "[raw_contact].[extra2] AS [extra2], "
    "[raw_contact].[extra3] AS [extra3], "
    "[raw_contact].[extra4] AS [extra4], "
    "[deleted_raw_contact].[delete_source] AS [delete_source],"
    "[deleted_raw_contact].[delete_time] AS [delete_time], "
    "[deleted_raw_contact].[delete_account] AS [delete_account], "
    "[deleted_raw_contact].[backup_data] AS [backup_data], "
    "[account].[account_name] AS [account_name], "
    "[account].[account_type] AS [account_type], "
    "[photo_files].[file_size] AS [file_size], "
    "[photo_files].[file_height] AS [file_height], "
    "[photo_files].[file_width] AS [file_width] "
    "FROM   [deleted_raw_contact] "
    "LEFT JOIN [contact] ON ([contact].[id] = [deleted_raw_contact].[contact_id]) "
    "LEFT JOIN [raw_contact] AS [name_raw_contact] ON ([name_raw_contact].[id] = "
    "[deleted_raw_contact].[raw_contact_id] "
    "AND [name_raw_contact].[is_deleted] = 1)"
    "LEFT JOIN [account] ON ([name_raw_contact].[account_id] = [account].[id]) "
    "LEFT JOIN [photo_files] ON ([photo_files].[id] = [name_raw_contact].[photo_file_id]) ";

constexpr const char *UPDATE_CONTACT_BY_DELETE_CONTACT_DATA =
    "CREATE TRIGGER IF NOT EXISTS [update_contact_by_delete_contact_data] AFTER DELETE ON [contact_data] FOR EACH ROW "
    "BEGIN "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_display_name] = 0 "
    "WHERE "
    "[OLD].[raw_contact_id] = [name_raw_contact_id] "
    "AND [OLD].[type_id] = 6; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_email] = 0 "
    "WHERE "
    "[OLD].[raw_contact_id] = [name_raw_contact_id] "
    "AND [OLD].[type_id] = 1; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_group] = 0 "
    "WHERE "
    "[OLD].[raw_contact_id] = [name_raw_contact_id] "
    "AND [OLD].[type_id] = 9; "
    "UPDATE "
    "[contact] "
    "SET "
    "[has_phone_number] = 0 "
    "WHERE "
    "[OLD].[raw_contact_id] = [name_raw_contact_id] "
    "AND [OLD].[type_id] = 5; "
    "END";

constexpr const char *UPDATE_CONTACT_BY_UPDATE_CONTACT_DATA =
    "CREATE TRIGGER IF NOT EXISTS [update_contact_by_update_contact_data] AFTER UPDATE ON [contact_data] FOR EACH ROW "
    "BEGIN "
    "UPDATE contact "
    "SET "
    "has_display_name = CASE WHEN NEW.type_id = 6 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_display_name END,"
    "has_email = CASE WHEN NEW.type_id = 1 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_email END,"
    "has_group = CASE WHEN NEW.type_id = 9 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_group END,"
    "has_phone_number = CASE WHEN NEW.type_id = 5 AND NEW.detail_info IS NOT NULL THEN 1 ELSE has_phone_number END "
    "WHERE NEW.raw_contact_id = name_raw_contact_id;"
    "END";

constexpr const char *UPDATE_RAW_CONTACT_VERSION =
    "CREATE TRIGGER IF NOT EXISTS [update_raw_contact_version] AFTER UPDATE ON [raw_contact] "
    "BEGIN "
    "UPDATE "
    "[raw_contact] "
    "SET "
    "[version] = [OLD].[version] + 1 "
    "WHERE "
    "[id] = [OLD].[id] AND [NEW].[is_deleted] != [OLD].[is_deleted]; "
    "END";

constexpr const char *UPDATE_CONTACT_DATA_VERSION =
    "CREATE TRIGGER IF NOT EXISTS [update_contact_data_version] AFTER UPDATE ON [contact_data] "
    "BEGIN "
    "IF UPDATE "
    "[version] RETURN; "
    "UPDATE "
    "[contact_data] "
    "SET "
    "[version] = [OLD].[version] + 1 "
    "WHERE "
    "[id] = [OLD].[id]; "
    "END";

constexpr const char *INSERT_CONTACT_QUICK_SEARCH =
    "CREATE TRIGGER IF NOT EXISTS [insert_contact_quick_search]AFTER INSERT ON [contact] BEGIN "
    "UPDATE [contact] SET [quick_search_key] = [NEW].[id] WHERE [id] = [NEW].[id]; END";

constexpr const char *MERGE_INFO =
    "CREATE TABLE IF NOT EXISTS [merge_info]("
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[raw_contact_id] INTEGER NOT NULL DEFAULT 0 )";

constexpr const char *MERGE_INFO_INDEX =
    "CREATE INDEX IF NOT EXISTS [merge_info_index] ON [merge_info] ([raw_contact_id])";

constexpr const char *CREATE_CLOUD_RAW_CONTACT =
    "CREATE TABLE IF NOT EXISTS [cloud_raw_contact]("
    "[uuid] TEXT PRIMARY KEY, "
    "[data] TEXT, "
    "[recycled] boolean, "
    "[recycledTime] INTEGER, "
    "[attachments] ASSETS)";

constexpr const char *CREATE_CLOUD_GROUPS =
    "CREATE TABLE IF NOT EXISTS [cloud_groups]("
    "[uuid] TEXT PRIMARY KEY, "
    "[data] TEXT, "
    "[recycled] boolean, "
    "[recycledTime] INTEGER, "
    "[attachments] ASSETS)";

constexpr const char *CREATE_SETTINGS =
    "CREATE TABLE IF NOT EXISTS [settings]("
    "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
    "[contact_change_time] DATETIME, "
    "[contacts_on_card] TEXT DEFAULT ',', "
    "[refresh_contacts]  INTEGER NOT NULL DEFAULT 0, "
    "[clone_time_stamp]  INTEGER NOT NULL DEFAULT 0, "
    "[cloud_raw_contact_cursor]  INTEGER NOT NULL DEFAULT -1, "
    "[main_space_need_sync_flag] INTEGER NOT NULL DEFAULT 0, "
    "[refresh_location] INTEGER NOT NULL DEFAULT 0, "
    "[blocklist_migrate_status]  INTEGER NOT NULL DEFAULT 0, "
    "[cloud_sync_delete_flag] INTEGER NOT NULL DEFAULT 0)";

constexpr const char *CREATE_HW_ACCOUNT =
    "CREATE TABLE IF NOT EXISTS [hw_account]("
    "[contact_id] INTEGER NOT NULL DEFAULT 0, "
    "[account_id] TEXT, "
    "[phone_number] TEXT, "
    "[email] TEXT)";

constexpr const char *INIT_CHANGE_TIME =
    "INSERT INTO settings (contact_change_time) VALUES (datetime('now'))";

constexpr int CALL_IS_UNREAD = 0;
constexpr int CALL_IS_READ = 1;
constexpr int CALL_DIRECTION_IN = 0;
constexpr int CALL_DIRECTION_OUT = 1;
constexpr int CALL_ANSWER_MISSED = 0;
constexpr int CALL_ANSWER_ACTIVED = 1;
constexpr const char *SETTING_DB_URI = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr int MAX_ACCOUNT_ID = 1000; // 遍历账号用的最大ID（暂定）
} // namespace Contacts
} // namespace OHOS
#endif // COMMON_H
