/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "contacts_data_ability.h"

#include <iostream>
#include <mutex>
#include <regex>

#include "blocklist_database.h"
#include "common.h"
#include "contacts_columns.h"
#include "contacts_common_event.h"
#include "contacts_json_utils.h"
#include "contacts_datashare_stub_impl.h"
#include "datashare_ext_ability_context.h"
#include "datashare_predicates.h"
#include "database_disaster_recovery.h"
#include "file_utils.h"
#include "hilog_wrapper.h"
#include "profile_database.h"
#include "rdb_predicates.h"
#include "rdb_utils.h"
#include "sql_analyzer.h"
#include "uri_utils.h"
#include "datashare_helper.h"
#include "datashare_log.h"
#include "datashare_errno.h"
#include "contacts_type.h"
#include "common_tool_type.h"
#include "privacy_contacts_manager.h"
#include "pixel_map_util.h"
#include <fcntl.h>

namespace OHOS {
namespace AbilityRuntime {
namespace {
std::mutex g_mutex;
std::mutex g_mutexQuery;
}
std::shared_ptr<Contacts::ContactsDataBase> ContactsDataAbility::contactDataBase_ = nullptr;
std::shared_ptr<Contacts::BlocklistDataBase> ContactsDataAbility::blocklistDataBase_ = nullptr;
std::shared_ptr<Contacts::ProfileDatabase> ContactsDataAbility::profileDataBase_ = nullptr;
std::shared_ptr<Contacts::ContactConnectAbility> ContactsDataAbility::contactsConnectAbility_ = nullptr;
std::map<std::string, int> ContactsDataAbility::uriValueMap_ = {
    {"/com.ohos.contactsdataability/contacts/contact", Contacts::CONTACTS_CONTACT},
    // 查询展示回收站联系人   彻底删除联系人
    {"/com.ohos.contactsdataability/contacts/deleted_raw_contact", Contacts::CONTACTS_DELETE},
    {"/com.ohos.contactsdataability/contacts/deleted_merge_contact", Contacts::CONTACTS_DELETE_MERGE},
    // 历史代码，实际未使用
    {"/com.ohos.contactsdataability/contacts/deleted_raw_contact_record", Contacts::CONTACTS_DELETE_RECORD},
    {"/com.ohos.contactsdataability/contacts/raw_contact", Contacts::CONTACTS_RAW_CONTACT},
    {"/com.ohos.contactsdataability/contacts/account", Contacts::ACCOUNT},
    {"/com.ohos.contactsdataability/contacts/raw_contact/query_merge_list", Contacts::QUERY_MERGE_LIST},
    {"/com.ohos.contactsdataability/contacts/raw_contact/split_contact", Contacts::SPLIT_CONTACT},
    {"/com.ohos.contactsdataability/contacts/raw_contact/manual_merge", Contacts::MANUAL_MERGE},
    {"/com.ohos.contactsdataability/contacts/raw_contact/auto_merge", Contacts::AUTO_MERGE},
    {"/com.ohos.contactsdataability/contacts/contact_data", Contacts::CONTACTS_CONTACT_DATA},
    {"/com.ohos.contactsdataability/contacts/contact_type", Contacts::CONTACT_TYPE},
    {"/com.ohos.contactsdataability/contacts/groups", Contacts::CONTACTS_GROUPS},
    {"/com.ohos.contactsdataability/contacts/contact_blocklist", Contacts::CONTACTS_BLOCKLIST},
    {"/com.ohos.contactsdataability/contacts/photo_files", Contacts::CONTACTS_PHOTO_FILES},
    {"/com.ohos.contactsdataability/contacts/search_contact", Contacts::CONTACTS_SEARCH_CONTACT},
    {"/com.ohos.contactsdataability/contacts/backup", Contacts::CONTACT_BACKUP},
    {"/com.ohos.contactsdataability/contacts/cloud", Contacts::CLOUD_DATA},
    {"/com.ohos.contactsdataability/contacts/upload_data_to_cloud", Contacts::UPLOAD_DATA_TO_CLOUD},
    {"/com.ohos.contactsdataability/contacts/query_uuid_not_in_raw_contact", Contacts::QUERY_UUID_NOT_IN_RAW_CONTACT},
    {"/com.ohos.contactsdataability/contacts/sync_contact_data", Contacts::PULLDOWN_SYNC_CONTACT_DATA},
    {"/com.ohos.contactsdataability/contacts/setting", Contacts::QUERY_IS_REFRESH_CONTACT},
    {"/com.ohos.contactsdataability/contacts/trigger_upload_cloud", Contacts::TRIGGER_UPLOAD_CLOUD},
    {"/com.ohos.contactsdataability/contacts/query_big_length_name", Contacts::QUERY_BIG_LENGTH_NAME},
    {"/com.ohos.contactsdataability/contacts/backup_restore", Contacts::BACKUP_RESTORE},
    {"/com.ohos.contactsdataability/contacts/hw_account", Contacts::HW_ACCOUNT},
    {"/com.ohos.contactsdataability/contacts/merge_contact", Contacts::MERGE_CONTACT},
    {"/com.ohos.contactsdataability/profile/backup", Contacts::PROFILE_BACKUP},
    {"/com.ohos.contactsdataability/contacts/recover", Contacts::CONTACT_RECOVER},
    {"/com.ohos.contactsdataability/profile/recover", Contacts::PROFILE_RECOVER},
    {"/com.ohos.contactsdataability/profile/contact", Contacts::PROFILE_CONTACT},
    {"/com.ohos.contactsdataability/profile/raw_contact", Contacts::PROFILE_RAW_CONTACT},
    {"/com.ohos.contactsdataability/profile/contact_data", Contacts::PROFILE_CONTACT_DATA},
    {"/com.ohos.contactsdataability/profile/groups", Contacts::PROFILE_GROUPS},
    {"/com.ohos.contactsdataability/profile/contact_blocklist", Contacts::PROFILE_BLOCKLIST},
    {"/com.ohos.contactsdataability/profile/photo_files", Contacts::PROFILE_PHOTO_FILES},
    {"/com.ohos.contactsdataability/profile/search_contact", Contacts::PROFILE_SEARCH_CONTACT},
    {"/com.ohos.contactsdataability/profile/deleted_raw_contact_record", Contacts::PROFILE_DELETE_RECORD},
    {"/com.ohos.contactsdataability/profile/deleted_raw_contact", Contacts::PROFILE_DELETE},
    {"/com.ohos.contactsdataability/profile/contact_type", Contacts::PROFILE_TYPE},
    // 恢复回收站联系人
    {"/com.ohos.contactsdataability/contacts/recycle_restore", Contacts::RECYCLE_RESTORE},
    // raw_contacts 表当前自增ID
    {"/com.ohos.contactsdataability/raw_contact/place_holder", Contacts::CONTACTS_PLACE_HOLDER},
    // 调用设置全搜同步数据的接口
    {"/com.ohos.contactsdataability/contacts/set_full_search_switch_open", Contacts::SET_FULL_SEARCH_SWITCH_OPEN},
    {"/com.ohos.contactsdataability/contacts/set_full_search_switch_close", Contacts::SET_FULL_SEARCH_SWITCH_CLOSE},
    {"/com.ohos.contactsdataability/contacts/restore_contact_by_double_to_single_upgrade_data",
        Contacts::RESTORE_CONTACT_BY_DOUBLE_TO_SINGLE_UPGRADE_DATA},
    // 联系人一键恢复时，在drop了本地表，新建表后，需要通知全搜处理（1）清空搜索库的对应表，drop表不能被默认感知处理，需要调接口处理
    // （2）重新建立触发器，drop表后，原始表的触发器也被删除，需要重建；触发器用于监听数据变化，同步至全搜
    {"/com.ohos.contactsdataability/contacts/clear_and_recreate_trigger_search_table",
        Contacts::CLEAR_AND_RECREATE_TRIGGER_SEARCH_TABLE},
    {"/com.ohos.contactsdataability/contacts/split_aggregation_contact", Contacts::SPLIT_AGGREGATION_CONTACT},
    {"/com.ohos.contactsdataability/contacts/sync_birthday_to_calendar", Contacts::SYNC_BIRTHDAY_TO_CALENDAR},
    {"/com.ohos.contactsdataability/contacts/rebuild_contact_search_index", Contacts::REBUILD_CONTACT_SERACH_INDEX},
    {"/com.ohos.contactsdataability/contacts/query_contact_count_double_db", Contacts::QUERY_CONTACT_COUNT_DOUBLE_DB},
    {"/com.ohos.contactsdataability/contacts/add_contact_batch", Contacts::ADD_CONTACT_INFO_BATCH},
    {"/com.ohos.contactsdataability/contacts/contacts_hard_delete", Contacts::CONTACTS_HARD_DELETE},
    {"/com.ohos.contactsdataability/contacts/update_yellow_page_info_for_callLog",
        Contacts::UPDATE_YELLOW_PAGE_INFO_FOR_CALLLOG},
    {"/com.ohos.contactsdataability/contacts/privacy_contacts_backup", Contacts::PRIVACY_CONTACTS_BACKUP},
    {"/com.ohos.contactsdataability/contacts/company_classify", Contacts::CONTACTS_COMPANY_CLASSIFY},
    {"/com.ohos.contactsdataability/contacts/query_count_without_company", Contacts::QUERY_COUNT_WITHOUT_COMPANY},
    {"/com.ohos.contactsdataability/contacts/location_classify", Contacts::CONTACTS_LOCATION_CLASSIFY},
    {"/com.ohos.contactsdataability/contacts/query_count_without_location", Contacts::QUERY_COUNT_WITHOUT_LOCATION},
    {"/com.ohos.contactsdataability/contacts/frequent_classify", Contacts::CONTACTS_FREQUENT_CLASSIFY},
    {"/com.ohos.contactsdataability/contacts/contact_location", Contacts::CONTACTS_CONTACT_LOCATION},
    {"/com.ohos.contactsdataability/contacts/contact_detect_repair", Contacts::CONTACTS_DETECT_REPAIR},
    {"/com.ohos.contactsdataability/contacts/poster", Contacts::CONTACTS_POSTER},
    {"/com.ohos.contactsdataability/contacts/download_posters", Contacts::CONTACTS_DOWNLOAD_POSTERS},
    // 添加头像失败时删除异常数据
    {"/com.ohos.contactsdataability/contacts/add_failed_delete", Contacts::CONTACTS_ADD_FAILED_DELETE},
};
// LCOV_EXCL_START
ContactsDataAbility* ContactsDataAbility::Create()
{
    return new ContactsDataAbility();
}
// LCOV_EXCL_STOP
ContactsDataAbility::ContactsDataAbility() : DataShareExtAbility()
{
}

ContactsDataAbility::~ContactsDataAbility()
{
}
// LCOV_EXCL_START
static DataShare::DataShareExtAbility *ContactsDataShareCreator(const std::unique_ptr<Runtime> &runtime)
{
    HILOG_INFO("ContactsDataAbility %{public}s", __func__);
    return ContactsDataAbility::Create();
}
// LCOV_EXCL_STOP
__attribute__((constructor)) void RegisterDataShareCreator()
{
    HILOG_INFO("RegisterDataShareCreator %{public}s", __func__);
    DataShare::DataShareExtAbility::SetCreator(ContactsDataShareCreator);
}
// LCOV_EXCL_START
sptr<IRemoteObject> ContactsDataAbility::OnConnect(const AAFwk::Want &want)
{
    HILOG_INFO("ContactsDataAbility %{public}s begin, ts = %{public}lld", __func__, (long long) time(NULL));
    Extension::OnConnect(want);
    sptr<DataShare::ContactsDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::ContactsDataShareStubImpl();
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetContactsDataAbility(std::static_pointer_cast<ContactsDataAbility>(shared_from_this()));
    HILOG_INFO("ContactsDataAbility %{public}s end, begin init ContactsDataBase, "
        "ts = %{public}lld", __func__, (long long) time(NULL));
    // 创建实例，否则，不调用datashare接口，无法触发数据库初始化
    Contacts::ContactsDataBase::GetInstance();
    return remoteObject->AsObject();
}

void ContactsDataAbility::OnStart(const Want &want)
{
    HILOG_INFO("ContactsDataAbility %{public}s begin, ts = %{public}lld", __func__, (long long) time(NULL));
    Extension::OnStart(want);
    auto context = AbilityRuntime::Context::GetApplicationContext();
    contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
    if (context != nullptr) {
        std::string basePath = context->GetDatabaseDir();
        Contacts::ContactsPath::RDB_PATH = basePath;
        Contacts::ContactsPath::RDB_BACKUP_PATH = basePath + "/backup/";
        Contacts::ContactsPath::RDB_EL1_PATH = "/data/storage/el1/database";
        Contacts::ContactsPath::RDB_EL5_PATH = "/data/storage/el5/database";
    }
}
// LCOV_EXCL_STOP
/**
 * @brief Check whether BeginTransaction of ContactsDataAbility is empty
 *
 * @param code the return number of BeginTransaction
 * @param mutex transmission parameter : lock
 *
 * @return True if BeginTransaction is empty; flase otherwise
 */
bool ContactsDataAbility::IsBeginTransactionOK(int code, std::mutex &mutex)
{
    bool ret = mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsBeginTransactionOK failed");
        if (ret) {
            mutex.unlock();
        }
        return false;
    }
    return true;
}

/**
 * @brief Check if ContactsDataAbility Commit is empty
 *
 * @param code the return number of Commit
 * @param mutex transmission parameter : lock
 *
 * @return True if ContactsDataAbility Commit is empty; flase otherwise
 */
bool ContactsDataAbility::IsCommitOK(int code, std::mutex &mutex)
{
    bool ret = mutex.try_lock();
    if (code != 0) {
        HILOG_ERROR("IsCommitOK failed");
        if (ret) {
            mutex.unlock();
        }
        return false;
    }
    return true;
}

void ContactsDataAbility::handleUpdateCalllogAfterInsertData(int code,
    const DataShare::DataShareValuesBucket &value)
{
    if (code == Contacts::CONTACTS_CONTACT_DATA) {
        std::vector<int> needUpdateCalllogRawIdVector;
        contactDataBase_->parseNeedUpdateCalllogRawId(value, needUpdateCalllogRawIdVector);
        contactDataBase_->MergeUpdateTask(Contacts::ContactsDataBase::store_,
            needUpdateCalllogRawIdVector, false);
    }
}

/**
 * @brief Insert ContactsDataAbility into the database
 *
 * @param uri URI of the data table tobe inserted
 * @param value Inserted data value of the database
 *
 * @return Insert database results code
 */
int ContactsDataAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CONTACTS)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("ContactsDataAbility CheckValuesBucket error");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    OHOS::Uri uriTemp = uri;
    time_t ts = time(NULL);
    HILOG_INFO("getLockThen Insert ts = %{public}lld", (long long) ts);
    std::string isSyncFromCloud = UriParseParam(uriTemp);
    int code = UriParseAndSwitch(uriTemp);
    int ret = contactDataBase_->BeginTransaction();
    if (!IsBeginTransactionOK(ret, g_mutex)) {
        HILOG_ERROR("ContactsDataAbility Insert IsBeginTransactionOK error");
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    int resultId = InsertExecute(code, valuesBucket, isSyncFromCloud);
    if (resultId == Contacts::OPERATION_ERROR) {
        HILOG_ERROR("ContactsDataAbility Insert InsertExecute error");
        contactDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::OPERATION_ERROR;
    }
    ret = contactDataBase_->Commit();
    if (!IsCommitOK(ret, g_mutex)) {
        HILOG_ERROR("ContactsDataAbility Insert IsCommitOK error");
        contactDataBase_->RollBack();
        g_mutex.unlock();
        return Contacts::RDB_EXECUTE_FAIL;
    }
    // insert ok了
    handleUpdateCalllogAfterInsertData(code, value);
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri, isSyncFromCloud);
    HILOG_INFO("ContactsDataAbility Insert end,resultId = %{public}d,ts = %{public}lld", resultId, (long long) ts);
    return resultId;
}

