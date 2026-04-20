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

#ifndef CONTACTSDATAABILITY_ASYNC_TASK_H
#define CONTACTSDATAABILITY_ASYNC_TASK_H

#include <atomic>
#include <chrono>
#include <ctime>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "common.h"
#include "contacts_database.h"
#include "contacts_update_helper.h"
#include "hilog_wrapper.h"
#include "match_candidate.h"
#include "privacy_kit.h"

namespace OHOS {
namespace Contacts {
// 计数器
class Counter {
public:
    int count = 0;
    std::mutex g_mtx;

    int incrementOne()
    {
        std::unique_lock<std::mutex> locker(g_mtx);
        ++count;
        return count;
    }

    int incrementCount(int incrementCount)
    {
        std::unique_lock<std::mutex> locker(g_mtx);
        count += incrementCount;
        return count;
    }

    int getAndReset()
    {
        std::unique_lock<std::mutex> locker(g_mtx);
        int tmp = count;
        count = 0;
        return tmp;
    }

    int get()
    {
        std::unique_lock<std::mutex> locker(g_mtx);
        return count;
    }
};
/**
 * 阻塞队列
 * @tparam T
 */
template<typename T>
class BoundedBlockingQueue {
public:
    // make class non-copyable
    BoundedBlockingQueue(const BoundedBlockingQueue<T> &) = delete;

    BoundedBlockingQueue &operator=(const BoundedBlockingQueue<T> &) = delete;

    /**
     * 创建队列
     * @param maxSize
     */
    explicit BoundedBlockingQueue<T>(size_t maxSize)
        : mtx_(), maxSize_(maxSize)
    {
    }

    /**
     * 清空
     */
    void clear()
    {
        std::unique_lock<std::mutex> locker(mtx_);
        while (queue_.size() > 0)
            queue_.pop();
    }

    /**
     * 添加元素
     * @param x
     * @return
     */
    bool put(const T &x)
    {
        // 添加任务
        std::unique_lock<std::mutex> locker(mtx_);
        // 如果队列元素个数大于maxSize
        // 此处应该直接返回false，打印异常
        if (maxSize_ > 0 && queue_.size() >= maxSize_) {
            HILOG_ERROR("blockQueue maxSize error");
            return false;
        }
        queue_.push(x);
        // 添加后，通知不为空
        notEmptyCV_.notify_one();
        return true;
    }

    /**
     * 获取元素，将元素放到outRes
     * @param outRes
     * @return
     */
    bool take(T &outRes)
    {
        std::unique_lock<std::mutex> locker(mtx_);
        // 队列为空，阻塞等待任务
        while (queue_.empty()) {
            HILOG_INFO("blockQueue is empty, wait condition, ts = %{public}lld", (long long) time(NULL));
            notEmptyCV_.wait(locker);
        }
        // 获取元素
        outRes = queue_.front();
        queue_.pop();
        HILOG_INFO("take one element, remain: %{public}d, ts = %{public}lld",
            static_cast<int>(queue_.size()), (long long) time(NULL));
        return true;
    }

    /**
     * 检查是否为空
     * @return
     */
    bool empty() const
    {
        std::unique_lock<std::mutex> locker(mtx_);
        return queue_.empty();
    }

    /**
     *
     * @return
     */
    size_t size() const
    {
        std::unique_lock<std::mutex> locker(mtx_);
        return queue_.size();
    }

    size_t maxSize() const
    {
        std::unique_lock<std::mutex> locker(mtx_);
        return maxSize_;
    }

    void setMaxSize(size_t maxSize)
    {
        std::unique_lock<std::mutex> locker(mtx_);
        maxSize_ = maxSize;
    }

private:
    // 锁对象
    mutable std::mutex mtx_;
    // 条件变量
    std::condition_variable notEmptyCV_;
    // 队列最大长度
    size_t maxSize_;
    // 队列
    std::queue<T> queue_;
};

//  任务基类
class AsyncItem {
public:
    virtual ~AsyncItem()
    {
    }

