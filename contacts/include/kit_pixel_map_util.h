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
#ifndef KIT_PIXEL_MAP_UTIL_H
#define KIT_PIXEL_MAP_UTIL_H

#include "pixel_map.h"

namespace OHOS {
namespace Contacts {
namespace KitPixelMapUtil {
enum class AddPortraitType {
    ADD_URI_PORTRAIT = 1,
    ADD_PIXELMAP_PORTRAIT,
    UPDATE_URI_PORTRAIT,
    UPDATE_PIXELMAP_PORTRAIT,
};
int SavePixelMapToFile(const std::shared_ptr<Media::PixelMap>& pixelMap, int fd);
std::unique_ptr<Media::PixelMap> GetPixelMapFromUri(const std::string &uri);
void CropPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap);
void GetPixelMapSize(const std::shared_ptr<Media::PixelMap>& pixelMap, int32_t &height, int32_t &width);
int32_t GetAddPortraitType(bool isAddType, bool isUriPortrait);
}
} // namespace Contacts
} // namespace OHOS
#endif // KIT_PIXEL_MAP_UTIL_H