int ContactsDataAbility::InsertExecute(int &code, const OHOS::NativeRdb::ValuesBucket &value, std::string isSync)
{
    HILOG_INFO("ContactsDataAbility InsertExecute start code = %{public}d,isSync = %{public}s,ts = %{public}lld",
               code, isSync.c_str(), (long long) time(NULL));
    int rowId = Contacts::RDB_EXECUTE_FAIL;
    switch (code) {
        // 新建联系人
        case Contacts::CONTACTS_RAW_CONTACT:
        case Contacts::PROFILE_RAW_CONTACT:
            // contact Basic Information
            rowId = contactDataBase_->InsertRawContact(Contacts::ContactTableName::RAW_CONTACT, value);
            break;
        case Contacts::CONTACTS_CONTACT_DATA:
        case Contacts::PROFILE_CONTACT_DATA:
            // contact Information add
            rowId = contactDataBase_->InsertContactData(Contacts::ContactTableName::CONTACT_DATA, value, isSync);
            break;
        case Contacts::CONTACTS_GROUPS:
        case Contacts::PROFILE_GROUPS:
            // insert group
            rowId = contactDataBase_->InsertGroup(Contacts::ContactTableName::GROUPS, value);
            break;
        case Contacts::CONTACTS_BLOCKLIST:
        case Contacts::PROFILE_BLOCKLIST:
            // add blocklist
            rowId = contactDataBase_->InsertBlockList(Contacts::ContactTableName::CONTACT_BLOCKLIST, value);
            break;
        case Contacts::CLOUD_DATA:
            // add cloud_rawContacts
            rowId = contactDataBase_->InsertCloudRawContacts(Contacts::ContactTableName::CLOUD_RAW_CONTACT, value);
            break;
        case Contacts::HW_ACCOUNT:
            // add cloud_rawContacts
            rowId = contactDataBase_->InsertCloudRawContacts(Contacts::ContactTableName::HW_ACCOUNT, value);
            break;
            // 只有云同步 才会直接新增 delete uri的数据，应用删除时走的 delete contact uri逻辑
        case Contacts::CONTACTS_DELETE:
            // add delete_raw_contact form cloud
            rowId = contactDataBase_->InsertDeleteRawContact(Contacts::ContactTableName::DELETE_RAW_CONTACT, value);
            break;
        case Contacts::CONTACTS_PLACE_HOLDER:
            // 历史原因，因为 vcard 通信框架已经使用了 insert 接口，所以只好放在这里处理
            rowId = contactDataBase_->UpdateRawContactSeq(value);
            break;
        case Contacts::PRIVACY_CONTACTS_BACKUP:
            // 隐私空间新增联系人，同步到privacy_contacts_backup表
            rowId = contactDataBase_->InsertPrivacyContactsBackup(
                Contacts::ContactTableName::PRIVACY_CONTACTS_BACKUP, value);
            break;
        case Contacts::CONTACTS_POSTER:
            // add poster
            rowId = contactDataBase_->InsertPoster(Contacts::ContactTableName::POSTER, value);
            break;
        default:
            rowId = Contacts::OPERATION_ERROR;
            HILOG_INFO("ContactsDataAbility ====>no match uri action, %{public}d", code);
            break;
    }
    return rowId;
}
// LCOV_EXCL_START
void ContactsDataAbility::generateDisplayNameBucket(std::vector<DataShare::DataShareValuesBucket> &values)
{
    // DataShareValuesBucket转为ValuesBucket，用户从bucket获取值，从DataShareValuesBucket获取值有问题
    std::vector<OHOS::NativeRdb::ValuesBucket> valuesRdb;
    ContactsDataAbility::transferDataShareToRdbBucket(values, valuesRdb);
    // 如果调用接口，传的type信息是type文本，就需要转换成typeId
    transferTypeInfo(values, valuesRdb);
    int rawContactId = ContactsDataAbility::getIntValueFromRdbBucket(valuesRdb[0],
        Contacts::ContactDataColumns::RAW_CONTACT_ID);
    // 查询values是否包含displayname
    // 包含，说明不需要生成displayNameBucket记录，返回values
    // 包含但是displayName没值，PhoneticName有值，需要从PhoneticName把值设置到displayName
    int index = 0;
    DataShare::DataShareValuesBucket *dataShareValuesBucketDisplayNamePtr = nullptr;
    for (auto &value : values) {
        // 获取typeId
        int typeId = ContactsDataAbility::getIntValueFromRdbBucket(valuesRdb[index],
            Contacts::ContactDataColumns::TYPE_ID);
        // 存在displayname，不需要生成
        if (typeId == Contacts::ContentTypeData::NAME_INT_VALUE) {
            std::string displayName = ContactsDataAbility::getStringValueFromRdbBucket(valuesRdb[index],
                Contacts::ContactDataColumns::DETAIL_INFO);
            if (!displayName.empty()) {
                // 名称信息正常，不需要生成，使用原始的就可以
                return;
            }
            // 如果有phoneticName，但displayName为空，将displayName信息设置为phoneticName
            std::string phoneticName = ContactsDataAbility::getStringValueFromRdbBucket(valuesRdb[index],
                Contacts::ContactDataColumns::PHONETIC_NAME);
            if (!phoneticName.empty()) {
                HILOG_INFO("displayname is empty, get from phoneticName");
                // 需要将displayName设置为phoneticName
                putDisplayNameInfo(value, phoneticName, rawContactId);
                return;
            }
            // 集合中存在displayName对应的bucket，生成displayName的话，从这里修改
            dataShareValuesBucketDisplayNamePtr = &value;
            break;
        }
        ++index;
    }

    // 名称不存在；且不能从phoneticName获取，需要判断是否从其他信息获取
    std::string displayName = generateDisplayName(valuesRdb);
    // 如果没获取到，说明是无名氏，不需要生成displayName，使用原始数据
    if (displayName.empty()) {
        HILOG_INFO("get displayname from other info finish, displayname is empty");
        return;
    }
    HILOG_INFO("displayname is empty, get from other info");
    if (dataShareValuesBucketDisplayNamePtr == nullptr) {
        // 生成 displayNameBucket
        HILOG_INFO("add to bucket displayname");
        DataShare::DataShareValuesBucket displayNameBucket;
        putDisplayNameInfo(displayNameBucket, displayName, rawContactId);
        values.push_back(displayNameBucket);
    } else {
        // update displayNameBucket
        HILOG_INFO("update bucket displayname");
        putDisplayNameInfo(*dataShareValuesBucketDisplayNamePtr, displayName, rawContactId);
    }
}
// LCOV_EXCL_STOP
// LCOV_EXCL_START
void ContactsDataAbility::transferTypeInfo(std::vector<DataShare::DataShareValuesBucket> &values,
    std::vector<OHOS::NativeRdb::ValuesBucket> &valuesRdb)
{
    int index = 0;
    for (auto &valueRdb : valuesRdb) {
        // 如果存在content_type（三方接口方式调用），转为typeId
        if (valueRdb.HasColumn(Contacts::ContentTypeColumns::CONTENT_TYPE)) {
            std::string typeText = ContactsDataAbility::getStringValueFromRdbBucket(valuesRdb[index],
                Contacts::ContentTypeColumns::CONTENT_TYPE);
            if (typeText.empty()) {
                HILOG_ERROR("GetTypeText type is required");
                continue;
            }
            // get type id
            Contacts::ContactsType contactsType;
            int typeId = contactsType.LookupTypeIdOrInsertTypeValue(Contacts::ContactsDataBase::store_, typeText);
            if (typeId == Contacts::RDB_EXECUTE_FAIL) {
                HILOG_ERROR("insert typeinfo fail");
                continue;
            }
            // 生成了id，将原始的CONTENT_TYPE去掉，设置上typeId，避免后续重复查询
            values[index].valuesMap.erase(Contacts::ContentTypeColumns::CONTENT_TYPE);
            values[index].Put(Contacts::ContactDataColumns::TYPE_ID, typeId);
            valueRdb.Delete(Contacts::ContentTypeColumns::CONTENT_TYPE);
            valueRdb.PutInt(Contacts::ContactDataColumns::TYPE_ID, typeId);
        }
        ++index;
    }
}
// LCOV_EXCL_STOP