    virtual void Run() = 0;
};

// 异步任务锁
class AsyncTaskMutex {
public:
    void lock()
    {
        while (flag.test_and_set(std::memory_order_acquire)) {
        }
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }

private:
    // 原子状态
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

// 异步任务队列，其中可以指定线程数，会启动指定数量的线程，从队列中获取任务执行
// 无法停止
class AsyncTaskQueue {
public:
    // 异步任务队列为单例
    // single instance
    static AsyncTaskQueue *Instance()
    {
        static AsyncTaskQueue obj;
        return &obj;
    }

public:
    // clear task
    void Clear()
    {
        que.clear();
    }

    // que empty
    bool Empty() const
    {
        return que.empty();
    }

    size_t Size() const
    {
        return que.size();
    }

    size_t GetThreads() const
    {
        std::lock_guard<AsyncTaskMutex> lk(mtx);
        return threads;
    }

    // 添加任务
    bool Push(std::unique_ptr<AsyncItem> &task)
    {
        HILOG_INFO("async task push task, ts = %{public}lld!", (long long) time(NULL));
        return que.put(task.release());
    }

    // startTask
    void Start(size_t threads = 1, size_t maxSize = 1000000)
    {
        std::lock_guard<AsyncTaskMutex> lk(mtx);
        // 如果已经启动，直接返回
        if (this->threads > 0) {
            // 已经启动
            return;
        }
        this->threads = threads;
        this->maxSize = maxSize;
        que.setMaxSize(maxSize);
        // 启动指定数量的线程，执行队列中的任务
        for (size_t i = 0; i < this->threads; i++) {
            std::thread(std::bind(&AsyncTaskQueue::Run, this)).detach();
        }
    }

public:
    void Run()
    {
        AsyncItem *item = nullptr;
        while (this->threads > 0) {
            if (Pop(item)) {
                if (item != nullptr) {
                    item->Run();
                    delete item;
                    item = nullptr;
                }
            }
        }
    }

private:
    // 异步任务最大长度
    size_t maxSize;
    // 线程数
    size_t threads;
    // 异步任务锁
    mutable AsyncTaskMutex mtx;
    // 任务队列，阻塞队列
    BoundedBlockingQueue<AsyncItem *> que{1000000};

    AsyncTaskQueue()
    {
        this->maxSize = 0;
        this->threads = 0;
    }

