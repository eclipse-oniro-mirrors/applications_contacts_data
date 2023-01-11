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

#ifndef CONTACTSDATAABILITY_URI_COMMON_H
#define CONTACTSDATAABILITY_URI_COMMON_H

namespace Contacts {
namespace Test {
class ContactTabName {
public:
    ~ContactTabName();
    static constexpr const char *RAW_CONTACT = "raw_contact";
    static constexpr const char *CONTACT_DATA = "contact_data";
    static constexpr const char *CONTACT = "contact";
    static constexpr const char *GROUPS = "groups";
    static constexpr const char *CONTACT_BLOCKLIST = "contact_blocklist";
    static constexpr const char *DELETED_RAW_CONTACT = "deleted_raw_contact";
    static constexpr const char *DELETED_RAW_CONTACT_RECORD = "deleted_raw_contact_record";
    static constexpr const char *SEARCH_CONTACT = "search_contact";
};

class CallLogUri {
public:
    ~CallLogUri();
    static constexpr const char *CALL_LOG = "datashare:///com.ohos.calllogability/calls/calllog";
    static constexpr const char *ERROR_URI = "datashare:///com.ohos.calllogability/calls/calllogs";
};

class VoicemailUri {
public:
    ~VoicemailUri();
    static constexpr const char *VOICEMAIL = "datashare:///com.ohos.voicemailability/calls/voicemail";
    static constexpr const char *ERROR_URI = "datashare:///com.ohos.voicemailability/calls/voicemails";
};

class ContactsUri {
public:
    ~ContactsUri();
    static constexpr const char *RAW_CONTACT = "datashare:///com.ohos.contactsdataability/contacts/raw_contact";
    static constexpr const char *DELETED_RAW_CONTACT =
        "datashare:///com.ohos.contactsdataability/contacts/deleted_raw_contact";
    static constexpr const char *DELETED_RAW_CONTACT_RECORD =
        "datashare:///com.ohos.contactsdataability/contacts/deleted_raw_contact_record";
    static constexpr const char *BLOCKLIST = "datashare:///com.ohos.contactsdataability/contacts/contact_blocklist";
    static constexpr const char *GROUPS = "datashare:///com.ohos.contactsdataability/contacts/groups";
    static constexpr const char *CONTACT_DATA = "datashare:///com.ohos.contactsdataability/contacts/contact_data";
    static constexpr const char *CONTACT = "datashare:///com.ohos.contactsdataability/contacts/contact";
    static constexpr const char *SEARCH = "datashare:///com.ohos.contactsdataability/contacts/search_contact";
    static constexpr const char *ERROR_URI = "datashare:///com.ohos.contactsdataability/contacts/raw_contacts";
    static constexpr const char *BACKUP = "datashare:///com.ohos.contactsdataability/contacts/backup";
    static constexpr const char *RECOVER = "datashare:///com.ohos.contactsdataability/contacts/recover";
    static constexpr const char *GROUPS_ERROR = "datashare:///com.ohos.contactsdataability/contacts/group";
    static constexpr const char *MERGE_LIST =
        "datashare:///com.ohos.contactsdataability/contacts/raw_contact/query_merge_list";
    static constexpr const char *AUTO_MERGE =
        "datashare:///com.ohos.contactsdataability/contacts/raw_contact/auto_merge";
    static constexpr const char *MANUAL_MERGE =
        "datashare:///com.ohos.contactsdataability/contacts/raw_contact/manual_merge";
    static constexpr const char *SPLIT_CONTACT =
        "datashare:///com.ohos.contactsdataability/contacts/raw_contact/split_contact";
    static constexpr const char *MERGE_ERROR =
        "datashare:///com.ohos.contactsdataability/contacts/raw_contact/auto_mergeee";
    static constexpr const char *MERGE_LIST_ERROR =
        "datashare:///com.ohos.contactsdataability/contacts/raw_contact/merge_lists_error";
};

class ProfileUri {
public:
    ~ProfileUri();
    static constexpr const char *RAW_CONTACT = "datashare:///com.ohos.contactsdataability/profile/raw_contact";
    static constexpr const char *DELETED_RAW_CONTACT =
        "datashare:///com.ohos.contactsdataability/profile/deleted_raw_contact";
    static constexpr const char *BLOCKLIST = "datashare:///com.ohos.contactsdataability/profile/contact_blocklist";
    static constexpr const char *GROUPS = "datashare:///com.ohos.contactsdataability/profile/groups";
    static constexpr const char *CONTACT_DATA = "datashare:///com.ohos.contactsdataability/profile/contact_data";
    static constexpr const char *CONTACT = "datashare:///com.ohos.contactsdataability/profile/contact";
    static constexpr const char *SEARCH = "datashare:///com.ohos.contactsdataability/profile/search_contact";
    static constexpr const char *ERROR_URI = "datashare:///com.ohos.contactsdataability/profile/raw_contacts";
};

class Time {
public:
    ~Time();
    static constexpr int SLEEP_TIME_MERGE = 2000;
    static constexpr int SLEEP_TIME_MERGE_DELETE = 5000;
    static constexpr int ASYNC_SLEEP_TIME = 2000;
};

class DataPath {
public:
    ~DataPath();
    static constexpr const char *RDB_PATH = "/data/app/el2/100/database/com.ohos.contactsdataability/";
    static constexpr const char *RDB_BACKUP_PATH =
        "/data/app/el2/100/database/com.ohos.contactsdataability/backup/";
};
} // namespace Test
} // namespace Contacts
#endif // CONTACTSDATAABILITY_URI_COMMON_H