void ContactsDataAbility::putDisplayNameInfo(DataShare::DataShareValuesBucket &displayNameBucket,
    std::string &displayName, int rawContactId)
{
    // DataShareValuesBucket内部使用insert(pair)方式添加元素，如果键已经存在，不会覆盖，所以需要先移除键
    std::map<std::string, DataShare::DataShareValueObject::Type> &valuesMap = displayNameBucket.valuesMap;
    valuesMap.erase(Contacts::ContactDataColumns::DETAIL_INFO);
    displayNameBucket.Put(Contacts::ContactDataColumns::DETAIL_INFO, displayName);
    valuesMap.erase(Contacts::ContactDataColumns::ALPHA_NAME);
    displayNameBucket.Put(Contacts::ContactDataColumns::ALPHA_NAME, displayName);
    valuesMap.erase(Contacts::ContactDataColumns::TYPE_ID);
    displayNameBucket.Put(Contacts::ContactDataColumns::TYPE_ID, Contacts::ContentTypeData::NAME_INT_VALUE);
    // 对于名称信息为空，从其他方案获得的，标记EXTEND7为1
    valuesMap.erase(Contacts::ContactDataColumns::EXTEND7);
    displayNameBucket.Put(Contacts::ContactDataColumns::EXTEND7, Contacts::DISPLAY_NAME_GENERATE_FROM_OTHER_INFO_FLAG);
    valuesMap.erase(Contacts::ContactDataColumns::RAW_CONTACT_ID);
    displayNameBucket.Put(Contacts::ContactDataColumns::RAW_CONTACT_ID, rawContactId);
}

std::string ContactsDataAbility::generateDisplayName(const std::vector<OHOS::NativeRdb::ValuesBucket> &valuesRdb)
{
    // 记录{typeId, typeId对用value}，对于电话，地址等信息，会对应多条记录，所以value使用vector
    std::map<int, std::vector<std::string>> typeIdDetailInfoMap;
    // 查询values是否包含displayname，不包含，说明没有填写名称信息，从其他信息获取，设置到名称
    for (auto &value : valuesRdb) {
        // 获取typeId
        int typeId = ContactsDataAbility::getIntValueFromRdbBucket(value, Contacts::ContactDataColumns::TYPE_ID);
        // 获得详情信息
        std::string detailInfo = ContactsDataAbility::getStringValueFromRdbBucket(value,
            Contacts::ContactDataColumns::DETAIL_INFO);
        typeIdDetailInfoMap[typeId].push_back(std::move(detailInfo));
        // 公司和职位在一行contactdata中，都要加入map信息
        if (typeId == Contacts::ContentTypeData::ORGANIZATION_INT_VALUE) {
            std::string position = ContactsDataAbility::getStringValueFromRdbBucket(value,
                Contacts::ContactDataColumns::POSITION);
            typeIdDetailInfoMap[typeId].push_back(std::move(position));
        }
    }

    // 不存在名称，从其他信息获取
    // 昵称-》公司，组织-》电话号码-》邮箱地址
    std::string displayName;
    std::vector<int> otherTypeVector = {Contacts::ContentTypeData::NICKNAME_INT_VALUE,
        Contacts::ContentTypeData::ORGANIZATION_INT_VALUE,
        Contacts::ContentTypeData::PHONE_INT_VALUE,
        Contacts::ContentTypeData::EMAIL_INT_VALUE};
    for (auto typeId : otherTypeVector) {
        for (auto &detailInfo : typeIdDetailInfoMap[typeId]) {
            // 如果不为空，就根据这个信息生成displayName
            if (!detailInfo.empty()) {
                displayName = generateDisplayName(typeId, detailInfo);
                return displayName;
            }
        }
    }

    // 没有从其他信息获取到displayName
    return displayName;
}

