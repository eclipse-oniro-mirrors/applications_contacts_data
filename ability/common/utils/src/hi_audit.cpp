/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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

#include "hi_audit.h"

#include <chrono>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <regex>
#include "hilog_wrapper.h"
#include "zip_util.h"

namespace OHOS {
struct HiAuditConfig {
    std::string logPath;
    std::string logName;
    uint32_t logSize; // 2kb
    uint32_t fileSize; // 3M
    uint32_t fileCount; // 10
};

const HiAuditConfig HIAUDIT_CONFIG = {
    "/data/storage/el2/log/audit/", "contacts", 2 * 1024, 3 * 1024 * 1024, 10};
constexpr int8_t MILLISECONDS_LENGTH = 3;
constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int MAX_TIME_BUFF = 64; // 64 : for example 2021-05-27-01-01-01
const std::string HIAUDIT_LOG_NAME = HIAUDIT_CONFIG.logPath + HIAUDIT_CONFIG.logName + "_audit.csv";
const std::unordered_map<OperationType, std::string> HiAudit::operationType{
    {OperationType::OPERATION_ADD, "ADD"},
    {OperationType::OPERATION_MOD, "MOD"},
    {OperationType::OPERATION_DELETE, "DEL"}
};
constexpr int32_t CONTACT_INFO_TRUNCATE_BYTE = 15;
constexpr int32_t PHONE_NUMBER_SUFFIX_LENGTH = 6;

HiAudit::HiAudit()
{
    Init();
}

HiAudit::~HiAudit()
{
    if (writeFd_ >= 0) {
        close(writeFd_);
    }
}

HiAudit& HiAudit::GetInstance()
{
    static HiAudit hiAudit;
    return hiAudit;
}

void HiAudit::Init()
{
    if (access(HIAUDIT_CONFIG.logPath.c_str(), F_OK) != 0) {
        int ret = mkdir(HIAUDIT_CONFIG.logPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
        if (ret != 0) {
            HILOG_ERROR("Failed to create directory %{public}s.", HIAUDIT_CONFIG.logPath.c_str());
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    writeFd_ = open(HIAUDIT_LOG_NAME.c_str(), O_CREAT | O_APPEND | O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (writeFd_ < 0) {
        HILOG_ERROR("writeFd_ open error errno: %{public}d", errno);
    }
    struct stat st;
    writeLogSize_ = stat(HIAUDIT_LOG_NAME.c_str(), &st) ? 0 : static_cast<uint64_t>(st.st_size);
    HILOG_INFO("writeLogSize: %{public}u", writeLogSize_.load());
}

uint64_t HiAudit::GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

std::string HiAudit::GetFormattedTimestamp(time_t timeStamp, const std::string& format)
{
    auto seconds = timeStamp / SEC_TO_MILLISEC;
    char date[MAX_TIME_BUFF] = {0};
    struct tm result {};
    if (localtime_r(&seconds, &result) != nullptr) {
        strftime(date, MAX_TIME_BUFF, format.c_str(), &result);
    }
    return std::string(date);
}

std::string HiAudit::GetFormattedTimestampEndWithMilli()
{
    uint64_t milliSeconds = GetMilliseconds();
    std::string formattedTimeStamp = GetFormattedTimestamp(milliSeconds, "%Y%m%d%H%M%S");
    std::stringstream ss;
    ss << formattedTimeStamp;
    milliSeconds = milliSeconds % SEC_TO_MILLISEC;
    ss << std::setfill('0') << std::setw(MILLISECONDS_LENGTH) << milliSeconds;
    return ss.str();
}

void HiAudit::Write(const AuditLog& auditLog)
{
    HILOG_INFO("write");
    std::lock_guard<std::mutex> lock(mutex_);
    if (writeLogSize_ == 0) {
        WriteToFile(auditLog.TitleString() + "\n");
    }
    std::string writeLog = GetFormattedTimestampEndWithMilli() + ", " +
        + "com.ohos.contactsdataability, false, " + auditLog.ToString();
    if (writeLog.length() > HIAUDIT_CONFIG.logSize) {
        writeLog = writeLog.substr(0, HIAUDIT_CONFIG.logSize);
    }
    writeLog = writeLog + "\n";
    WriteToFile(writeLog);
}

void HiAudit::WriteLog(const std::string &request, const OperationType &operationType,
    const std::vector<OHOS::NativeRdb::ValuesBucket> &contactVector, const size_t &contactsCount)
{
    std::string operationTypeStr;
    if (auto iter = HiAudit::operationType.find(operationType); iter != HiAudit::operationType.end()) {
        operationTypeStr = iter->second;
    }
    auto contactVectorSize = contactVector.size();
    HILOG_INFO("WriteLog, request: %{public}s, operationType: %{public}s, contactVector: %{public}zu",
        request.c_str(), operationTypeStr.c_str(), contactVectorSize);
    auto preContactCount = contactsCount;
    if (operationType == OperationType::OPERATION_ADD) {
        preContactCount = Minus(contactsCount, contactVectorSize);
    }
    for (size_t i = 0; i < contactVectorSize; i++) {
        auto contact = contactVector[i];
        OHOS::DetailedAuditLog auditLog;
        auditLog.operationScenario = request;
        auditLog.operationType = operationTypeStr;
        auditLog.operationStatus = operationTypeStr + " success";
        auditLog.operationCount = 1;
        switch (operationType) {
            case OperationType::OPERATION_ADD:
                auditLog.preContactCount = preContactCount + i;
                auditLog.postContactCount = auditLog.preContactCount + auditLog.operationCount;
                break;
            case OperationType::OPERATION_DELETE:
                auditLog.preContactCount = Minus(preContactCount, i);
                auditLog.postContactCount = Minus(auditLog.preContactCount, auditLog.operationCount);
                break;
            default:
                auditLog.postContactCount = auditLog.preContactCount;
                break;
        }
        AssembleAuditLog(auditLog, contact);
        Write(auditLog);
    }
}

size_t HiAudit::Minus(const size_t &a, const size_t &b)
{
    if (a >= b) {
        return a - b;
    }
    return 0;
}

void HiAudit::AssembleAuditLog(OHOS::DetailedAuditLog &auditLog, const NativeRdb::ValuesBucket &contact)
{
    auditLog.isUserBehavior = true;
    auditLog.cause = "USER BEHAVIOR";
    NativeRdb::ValueObject valueObj;
    std::string displayName;
    auto getObjectSuccess = contact.GetObject("display_name", valueObj);
    if (getObjectSuccess) {
        valueObj.GetString(displayName);
    }
    std::string phoneNumbers;
    getObjectSuccess = contact.GetObject("phoneNumbers", valueObj);
    if (getObjectSuccess) {
        valueObj.GetString(phoneNumbers);
    }
    int64_t contactId = -1;
    getObjectSuccess = contact.GetObject("contact_id", valueObj);
    if (getObjectSuccess) {
        valueObj.GetLong(contactId);
    }
    int64_t accountId = 1;
    getObjectSuccess = contact.GetObject("account_id", valueObj);
    if (getObjectSuccess) {
        valueObj.GetLong(accountId);
    }
    auditLog.contactId = contactId;
    auditLog.affectedAccount = std::to_string(accountId);
    auditLog.key1 = GenerateKey1(displayName);
    auditLog.key2 = GenerateKey2(displayName, phoneNumbers);
}

void HiAudit::GetWriteFilePath()
{
    if (writeLogSize_ < HIAUDIT_CONFIG.fileSize) {
        return;
    }

    close(writeFd_);
    ZipAuditLog();
    CleanOldAuditFile();

    writeFd_ = open(HIAUDIT_LOG_NAME.c_str(), O_CREAT | O_TRUNC | O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (writeFd_ < 0) {
        HILOG_ERROR("fd open error errno: %{public}d", errno);
    }
    
    writeLogSize_ = 0;
}

void HiAudit::CleanOldAuditFile()
{
    DIR* dir = opendir(HIAUDIT_CONFIG.logPath.c_str());
    if (dir == nullptr) {
        HILOG_ERROR("failed open dir, errno: %{public}d.", errno);
        return;
    }
    uint32_t zipFileSize = 0;
    std::string oldestAuditFile;
    while (true) {
        struct dirent* ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }
        if (std::string(ptr->d_name).find(HIAUDIT_CONFIG.logName) != std::string::npos &&
            std::string(ptr->d_name).find("zip") != std::string::npos) {
            zipFileSize = zipFileSize + 1;
            if (oldestAuditFile.empty()) {
                oldestAuditFile = HIAUDIT_CONFIG.logPath + std::string(ptr->d_name);
                continue;
            }
            struct stat st;
            stat((HIAUDIT_CONFIG.logPath + std::string(ptr->d_name)).c_str(), &st);
            struct stat oldestSt;
            stat(oldestAuditFile.c_str(), &oldestSt);
            if (st.st_mtime < oldestSt.st_mtime) {
                oldestAuditFile = HIAUDIT_CONFIG.logPath + std::string(ptr->d_name);
            }
        }
    }
    closedir(dir);
    if (zipFileSize > HIAUDIT_CONFIG.fileCount) {
        remove(oldestAuditFile.c_str());
    }
}

void HiAudit::WriteToFile(const std::string& content)
{
    GetWriteFilePath();
    if (writeFd_ < 0) {
        HILOG_ERROR("fd invalid.");
        return;
    }
    write(writeFd_, content.c_str(), content.length());
    writeLogSize_ = writeLogSize_ + content.length();
}

void HiAudit::ZipAuditLog()
{
    std::string zipFileName = HIAUDIT_CONFIG.logPath + HIAUDIT_CONFIG.logName + "_audit_" +
        GetFormattedTimestamp(GetMilliseconds(), "%Y%m%d%H%M%S");
    std::rename(HIAUDIT_LOG_NAME.c_str(), (zipFileName + ".csv").c_str());
    zipFile compressZip = HiviewDFX::ZipUtil::CreateZipFile(zipFileName + ".zip");
    if (compressZip == nullptr) {
        HILOG_WARN("open zip file failed.");
        return;
    }
    if (HiviewDFX::ZipUtil::AddFileInZip(compressZip, zipFileName + ".csv", HiviewDFX::KEEP_NONE_PARENT_PATH) == 0) {
        remove((zipFileName + ".csv").c_str());
    }
    HiviewDFX::ZipUtil::CloseZipFile(compressZip);
}

std::string HiAudit::GenerateKey(const std::string &data)
{
    if (data.empty()) {
        return "";
    }
    auto sha256 = HiAudit::SHA256(data);
    auto result = Base64Encode(sha256.substr(0, CONTACT_INFO_TRUNCATE_BYTE));
    return result;
}

std::string HiAudit::GenerateKey1(const std::string &displayName)
{
    if (displayName.empty()) {
        HILOG_WARN("GenerateKey1, displayName is null");
        return "";
    }
    try {
        std::string key1 = GenerateKey(displayName);
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring wideDisplayName = converter.from_bytes(displayName);
        std::wstring wideKey1Prefix(wideDisplayName.size(), '*');
        if (wideDisplayName.size() > 1) {
            wideKey1Prefix[0] = wideDisplayName[0];
        }
        std::stringstream ss;
        ss << converter.to_bytes(wideKey1Prefix) << "|" << key1;
        return ss.str();
    } catch (std::exception &e) {
        HILOG_ERROR("GenerateKey1, convert string error: %{public}s", e.what());
    }
    return "";
}

std::string HiAudit::GenerateKey2(const std::string &displayName, const std::string &phoneNums)
{
    if (displayName.empty() && phoneNums.empty()) {
        HILOG_WARN("GenerateKey2, displayName and phone is null");
        return "";
    }
    std::string data;
    size_t index = 0;
    size_t pos = phoneNums.find(',', index);
    while (pos != std::string::npos) {
        auto phoneNum = phoneNums.substr(index, pos - index);
        index = pos + 1;
        auto key = ProcessPhoneNumber(displayName, phoneNum);
        if (!data.empty()) {
            data.append("|");
        }
        data.append(key);
        pos = phoneNums.find(',', index);
    }
    std::string phoneNum;
    if (index < phoneNums.length()) {
        phoneNum = phoneNums.substr(index);
    }
    auto key = ProcessPhoneNumber(displayName, phoneNum);
    if (!data.empty()) {
        data.append("|");
    }
    data.append(key);
    return data;
}

std::string HiAudit::ProcessPhoneNumber(const std::string &displayName, const std::string &phoneNum)
{
    auto phoneNumSize = phoneNum.size();
    auto processedPhoneNum = phoneNum;
    if (phoneNumSize > PHONE_NUMBER_SUFFIX_LENGTH) {
        auto suffixLen = phoneNumSize - PHONE_NUMBER_SUFFIX_LENGTH;
        processedPhoneNum = phoneNum.substr(suffixLen, PHONE_NUMBER_SUFFIX_LENGTH);
    }
    std::string key2 = GenerateKey(displayName + processedPhoneNum);
    std::string key2Prefix;
    key2Prefix.resize(phoneNumSize);
    if (phoneNumSize < 3) {
        key2Prefix.assign(phoneNumSize, '*');
    } else if (phoneNumSize <= 7) {
        thread_local std::regex pattern(R"(^(.)(\d{1,5})(.))");
        std::string mask(phoneNumSize - 2, '*');
        std::string replacement = "$1" + mask + "$3";
        key2Prefix = std::regex_replace(phoneNum, pattern, replacement);
    } else {
        thread_local std::regex pattern(R"(^(.{3})(\d{1,})(.{4}))");
        std::string mask(phoneNumSize - 7, '*');
        std::string replacement = "$1" + mask + "$3";
        key2Prefix = std::regex_replace(phoneNum, pattern, replacement);
    }
    std::stringstream ss;
    ss << key2Prefix << "|" << key2;
    return ss.str();
}

std::string HiAudit::SHA256(const std::string &data)
{
    uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    const int len = 2;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(len) << std::setfill('0') << static_cast<int32_t>(hash[i]);
    }
    return ss.str();
}

std::string HiAudit::Base64Encode(const std::string &data)
{
    size_t size = data.size();
    size_t base64_len = (size + 2) / 3 * 4;
    if (base64_len == 0) {
        return "";
    }
    std::string ret;
    ret.resize(base64_len);
    EVP_EncodeBlock(reinterpret_cast<uint8_t *>(ret.data()), reinterpret_cast<const uint8_t *>(data.c_str()), size);
    return ret;
}
} // namespace OHOS
