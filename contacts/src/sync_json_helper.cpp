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

#include "sync_json_helper.h"

namespace OHOS {
namespace ContactsApi {

std::string BatchesSetToJson(const std::set<int> &batches)
{
    json arr = json::array();
    for (int batch : batches) {
        arr.push_back(batch);
    }
    return arr.dump();
}

std::set<int> ParseBatchesToSet(const std::string &jsonStr, bool *outValid)
{
    if (outValid != nullptr) {
        *outValid = false;
    }
    if (jsonStr.empty()) {
        return {};
    }
    json arr = json::parse(jsonStr, nullptr, false);
    if (arr.is_discarded() || !arr.is_array()) {
        return {};
    }
    std::set<int> result;
    for (const auto &item : arr) {
        if (!item.is_number_integer()) {
            return {};
        }
        result.insert(item.get<int>());
    }
    if (outValid != nullptr) {
        *outValid = true;
    }
    return result;
}

std::vector<int> ParseCompletedBatches(const std::string &jsonStr)
{
    auto batchSet = ParseBatchesToSet(jsonStr);
    return std::vector<int>(batchSet.begin(), batchSet.end());
}

bool AreAllBatchesCompleted(const std::string &completedBatchesStr, int totalBatches)
{
    if (completedBatchesStr.empty() || totalBatches <= 0) {
        return false;
    }
    bool valid = false;
    auto completed = ParseBatchesToSet(completedBatchesStr, &valid);
    if (!valid) {
        return false;
    }
    for (int i = 1; i <= totalBatches; ++i) {
        if (completed.find(i) == completed.end()) {
            return false;
        }
    }
    return true;
}

std::string MergeCompletedBatches(const std::string &existingBatchesStr, int newBatch, bool &outValid)
{
    bool valid = false;
    auto batches = ParseBatchesToSet(existingBatchesStr, &valid);
    if (!valid && !existingBatchesStr.empty()) {
        outValid = false;
        return existingBatchesStr;
    }
    batches.insert(newBatch);
    outValid = true;
    return BatchesSetToJson(batches);
}

std::string MakeInitialCompletedBatches(int firstBatch)
{
    json arr = json::array();
    arr.push_back(firstBatch);
    return arr.dump();
}

} // namespace ContactsApi
} // namespace OHOS