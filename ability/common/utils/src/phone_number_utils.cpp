/*
 * Copyright (C) 2025-2025 Huawei Technologies Co., Ltd.
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

#include "phone_number_utils.h"
#include "phonenumberutil.h"
#include "unicode/locid.h"
#include "hilog_wrapper.h"

using i18n::phonenumbers::PhoneNumberUtil;

namespace OHOS {
namespace Contacts {
static const std::string PHONE_NUMBER_PREFIX = "106";
static const uint32_t REGION_LEN = 2;

bool PhoneNumberUtils::IsValidPhoneNumber(const std::string &number, std::string &country)
{
    PhoneNumberUtil *util = PhoneNumberUtil::GetInstance();
    if (util == nullptr) {
        HILOG_ERROR("PhoneNumberUtils::IsValidPhoneNumber: util is nullptr.");
        return false;
    }

    if (!IsValidRegion(country)) {
        icu::Locale locale = icu::Locale::createFromName(country.c_str());
        country = locale.getCountry();
        HILOG_WARN("PhoneNumberUtils::IsValidPhoneNumber: new country is %{public}s.", country.c_str());
    }

    i18n::phonenumbers::PhoneNumber phoneNumber;
    PhoneNumberUtil::ErrorType type = util->Parse(number, country, &phoneNumber);
    if (type != PhoneNumberUtil::ErrorType::NO_PARSING_ERROR) {
        return false;
    }
    return util->IsValidNumber(phoneNumber);
}

std::string PhoneNumberUtils::Format(const std::string &number, const std::string &country)
{
    PhoneNumberUtil *util = PhoneNumberUtil::GetInstance();
    if (util == nullptr) {
        HILOG_ERROR("PhoneNumberUtils::format: util is nullptr.");
        return "";
    }

    std::string formattedNumber;
    i18n::phonenumbers::PhoneNumber phoneNumber;
    PhoneNumberUtil::ErrorType type = util->ParseAndKeepRawInput(number, country, &phoneNumber);
    if (type != PhoneNumberUtil::ErrorType::NO_PARSING_ERROR) {
        return "";
    }
    if (number.compare(0, PHONE_NUMBER_PREFIX.length(), PHONE_NUMBER_PREFIX) == 0) {
        util->FormatInOriginalFormat(phoneNumber, country, &formattedNumber);
    } else {
        util->Format(phoneNumber, PhoneNumberUtil::PhoneNumberFormat::E164, &formattedNumber);
    }
    return formattedNumber;
}

bool PhoneNumberUtils::IsValidRegion(const std::string &region)
{
    std::string::size_type size = region.size();
    if (size != REGION_LEN) {
        return false;
    }
    for (size_t i = 0; i < REGION_LEN; ++i) {
        if ((region[i] > 'Z') || (region[i] < 'A')) {
            return false;
        }
    }
    return true;
}
}  // namespace Contacts
}  // namespace OHOS