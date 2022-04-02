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
};

class CallLogColumns {
public:
    ~CallLogColumns();
    static constexpr const char *DISPLAY_NAME = "display_name";
    static constexpr const char *QUICK_SEARCH_KEY = "quicksearch_key";
    static constexpr const char *PHONE_NUMBER = "phone_number";
    static constexpr const char *BEGIN_TIME = "begin_time";
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
};

class ContactBlockListColumns {
public:
    ~ContactBlockListColumns();
    static constexpr const char *ID = "id";
    static constexpr const char *TYPES = "types";
    static constexpr const char *PHONE_NUMBER = "phone_number";
    static constexpr const char *CONTENT = "content";
    static constexpr const char *TIME_STAMP = "time_stamp";
};

class ContactDataColumns {
public:
    ~ContactDataColumns();
    static constexpr const char *TYPE_ID = "type_id";
    static constexpr const char *RAW_CONTACT_ID = "raw_contact_id";
    static constexpr const char *READ_ONLY = "read_only";
    static constexpr const char *VERSION = "version";
    static constexpr const char *IS_PERFERRED_NUMBER = "is_preferred_number";
    static constexpr const char *DETAIL_INFO = "detail_info";
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
    static constexpr const char *BLOB_DATA = "blob_data";
    static constexpr const char *SYNC_1 = "sync_1";
    static constexpr const char *SYNC_2 = "sync_2";
    static constexpr const char *SYNC_3 = "sync_3";
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
    static constexpr const char *MERGE_MODE = "merge_mode";
    static constexpr const char *IS_NEED_MERGE = "is_need_merge";
    static constexpr const char *MERGE_STATUS = "merge_status";
    static constexpr const char *IS_MERGE_TARGET = "is_merge_target";
    static constexpr const char *VIBRATION_SETTING = "vibration_setting";
    static constexpr const char *SYNC_ID = "sync_id";
    static constexpr const char *SYNC_1 = "sync_1";
    static constexpr const char *SYNC_2 = "sync_2";
    static constexpr const char *SYNC_3 = "sync_3";
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
};
class MergeInfo {
public:
    ~MergeInfo();
    static constexpr const char *RAW_CONTACT_ID = "raw_contact_id";
};
} // namespace Contacts
} // namespace OHOS
#endif // CONTACT_COLUMNS_H
