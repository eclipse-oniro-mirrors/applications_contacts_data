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

#ifndef CONTACT_COLUMNS_H
#define CONTACT_COLUMNS_H

#include <map>

namespace OHOS {
namespace Contacts {
class ContactTableName {
public:
    ~ContactTableName();
    static constexpr const char *ACCOUNT = "account";
    static constexpr const char *CONTACT_TYPE = "contact_type";
    static constexpr const char *DELETE_RAW_CONTACT = "deleted_raw_contact";
    static constexpr const char *GROUPS = "groups";
    static constexpr const char *CONTACT = "contact";
    static constexpr const char *CONTACT_BLOCKLIST = "contact_blocklist";
    static constexpr const char *CONTACT_DATA = "contact_data";
    static constexpr const char *LOCAL_LANG = "local_lang";
    static constexpr const char *PHOTO_FILES = "photo_files";
    static constexpr const char *RAW_CONTACT = "raw_contact";
    static constexpr const char *SEARCH_CONTACT = "search_contact";
    static constexpr const char *DATABASE_BACKUP_TASK = "database_backup_task";
    static constexpr const char *MERGE_INFO = "merge_info";
    static constexpr const char *CLOUD_RAW_CONTACT = "cloud_raw_contact";
    static constexpr const char *CLOUD_GROUP = "cloud_groups";
    static constexpr const char *CLOUD_CONTACT_BLOCKLIST = "cloud_contact_blocklist";
    static constexpr const char *SETTINGS = "settings";
    static constexpr const char *HW_ACCOUNT = "hw_account";
    static constexpr const char *PRIVACY_CONTACTS_BACKUP = "privacy_contacts_backup";
    static constexpr const char *POSTER = "poster";
};

class CallLogColumns {
public:
    ~CallLogColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *DISPLAY_NAME = "display_name";
    static constexpr const char *QUICK_SEARCH_KEY = "quicksearch_key";
    static constexpr const char *PHONE_NUMBER = "phone_number";
    static constexpr const char *BEGIN_TIME = "begin_time";
    static constexpr const char *CALL_DIRECTION = "call_direction";
    static constexpr const char *ANSWER_STATE = "answer_state";
    static constexpr const char *IS_READ = "is_read";
    static constexpr const char *PRIVACY_TAG = "privacy_tag";
    // 畅连使用此字段，作为通话记录关联的联系人id
    static constexpr const char *EXTRA1 = "extra1";
    // 畅连使用此字段，作为通话记录联系人畅连头像显示
    static constexpr const char *EXTRA4 = "extra4";
    static constexpr const char *FORMAT_PHONE_NUMBER = "format_phone_number";
    static constexpr const char *IS_CNAP = "is_cnap";
    static constexpr const char *FEATURES = "features";
    static constexpr const char *NEWCALL = "new_calling";
    // 摘要生成时间
    static constexpr const char *START_TIME = "start_time";
    // 摘要类型
    static constexpr const char *ABSTRACT_TYPE = "abstract_type";
    // 摘要内容
    static constexpr const char *ABSTRACT_PROFILE = "abstract_profile";
    // 摘要状态
    static constexpr const char *ABS_STATUS = "abs_status";
    // 摘要返回码
    static constexpr const char *ABS_CODE = "abs_code";
    // 失败摘要记录id
    static constexpr const char *FAIL_ABS_RECORD_ID = "fail_abs_record_id";
    // 失败摘要记录状态
    static constexpr const char *FAIL_ABS_RECORD_STATUS = "fail_abs_record_status";
    // 失败摘要记录更新者
    static constexpr const char *FAIL_ABS_RECORD_UPDATER = "fail_abs_record_updater";
    // 失败摘要记录更新时间
    static constexpr const char *FAIL_ABS_RECORD_UPDATE_TIME = "fail_abs_record_update_time";
    // 摘要完成时间码
    static constexpr const char *ABS_FIN_TIMES_CODE = "abs_fin_times_code";
    // 备忘录的id
    static constexpr const char *NOTES_ID = "notes_id";
    // 备忘录状态
    static constexpr const char *NOTES_STATUS = "notes_status";
    // 备忘录状态更新者
    static constexpr const char *NOTES_STATUS_UPDATER = "notes_status_updater";
    // 备忘录状态更新时间
    static constexpr const char *NOTES_STATUS_UPDATE_TIME = "notes_status_update_time";
};

class DatabaseBackupColumns {
public:
    ~DatabaseBackupColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *BACKUP_TIME = "backup_time";
    static constexpr const char *BACKUP_PATH = "backup_path";
    static constexpr const char *REMARKS = "remarks";
};

class CallsTableName {
public:
    ~CallsTableName();
    static constexpr const char *CALLLOG = "calllog";
    static constexpr const char *VOICEMAIL = "voicemail";
    static constexpr const char *REPLYING = "replying";
};

class ViewName {
public:
    ~ViewName();
    static constexpr const char *SEARCH_CONTACT_VIEW = "search_contact_view";
    static constexpr const char *VIEW_CONTACT_DATA = "view_contact_data";
    static constexpr const char *VIEW_CONTACT = "view_contact";
    static constexpr const char *VIEW_CONTACT_LOCATION = "view_contact_location";
    static constexpr const char *VIEW_RAW_CONTACT = "view_raw_contact";
    static constexpr const char *VIEW_GROUPS = "view_groups";
    static constexpr const char *VIEW_DELETED = "view_deleted";
};

class AliasName {
public:
    ~AliasName();
    static constexpr const char *ACCOUNT_DEFAULT = "accountDefault";
    static constexpr const char *NAME_RAW_CONTACT = "name_raw_contact";
    static constexpr const char *DATA_ID = "data_id";
    static constexpr const char *CONTACT = "contact";
    static constexpr const char *DATA = "data";
};

class ContactPublicColumns {
public:
    ~ContactPublicColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *COUNT = "count";
};

class AccountColumns {
public:
    ~AccountColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *ACCOUNT_NAME = "account_name";
    static constexpr const char *ACCOUNT_TYPE = "account_type";
    static constexpr const char *DATA_INFO = "data_info";
};

class ContentTypeColumns {
public:
    ~ContentTypeColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *CONTENT_TYPE = "content_type";
};

class DeleteRawContactColumns {
public:
    ~DeleteRawContactColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *CONTACT_ID = "contact_id";
    static constexpr const char *RAW_CONTACT_ID = "raw_contact_id";
    static constexpr const char *DELETE_SOURCE = "delete_source";
    static constexpr const char *DELETE_TIME = "delete_time";
    static constexpr const char *DISPLAY_NAME = "display_name";
    static constexpr const char *DELETE_ACCOUNT = "delete_account";
    static constexpr const char *BACKUP_DATA = "backup_data";
    static constexpr const char *DELETE_DATE = "delete_date";
    static constexpr const char *IS_DELETED = "is_deleted";
};

class GroupsColumns {
public:
    ~GroupsColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *ACCOUNT_ID = "account_id";
    static constexpr const char *GROUP_NAME = "group_name";
    static constexpr const char *GROUP_NOTES = "group_notes";
    static constexpr const char *IS_DELETED = "is_deleted";
    static constexpr const char *GROUP_RINGTONE = "group_ringtone";
    static constexpr const char *RINGTONE_MODIFY_TIME = "ringtone_modify_time";
    static constexpr const char *LASTEST_MODIFY_TIME = "lastest_modify_time";
};

class ContactColumns {
public:
    ~ContactColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *NAME_RAW_CONTACT_ID = "name_raw_contact_id";
    static constexpr const char *PHOTO_FILE_ID = "photo_file_id";
    static constexpr const char *PHOTO_ID = "photo_id";
    static constexpr const char *PERSONAL_RINGTONE = "personal_ringtone";
    static constexpr const char *IS_TRANSFER_VOICEMAIL = "is_transfer_voicemail";
    static constexpr const char *COMPANY = "company";
    static constexpr const char *POSITION = "position";
    static constexpr const char *QUICK_SEARCH_KEY = "quick_search_key";
    static constexpr const char *CONTACTED_COUNT = "contacted_count";
    static constexpr const char *LASTEST_CONTACTED_TIME = "lastest_contacted_time";
    static constexpr const char *FAVORITE = "favorite";
    static constexpr const char *FAVORITE_ORDER = "favorite_order";
    static constexpr const char *READ_ONLY = "read_only";
    static constexpr const char *PERSONAL_NOTIFICATION_RINGTONE = "personal_notification_ringtone";
    static constexpr const char *HAS_PHONE_NUMBER = "has_phone_number";
    static constexpr const char *HAS_DISPLAY_NAME = "has_display_name";
    static constexpr const char *HAS_EMAIL = "has_email";
    static constexpr const char *HAS_GROUP = "has_group";
    static constexpr const char *FORM_ID = "form_id";
    static constexpr const char *FOCUS_MODE_LIST = "focus_mode_list";
    static constexpr const char *PREFER_AVATAR = "prefer_avatar";
    static constexpr const char *CONTACT_LAST_UPDATED_TIMESTAMP = "contact_last_updated_timestamp";
    static constexpr const char *RINGTONE_PATH = "ringtone_path";
};

class ContactBlockListColumns {
public:
    ~ContactBlockListColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *TYPES = "types";
    static constexpr const char *MARK = "mark";
    static constexpr const char *NAME = "name";
    static constexpr const char *PHONE_NUMBER = "phone_number";
    static constexpr const char *INTERCEPTION_CALL_COUNT = "interception_call_count";
    static constexpr const char *INTERCEPTION_MSG_COUNT = "interception_msg_count";
    static constexpr const char *CONTENT = "content";
    static constexpr const char *TIME_STAMP = "time_stamp";
    static constexpr const char *FORMAT_PHONE_NUMBER = "format_phone_number";
};

class ContactDataColumns {
public:
    ~ContactDataColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *TYPE_ID = "type_id";
    static constexpr const char *RAW_CONTACT_ID = "raw_contact_id";
    static constexpr const char *READ_ONLY = "read_only";
    static constexpr const char *VERSION = "version";
    static constexpr const char *IS_PERFERRED_NUMBER = "is_preferred_number";
    static constexpr const char *DETAIL_INFO = "detail_info";
    static constexpr const char *LOCATION = "location";
    static constexpr const char *FORMAT_PHONE_NUMBER = "format_phone_number";
    static constexpr const char *POSITION = "position";
    static constexpr const char *EXTEND1 = "extend1";
    static constexpr const char *EXTEND2 = "extend2";
    static constexpr const char *EXTEND3 = "extend3";
    static constexpr const char *EXTEND4 = "extend4";
    static constexpr const char *ALPHA_NAME = "alpha_name";
    static constexpr const char *OTHER_LAN_LAST_NAME = "other_lan_last_name";
    static constexpr const char *OTHER_LAN_FIRST_NAME = "other_lan_first_name";
    static constexpr const char *EXTEND5 = "extend5";
    static constexpr const char *PHONETIC_NAME = "phonetic_name";
    static constexpr const char *LAN_STYLE = "lan_style";
    static constexpr const char *CUSTOM_DATA = "custom_data";
    static constexpr const char *EXTEND6 = "extend6";
    static constexpr const char *EXTEND7 = "extend7";
    static constexpr const char *EXTEND8 = "extend8";
    static constexpr const char *EXTEND9 = "extend9";
    static constexpr const char *EXTEND10 = "extend10";
    static constexpr const char *EXTEND11 = "extend11";
    static constexpr const char *BLOB_DATA = "blob_data";
    static constexpr const char *SYNC_1 = "sync_1";
    static constexpr const char *SYNC_2 = "sync_2";
    static constexpr const char *SYNC_3 = "sync_3";    
    static constexpr const char *CALENDAR_EVENT_ID = "calendar_event_id";
    static constexpr const char *BLOB_SOURCE = "blob_source";
    static constexpr const char *IS_SYNC_BIRTHDAY_TO_CALENDAR = "is_sync_birthday_to_calendar";
};

class LocalLanguageColumns {
public:
    ~LocalLanguageColumns();
    static constexpr const char *LOCAL = "local";
};

class PhotoFilesColumns {
public:
    ~PhotoFilesColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *FILE_WIDTH = "file_width";
    static constexpr const char *FILE_HEIGHT = "file_height";
    static constexpr const char *FILE_SIZE = "file_size";
};

class RawContactColumns {
public:
    ~RawContactColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *CONTACT_ID = "contact_id";
    static constexpr const char *PHOTO_ID = "photo_id";
    static constexpr const char *PHOTO_FILE_ID = "photo_file_id";
    static constexpr const char *IS_TRANSFER_VOICEMAIL = "is_transfer_voicemail";
    static constexpr const char *PERSONAL_RINGTONE = "personal_ringtone";
    static constexpr const char *IS_DELETED = "is_deleted";
    static constexpr const char *PHOTO_FIRST_NAME = "photo_first_name";
    static constexpr const char *ACCOUNT_ID = "account_id";
    static constexpr const char *VERSION = "version";
    static constexpr const char *DISPLAY_NAME = "display_name";
    static constexpr const char *SORT = "sort";
    static constexpr const char *CONTACTED_COUNT = "contacted_count";
    static constexpr const char *LASTEST_CONTACTED_TIME = "lastest_contacted_time";
    static constexpr const char *FAVORITE = "favorite";
    static constexpr const char *FAVORITE_ORDER = "favorite_order";
    static constexpr const char *PHONETIC_NAME = "phonetic_name";
    static constexpr const char *PHONETIC_NAME_TYPE = "phonetic_name_type";
    static constexpr const char *COMPANY = "company";
    static constexpr const char *POSITION = "position";
    static constexpr const char *READ_ONLY = "read_only";
    static constexpr const char *SORT_FIRST_LETTER = "sort_first_letter";
    static constexpr const char *SORT_KEY = "sort_key";
    static constexpr const char *MERGE_MODE = "merge_mode";
    static constexpr const char *IS_NEED_MERGE = "is_need_merge";
    static constexpr const char *MERGE_STATUS = "merge_status";
    static constexpr const char *IS_MERGE_TARGET = "is_merge_target";
    static constexpr const char *VIBRATION_SETTING = "vibration_setting";
    static constexpr const char *SYNC_ID = "sync_id";
    static constexpr const char *SYNC_1 = "sync_1";
    static constexpr const char *SYNC_2 = "sync_2";
    static constexpr const char *SYNC_3 = "sync_3";
    static constexpr const char *DIRTY = "dirty";
    static constexpr const char *UUID = "uuid";
    static constexpr const char *EXTRA1 = "extra1";
    static constexpr const char *EXTRA2 = "extra2";
    static constexpr const char *EXTRA3 = "extra3";
    static constexpr const char *EXTRA4 = "extra4";
    static constexpr const char *AGGREGATION_STATUS = "aggregation_status";
    static constexpr const char *PRIMARY_CONTACT = "primary_contact";
    static constexpr const char *UNIQUE_KEY = "unique_key";
    static constexpr const char *FOCUS_MODE_LIST = "focus_mode_list";
    static constexpr const char *FORM_ID = "form_id";

};

class SearchContactColumns {
public:
    ~SearchContactColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *ACCOUNT_ID = "account_id";
    static constexpr const char *CONTACT_ID = "contact_id";
    static constexpr const char *RAW_CONTACT_ID = "raw_contact_id";
    static constexpr const char *SEARCH_NAME = "search_name";
    static constexpr const char *DISPLAY_NAME = "display_name";
    static constexpr const char *PHONETIC_NAME = "phonetic_name";
    static constexpr const char *FAVORITE = "favorite";
    static constexpr const char *PHOTO_ID = "photo_id";
    static constexpr const char *PHOTO_FILE_ID = "photo_file_id";
};

class PrivacyContactsBackupColumns {
public:
    ~PrivacyContactsBackupColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *CONTACT_DATA_ID = "contact_data_id";
    static constexpr const char *RAW_CONTACT_ID = "raw_contact_id";
    static constexpr const char *PHONE_NUMBER = "phone_number";
    static constexpr const char *FORMAT_PHONE_NUMBER = "format_phone_number";
    static constexpr const char *CALL_SYNCED = "call_synced";
};

class AccountData {
public:
    ~AccountData();
    static constexpr const char *ACCOUNT_NAME = "phone";
    static constexpr const char *ACCOUNT_TYPE = "com.ohos.contacts";
};

class ContentTypeData {
public:
    ~ContentTypeData();
    static constexpr const char *EMAIL = "email";
    static constexpr const char *IM = "im";
    static constexpr const char *NICKNAME = "nickname";
    static constexpr const char *ORGANIZATION = "organization";
    static constexpr const char *PHONE = "phone";
    static constexpr const char *NAME = "name";
    static constexpr const char *ADDRESS = "postal_address";
    static constexpr const char *PHOTO = "photo";
    static constexpr const char *GROUP_MEMBERSHIP = "group_membership";
    static constexpr const char *NOTE = "note";
    static constexpr const char *CONTACT_EVENT = "contact_event";
    static constexpr const char *WEBSITE = "website";
    static constexpr const char *RELATION = "relation";
    static constexpr const char *CONTACT_MISC = "contact_misc";
    static constexpr const char *HICALL_DEVICE = "hicall_device";
    static constexpr const char *CAMCARD = "camcard";
    static constexpr const char *SIP_ADDRESS = "sip_address";
    static constexpr const char *POSTER = "poster";
    static constexpr const int EMAIL_INT_VALUE = 1;
    static constexpr const int IM_INT_VALUE = 2;
    static constexpr const int NICKNAME_INT_VALUE = 3;
    static constexpr const int ORGANIZATION_INT_VALUE = 4;
    static constexpr const int PHONE_INT_VALUE = 5;
    static constexpr const int NAME_INT_VALUE = 6;
    static constexpr const int ADDRESS_INT_VALUE = 7;
    static constexpr const int PHOTO_INT_VALUE = 8;
    static constexpr const int GROUP_MEMBERSHIP_INT_VALUE = 9;
    static constexpr const int NOTE_INT_VALUE = 10;
    static constexpr const int CONTACT_EVENT_INT_VALUE = 11;
    static constexpr const int WEBSITE_INT_VALUE = 12;
    static constexpr const int RELATION_INT_VALUE = 13;
    static constexpr const int CONTACT_MISC_INT_VALUE = 14;
    static constexpr const int HICALL_DEVICE_INT_VALUE = 15;
    static constexpr const int CAMCARD_INT_VALUE = 16;
    static constexpr const int SIP_ADDRESS_INT_VALUE = 17;
    static constexpr const int POSTER_VALUE = 18;
};
class MergeInfo {
public:
    ~MergeInfo();
    static constexpr const char *RAW_CONTACT_ID = "raw_contact_id";
};
class CloudRawContactColumns {
public:
    ~CloudRawContactColumns();
    static constexpr const char *UUID = "uuid";
    static constexpr const char *DATA = "data";
    static constexpr const char *RECYCLED = "recycled";
    static constexpr const char *RECYCLED_TIME = "recycledTime";
    static constexpr const char *ATTACHMENTS = "attachments";
};
class CloudGroupsColumns {
public:
    ~CloudGroupsColumns();
    static constexpr const char *UUID = "uuid";
    static constexpr const char *DATA = "data";
    static constexpr const char *RECYCLED = "recycled";
    static constexpr const char *RECYCLED_TIME = "recycledTime";
    static constexpr const char *ATTACHMENTS = "attachment";
};
class SettingsColumns {
public:
    ~SettingsColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *CONTACT_CHANGE_TIME = "contact_change_time";
    static constexpr const char *CALLLOG_CHANGE_TIME = "calllog_change_time";
    static constexpr const char *REFRESH_CONTACTS = "refresh_contacts";
    static constexpr const char *BLOCKLIST_MIGRATE_STATUS = "blocklist_migrate_status";
    static constexpr const int REFRESH_CONTACTS_UUID = 9;
    static constexpr const char *REFRESH_LOCATION = "refresh_location";
};
class HwAccountColumns {
public:
    ~HwAccountColumns();
    static constexpr const char *CONTACT_ID = "contact_id";
    static constexpr const char *ACCOUNT_ID = "account_id";
    static constexpr const char *PHONE_NUMBER = "phone_number";
    static constexpr const char *EMAIL = "email";
};

constexpr const char *RAW_CONTACT_ADD_PRIMARY_CONTACT =
    "ALTER TABLE raw_contact ADD COLUMN primary_contact INTEGER DEFAULT 0;";
constexpr const char *RAW_CONTACT_ADD_EXTRA1 = "ALTER TABLE raw_contact ADD COLUMN extra1 TEXT;";
constexpr const char *RAW_CONTACT_ADD_EXTRA2 = "ALTER TABLE raw_contact ADD COLUMN extra2 TEXT;";
constexpr const char *RAW_CONTACT_ADD_EXTRA3 = "ALTER TABLE raw_contact ADD COLUMN extra3 TEXT;";
constexpr const char *RAW_CONTACT_ADD_EXTRA4 = "ALTER TABLE raw_contact ADD COLUMN extra4 TEXT;";
constexpr const char *RAW_CONTACT_ADD_DIRTY = "ALTER TABLE raw_contact ADD COLUMN dirty INTEGER DEFAULT 0;";
constexpr const char *RAW_CONTACT_ADD_UUID = "ALTER TABLE raw_contact ADD COLUMN uuid TEXT;";
constexpr const char *RAW_CONTACT_ADD_SORT_KEY = "ALTER TABLE raw_contact ADD COLUMN sort_key TEXT;";
constexpr const char *RAW_CONTACT_ADD_UNIQUE_KEY = "ALTER TABLE raw_contact ADD COLUMN unique_key TEXT;";
constexpr const char *RAW_CONTACT_ADD_FOCUS_MODE_LIST =
    "ALTER TABLE raw_contact ADD COLUMN focus_mode_list TEXT;";
constexpr const char *RAW_CONTACT_ADD_FORM_ID = "ALTER TABLE raw_contact ADD COLUMN form_id TEXT;";
constexpr const char *RAW_CONTACT_ADD_AGGREGATION_STATUS =
    "ALTER TABLE raw_contact ADD COLUMN aggregation_status INTEGER NOT NULL DEFAULT 0;";

const std::map<std::string, const char *> RAW_CONTACT_ADD_COLUMNS = {
    {RawContactColumns::PRIMARY_CONTACT, RAW_CONTACT_ADD_PRIMARY_CONTACT},
    {RawContactColumns::EXTRA1, RAW_CONTACT_ADD_EXTRA1},
    {RawContactColumns::EXTRA2, RAW_CONTACT_ADD_EXTRA2},
    {RawContactColumns::EXTRA3, RAW_CONTACT_ADD_EXTRA3},
    {RawContactColumns::EXTRA4, RAW_CONTACT_ADD_EXTRA4},
    {RawContactColumns::DIRTY, RAW_CONTACT_ADD_DIRTY},
    {RawContactColumns::UUID, RAW_CONTACT_ADD_UUID},
    {RawContactColumns::SORT_KEY, RAW_CONTACT_ADD_SORT_KEY},
    {RawContactColumns::UNIQUE_KEY, RAW_CONTACT_ADD_UNIQUE_KEY},
    {RawContactColumns::FOCUS_MODE_LIST, RAW_CONTACT_ADD_FOCUS_MODE_LIST},
    {RawContactColumns::FORM_ID, RAW_CONTACT_ADD_FORM_ID},
    {RawContactColumns::AGGREGATION_STATUS, RAW_CONTACT_ADD_AGGREGATION_STATUS},
};

constexpr const char *CONTACT_ADD_CONTACT_LAST_UPDATED_TIMESTAMP =
    "ALTER TABLE contact ADD COLUMN contact_last_updated_timestamp INTEGER;";
constexpr const char *CONTACT_ADD_FORM_ID = "ALTER TABLE contact ADD COLUMN form_id TEXT;";
constexpr const char *CONTACT_ADD_FOCUS_MODE_LIST = "ALTER TABLE contact ADD COLUMN focus_mode_list TEXT;";
constexpr const char *CONTACT_ADD_RINGTONE_PATH = "ALTER TABLE contact ADD COLUMN ringtone_path TEXT;";
constexpr const char *CONTACT_ADD_PREFER_AVATAR = "ALTER TABLE contact ADD COLUMN prefer_avatar INTEGER;";

const std::map<std::string, const char *> CONTACT_ADD_COLUMNS = {
    {ContactColumns::CONTACT_LAST_UPDATED_TIMESTAMP, CONTACT_ADD_CONTACT_LAST_UPDATED_TIMESTAMP},
    {ContactColumns::FORM_ID, CONTACT_ADD_FORM_ID},
    {ContactColumns::FOCUS_MODE_LIST, CONTACT_ADD_FOCUS_MODE_LIST},
    {ContactColumns::RINGTONE_PATH, CONTACT_ADD_RINGTONE_PATH},
    {ContactColumns::PREFER_AVATAR, CONTACT_ADD_PREFER_AVATAR},
};

constexpr const char *CONTACT_DATA_ADD_EXTEND8 = "ALTER TABLE contact_data ADD COLUMN extend8 TEXT;";
constexpr const char *CONTACT_DATA_ADD_EXTEND9 = "ALTER TABLE contact_data ADD COLUMN extend9 TEXT;";
constexpr const char *CONTACT_DATA_ADD_EXTEND10 = "ALTER TABLE contact_data ADD COLUMN extend10 TEXT;";
constexpr const char *CONTACT_DATA_ADD_EXTEND11 = "ALTER TABLE contact_data ADD COLUMN extend11 TEXT;";
constexpr const char *CONTACT_DATA_ADD_FORMAT_PHONE_NUMBER =
        "ALTER TABLE contact_data ADD COLUMN format_phone_number TEXT;";
constexpr const char *CONTACT_DATA_ADD_BLOB_SOURCE =
        "ALTER TABLE contact_data ADD COLUMN blob_source INTEGER DEFAULT 0;";
constexpr const char *CONTACT_DATA_ADD_CALENDAR_EVENT_ID =
        "ALTER TABLE contact_data ADD COLUMN calendar_event_id TEXT;";
constexpr const char *CONTACT_DATA_ADD_IS_SYNC_BIRTHDAY_TO_CALENDAR =
        "ALTER TABLE contact_data ADD COLUMN is_sync_birthday_to_calendar INTEGER NOT NULL DEFAULT 0;";
constexpr const char *CONTACT_DATA_ADD_LOCATION = "ALTER TABLE contact_data ADD COLUMN location TEXT;";

const std::map<std::string, const char *> CONTACT_DATA_ADD_COLUMNS = {
    {ContactDataColumns::EXTEND8, CONTACT_DATA_ADD_EXTEND8},
    {ContactDataColumns::EXTEND9, CONTACT_DATA_ADD_EXTEND9},
    {ContactDataColumns::EXTEND10, CONTACT_DATA_ADD_EXTEND10},
    {ContactDataColumns::EXTEND11, CONTACT_DATA_ADD_EXTEND11},
    {ContactDataColumns::FORMAT_PHONE_NUMBER, CONTACT_DATA_ADD_FORMAT_PHONE_NUMBER},
    {ContactDataColumns::BLOB_SOURCE, CONTACT_DATA_ADD_BLOB_SOURCE},
    {ContactDataColumns::CALENDAR_EVENT_ID, CONTACT_DATA_ADD_CALENDAR_EVENT_ID},
    {ContactDataColumns::IS_SYNC_BIRTHDAY_TO_CALENDAR, CONTACT_DATA_ADD_IS_SYNC_BIRTHDAY_TO_CALENDAR},
    {ContactDataColumns::LOCATION, CONTACT_DATA_ADD_LOCATION},
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_COLUMNS_H
