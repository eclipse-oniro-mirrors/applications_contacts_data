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

using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::ContactsFfi;
using namespace OHOS::DataShare;
using namespace OHOS::Media;
using namespace OHOS::FFI;

namespace OHOS {
namespace ContactsFfi {

constexpr int OPEN_FILE_FAILED = -1;
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

static std::string QueryContactId(int64_t contextId, int rawContactId)
{
    auto dataShareHelper = GetDsHelper(contextId);
    if (dataShareHelper == nullptr) {
        return "";
    }
    ContactsControl contactsControl;
    std::vector<std::string> columns;
    columns.push_back("contact_id");
    auto resultSet = contactsControl.QueryContactByRawContactId(dataShareHelper, columns, rawContactId);
    if (resultSet == nullptr) {
        dataShareHelper->Release();
        return "";
    }
    int rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != 0 || rowCount == 0 || resultSet->GoToFirstRow() != 0) {
        resultSet->Close();
        dataShareHelper->Release();
        return "";
    }
    std::string contactId;
    int columnIndex = 0;
    resultSet->GetColumnIndex("contact_id", columnIndex);
    resultSet->GetString(columnIndex, contactId);
    resultSet->Close();
    dataShareHelper->Release();
    return contactId;
}

static int SavePixelMapToFile(const std::shared_ptr<PixelMap>& pixelMap, int fd)
{
    if (pixelMap == nullptr) {
        return -1;
    }
    ImageInfo info;
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
    ImageInfo info;
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
    dataShareHelper->Release();
}

static std::shared_ptr<PixelMap> GetValidPixelMap(
    int64_t photoId,
    std::shared_ptr<DataShareHelper>& dataShareHelper,
    int32_t* errCode)
{
    auto pixelMapImpl = FFIData::GetData<PixelMapImpl>(photoId);
    if (pixelMapImpl == nullptr) {
        HILOG_ERROR("GetValidPixelMap pixelMapImpl is null");
        dataShareHelper->Release();
        *errCode = ERROR;
        return nullptr;
    }

    std::shared_ptr<PixelMap> pixelMap = pixelMapImpl->GetRealPixelMap();
    if (pixelMap == nullptr) {
        HILOG_ERROR("GetValidPixelMap pixelMap is null");
        dataShareHelper->Release();
        *errCode = ERROR;
        return nullptr;
    }
    return pixelMap;
}

static int SavePortraitData(ContactsControl& contactsControl, std::shared_ptr<DataShareHelper>& dataShareHelper,
    const std::string& contactId, int64_t rawContactId, const std::string& fileName,
    int32_t srcHeight, int32_t srcWidth, CPortrait* portrait, bool isAddType)
{
    DataShareValuesBucket valuesBucket;
    valuesBucket.Put("PortraitFileName", fileName);
    valuesBucket.Put("contactId", contactId);
    valuesBucket.Put("rawContactId", std::to_string(rawContactId));
    valuesBucket.Put("srcHeight", srcHeight);
    valuesBucket.Put("srcWidth", srcWidth);
    bool isUriPortrait = portrait->hasUri && portrait->uri != nullptr && strlen(portrait->uri) > 0;
    int addPortraitType = isAddType ? (isUriPortrait ? 1 : 2) : 3;
    valuesBucket.Put("addPortraitType", addPortraitType);

    std::vector<DataShareValuesBucket> valueContactData;
    valueContactData.push_back(valuesBucket);
    int code = contactsControl.ContactDataInsert(dataShareHelper, valueContactData);
    dataShareHelper->Release();
    return code;
}

static int OpenAndSavePortrait(ContactsControl& contactsControl, std::shared_ptr<DataShareHelper>& dataShareHelper,
    const std::string& contactId, int64_t rawContactId, std::shared_ptr<PixelMap>& pixelMap,
    int32_t& srcHeight, int32_t& srcWidth)
{
    std::string fileName = contactId + "_" + std::to_string(rawContactId) + ".jpg";
    int fd = contactsControl.OpenFileByDataShare(fileName, dataShareHelper);
    if (fd == OPEN_FILE_FAILED || fd == PERMISSION_ERROR) {
        HILOG_ERROR("OpenAndSavePortrait OpenFileByDataShare failed");
        HandleInsertFailed(contactsControl, dataShareHelper, rawContactId, fileName);
        return fd;
    }

    GetPixelMapSize(pixelMap, srcHeight, srcWidth);
    int result = SavePixelMapToFile(pixelMap, fd);
    close(fd);
    if (result != ERR_OK) {
        HandleInsertFailed(contactsControl, dataShareHelper, rawContactId, fileName);
        return result;
    }
    return ERR_OK;
}

int32_t CJInsertPortrait(int64_t contextId, int64_t rawContactId, CPortrait* portrait,
    bool isAddType)
{
    int32_t errCode = ERR_OK;
    if (portrait == nullptr || !portrait->hasPhoto) {
        HILOG_ERROR("CJInsertPortrait portrait is null or has no photo");
        return ERROR;
    }

    auto dataShareHelper = GetDsHelper(contextId);
    if (dataShareHelper == nullptr) {
        HILOG_ERROR("CJInsertPortrait Permission denied!");
        return PERMISSION_ERROR;
    }

    std::shared_ptr<PixelMap> pixelMap = GetValidPixelMap(portrait->photoId, dataShareHelper, &errCode);
    if (pixelMap == nullptr) {
        return errCode;
    }

    std::string contactId = QueryContactId(contextId, rawContactId);
    if (contactId.empty()) {
        HILOG_ERROR("CJInsertPortrait contactId is empty");
        dataShareHelper->Release();
        return OPEN_FILE_FAILED;
    }

    ContactsControl contactsControl;
    int32_t srcHeight = 0;
    int32_t srcWidth = 0;
    int result = OpenAndSavePortrait(contactsControl, dataShareHelper, contactId, rawContactId,
        pixelMap, srcHeight, srcWidth);
    if (result != ERR_OK) {
        return result;
    }

    std::string fileName = contactId + "_" + std::to_string(rawContactId) + ".jpg";
    return SavePortraitData(contactsControl, dataShareHelper, contactId, rawContactId, fileName,
        srcHeight, srcWidth, portrait, isAddType);
}

} // namespace ContactsFfi
} // namespace OHOS
