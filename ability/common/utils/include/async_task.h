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

namespace OHOS {
namespace Contacts {
class AsyncItem {
public:
    virtual ~AsyncItem()
    {
    }

    virtual void Run() = 0;
};

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
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

class AsyncTaskQueue {
public:
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
        std::lock_guard<AsyncTaskMutex> lk(mtx);
        while (que.size() > 0)
            que.pop();
    }

    // que empty
    bool Empty() const
    {
        std::lock_guard<AsyncTaskMutex> lk(mtx);
        return que.empty();
    }

    size_t Size() const
    {
        std::lock_guard<AsyncTaskMutex> lk(mtx);
        return que.size();
    }

    size_t GetThreads() const
    {
        return threads;
    }

    bool Push(std::unique_ptr<AsyncItem> &task)
    {
        std::lock_guard<AsyncTaskMutex> lk(mtx);
        if (maxSize > 0 && que.size() >= maxSize) {
            HILOG_ERROR("AsyncTask maxSize error");
            return false;
        }
        que.push(task.release());
        return true;
    }

    // startTask
    void Start(size_t threads = 1, size_t maxSize = 1000000)
    {
        if (this->threads > 0) {
            return;
        }
        this->threads = threads;
        this->maxSize = maxSize;
        for (size_t i = 0; i < this->threads; i++) {
            std::thread(std::bind(&AsyncTaskQueue::Run, this)).detach();
        }
    }

public:
    void Run()
    {
        AsyncItem *item = nullptr;
        while (this->threads > 0) {
            if (Pop(&item)) {
                if (item != nullptr) {
                    item->Run();
                    delete item;
                    item = nullptr;
                }
            } else {
                std::chrono::milliseconds dura(1);
                std::this_thread::sleep_for(dura);
            }
        }
    }

private:
    size_t maxSize;
    size_t threads;
    mutable AsyncTaskMutex mtx;
    std::queue<AsyncItem *> que;
    AsyncTaskQueue()
    {
        this->maxSize = 0;
        this->threads = 0;
    }

    bool Pop(AsyncItem **item)
    {
        std::lock_guard<AsyncTaskMutex> lk(mtx);
        if (que.empty()) {
            return false;
        }
        *item = que.front();
        que.pop();
        return true;
    }
};

// impl run
class AsyncTask : public AsyncItem {
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store;
    std::vector<int> rawContactIdVector;
    bool isDeleted;

public:
    void Run()
    {
        ContactsUpdateHelper contactsUpdateHelper;
        contactsUpdateHelper.UpdateCallLogByPhoneNum(rawContactIdVector, store, isDeleted);
        std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
        contactsDataBase->InsertMergeData(store, rawContactIdVector);
        contactsDataBase->MarkMerge(store);
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
    {
    }
};

class AsyncDeleteContactsTask : public AsyncItem {
    std::vector<OHOS::NativeRdb::ValuesBucket> queryValuesBucket;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store;

public:
    void Run()
    {
        std::shared_ptr<ContactsDataBase> contactsDataBase = ContactsDataBase::GetInstance();
        contactsDataBase->DeleteRecordInsert(store, queryValuesBucket);
    }

public:
    AsyncDeleteContactsTask(std::shared_ptr<OHOS::NativeRdb::RdbStore> &store,
        std::vector<OHOS::NativeRdb::ValuesBucket> &queryValuesBucket)
    {
        this->queryValuesBucket = queryValuesBucket;
        this->store = store;
    }

public:
    AsyncDeleteContactsTask()
    {
    }
};
} // namespace Contacts
} // namespace OHOS

#endif // CONTACTSDATAABILITY_ASYNC_TASK_H
