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

#ifndef SYNC_JSON_HELPER_H
#define SYNC_JSON_HELPER_H

#include "nlohmann/json.hpp"
#include <string>
#include <vector>
#include <set>

using json = nlohmann::json;

namespace OHOS {
namespace ContactsApi {

/**
 * @brief Convert a set of integers to a sorted JSON array string
 * @param batches set of batch numbers
 * @return JSON array string, e.g. "[1,2,3]" or "[]"
 */
std::string BatchesSetToJson(const std::set<int> &batches);

/**
 * @brief Parse JSON array string into a set of integers (deduplicated, sorted).
 * Strictly validates all elements: returns empty set if any element is not an integer.
 * @param jsonStr JSON array string, e.g. "[1,2,3]"
 * @param outValid[out] if non-null, set to true if parsed successfully (all integers),
 *                      false if empty input or non-array structure
 * @return set of parsed integers (empty on parse error or non-integer element)
 */
std::set<int> ParseBatchesToSet(const std::string &jsonStr, bool *outValid = nullptr);

/**
 * @brief Parse a JSON array string into a vector of integers.
 * Returns empty vector if any element is not an integer.
 * @param jsonStr JSON array string
 * @return vector of parsed integers (empty on parse error)
 */
std::vector<int> ParseCompletedBatches(const std::string &jsonStr);

/**
 * @brief Check if all batches 1..totalBatches are present in completedBatches.
 * Returns false if completedBatchesStr contains non-integer elements.
 * @param completedBatchesStr JSON array string of completed batch numbers
 * @param totalBatches total number of batches expected
 * @return true if all batches 1..totalBatches are in the completed set
 */
bool AreAllBatchesCompleted(const std::string &completedBatchesStr, int totalBatches);

/**
 * @brief Merge a new batch number into existing completed_batches JSON string
 * @param existingBatchesStr existing JSON array string
 * @param newBatch new batch number to merge
 * @param outValid[out] set to true if merge succeeded, false if existing data was corrupt
 * @return merged JSON array string
 */
std::string MergeCompletedBatches(const std::string &existingBatchesStr, int newBatch, bool &outValid);

/**
 * @brief Create initial completed_batches JSON for first batch sync
 * @param firstBatch batch number (usually 1)
 * @return JSON array string, e.g. "[1]"
 */
std::string MakeInitialCompletedBatches(int firstBatch);

} // namespace ContactsApi
} // namespace OHOS

#endif // SYNC_JSON_HELPER_H