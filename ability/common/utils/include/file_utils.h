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
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace OHOS {
namespace Contacts {
class FileUtils {
public:
    FileUtils();
    ~FileUtils();
    int IsFolderExist(std::string path);
    void Mkdir(const std::string &path, mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    bool DirIterator(const std::string& filePath, std::vector<std::string>& iters);
    bool IsDir(const std::string& filePath);
    bool IsPathExist(const std::string& path);
    std::string GetFileNameFromPath(const std::string& fullPath);
    bool CopyPath(const std::string& srcPath, const std::string& dstPath);
    bool ParentPathExist(const std::string& path);
    bool GetGroupPath(std::string &groupPath);
    bool GetGroupDir(std::string &path);
    bool GetFileSize(const int fd, int32_t &fileSize);
    bool DeleteFile(const std::string &filePath);
};
} // namespace Contacts
} // namespace OHOS
#endif // FILE_UTILS_H
