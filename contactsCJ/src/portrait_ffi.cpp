/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "portrait_ffi.h"
#include "ability_runtime/cj_ability_context.h"
#include "contacts_control.h"
#include "contacts_utils.h"
#include "datashare_helper.h"
#include "hilog_wrapper_api.h"
#include "native/ffi_remote_data.h"
#include "pixel_map_impl.h"
#include "image_packer.h"
#include <unistd.h>
#include <climits>

using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::ContactsFfi;
using namespace OHOS::DataShare;
using namespace OHOS::Media;
using namespace OHOS::FFI;

namespace OHOS {
namespace ContactsFfi {

constexpr int OPEN_FILE_FAILED = -1;
// ContactsDataShareStubImpl::OpenFile returns -2 on permission denial (see NAPI
// contacts_api.cpp:1328 which checks fd == RDB_PERMISSION_ERROR).
constexpr int RDB_PERMISSION_ERROR = -2;
constexpr int ERR_OK = 0;
constexpr uint8_t IMAGE_QUALITY = 90;
constexpr uint32_t IMAGE_NUMBER_HINT = 1;
const std::string IMAGE_FORMAT = "image/jpeg";
constexpr int32_t IMAGE_MAX_HEIGHT = 1080;
constexpr int32_t IMAGE_MAX_WIDTH = 1920;

static std::shared_ptr<DataShareHelper> GetDsHelper(int64_t contextId)
{
    sptr<CJAbilityContext> context = FFIData::GetData<CJAbilityContext>(contextId);
    if (context == nullptr) {
        HILOG_ERROR("GetDsHelper context is null");
        return nullptr;
    }
    return DataShareHelper::Creator(context->GetToken(), CONTACTS_DATA_URI);
}

static std::string QueryContactId(std::shared_ptr<DataShareHelper>& dataShareHelper, int rawContactId)
{
    ContactsControl contactsControl;
    std::vector<std::string> columns;
    columns.push_back("contact_id");
    auto resultSet = contactsControl.QueryContactByRawContactId(dataShareHelper, columns, rawContactId);
    if (resultSet == nullptr) {
        return "";
    }
    int rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != 0 || rowCount == 0 || resultSet->GoToFirstRow() != 0) {
        resultSet->Close();
        return "";
    }
    std::string contactId;
    int columnIndex = 0;
    if (resultSet->GetColumnIndex("contact_id", columnIndex) != 0) {
        HILOG_ERROR("QueryContactId GetColumnIndex failed for contact_id");
        resultSet->Close();
        return "";
    }
    if (resultSet->GetString(columnIndex, contactId) != 0) {
        HILOG_ERROR("QueryContactId GetString failed for contact_id");
        resultSet->Close();
        return "";
    }
    resultSet->Close();
    return contactId;
}

static int SavePixelMapToFile(const std::shared_ptr<PixelMap>& pixelMap, int fd)
{
    if (pixelMap == nullptr) {
        return -1;
    }
    // Value-initialize so info.size is zeroed when GetImageInfo does not fill it; the API returns
    // void and cannot be checked. scale() also returns void.
    ImageInfo info{};
    pixelMap->GetImageInfo(info);
    int32_t height = (info.size.height != 0) ? info.size.height : 100;
    int32_t width = (info.size.width != 0) ? info.size.width : 100;
    if (height > IMAGE_MAX_HEIGHT || width > IMAGE_MAX_WIDTH) {
        float scale = height > width ?
            (static_cast<float>(IMAGE_MAX_HEIGHT) / height) : (static_cast<float>(IMAGE_MAX_WIDTH) / width);
        pixelMap->scale(scale, scale);
    }
    PackOption option = {
        .format = IMAGE_FORMAT,
        .quality = IMAGE_QUALITY,
        .numberHint = IMAGE_NUMBER_HINT,
        .desiredDynamicRange = EncodeDynamicRange::AUTO,
        .needsPackProperties = true,
    };
    ImagePacker imagePacker;
    uint32_t result = imagePacker.StartPacking(fd, option);
    if (result != 0) {
        HILOG_ERROR("SavePixelMapToFile StartPacking failed %{public}u", result);
        return result;
    }
    result = imagePacker.AddImage(*pixelMap);
    if (result != 0) {
        HILOG_ERROR("SavePixelMapToFile AddImage failed %{public}u", result);
        return result;
    }
    result = imagePacker.FinalizePacking();
    if (result != 0) {
        HILOG_ERROR("SavePixelMapToFile FinalizePacking failed %{public}u", result);
        return result;
    }
    return ERR_OK;
}

static void GetPixelMapSize(const std::shared_ptr<PixelMap>& pixelMap, int32_t &height, int32_t &width)
{
    if (pixelMap == nullptr) {
        height = 0;
        width = 0;
        return;
    }
    // Value-initialize so height/width are 0 even if GetImageInfo fails to fill them; the API
    // returns void so the return value cannot be checked.
    ImageInfo info{};
    pixelMap->GetImageInfo(info);
    height = info.size.height;
    width = info.size.width;
}

static void HandleInsertFailed(ContactsControl& contactsControl,
    std::shared_ptr<DataShareHelper>& dataShareHelper, int64_t rawContactId, const std::string& fileName)
{
    DataSharePredicates predicates;
    predicates.EqualTo("id", rawContactId);
    contactsControl.HandleAddFailed(dataShareHelper, predicates, fileName);
}

static std::shared_ptr<PixelMap> GetValidPixelMap(
    int64_t photoId,
    int32_t* errCode)
{
    auto pixelMapImpl = FFIData::GetData<PixelMapImpl>(photoId);
    if (pixelMapImpl == nullptr) {
        HILOG_ERROR("GetValidPixelMap pixelMapImpl is null");
        *errCode = ERROR;
        return nullptr;
    }

    std::shared_ptr<PixelMap> pixelMap = pixelMapImpl->GetRealPixelMap();
    if (pixelMap == nullptr) {
        HILOG_ERROR("GetValidPixelMap pixelMap is null");
        *errCode = ERROR;
        return nullptr;
    }
    return pixelMap;
}

static int SavePortraitData(PortraitContext ctx, ContactIdentity identity, ImageSize imageSize,
    CPortrait* portrait, bool isAddType)
{
    DataShareValuesBucket valuesBucket;
    valuesBucket.Put("PortraitFileName", identity.fileName);
    valuesBucket.Put("contactId", identity.contactId);
    valuesBucket.Put("rawContactId", std::to_string(identity.rawContactId));
    valuesBucket.Put("srcHeight", imageSize.height);
    valuesBucket.Put("srcWidth", imageSize.width);
    bool isUriPortrait = portrait->hasUri && portrait->uri != nullptr && strlen(portrait->uri) > 0;
    int addPortraitType = isAddType ? (isUriPortrait ? 1 : 2) : 3;
    valuesBucket.Put("addPortraitType", addPortraitType);

    std::vector<DataShareValuesBucket> valueContactData;
    valueContactData.push_back(valuesBucket);
    return ctx.contactsControl.ContactDataInsert(ctx.dataShareHelper, valueContactData);
}

static int OpenAndSavePortrait(PortraitContext ctx, ContactIdentity identity,
    std::shared_ptr<PixelMap>& pixelMap, ImageSize& imageSize)
{
    int fd = ctx.contactsControl.OpenFileByDataShare(identity.fileName, ctx.dataShareHelper);
    if (fd == OPEN_FILE_FAILED || fd == PERMISSION_ERROR || fd == RDB_PERMISSION_ERROR) {
        HILOG_ERROR("OpenAndSavePortrait OpenFileByDataShare failed, fd: %{public}d", fd);
        HandleInsertFailed(ctx.contactsControl, ctx.dataShareHelper, identity.rawContactId, identity.fileName);
        return fd;
    }

    GetPixelMapSize(pixelMap, imageSize.height, imageSize.width);
    int result = SavePixelMapToFile(pixelMap, fd);
    close(fd);
    if (result != ERR_OK) {
        HandleInsertFailed(ctx.contactsControl, ctx.dataShareHelper, identity.rawContactId, identity.fileName);
        return result;
    }
    return ERR_OK;
}

// contactId is queried from the DB and spliced into a file URI (savePhoto/<fileName>); reject
// path separators and ".." so a malicious/odd contact_id cannot traverse the filesystem.
static bool IsValidContactId(const std::string& contactId)
{
    if (contactId.empty()) {
        return false;
    }
    if (contactId.find('/') != std::string::npos || contactId.find('\\') != std::string::npos) {
        return false;
    }
    if (contactId.find("..") != std::string::npos) {
        return false;
    }
    return true;
}

// Writes the pixel map to the portrait file and persists the portrait row; releases the helper.
// ContactIdentity (contactId/rawContactId/fileName) is built by the caller to keep the
// parameter count at 5.
static int32_t FinalizePortraitSave(const ContactIdentity& identity, CPortrait* portrait, bool isAddType,
    std::shared_ptr<DataShareHelper>& dataShareHelper, std::shared_ptr<PixelMap>& pixelMap)
{
    ContactsControl contactsControl;
    PortraitContext ctx = {contactsControl, dataShareHelper};
    ImageSize imageSize;
    int result = OpenAndSavePortrait(ctx, identity, pixelMap, imageSize);
    if (result != ERR_OK) {
        dataShareHelper->Release();
        return result;
    }
    result = SavePortraitData(ctx, identity, imageSize, portrait, isAddType);
    dataShareHelper->Release();
    return result;
}

int32_t CJInsertPortrait(int64_t contextId, int64_t rawContactId, CPortrait* portrait,
    bool isAddType)
{
    int32_t errCode = ERR_OK;
    if (portrait == nullptr || !portrait->hasPhoto) {
        HILOG_ERROR("CJInsertPortrait portrait is null or has no photo");
        return ERROR;
    }
    // rawContactId is int64_t but QueryContactId/QueryContactByRawContactId take int; guard the
    // implicit narrowing so a value beyond INT_MAX is not silently truncated to another contact.
    if (rawContactId < 0 || rawContactId > INT_MAX) {
        HILOG_ERROR("CJInsertPortrait rawContactId out of int range: %{public}lld",
            static_cast<long long>(rawContactId));
        return PARAMETER_ERROR;
    }

    auto dataShareHelper = GetDsHelper(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJInsertPortrait Permission denied!");
        return PERMISSION_ERROR;
    }

    std::shared_ptr<PixelMap> pixelMap = GetValidPixelMap(portrait->photoId, &errCode);
    if (pixelMap == nullptr) {
        dataShareHelper->Release();
        return errCode;
    }

    std::string contactId = QueryContactId(dataShareHelper, rawContactId);
    if (contactId.empty()) {
        HILOG_ERROR("CJInsertPortrait contactId is empty");
        dataShareHelper->Release();
        return OPEN_FILE_FAILED;
    }
    if (!IsValidContactId(contactId)) {
        HILOG_ERROR("CJInsertPortrait contactId contains invalid path characters");
        dataShareHelper->Release();
        return PARAMETER_ERROR;
    }

    ContactIdentity identity;
    identity.contactId = contactId;
    identity.rawContactId = rawContactId;
    identity.fileName = contactId + "_" + std::to_string(rawContactId) + ".jpg";
    return FinalizePortraitSave(identity, portrait, isAddType, dataShareHelper, pixelMap);
}

} // namespace ContactsFfi
} // namespace OHOS
