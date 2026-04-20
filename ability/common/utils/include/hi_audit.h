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
#ifndef HI_AUDIT_H
#define HI_AUDIT_H

#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include "nocopyable.h"
#include "rdb_store.h"

namespace OHOS {

struct AuditLog {
    bool isUserBehavior;
    std::string cause;
    std::string operationType;
    std::string operationScenario;
    size_t operationCount;
    std::string operationStatus;
    std::string extend;

    const virtual std::string TitleString() const
    {
        return "happenTime, packageName, isForeground, isUserBehavior, cause, operationType, operationScenario, "
               "operationCount, operationStatus, extend";
    }

    const virtual std::string ToString() const
    {
        return std::to_string(isUserBehavior) + ", " + cause + ", " + operationType + ", " + operationScenario +
            ", " + std::to_string(operationCount) + ", " + operationStatus + ", " + extend;
    }
};

struct FileAuditLog : public AuditLog {
    std::string type;
    std::string path;
    std::string targetPath;
    std::string createTime;
    uint64_t size;

    const std::string TitleString() const
    {
        return AuditLog::TitleString() + ", type, path, targetPath, createTime, size";
    }

    const std::string ToString() const
    {
        return AuditLog::ToString() + ", " + type + ", " + path + ", " + targetPath + ", " + createTime +
            ", " + std::to_string(size);
    }
};

struct DatabaseAuditLog : public AuditLog {
    std::string dbStatus;
    std::string hwidStatus;

    const std::string TitleString() const
    {
        return AuditLog::TitleString() + ", dbStatus, hwidStatus";
    }

    const std::string ToString() const
    {
        return AuditLog::ToString() + ", " + dbStatus + ", " + hwidStatus;
    }
};

struct DetailedAuditLog : public AuditLog {
    // 联系人名称的前120bit
    std::string key1;
    // 联系人名称加手机号后6位的前120bit
    std::string key2;
    // 影响的 contactId
    int32_t contactId;
    // 修改前本端联系人总数
    size_t preContactCount;
    // 修改后联系人总数
    size_t postContactCount;
    // 影响的账户（原生联系人账户或具体的某个三方联系人账户）
    std::string affectedAccount;
    std::string otherInfo;

    const std::string TitleString() const
    {
        return AuditLog::TitleString() + ", key1, key2, contactId, preContactCount, postContactCount, "
                                         "affectedAccount, otherInfo";
    }

    const std::string ToString() const
    {
        return AuditLog::ToString() + ", " + key1 + ", " + key2 + ", " + std::to_string(contactId) + ", " +
               std::to_string(preContactCount) + ", " + std::to_string(postContactCount) + ", " + affectedAccount +
               ", " + otherInfo;
    }
};

enum class OperationType {
    OPERATION_DELETE,
    OPERATION_ADD,
    OPERATION_MOD,
};

class HiAudit : public NoCopyable {
public:
    static HiAudit& GetInstance();
    void Write(const AuditLog& auditLog);
    void WriteLog(const std::string &request, const OperationType &operationType,
        const std::vector<OHOS::NativeRdb::ValuesBucket> &contactVector, const size_t &contactsCount);
    static const std::unordered_map<OperationType, std::string> operationType;

private:
    HiAudit();
    ~HiAudit();

    void Init();
    void GetWriteFilePath();
    void WriteToFile(const std::string& log);
    uint64_t GetMilliseconds();
    std::string GetFormattedTimestamp(time_t timeStamp, const std::string& format);
    std::string GetFormattedTimestampEndWithMilli();
    void CleanOldAuditFile();
    void ZipAuditLog();
    void AssembleAuditLog(OHOS::DetailedAuditLog &auditLog, const NativeRdb::ValuesBucket &contact);
    static size_t Minus(const size_t &a, const size_t &b);
    static std::string GenerateKey(const std::string &data);
    static std::string GenerateKey1(const std::string &displayName);
    static std::string GenerateKey2(const std::string &displayName, const std::string &phoneNum);
    static std::string ProcessPhoneNumber(const std::string &displayName, const std::string &phoneNum);
    static std::string SHA256(const std::string &data);
    static std::string Base64Encode(const std::string &data);
    
private:
    std::mutex mutex_;
    int writeFd_;
    std::atomic<uint32_t> writeLogSize_ = 0;
};
} // namespace OHOS
#endif // HI_AUDIT_H
