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

#include "pixel_map_util.h"
#include "image_packer.h"
#include "hilog_wrapper.h"
#include "image_source.h"
#include "file_utils.h"

namespace OHOS {
namespace Contacts {
namespace PixelMapUtil {
using namespace Media;

constexpr uint8_t DEFAULT_IMAGE_QUALITY = 90;
constexpr uint32_t IMAGE_NUMBER_HINT = 1;
constexpr int32_t SHORT_SIDE_THRESHOLD = 512;
constexpr int32_t LONG_SIDE_THRESHOLD = 1920;
constexpr int32_t MAXIMUM_LONG_SIDE = 4096;
constexpr int32_t PHOTO_TYPE_BIG = 3;
constexpr int32_t PHOTO_TYPE_SMALL = 1;
constexpr int32_t MIN_RESOLUTION = 409600;
constexpr int32_t MIN_TUMBNAIL_FILE_SIZE = 1024 * 1024;
constexpr float SMALL_AVATAR_SIZE = 280;
const std::string IMAGE_FORMAT = "image/jpeg";

uint32_t SavePixelMapToFile(const std::unique_ptr<PixelMap>& pixelMap, const int fileDescriptor)
{
    PackOption option = {
        .format = IMAGE_FORMAT,
        .quality = DEFAULT_IMAGE_QUALITY,
        .numberHint = IMAGE_NUMBER_HINT,
        .desiredDynamicRange = EncodeDynamicRange::AUTO,
    };
    ImagePacker imagePacker;
    uint32_t err = imagePacker.StartPacking(fileDescriptor, option);
    if (err != 0) {
        HILOG_ERROR("PixelMapUtil SavePixelMapToFile Failed to StartPacking %{public}d", err);
        return err;
    }
    err = imagePacker.AddImage(*pixelMap);
    if (err != 0) {
        HILOG_ERROR("PixelMapUtil SavePixelMapToFile Failed to AddPicture %{public}d", err);
        return err;
    }
    err = imagePacker.FinalizePacking();
    if (err != 0) {
        HILOG_ERROR("PixelMapUtil SavePixelMapToFile Failed to FinalizePacking %{public}d", err);
        return err;
    }
    HILOG_DEBUG("PixelMapUtil SavePixelMapToFile SavePixelMapToFile success");
    return 0;
}

std::unique_ptr<PixelMap> GetPixelMapFromFile(const std::string &filePath)
{
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource(filePath, opts, errorCode);
    if (imageSource == nullptr) {
        HILOG_ERROR("PixelMapUtil GetPixelMapFromFile CreateImageSource Failed %{public}u", errorCode);
        return nullptr;
    }
    int32_t value = 0;
    imageSource->GetImagePropertyInt(0, "Orientation", value);
    const DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (pixelMap == nullptr) {
        HILOG_ERROR("PixelMapUtil GetPixelMapFromFile CreatePixelMap Failed %{public}u", errorCode);
        return nullptr;
    }
    HILOG_DEBUG("PixelMapUtil GetPixelMapFromFile success");
    pixelMap->rotate(value);
    return pixelMap;
}

uint32_t GetArrayBufferFromPixelMap(const std::unique_ptr<PixelMap> &pixelMap, std::vector<uint8_t> &data)
{
    CropPixelMap(pixelMap);
    ImageInfo info;
    pixelMap->GetImageInfo(info);
    int32_t height = (info.size.height != 0) ? info.size.height : 100;
    int32_t width = (info.size.width != 0) ? info.size.width : 100;
    HILOG_INFO("GetArrayBufferFromPixelMap height %{public}d width %{public}d", height, width);
    data.resize(pixelMap->GetByteCount());
    ImagePacker imagePacker;
    PackOption option = {
        .format = IMAGE_FORMAT,
        .quality = DEFAULT_IMAGE_QUALITY,
        .numberHint = IMAGE_NUMBER_HINT,
        .desiredDynamicRange = EncodeDynamicRange::AUTO,
    };
    uint32_t err = imagePacker.StartPacking(data.data(), data.size(), option);
    if (err != 0) {
        HILOG_ERROR("GetArrayBufferFromPixelMap Failed to StartPacking %{public}d", err);
        return err;
    }
    err = imagePacker.AddImage(*pixelMap);
    if (err != 0) {
        HILOG_ERROR("GetArrayBufferFromPixelMap Failed to AddPicture %{public}d", err);
        return err;
    }
    int64_t packedSize = 0;
    imagePacker.FinalizePacking(packedSize);
    HILOG_DEBUG("PixelMapUtil GetArrayBufferFromPixelMap success");
    return 0;
}

// 以图片宽为基准，缩放头像至280*280像素（与联系人应用侧统一）
void CropPixelMap(const std::unique_ptr<PixelMap> &pixelMap)
{
    ImageInfo info;
    pixelMap->GetImageInfo(info);
    int32_t height = (info.size.height != 0) ? info.size.height : 100;
    int32_t width = (info.size.width != 0) ? info.size.width : 100;
    int32_t shortSide = height > width ? width : height;
    int32_t cropY = (height - width) / 2;
    cropY = cropY < 0 ? 0 : cropY;
    Rect rect { 0, cropY, shortSide, shortSide };
    pixelMap->crop(rect);
    float scale = 0.5;
    if (shortSide > 0) {
        scale =  SMALL_AVATAR_SIZE / shortSide;
    }
    HILOG_INFO("CropPixelMap scale %{public}f height %{public}d width %{public}d ", scale, height, width);
    pixelMap->scale(scale, scale);
}

void GetThumbnailPixelMap(const std::unique_ptr<Media::PixelMap>& pixelMap, const int fileDescriptor)
{
    FileUtils fileUtils;
    int32_t fileSize = 0;
    // 大于1M图片需进行裁剪，以媒体库缩略图规格进行缩放;
    if (!fileUtils.GetFileSize(fileDescriptor, fileSize) || fileSize <= MIN_TUMBNAIL_FILE_SIZE) {
        return;
    }
    Media::ImageInfo info;
    pixelMap->GetImageInfo(info);
    int32_t height = (info.size.height != 0) ? info.size.height : 100;
    int32_t width = (info.size.width != 0) ? info.size.width : 100;
    auto scale = GetThumbnailScale(height, width);
    pixelMap->scale(scale, scale);
    HILOG_INFO("GetThumbnailPixelMap height %{public}d width %{public}d sacle %{public}f", height, width, scale);
}

// 与媒体库缩略图尺寸保存一致
// 长宽比例和原图保持一致。
// 长边不得超过1920，若超出，把长边设置为1920，短边等比例缩放
// 若原图的短边大于等于512，完成步骤1后，短边小于512，则把短边设置为512，长边等比例缩放
// 若完成步骤2后，长边大于4096，则把长边设置为4096，短边等比例缩放。
float GetThumbnailScale(int32_t height, int32_t width)
{
    int32_t longSide = height > width ? height : width;
    int32_t shortSide = height > width ? width : height;
    int32_t srcShortSide = shortSide;
    double ratio = static_cast<double>(longSide) / shortSide;
    if (longSide > LONG_SIDE_THRESHOLD) {
        longSide = LONG_SIDE_THRESHOLD;
        shortSide = static_cast<int>(SHORT_SIDE_THRESHOLD / ratio);
    }
    if (shortSide < SHORT_SIDE_THRESHOLD && srcShortSide >= SHORT_SIDE_THRESHOLD) {
        shortSide = SHORT_SIDE_THRESHOLD;
        longSide = static_cast<int>(SHORT_SIDE_THRESHOLD * ratio);
    }
    if (longSide > MAXIMUM_LONG_SIDE) {
        longSide = MAXIMUM_LONG_SIDE;
        shortSide = static_cast<int>(MAXIMUM_LONG_SIDE / ratio);
    }
    return static_cast<double>(shortSide) / srcShortSide;
}

int GetBlobSourceFromPixelMap(const std::unique_ptr<PixelMap> &pixelMap)
{
    Media::ImageInfo info;
    pixelMap->GetImageInfo(info);
    int32_t height = info.size.height;
    int32_t width = info.size.width;
    return (static_cast<int64_t>(height) * width) <= MIN_RESOLUTION ? PHOTO_TYPE_SMALL : PHOTO_TYPE_BIG;
}

}
} // namespace Contacts
} // namespace OHOS