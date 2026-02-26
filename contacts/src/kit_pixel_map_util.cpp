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

#include "kit_pixel_map_util.h"
#include "image_packer.h"
#include "hilog_wrapper.h"
#include "file_uri.h"
#include <unistd.h>

namespace OHOS {
namespace Contacts {
namespace KitPixelMapUtil {
constexpr uint8_t IMAGE_QUALITY = 90;
constexpr uint32_t IMAGE_NUMBER_HINT = 1;
constexpr int32_t IMAGE_MAX_HEIGHT = 1080;
constexpr int32_t IMAGE_MAX_WIDTH = 1920;
const std::string IMAGE_FORMAT = "image/jpeg";


/**
 * @brief save PixelMap to fileDescriptor
 *
 * @param pixelMap PixelMap
   @param fd fileDescriptor
 *
 */
int SavePixelMapToFile(const std::shared_ptr<Media::PixelMap>& pixelMap, int fd)
{
    if (pixelMap == nullptr) {
        return -1;
    }
    // 将1K分辨率以上的pixelMap下采样到1K；
    CropPixelMap(pixelMap);
    Media::PackOption option = {
        .format = IMAGE_FORMAT,
        .quality = IMAGE_QUALITY,
        .numberHint = IMAGE_NUMBER_HINT,
        .desiredDynamicRange = Media::EncodeDynamicRange::AUTO,
        .needsPackProperties = true,
    };
    Media::ImagePacker imagePacker;
    uint32_t result = imagePacker.StartPacking(fd, option);
    if (result != 0) {
        HILOG_ERROR("KitPixelMapUtil SavePixelMapToFile fd Failed to StartPacking %{public}d", result);
        return result;
    }
    result = imagePacker.AddImage(*pixelMap);
    if (result != 0) {
        HILOG_ERROR("KitPixelMapUtil SavePixelMapToFile fd Failed to AddPicture %{public}d", result);
        return result;
    }
    result = imagePacker.FinalizePacking();
    if (result != 0) {
        HILOG_ERROR("KitPixelMapUtil SavePixelMapToFile fd Failed FinalizePacking %{public}d", result);
        return result;
    }
    HILOG_INFO("KitPixelMapUtil SavePixelMapToFile fd success");
    return result;
}

/**
 * @brief get PixelMap from file uri
 *
 * @param pixelMap PixelMap
   @param uri file uri
 *
 */
std::unique_ptr<Media::PixelMap> GetPixelMapFromUri(const std::string &uri)
{
    AppFileService::ModuleFileUri::FileUri fileUri(uri);
    uint32_t errorCode = 0;
    Media::SourceOptions opts;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(fileUri.GetRealPath(), opts, errorCode);
    if (imageSource == nullptr) {
        HILOG_ERROR("KitPixelMapUtil GetPixelMapFromUri CreateImageSource Failed %{public}u", errorCode);
        return nullptr;
    }
    const Media::DecodeOptions decodeOpts;
    std::unique_ptr<Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (pixelMap == nullptr) {
        HILOG_ERROR("KitPixelMapUtil GetPixelMapFromUri CreatePixelMap Failed %{public}u", errorCode);
    }
    return pixelMap;
}

void CropPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    Media::ImageInfo info;
    pixelMap->GetImageInfo(info);
    int32_t height = (info.size.height != 0) ? info.size.height : 100;
    int32_t width = (info.size.width != 0) ? info.size.width : 100;
    if (height <= IMAGE_MAX_HEIGHT && width <= IMAGE_MAX_WIDTH) {
        return;
    }
    float scale = height > width ?
        (static_cast<float>(IMAGE_MAX_HEIGHT) / height) : ( static_cast<float>(IMAGE_MAX_WIDTH) / width);
    HILOG_INFO("KitPixelMapUtil CropPixelMap height %{public}d width %{public}d scale %{public}f",
        height, width, scale);
    pixelMap->scale(scale, scale);
}

void GetPixelMapSize(const std::shared_ptr<Media::PixelMap>& pixelMap, int32_t &height, int32_t &width)
{
    if (pixelMap == nullptr) {
        HILOG_ERROR("GetPixelMapSize failed, pixelMap is null");
        return;
    }
    Media::ImageInfo info;
    pixelMap->GetImageInfo(info);
    height = info.size.height;
    width = info.size.width;
}

int32_t GetAddPortraitType(bool isAddType, bool isUriPortrait)
{
    if (isAddType) {
        return isUriPortrait ? static_cast<int32_t>(AddPortraitType::ADD_URI_PORTRAIT) :
            static_cast<int32_t>(AddPortraitType::ADD_PIXELMAP_PORTRAIT);
    }
    return isUriPortrait ? static_cast<int32_t>(AddPortraitType::UPDATE_URI_PORTRAIT) :
        static_cast<int32_t>(AddPortraitType::UPDATE_URI_PORTRAIT);
}

}
} // namespace Contacts
} // namespace OHOS