    /**
     * 弹出一个任务，将任务弹给item参数
    */
    bool Pop(AsyncItem *&item)
    {
        que.take(item);
        return true;
    }
};


// 异步任务
// impl run
class AsyncTask : public AsyncItem {
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store;
    std::vector<int> rawContactIdVector;
    bool isDeleted;
    int sleepTime = 50;  // Query need sleep 50ms

public:
    void Run()
    {
        HILOG_INFO("AsyncTask start, isDeleted = %{public}s, ts = %{public}lld,",
            std::to_string(isDeleted).c_str(),
            (long long) time(NULL));
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        ContactsUpdateHelper contactsUpdateHelper;
        contactsUpdateHelper.UpdateCallLogByPhoneNum(rawContactIdVector, store, isDeleted);
    }

public:
    AsyncTask(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store, std::vector<int> &rawContactIdVector, bool isDeleted)
    {
        this->store = store;
        this->rawContactIdVector = rawContactIdVector;
        this->isDeleted = isDeleted;
    }

public:
    AsyncTask()
    {}
};

class AsyncDeleteRelationContactsTask : public AsyncItem {
    int rawContactId;

public:
    void Run()
    {
        HILOG_INFO("AsyncDeleteRelationContactsTask start,ts = %{public}lld", (long long) time(NULL));
        std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
        contactsDataBase->HandleHwAccount(rawContactId);
    }

public:
    AsyncDeleteRelationContactsTask(int rawContactId)
    {
        this->rawContactId = rawContactId;
    }

public:
    AsyncDeleteRelationContactsTask()
    {
    }
};

class AsyncBlocklistMigrateTask : public AsyncItem {
public:
    void Run()
    {
        HILOG_INFO("AsyncDeleteRelationContactsTask start,ts = %{public}lld", (long long) time(NULL));
        std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
        contactsDataBase->MoveBlocklistData();
    }
    
public:
    AsyncBlocklistMigrateTask()
    {
    }
};

class AsyncDataShareProxyTask : public AsyncItem {
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper;
    DataShare::DataShareValuesBucket valuesBucket;
    DataShare::DataSharePredicates predicates;
    Uri proxyUri = Uri("");

public:
    void Run()
    {
        OHOS::Uri uriTemp = proxyUri;
        HILOG_INFO("AsyncDataShareProxyTask start.uri = %{public}s,ts = %{public}lld", uriTemp.ToString().c_str(),
            (long long) time(NULL));
        if (dataShareHelper == nullptr) {
            HILOG_ERROR("Update failed, AsyncDataShareProxyTask start dataShareHelper is null");
            return;
        }
        dataShareHelper->Update(proxyUri, predicates, valuesBucket);
    }

public:
    AsyncDataShareProxyTask(std::shared_ptr<DataShare::DataShareHelper> &dataShareHelper,
        DataShare::DataShareValuesBucket &valuesBucket, DataShare::DataSharePredicates &predicates, Uri &proxyUri)
    {
        this->dataShareHelper = dataShareHelper;
        this->valuesBucket = valuesBucket;
        this->predicates = predicates;
        this->proxyUri = proxyUri;
    }

public:
    AsyncDataShareProxyTask()
    {
    }
};

class AsyncMeetimeAvatarAddTask : public AsyncItem {
    std::vector<int> rawContactIdVector;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore;
    int sleepTime = 50; // Query need sleep 50ms
 
public:
    void Run()
    {
        HILOG_INFO("AsyncMeetimeAvatarAddTask start, ts = %{public}lld", (long long) time(NULL));
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        ContactsUpdateHelper contactsUpdateHelper;
        contactsUpdateHelper.UpdateCallLogPhoto(rawContactIdVector, rdbStore);
    }
 
public:
    AsyncMeetimeAvatarAddTask(std::vector<int> &rawContactIdVector,
      std::shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore)
    {
        this->rawContactIdVector = rawContactIdVector;
        this->rdbStore = rdbStore;
    }
 
public:
    AsyncMeetimeAvatarAddTask()
    {
    }
};

class AddPermissionUsedRecordTask : public AsyncItem {
    int callerToken;
    int callerPid;
    std::string permissionName;
    Counter &successCount;
    Counter &failCount;
 
public:
    void Run()
    {
        int successCountTotal = successCount.getAndReset();
        int failCountTotal = failCount.getAndReset();
        int32_t ret = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(callerToken, permissionName,
            successCountTotal, failCountTotal);
        if (ret != 0) {
            HILOG_INFO("AddPermissionUsedRecord failed,permissionName = %{public}s,callerPid = %{public}d,"
                "successCount = %{public}d,failCount = %{public}d,ret = %{public}d", permissionName.c_str(),
                callerPid, successCountTotal, failCountTotal, ret);
        }
    }

public:
    AddPermissionUsedRecordTask(int callerToken, int callerPid, std::string permissionName,
      Counter &successCount, Counter &failCount):callerToken(callerToken), callerPid(callerPid),
        permissionName(permissionName), successCount(successCount), failCount(failCount)
    {
    }
};

class AddPermissionUsedRecordContactTask : public AsyncItem {
    int callerToken;
    int callerPid;
    std::string permissionName;
    Counter &successCount;
    Counter &failCount;
 
public:
    void Run()
    {
        int successCountTotal = successCount.getAndReset();
        int failCountTotal = failCount.getAndReset();
        int32_t ret = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(callerToken,
            permissionName, successCountTotal, failCountTotal);
        if (ret != 0) {
            HILOG_INFO("AddPermissionUsedRecord contactsTelePhony failed, "
                "permissionName = %{public}s, callerPid = %{public}d,"
                "successCount = %{public}d,failCount = %{public}d,ret = %{public}d", permissionName.c_str(),
                callerPid, successCountTotal, failCountTotal, ret);
        }
    }

public:
    AddPermissionUsedRecordContactTask(int callerToken, int callerPid, std::string permissionName,
      Counter &successCount, Counter &failCount):callerToken(callerToken), callerPid(callerPid),
        permissionName(permissionName), successCount(successCount), failCount(failCount)
    {
    }
};


} // namespace Contacts
} // namespace OHOS

#endif // CONTACTSDATAABILITY_ASYNC_TASK_H