std::string ContactsDataAbility::generateDisplayName (int &typeId, std::string &fromDetailInfo)
{
    std::string displayName;
    if (typeId == Contacts::ContentTypeData::NICKNAME_INT_VALUE
        || typeId == Contacts::ContentTypeData::ORGANIZATION_INT_VALUE
        || typeId == Contacts::ContentTypeData::EMAIL_INT_VALUE) {
        displayName = fromDetailInfo;
    }  else if (typeId == Contacts::ContentTypeData::PHONE_INT_VALUE) {
        // 电话号码，做去除空白字符信息处理
        std::regex pattern("\\s");
        std::string transferStr = std::regex_replace(fromDetailInfo, pattern, "");
        displayName = transferStr;
    }
    return displayName;
}

std::string ContactsDataAbility::getStringValueFromRdbBucket(const OHOS::NativeRdb::ValuesBucket &valuesBucket,
    const std::string &colName)
{
    OHOS::NativeRdb::ValueObject value;
    std::string colVal;
    valuesBucket.GetObject(colName, value);
    value.GetString(colVal);
    return colVal;
}

void ContactsDataAbility::transferDataShareToRdbBucket(
    const std::vector<DataShare::DataShareValuesBucket> &values,
    std::vector<OHOS::NativeRdb::ValuesBucket> &valuesRdb)
{
    if (values.empty()) {
        return;
    }

    for (const DataShare::DataShareValuesBucket &dataShareValuesBucket : values) {
        valuesRdb.push_back(RdbDataShareAdapter::RdbUtils::ToValuesBucket(dataShareValuesBucket));
    }
}

int ContactsDataAbility::getIntValueFromRdbBucket(
    const OHOS::NativeRdb::ValuesBucket &valuesBucket,
    const std::string &colName)
{
    int colVal = Contacts::OPERATION_ERROR;
    if (!valuesBucket.HasColumn(colName)) {
        return colVal;
    }

    OHOS::NativeRdb::ValueObject value;
    valuesBucket.GetObject(colName, value);
    if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_NULL) {
        HILOG_ERROR("get %{public}s value is nullptr, ts = %{public}lld", colName.c_str(), (long long) time(NULL));
        colVal = Contacts::OPERATION_ERROR;
    } else if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_INT) {
        value.GetInt(colVal);
    } else if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_STRING) {
        std::string tempString;
        value.GetString(tempString);
        if (!Contacts::CommonToolType::ConvertToInt(tempString, colVal)) {
            HILOG_ERROR("GetContactByValue ValueObjectType String tempString = %{public}s", tempString.c_str());
        }
    } else if (value.GetType() == OHOS::NativeRdb::ValueObjectType::TYPE_DOUBLE) {
        double temp = 0;
        value.GetDouble(temp);
        colVal = ceil(temp);
    } else {
        HILOG_ERROR("get %{public}s int value from OHOS::NativeRdb::ValuesBucket error, is other type",
            colName.c_str());
    }
    return colVal;
}

/**
 * @brief ContactsDataAbility BatchInsert database
 *
 * @param uri Determine the data table name based on the URI
 * @param value Inserted data value of the database
 *
 * @return BatchInsert database results code
 */
int ContactsDataAbility::BatchInsert(const Uri &uri,
    const std::vector<DataShare::DataShareValuesBucket> &values)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CONTACTS)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    unsigned int size = values.size();
    if (size < 1) {
        HILOG_INFO("ContactsDataAbility BatchInsert values < 1");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    // 如果名称信息没有值，其他信息（如公司，职位，手机号码等有值），需要根据其他信息的值，设置到名称（如将公司名称设置到displayName）
    // 生成displayName情况，需要再bucket集合新增元素，但集合为const，不可修改，要使用一个新的集合
    OHOS::Uri uriTemp = uri;
    int code = UriParseAndSwitch(uriTemp);
    std::vector<DataShare::DataShareValuesBucket> valuesHandle = values;
    std::string isSyncFromCloud = UriParseParam(uriTemp);
    std::string isFromBatch = UriParseBatchParam(uriTemp);
    HILOG_INFO("getLockThen BatchInsert,isSyncFromCloud = %{public}s,isFromBatch = %{public}s",
        isSyncFromCloud.c_str(), isFromBatch.c_str());
    // batchInsert onebyone会调用到单个插入，会发送大量消息
    // uri 带isfrombatch的标记，批量操作，否则，一个一个插入
    if (isFromBatch == "true" && (code == Contacts::CONTACTS_CONTACT_DATA || code == Contacts::PROFILE_CONTACT_DATA ||
        code == Contacts::CONTACTS_RAW_CONTACT)) {
        int batchCode = BatchInsertByMigrate(code, uri, valuesHandle);
        // 插入成功，且操作contact_data表，新增完数据后，处理更新calllog
        handleUpdateCalllogAfterBatchInsertData(batchCode, code, values);
        g_mutex.unlock();
        return batchCode;
    }
    if (code == Contacts::PRIVACY_CONTACTS_BACKUP) {
        std::vector<OHOS::NativeRdb::ValuesBucket> valueRdb;
        ContactsDataAbility::transferDataShareToRdbBucket(valuesHandle, valueRdb);
        int batchCode = contactDataBase_->BatchInsertPrivacyContactsBackup(
            Contacts::ContactTableName::PRIVACY_CONTACTS_BACKUP, valueRdb);
        g_mutex.unlock();
        return batchCode;
    }
    if (code == Contacts::CONTACTS_BLOCKLIST) {
        int batchCode = contactDataBase_->BatchInsertBlockList(valuesHandle);
        g_mutex.unlock();
        return batchCode;
    }
    if (code == Contacts::ADD_CONTACT_INFO_BATCH) {
        int ret = contactDataBase_->addContactInfoBatch(valuesHandle);
        DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri, "false");
        g_mutex.unlock();
        return ret;
    }
    int ret = batchInsertHandleOneByOne(uri, isSyncFromCloud, code, valuesHandle);
    // 插入成功，且操作contact_data表，新增完数据后，处理更新calllog
    handleUpdateCalllogAfterBatchInsertData(ret, code, values);
    g_mutex.unlock();
    HandleHwRelationData(values);
    return ret;
}

void ContactsDataAbility::handleUpdateCalllogAfterBatchInsertData(int ret,
    int code, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    // 插入成功，且操作contact_data表，新增完数据后，处理更新calllog
    if (ret != Contacts::OPERATION_ERROR && code == Contacts::CONTACTS_CONTACT_DATA) {
        std::vector<int> needUpdateCalllogRawIdVector;
        contactDataBase_->parseNeedUpdateCalllogRawIdBatch(values, needUpdateCalllogRawIdVector);
        contactDataBase_->MergeUpdateTask(Contacts::ContactsDataBase::store_, needUpdateCalllogRawIdVector, false);
    }
}

/**
 * @brief Process the data inserted into the relation linked list.
 *
 * @param values Inserted data value of the database
 */
void ContactsDataAbility::HandleHwRelationData(const std::vector<DataShare::DataShareValuesBucket> &values)
{
    std::vector<OHOS::NativeRdb::ValuesBucket> valuesRdb;
    ContactsDataAbility::transferDataShareToRdbBucket(values, valuesRdb);
    unsigned int size = valuesRdb.size();
    bool isContainPhoneOrEmail = false;
    for (unsigned int i = 0; i < size; i++) {
        int typeId = getIntValueFromRdbBucket(valuesRdb[i], Contacts::ContactDataColumns::TYPE_ID);
        if (typeId == Contacts::ContentTypeData::PHONE_INT_VALUE ||
            typeId == Contacts::ContentTypeData::EMAIL_INT_VALUE) {
            isContainPhoneOrEmail = true;
            break;
        }
    }
    HILOG_INFO("HandleHwRelationData isContainPhoneOrEmail = %{public}s, ts = %{public}lld",
        std::to_string(isContainPhoneOrEmail).c_str(), (long long) time(NULL));
}

