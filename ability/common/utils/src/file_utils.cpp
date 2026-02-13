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

#include "file_utils.h"

#include "common.h"
#include "hilog_wrapper.h"
#include <sys/stat.h>
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "bundle_mgr_interface.h"


namespace OHOS {
namespace Contacts {
const std::string GROUP_ID = "group.1521646752317339712";
const std::string APP_PATH_PREFIX = "/data/app/el2";
const std::string GROUP_PATH_PREFIX = "/data/storage/el2/group/";
const std::string PHOTO_STR = "/photo/";
FileUtils::FileUtils(void)
{
}

FileUtils::~FileUtils()
{
}

int FileUtils::IsFolderExist(std::string path)
{
    DIR *dp;
    if ((dp = opendir(path.c_str())) == nullptr) {
        HILOG_ERROR("FileUtils file NULL");
        return OPERATION_ERROR;
    }
    closedir(dp);
    return OPERATION_OK;
}

void FileUtils::Mkdir(const std::string &path, mode_t mode)
{
    if (IsFolderExist(path) == OPERATION_ERROR) {
        int isCreate = ::mkdir(path.c_str(), mode);
        HILOG_INFO("FileUtils : mkdir = %{public}d", isCreate);
    }
}

bool FileUtils::ParentPathExist(const std::string& path) {
    std::filesystem::path dirPath(path);
    std::filesystem::path parentDir = dirPath.parent_path();
    if (parentDir.empty()) {
        return true;
    }
    return std::filesystem::exists(parentDir);
}

bool FileUtils::DirIterator(const std::string& filePath, std::vector<std::string>& iters) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(filePath)) {
            if (entry.path().filename() == "." || entry.path().filename() == "..") {
                continue;
            }
            iters.emplace_back(entry.path().string());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        HILOG_INFO("FileUtils::DirIterator error iterating directory: %{public}s", e.what());
        return false;
    }
    return true;
}

std::string FileUtils::GetFileNameFromPath(const std::string& fullPath)
{
    size_t lastSlashPos = fullPath.find_last_of("/\\");
    if (lastSlashPos == std::string::npos) {
        return fullPath;
    }
    return fullPath.substr(lastSlashPos + 1);
}

bool FileUtils::IsDir(const std::string& filePath)
{
    struct stat info{};
    if (stat(filePath.c_str(), &info) != 0) {
        HILOG_INFO("FileUtils::IsDir: file path not exist.");
        return false;
    }
    if (!(info.st_mode & S_IFDIR)) {
        return false;
    }
    return true;
}

bool FileUtils::IsPathExist(const std::string& path)
{
    return (access(path.c_str(), F_OK) == 0);
}

bool FileUtils::CopyPath(const std::string& srcPath, const std::string& dstPath)
{
    if (!IsPathExist(srcPath)) {
        HILOG_ERROR("CopyPath: source path does not exist: %{private}s", srcPath.c_str());
        return false;
    }

    struct stat srcStat;
    if (stat(srcPath.c_str(), &srcStat) != 0) {
        HILOG_ERROR("CopyPath: failed to get source file info: %{private}s", srcPath.c_str());
        return false;
    }

    // 判断目标父路径是否存在，不存在则创建
    if (!ParentPathExist(dstPath)) {
        std::filesystem::path filePath(dstPath);
        std::filesystem::path parentDir = filePath.parent_path();
        std::error_code ec;
        std::filesystem::create_directories(parentDir, ec);
        if (ec) {
            HILOG_ERROR("CopyPath: create dir failed, msg: %{public}s", ec.message().c_str());
            return false;
        }
    }

    // 如果目标路径是文件且存在，则覆盖该文件
    if (IsPathExist(dstPath) && !IsDir(dstPath)) {
        HILOG_DEBUG("CopyPath: dstPath exists and is a file, will be overwritten: %{private}s", dstPath.c_str());
    }

    // 执行文件复制
    std::filesystem::path sPath(srcPath);
    std::filesystem::path dPath(dstPath);
    std::error_code ec;
    const auto copyOptions = std::filesystem::copy_options::overwrite_existing |
                             std::filesystem::copy_options::recursive |
                             std::filesystem::copy_options::skip_symlinks;
    std::filesystem::copy(sPath, dPath, copyOptions, ec);
    if (ec.value()) {
        HILOG_ERROR("CopyPath: copy file failed, errno=%{public}d", ec.value());
        return false;
    }
    return true;
}

bool FileUtils::GetGroupPath(std::string &groupPath)
{
    std::string appDir;
    if (!GetGroupDir(appDir)) {
        HILOG_ERROR("FileUtils GetGroupDir failed");
        return false;
    }
    if (appDir.size() == 0 || appDir.find(APP_PATH_PREFIX) != 0) {
        return false;
    }
    size_t lastSlashPos = appDir.rfind('/');
    if (lastSlashPos == std::string::npos || lastSlashPos >= appDir.size() - 1) {
        return false;
    }
    std::string lastPart = appDir.substr(lastSlashPos + 1);
    groupPath = GROUP_PATH_PREFIX + lastPart + PHOTO_STR;
    if (IsFolderExist(groupPath) == OPERATION_ERROR) {
        Mkdir(groupPath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
    }
    return true;
}
 
bool FileUtils::GetGroupDir(std::string &path)
{
    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smgr == nullptr) {
        HILOG_ERROR("FileUtils GetGroupDir smgr is nullptr");
        return false;
    }
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("FileUtils GetGroupDir remoteObject is nullptr");
        return false;
    }
    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("FileUtils GetGroupDir bundleMgr is nullptr");
        return false;
    }
    return bundleMgr->GetGroupDir(GROUP_ID, path);
}
 
bool FileUtils::GetFileSize(const int fd, int32_t &fileSize)
{
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        HILOG_ERROR("FileUtils GetFileSize failed");
        return false;
    }
    fileSize = file_stat.st_size;
    return true;
}
 
bool FileUtils::DeleteFile(const std::string &filePath)
{
    if (unlink(filePath.c_str()) != 0) {
        HILOG_ERROR("FileUtils DeleteFile failed");
        return false;
    }
    return true;
}

} // namespace Contacts
} // namespace OHOS