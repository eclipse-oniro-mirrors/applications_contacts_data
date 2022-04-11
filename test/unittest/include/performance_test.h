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

#ifndef PERFORMANCE_TEST_H
#define PERFORMANCE_TEST_H

#include "base_test.h"

namespace Contacts {
namespace Test {
class PerformanceTest : public BaseTest {
public:
    static constexpr int TIME_USEC_CALL_LOG_INSERT = 100000000;
    static constexpr int TIME_USEC_CALL_LOG_UPDATE = 3000000;
    static constexpr int TIME_USEC_CALL_LOG_QUERY = 5000000;
    static constexpr int TIME_USEC_CALL_LOG_DELETED = 700000;

    static constexpr int TIME_USEC_VOICEMAIL_INSERT = 100000000;
    static constexpr int TIME_USEC_VOICEMAIL_UPDATE = 300000;
    static constexpr int TIME_USEC_VOICEMAIL_QUERY = 5000000;
    static constexpr int TIME_USEC_VOICEMAIL_DELETED = 700000;

    static constexpr int TIME_USEC_RAW_CONTACT_INSERT = 110000000;
    static constexpr int TIME_USEC_RAW_CONTACT_UPDATE = 3000000;
    static constexpr int TIME_USEC_RAW_CONTACT_QUERY = 5000000;
    static constexpr int TIME_USEC_RAW_CONTACT_DELETED = 8000000;

    static constexpr int TIME_USEC_CONTACT_DATA_INSERT = 75000000;
    static constexpr int TIME_USEC_CONTACT_DATA_UPDATE = 40000000;
    static constexpr int TIME_USEC_CONTACT_DATA_QUERY = 7000000;
    static constexpr int TIME_USEC_CONTACT_DATA_DELETED = 40000000;

    PerformanceTest();
    ~PerformanceTest();
    int64_t GetCurrentTime();
    int CalcTime(int64_t startTime, int64_t endTime);
    void DeleteContact();
};
} // namespace Test
} // namespace Contacts
#endif // PERFORMANCE_TEST_H
