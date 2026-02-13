/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef PIXEL_MAP_UTIL_H
#define PIXEL_MAP_UTIL_H

#include "pixel_map.h"

namespace OHOS {
namespace Contacts {
namespace PixelMapUtil {
    /**
     * @brief save PixelMap to filePath
     *
     * @param pixelMap PixelMap
     * @param fileDescriptor fileDescriptor
     * @return result
     *
     */
    uint32_t SavePixelMapToFile(const std::unique_ptr<Media::PixelMap>& pixelMap, const int fileDescriptor);

    /**
     * @brief get PixelMap from filePath
     *
     * @param filePath filePath
     * @return pixelMap
     *
     */
    std::unique_ptr<Media::PixelMap> GetPixelMapFromFile(const std::string &filePath);

    /**
     * @brief get blob data from pixelMap
     *
     * @param pixelMap PixelMap
     * @param blob blob
     * @return result
     *
     */
    uint32_t GetArrayBufferFromPixelMap(const std::unique_ptr<Media::PixelMap> &pixelMap, std::vector<uint8_t> &blob);

    /**
     * @brief crop pixelMap
     *
     * @param pixelMap PixelMap
     *
     */
    void CropPixelMap(const std::unique_ptr<Media::PixelMap> &pixelMap);

    /**
     * @brief get thumbnail pixelMap
     *
     * @param pixelMap PixelMap
     * @param fileDescriptor fileDescriptor
     *
     */
    void GetThumbnailPixelMap(const std::unique_ptr<Media::PixelMap>& pixelMap, const int fileDescriptor);

    /**
     * @brief get thumbnail scale
     *
     * @param height height
     * @param width width
     * @return scale
     *
     */
    float GetThumbnailScale(int32_t height, int32_t width);

    /**
     * @brief get thumbnail scale
     *
     * @param pixelMap PixelMap
     * @return blob source
     *
     */
    int GetBlobSourceFromPixelMap(const std::unique_ptr<Media::PixelMap> &pixelMap);
};
} // namespace Contacts
} // namespace OHOS
#endif // PIXEL_MAP_UTIL_H