int ContactsDataAbility::batchInsertHandleOneByOne(const Uri &uri, std::string isSyncFromCloud,
    int code, std::vector<DataShare::DataShareValuesBucket> &valuesHandle)
{
    unsigned int size = valuesHandle.size();
    int ret = contactDataBase_->BeginTransaction();
    if (!IsBeginTransactionOK(ret, g_mutex)) {
        return Contacts::RDB_EXECUTE_FAIL;
    }
// LCOV_EXCL_START
    bool isContainEvent = false;
    int rawContactId = -1;
    for (unsigned int i = 0; i < size; i++) {
        DataShare::DataShareValuesBucket rawContactValues = valuesHandle[i];
        int result = ProcessSingleInsert(rawContactValues, isContainEvent, rawContactId, code, isSyncFromCloud);
        if (result != Contacts::OPERATION_OK) {
            return result;
        }
    }
    int markRet = contactDataBase_->Commit();
    if (!IsCommitOK(markRet, g_mutex)) {
        HILOG_ERROR("batchInsertHandleOneByOne IsCommitOK error!");
        return Contacts::RDB_EXECUTE_FAIL;
    }

    //valuesHandle 这个里面是否有生日，如果有更新到日历数据库，如果没有，则不处理
    if (isContainEvent && rawContactId != -1) {
        contactsConnectAbility_ = OHOS::Contacts::ContactConnectAbility::GetInstance();
        contactsConnectAbility_->
          ConnectEventsHandleAbility(Contacts::CONTACTS_BIRTHDAY_INSERT, std::to_string(rawContactId), "");
    }

    // 成功
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri, isSyncFromCloud);
    HILOG_INFO("batchInsertHandleOneByOne end ts = %{public}lld", (long long) time(NULL));
    return Contacts::OPERATION_OK;
// LCOV_EXCL_STOP
}

int ContactsDataAbility::ProcessSingleInsert(DataShare::DataShareValuesBucket &rawContactValues,
    bool &isContainEvent, int &rawContactId, int &code, std::string &isSyncFromCloud)
{
// LCOV_EXCL_START
    int errorCode = HandleContactPortrait(rawContactValues);
    if (errorCode != Contacts::OPERATION_OK) {
        return errorCode;
    }
    OHOS::NativeRdb::ValuesBucket value = RdbDataShareAdapter::RdbUtils::ToValuesBucket(rawContactValues);
    // 根据code，处理插入contact_data或contact等信息
    int rowRet = InsertExecute(code, value, isSyncFromCloud);
    if (rowRet == Contacts::OPERATION_ERROR) {
        HILOG_ERROR("batchInsertHandleOneByOne rowRet error!");
        contactDataBase_->RollBack();
        return rowRet;
    }
    if (!isContainEvent) {
        int typeId = Contacts::RDB_EXECUTE_FAIL;
        int id = OHOS::NativeRdb::E_OK;
        std::string typeText;
        contactDataBase_->GetTypeText(value, typeId, id, typeText);
        if (typeId == Contacts::ContentTypeData::CONTACT_EVENT_INT_VALUE) {
            isContainEvent = true;
        }
    }
    
    if (rawContactId == -1) {
        OHOS::NativeRdb::ValueObject valueObject;
        value.GetObject(Contacts::ContactDataColumns::RAW_CONTACT_ID, valueObject);
        contactDataBase_->GetContactByValue(rawContactId, valueObject);
        HILOG_INFO("batchInsertHandleOneByOne rawContactId  = %{public}d", rawContactId);
    }
    return Contacts::OPERATION_OK;
// LCOV_EXCL_STOP
}

int ContactsDataAbility::HandleContactPortrait(DataShare::DataShareValuesBucket &value)
{
    std::string portraitFileName = "";
    std::string contactId =  "";
    std::string rawContactId = "";
    bool isValid = false;
    GetPortraitValue(value, portraitFileName, contactId, rawContactId, isValid);
    if (!isValid || portraitFileName.empty() || contactId.empty() || rawContactId.empty()) {
        return Contacts::OPERATION_OK;
    }
    std::string groupPath;
    Contacts::FileUtils fileUtils;
    if (!fileUtils.GetGroupPath(groupPath) || groupPath.empty()) {
        HILOG_ERROR("HandleContactPortrait GetGroupPath failed");
        return Contacts::OPERATION_ERROR;
    }
    std::string srcFilePath = groupPath + portraitFileName;
    std::string filePath = groupPath + contactId + "_" + rawContactId;
    std::vector<uint8_t> blob;
    int32_t blobSource = -1;
    if (SavePortraitFileAndBlob(srcFilePath, filePath, blob, blobSource) != Contacts::OPERATION_OK) {
        // 保存失败时删除客户端存储的srcFilePath 和 服务端生成的filePath
        fileUtils.DeleteFile(srcFilePath);
        fileUtils.DeleteFile(filePath);
        return Contacts::OPERATION_ERROR;
    };
    // 保存成功仅删除 客户端保存的srcFilePath
    fileUtils.DeleteFile(srcFilePath);
    BuildPortraitValue(value, contactId, rawContactId, blob, blobSource);
    return Contacts::OPERATION_OK;
}

int ContactsDataAbility::SavePortraitFileAndBlob(const std::string &srcFilePath, const std::string &filePath,
    std::vector<uint8_t> &blob, int32_t &blobSource)
{
    mode_t filePermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    auto pixelMap = Contacts::PixelMapUtil::GetPixelMapFromFile(srcFilePath);
    if (pixelMap == nullptr) {
        HILOG_ERROR("SavePortraitFileAndBlob GetPixelMapFromFile failed");
        return Contacts::OPERATION_ERROR;
    }
    int srcFd = open(srcFilePath.c_str(), O_RDONLY, filePermissions);
    if (srcFd == Contacts::OPEN_FILE_FAILED) {
        HILOG_ERROR("SavePortraitFileAndBlob open srcFilePath failed");
        return Contacts::OPERATION_ERROR;
    }
    Contacts::PixelMapUtil::GetThumbnailPixelMap(pixelMap, srcFd);
    close(srcFd);
    if (pixelMap == nullptr) {
        HILOG_ERROR("SavePortraitFileAndBlob GetThumbnailPixelMap failed");
        return Contacts::OPERATION_ERROR;
    }
    int fd = open(filePath.c_str(), O_CREAT | O_WRONLY, filePermissions);
    if (fd == Contacts::OPEN_FILE_FAILED) {
        HILOG_ERROR("SavePortraitFileAndBlob open filePath failed");
        return Contacts::OPERATION_ERROR;
    }
    uint32_t errCode = Contacts::PixelMapUtil::SavePixelMapToFile(pixelMap, fd);
    close(fd);
    if (errCode != Contacts::OPERATION_OK) {
        HILOG_ERROR("SavePortraitFileAndBlob pixelMapTofile failed");
        return Contacts::OPERATION_ERROR;
    }
    blobSource = Contacts::PixelMapUtil::GetBlobSourceFromPixelMap(pixelMap);
    
    errCode = Contacts::PixelMapUtil::GetArrayBufferFromPixelMap(pixelMap, blob);
    if (errCode != Contacts::OPERATION_OK) {
        HILOG_ERROR("SavePortraitFileAndBlob get blob failed");
        return Contacts::OPERATION_ERROR;
    }
    return Contacts::OPERATION_OK;
}

void ContactsDataAbility::BuildPortraitValue(DataShare::DataShareValuesBucket &value, const std::string &contactId,
    const std::string &rawContactId, const std::vector<uint8_t> &blob, int32_t blobSource)
{
    value.Clear();
    value.Put("raw_contact_id", rawContactId);
    value.Put("blob_data", blob);
    value.Put("type_id", Contacts::ContentTypeData::PHOTO_INT_VALUE);
    std::string fileName = "photo/" + contactId + "_" + rawContactId;
    value.Put("blob_source", blobSource);
    value.Put("detail_info", fileName);
}

void ContactsDataAbility::GetPortraitValue(const DataShare::DataShareValuesBucket &dataShareValue,
    std::string &portraitFileName, std::string &contactId, std::string &rawContactId, bool &isValid)
{
    portraitFileName = GetStringValueFromDataShareValue(dataShareValue, "PortraitFileName", isValid);
    if (!isValid) {
        HILOG_DEBUG("not have portraitFileName");
        return;
    }
    contactId = GetStringValueFromDataShareValue(dataShareValue, "contactId", isValid);
    if (!isValid) {
        HILOG_DEBUG("not have contactId");
        return;
    }
    rawContactId = GetStringValueFromDataShareValue(dataShareValue, "rawContactId", isValid);
    if (!isValid) {
        HILOG_DEBUG("not have rawContactId");
        return;
    }

}

std::string ContactsDataAbility::GetStringValueFromDataShareValue(
    const DataShare::DataShareValuesBucket &dataShareValue, const std::string &valueName, bool &isValid)
{
    std::string value;
    DataShare::DataShareValueObject valueObject = dataShareValue.Get(valueName, isValid);
    if (!isValid) {
        return value;
    }
    
    if (const auto* stringPtr = std::get_if<std::string>(&valueObject.value)) {
        value = *stringPtr;
    }
    if (value.empty()) {
        isValid = false;
    }
    return value;
}

/**
 * @brief ContactsDataAbility BatchInsert database
 *
 * @param uri Determine the data table name based on the URI
 * @param value Inserted data value of the database
 *
 * @return BatchInsert database results code
 */
int ContactsDataAbility::BatchInsertByMigrate(
    int code, const Uri &uri, const std::vector<DataShare::DataShareValuesBucket> &values)
{
    HILOG_INFO("BatchInsertByMigrate start");
    OHOS::Uri uriTemp = uri;
    // 是否云同步
    std::string isSyncFromCloud = UriParseParam(uriTemp);
    HILOG_INFO("ContactsDataAbility BatchInsertByMigrate isSyncFromCloud = %{public}s, code = %{public}d",
        isSyncFromCloud.c_str(), code);
    int ret = 0;
    // 批量操作raw_contact
    if (code == Contacts::CONTACTS_RAW_CONTACT) {
        // 此处未记录通知id，批量新增联系人，一定后边有新增contact_data
        ret = contactDataBase_->BatchInsertRawContacts(values);
    } else {
        // 批量insert contact_data，这里边记录了需要通知的raw_contact_id
        ret = contactDataBase_->BatchInsert(Contacts::ContactTableName::CONTACT_DATA, values, isSyncFromCloud);
        if (Contacts::PrivacyContactsManager::IsPrivacySpace()) {
            Contacts::PrivacyContactsManager::GetInstance()->InsertContactDataToPrivacyBackup(values);
        }
    }
    DataBaseNotifyChange(Contacts::CONTACT_INSERT, uri, isSyncFromCloud);
    HILOG_INFO("BatchInsertByMigrate end");
    return ret;
}

/**
 * @brief Update ContactsDataAbility in the database
 *
 * @param uri URI of the data table to be inserted
 * @param predicates Conditions for updating data value
 *
 * @return Update database results code
 */
int ContactsDataAbility::Update(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, const DataShare::DataShareValuesBucket &value)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CONTACTS)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    OHOS::NativeRdb::ValuesBucket valuesBucket = RdbDataShareAdapter::RdbUtils::ToValuesBucket(value);
    Contacts::SqlAnalyzer sqlAnalyzer;
    bool isOk = sqlAnalyzer.CheckValuesBucket(valuesBucket);
    if (!isOk) {
        HILOG_ERROR("ContactsDataAbility CheckValuesBucket error");
        return Contacts::RDB_EXECUTE_FAIL;
    }
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    int retCode = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    std::string isSyncFromCloud = UriParseParam(uriTemp);
    HILOG_INFO("getLockThen Update,ts = %{public}lld", (long long) time(NULL));
    int code = UriParseAndSwitch(uriTemp);
    DataShare::DataSharePredicates dataSharePredicates = predicates;

    UpdateExecute(retCode, code, valuesBucket, dataSharePredicates, isSyncFromCloud);
    g_mutex.unlock();
    if (retCode > 0) {
        HILOG_INFO("ContactsDataAbility ====>update row is %{public}d,ts = %{public}lld,uri = %{public}s",
                   retCode, (long long) time(NULL), Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str());
        DataBaseNotifyChange(Contacts::CONTACT_UPDATE, uri, isSyncFromCloud);
    } else {
        HILOG_INFO("ContactsDataAbility ====>update row is %{public}d,ts = %{public}lld, skill notify",
            retCode, (long long) time(NULL));
    }
    return retCode;
}

void ContactsDataAbility::UpdateExecute(int &retCode, int code, const OHOS::NativeRdb::ValuesBucket &value,
    DataShare::DataSharePredicates &dataSharePredicates, std::string isSync)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    std::string manualMergeIdStr;
    switch (code) {
        case Contacts::CONTACTS_CONTACT:
        case Contacts::PROFILE_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT, dataSharePredicates);
            retCode = contactDataBase_->Update(value, rdbPredicates);
            break;
        case Contacts::MERGE_CONTACT:
            manualMergeIdStr =
                ContactsDataAbility::getStringValueFromRdbBucket(value, "manualMergeIdStr");
            contactsConnectAbility_->ConnectAbility("", "", "", "", "mergeContact", manualMergeIdStr);
            retCode = 0;
            break;
        case Contacts::UPDATE_YELLOW_PAGE_INFO_FOR_CALLLOG:
            HILOG_INFO("ContactsDataAbility ====>update row UPDATE_YELLOW_PAGE_INFO_FOR_CALLLOG");
            contactsConnectAbility_->ConnectAbility("", "", "", "", "updateYellowPageInfo", "");
            break;
        case Contacts::CONTACTS_RAW_CONTACT:
        case Contacts::PROFILE_RAW_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->UpdateRawContact(value, rdbPredicates, isSync);
            break;
        case Contacts::CONTACTS_CONTACT_DATA:
        case Contacts::PROFILE_CONTACT_DATA:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT_DATA, dataSharePredicates);
            retCode = contactDataBase_->UpdateContactData(value, rdbPredicates, isSync);
            break;
        case Contacts::CONTACTS_GROUPS:
        case Contacts::PROFILE_GROUPS:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::GROUPS, dataSharePredicates);
            retCode = contactDataBase_->Update(value, rdbPredicates);
            break;
        case Contacts::CLOUD_DATA:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CLOUD_RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->UpdateCloudRawContacts(value, rdbPredicates);
            break;
        case Contacts::QUERY_IS_REFRESH_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::SETTINGS, dataSharePredicates);
            retCode = contactDataBase_->UpdateTable(value, rdbPredicates);
            break;
        case Contacts::PRIVACY_CONTACTS_BACKUP:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::PRIVACY_CONTACTS_BACKUP, dataSharePredicates);
            retCode = contactDataBase_->UpdatePrivacyContactsBackup(value, rdbPredicates);
            break;
        case Contacts::CONTACTS_POSTER:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::POSTER, dataSharePredicates);
            retCode = contactDataBase_->Update(value, rdbPredicates);
            break;
        default:
            SwitchUpdate(retCode, code, value, dataSharePredicates);
            break;
    }
}

void ContactsDataAbility::SwitchUpdate(int &retCode, int &code, const OHOS::NativeRdb::ValuesBucket &value,
    DataShare::DataSharePredicates &dataSharePredicates)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (code) {
        case Contacts::CONTACTS_BLOCKLIST:
        case Contacts::PROFILE_BLOCKLIST:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::CONTACT_BLOCKLIST, dataSharePredicates);
            retCode = contactDataBase_->UpdateBlockList(value, rdbPredicates);
            break;
        case Contacts::UPLOAD_DATA_TO_CLOUD:
            retCode = contactDataBase_->SyncContacts();
            break;
        case Contacts::SET_FULL_SEARCH_SWITCH_OPEN:
            retCode = contactDataBase_->SetFullSearchSwitch(true);
            break;
        case Contacts::SET_FULL_SEARCH_SWITCH_CLOSE:
            retCode = contactDataBase_->SetFullSearchSwitch(false);
            break;
        case Contacts::TRIGGER_UPLOAD_CLOUD:
            contactsConnectAbility_->ConnectAbility("", "", "", "", "", "");
            retCode = Contacts::RDB_EXECUTE_OK;
            break;
        case Contacts::SPLIT_AGGREGATION_CONTACT:
            contactsConnectAbility_->ConnectAbility("", "", "", "", "split_aggregation_contact", "");
            retCode = Contacts::RDB_EXECUTE_OK;
            break;
        case Contacts::HW_ACCOUNT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::HW_ACCOUNT, dataSharePredicates);
            retCode = contactDataBase_->Update(value, rdbPredicates);
            break;
        case Contacts::SPLIT_CONTACT:
            retCode = contactDataBase_->Split(dataSharePredicates);
            break;
        case Contacts::MANUAL_MERGE:
            retCode = contactDataBase_->ReContactMerge(dataSharePredicates);
            break;
        case Contacts::AUTO_MERGE:
            retCode = contactDataBase_->ContactMerge();
            break;
        // 查询仅返回一个数量，如果使用query，会返回resultSet，所以放到了这里
        case Contacts::QUERY_CONTACT_COUNT_DOUBLE_DB:
            retCode = contactDataBase_->QueryContactCountDoubleDb();
            break;
        default:
            SwitchUpdateOthers(retCode, code, value, dataSharePredicates);
            break;
    }
}

void ContactsDataAbility::SwitchUpdateOthers(int &retCode, int &code, const OHOS::NativeRdb::ValuesBucket &value,
    DataShare::DataSharePredicates &dataSharePredicates)
{
    switch (code) {
        case Contacts::RESTORE_CONTACT_BY_DOUBLE_TO_SINGLE_UPGRADE_DATA:
            // 返回启动任务成功失败
            retCode = contactsConnectAbility_->ConnectAbility("", "",
                "", "", "restoreContactByDoubleToSingleUpgradeData", "");
            break;
        case Contacts::CONTACT_BACKUP:
        case Contacts::PROFILE_BACKUP:
            retCode = BackUp();
            break;
        case Contacts::CONTACT_RECOVER:
        case Contacts::PROFILE_RECOVER:
            retCode = Recover(code);
            break;
        case Contacts::REBUILD_CONTACT_SERACH_INDEX:
        case Contacts::CLEAR_AND_RECREATE_TRIGGER_SEARCH_TABLE:
            // 清空搜索表，重建触发器接口
            // 本接口慎重调用，当前仅drop联系人相关表才调用；以后如新场景需调用，需与数据库同事讨论对齐
            retCode = contactDataBase_->clearAndReCreateTriggerSearchTable();
            break;
        default:
            retCode = Contacts::RDB_EXECUTE_FAIL;
            HILOG_ERROR("ContactsDataAbility ====>no match uri action, %{public}d", code);
            break;
    }
}

/**
 * @brief Delete ContactsDataAbility from the database
 *
 * @param uri URI for the data table storing ContactsDataAbility
 * @param predicates Conditions for deleting data values
 *
 * @return Delete database results code
 */
int ContactsDataAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::WRITE_CONTACTS)) {
        HILOG_ERROR("Permission denied!");
        return Contacts::RDB_PERMISSION_ERROR;
    }
    g_mutex.lock();
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    int retCode = Contacts::RDB_EXECUTE_FAIL;
    OHOS::Uri uriTemp = uri;
    std::string isSyncFromCloud = UriParseParam(uriTemp);
    HILOG_INFO("getLockThen Delete, ts = %{public}lld",
        (long long) time(NULL));
    int code = UriParseAndSwitch(uriTemp);
    std::string handleType = UriParseHandleTypeParam(uriTemp);
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    DeleteExecute(retCode, code, dataSharePredicates, isSyncFromCloud, handleType);
    HILOG_INFO("Delete, size = %{public}d",
        retCode);
    g_mutex.unlock();
    DataBaseNotifyChange(Contacts::CONTACT_DELETE, uri, isSyncFromCloud);
    return retCode;
}

void ContactsDataAbility::DeleteExecute(
    int &retCode, int code, DataShare::DataSharePredicates &dataSharePredicates,
    std::string isSync, std::string handleType)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (code) {
        // 删除联系人
        case Contacts::CONTACTS_CONTACT:
        case Contacts::PROFILE_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT, dataSharePredicates);
            retCode = contactDataBase_->DeleteContact(rdbPredicates, isSync);
            break;
        case Contacts::CONTACTS_CONTACT_DATA:
        case Contacts::PROFILE_CONTACT_DATA:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT_DATA, dataSharePredicates);
            retCode = contactDataBase_->DeleteContactData(rdbPredicates, isSync);
            break;
        case Contacts::CONTACTS_GROUPS:
        case Contacts::PROFILE_GROUPS:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::GROUPS, dataSharePredicates);
            retCode = contactDataBase_->DeleteGroup(rdbPredicates);
            break;
        case Contacts::CONTACTS_BLOCKLIST:
        case Contacts::PROFILE_BLOCKLIST:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::CONTACT_BLOCKLIST, dataSharePredicates);
            retCode = contactDataBase_->DeleteBlockList(rdbPredicates);
            break;
        case Contacts::PRIVACY_CONTACTS_BACKUP:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::PRIVACY_CONTACTS_BACKUP, dataSharePredicates);
            retCode = contactDataBase_->DeletePrivacyContactsBackup(rdbPredicates);
            break;
        case Contacts::CONTACTS_POSTER:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::POSTER, dataSharePredicates);
            retCode = contactDataBase_->DeletePoster(rdbPredicates);
            break;
        default:
            DeleteExecuteSwitchSplit(retCode, code, dataSharePredicates, isSync, handleType);
            break;
    }
}

void ContactsDataAbility::DeleteExecuteSwitchSplit(
    int &retCode, int code, DataShare::DataSharePredicates &dataSharePredicates,
    std::string isSync, std::string handleType)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    switch (code) {
        // 彻底删除
        case Contacts::CONTACTS_DELETE:
        case Contacts::PROFILE_DELETE:
            rdbPredicates =predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::DELETE_RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->CompletelyDelete(rdbPredicates, handleType);
            break;
        // 硬删除联系人
        case Contacts::CONTACTS_HARD_DELETE:
            // 删除条件
            rdbPredicates =predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->HardDelete(rdbPredicates);
            break;
        case Contacts::CONTACTS_DELETE_RECORD:
        case Contacts::PROFILE_DELETE_RECORD:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::DELETE_RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->DeleteRecord(rdbPredicates);
            break;
        case Contacts::CLOUD_DATA:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::CLOUD_RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->DeleteCloudRawContact(rdbPredicates);
            break;
        // 最近删除恢复
        case Contacts::RECYCLE_RESTORE:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::DELETE_RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->RecycleRestore(rdbPredicates, isSync);
            break;
        case Contacts::CONTACTS_DELETE_MERGE:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::CONTACT, dataSharePredicates);
            retCode = contactDataBase_->DeleteMergeRawContact(rdbPredicates);
            break;
        case Contacts::CONTACTS_ADD_FAILED_DELETE:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::RAW_CONTACT, dataSharePredicates);
            retCode = contactDataBase_->HardDelete(rdbPredicates);
            break;
        default:
            retCode = Contacts::OPERATION_ERROR;
            break;
    }
}

/**
 * @brief Query contact data in the database
 *
 * @param uri URI of the data table that stores the contact data
 * @param columns Columns where the contact data is located
 * @param predicates Condition for querying data values
 *
 * @return Database query result
 */
std::shared_ptr<DataShare::DataShareResultSet> ContactsDataAbility::Query(const Uri &uri,
    const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    if (!Telephony::TelephonyPermission::CheckPermission(Telephony::Permission::READ_CONTACTS)) {
        HILOG_ERROR("ContactsDataAbility ====>Query Permission denied!");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_mutexQuery);
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    std::shared_ptr<OHOS::NativeRdb::ResultSet> result;
    OHOS::Uri uriTemp = uri;
    int parseCode = UriParseAndSwitch(uriTemp);
    HILOG_INFO("getLockThen ====>Query start, uri = %{public}s, ts = %{public}lld",
        Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(),
        (long long) time(NULL));
    std::vector<std::string> columnsTemp = columns;
    DataShare::DataSharePredicates dataSharePredicates = predicates;
    bool isUriMatch = QueryExecute(result, dataSharePredicates, columnsTemp, parseCode, uriTemp);
    if (!isUriMatch) {
        return nullptr;
    }
    if (result == nullptr) {
        HILOG_ERROR("AbsSharedResultSet result is nullptr in ContactsDataAbility::Query,ts = %{public}lld", (long long) time(NULL));
        return nullptr;
    }
    auto queryResultSet = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(result);
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult =
        std::make_shared<DataShare::DataShareResultSet>(queryResultSet);
    HILOG_WARN("ContactsDataAbility ====>Query end, uri = %{public}s, ts = %{public}lld",
        Contacts::ContactsDataBase::getUriLogPrintByUri(uriTemp).c_str(),
        (long long) time(NULL));
    return sharedPtrResult;
}

bool ContactsDataAbility::QueryExecute(std::shared_ptr<OHOS::NativeRdb::ResultSet> &result,
    DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp,
    int &parseCode, OHOS::Uri &uri)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    bool isUriMatch = true;
    switch (parseCode) {
        case Contacts::CONTACTS_CONTACT:
        case Contacts::PROFILE_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_CONTACT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_RAW_CONTACT:
        case Contacts::PROFILE_RAW_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_RAW_CONTACT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_CONTACT_DATA:
        case Contacts::PROFILE_CONTACT_DATA:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_CONTACT_DATA, dataSharePredicates);
            // 如果查询contact_data表，需要判断是不是传递了查询（删除|未删除）条件，如果没有传，则增加查询未删除的联系人条件
            AddQueryNotDeleteCondition(rdbPredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_GROUPS:
        case Contacts::PROFILE_GROUPS:
            rdbPredicates = predicatesConvert.ConvertPredicates(Contacts::ViewName::VIEW_GROUPS, dataSharePredicates);
            //单框架目前没有名片夹功能，先屏蔽查询名片夹群组
            AddQueryNotCCardGroupCondition(rdbPredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CLOUD_DATA:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CLOUD_RAW_CONTACT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::QUERY_UUID_NOT_IN_RAW_CONTACT:
            result = contactDataBase_->QueryUuidNotInRawContact();
            break;
        case Contacts::QUERY_BIG_LENGTH_NAME:
            result = contactDataBase_->QueryBigLengthNameContact();
            break;
        case Contacts::HW_ACCOUNT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::HW_ACCOUNT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::QUERY_IS_REFRESH_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::SETTINGS, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::PRIVACY_CONTACTS_BACKUP:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::PRIVACY_CONTACTS_BACKUP, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_COMPANY_CLASSIFY:
            result = contactDataBase_->QueryContactByCompanyGroup();
            break;
        case Contacts::QUERY_COUNT_WITHOUT_COMPANY:
            result = contactDataBase_->QueryCountWithoutCompany();
            break;
        case Contacts::CONTACTS_LOCATION_CLASSIFY:
            result = contactDataBase_->QueryContactByLocationGroup();
            break;
        case Contacts::QUERY_COUNT_WITHOUT_LOCATION:
            result = contactDataBase_->QueryCountWithoutLocation();
            break;
        case Contacts::CONTACTS_FREQUENT_CLASSIFY:
            result = contactDataBase_->QueryContactByRecentTime();
            break;
        case Contacts::CONTACTS_CONTACT_LOCATION:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT_DATA, dataSharePredicates);
            result = contactDataBase_->QueryLocationContact(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_DETECT_REPAIR:
            result = contactDataBase_->QueryDetectRepair();
            break;
        case Contacts::CONTACTS_POSTER:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::POSTER, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_DOWNLOAD_POSTERS:
            if (columnsTemp.size() > 0) {
                result = contactDataBase_->DownLoadPosters(columnsTemp[0]);
            }
            break;
        default:
            isUriMatch = QueryExecuteSwitchSplit(result, dataSharePredicates, columnsTemp, parseCode, uri);
            break;
    }
    return isUriMatch;
}

/**
 * 添加查询不包含名片夹群组的信息条件
 * @param rdbPredicates
 */
void ContactsDataAbility::AddQueryNotCCardGroupCondition (OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    HILOG_INFO("query contactsgroups, add group_name condition, ts = %{public}lld", (long long) time(NULL));
    rdbPredicates.And();
    rdbPredicates.NotEqualTo(Contacts::GroupsColumns::GROUP_NAME, "PREDEFINED_OHOS_GROUP_CCARD");
}
/**
 * 添加查询未删除的信息条件
 * @param rdbPredicates
 */
void ContactsDataAbility::AddQueryNotDeleteCondition (OHOS::NativeRdb::RdbPredicates &rdbPredicates)
{
    std::string whereSql = rdbPredicates.GetWhereClause();
    if (whereSql.find(Contacts::RawContactColumns::IS_DELETED) == std::string::npos) {
        HILOG_INFO("query contactdata, not has is_delete conditoin, add condition, ts = %{public}lld", (long long) time(NULL));
        // 不存在是否删除条件，需要设置为0
        rdbPredicates.And();
        rdbPredicates.EqualTo(Contacts::RawContactColumns::IS_DELETED, Contacts::NOT_DELETE_MARK);
    }
}

bool ContactsDataAbility::QueryExecuteSwitchSplit(std::shared_ptr<OHOS::NativeRdb::ResultSet> &result,
    DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp,
    int &parseCode, OHOS::Uri &uri)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    bool isUriMatch = true;
    switch (parseCode) {
        case Contacts::CONTACTS_BLOCKLIST:
        case Contacts::PROFILE_BLOCKLIST:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::CONTACT_BLOCKLIST, dataSharePredicates);
            result = blocklistDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::PULLDOWN_SYNC_CONTACT_DATA:
            contactsConnectAbility_->ConnectAbility("", "", "", "queryUuid", "", "");
            break;
        case Contacts::SYNC_BIRTHDAY_TO_CALENDAR:
            contactsConnectAbility_->ConnectEventsHandleAbility(Contacts::CONTACTS_BIRTHDAY_SYNC, "", "syncBirthdays");
            break;
        // 联系人冷启动后，触发，数据库处理：（1）判断是否有数据要恢复（2）判断是否有迁移数据，有的话，主空间需要触发次数据同步
        case Contacts::BACKUP_RESTORE:
            contactDataBase_->DataSyncAfterPrivateAndMainSpaceDataMigration();
            break;
        case Contacts::CONTACTS_DELETE:
        case Contacts::PROFILE_DELETE:
            rdbPredicates = predicatesConvert.ConvertPredicates(
                Contacts::ContactTableName::DELETE_RAW_CONTACT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::CONTACTS_SEARCH_CONTACT:
        case Contacts::PROFILE_SEARCH_CONTACT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ViewName::SEARCH_CONTACT_VIEW, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        case Contacts::QUERY_MERGE_LIST:
            result = contactDataBase_->SelectCandidate();
            break;
        case Contacts::CONTACT_TYPE:
        case Contacts::PROFILE_TYPE:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::CONTACT_TYPE, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        default:
            isUriMatch = QueryExecuteSwitchSplitOthers(result, dataSharePredicates, columnsTemp, parseCode, uri);
            break;
    }
    return isUriMatch;
}

bool ContactsDataAbility::QueryExecuteSwitchSplitOthers(std::shared_ptr<OHOS::NativeRdb::ResultSet> &result,
    DataShare::DataSharePredicates &dataSharePredicates, std::vector<std::string> &columnsTemp, int &parseCode,
    OHOS::Uri &uri)
{
    Contacts::PredicatesConvert predicatesConvert;
    OHOS::NativeRdb::RdbPredicates rdbPredicates("");
    bool isUriMatch = true;
    switch (parseCode) {
        case Contacts::ACCOUNT:
            rdbPredicates =
                predicatesConvert.ConvertPredicates(Contacts::ContactTableName::ACCOUNT, dataSharePredicates);
            result = contactDataBase_->Query(rdbPredicates, columnsTemp);
            break;
        default:
            isUriMatch = false;
            HILOG_ERROR("ContactsDataAbility ====>no matching uri action");
            break;
    }
    return isUriMatch;
}

std::string ContactsDataAbility::UriParseParam(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    std::string isSyncFromCloud;
    std::map<std::string, std::string> mapParams;
    mapParams = uriUtils.getQueryParameter(uri);
    if (!mapParams.empty()) {
        isSyncFromCloud = mapParams["isSyncFromCloud"];
    }
    return isSyncFromCloud;
}

std::string ContactsDataAbility::UriParseBatchParam(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    std::string isFromBatch;
    std::map<std::string, std::string> mapParams;
    mapParams = uriUtils.getQueryParameter(uri);
    if (!mapParams.empty()) {
        isFromBatch = mapParams["isFromBatch"];
    }
    return isFromBatch;
}

std::string ContactsDataAbility::UriParseHandleTypeParam(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    std::string handleType;
    std::map<std::string, std::string> mapParams;
    mapParams = uriUtils.getQueryParameter(uri);
    if (!mapParams.empty()) {
        handleType = mapParams["handleType"];
    }
    return handleType;
}

int ContactsDataAbility::UriParseAndSwitch(Uri &uri)
{
    Contacts::UriUtils uriUtils;
    int parseCode = uriUtils.UriParse(uri, uriValueMap_);
    if (parseCode != Contacts::OPERATION_ERROR) {
        SwitchProfile(uri);
    }
    return parseCode;
}

void ContactsDataAbility::SwitchProfile(Uri &uri)
{
    std::vector<std::string> pathVector;
    uri.GetPathSegments(pathVector);
    if (pathVector.size() > 1 && pathVector[1].find("profile") == std::string::npos) {
        contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
        contactDataBase_->store_ = contactDataBase_->contactStore_;
    } else {
        profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
        contactDataBase_->store_ = profileDataBase_->store_;
    }
}

int ContactsDataAbility::BackUp()
{
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    if (contactDataBase_->store_ == nullptr) {
        HILOG_ERROR("store_ is null, skip backup");
        return 0;
    }
    int retCode = contactDataBase_->store_->Backup("contacts.db.bak");
    if (retCode != OHOS::NativeRdb::E_OK) {
        HILOG_ERROR("backup err,retCode = %{public}d", retCode);
    } else {
        HILOG_INFO("backup succeed,retCode = %{public}d", retCode);
    }
    return retCode;
}

int ContactsDataAbility::Recover(int &code)
{
    std::string name = Contacts::PROFILE_DATABASE_NAME;
    if (code == Contacts::CONTACT_RECOVER) {
        name = Contacts::CONTACT_DATABASE_NAME;
    }
    std::shared_ptr<OHOS::Contacts::DataBaseDisasterRecovery> instance =
        OHOS::Contacts::DataBaseDisasterRecovery::GetInstance();
    int retCode = instance->RecoveryDatabase(name);
    contactDataBase_ = Contacts::ContactsDataBase::GetInstance();
    profileDataBase_ = Contacts::ProfileDatabase::GetInstance();
    return retCode;
}
void ContactsDataAbility::DataBaseNotifyChange(int code, Uri uri)
{
    HILOG_INFO("Contacts DataBaseNotifyChange start.");
    Contacts::ContactsCommonEvent::SendContactChange(code);
}
void ContactsDataAbility::DataBaseNotifyChange(int code, Uri uri, std::string isSync)
{
    HILOG_INFO("DataBaseNotifyChange isSync start.code = %{public}d, isSync = %{public}s,ts = %{public}lld",
        code, isSync.c_str(), (long long) time(NULL));
    Contacts::ContactsCommonEvent::SendContactChange(code);
    if (isSync != "true") {
        contactDataBase_->UpdateDirtyToOne();
        contactsConnectAbility_->ConnectAbility("", "", "", "", "", "");
    }
    contactDataBase_->UpdateContactTimeStamp();
}
} // namespace AbilityRuntime
} // namespace OHOS