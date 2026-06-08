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

#ifndef CONTACTSDATAABILITY_DELAY_ASYNC_TASK_H
#define CONTACTSDATAABILITY_DELAY_ASYNC_TASK_H

#include <chrono>
#include <ctime>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include "async_task.h"

namespace OHOS {
namespace Contacts {
// 延迟任务
class DelayAsyncTask {
public:
    // 延迟异步任务队列为单例
    // single instance
    static DelayAsyncTask *GetInstanceDelay1S()
    {
        static DelayAsyncTask obj;
        return &obj;
    }

private:
    // 控制任务队列并发处理
    std::mutex g_mtx;
    // 根据名字，去重
    std::map<std::string, std::shared_ptr<AsyncItem>> taskMap;

    std::vector<std::shared_ptr<AsyncItem>> taskQueue;

    // 条件变量
    std::condition_variable notEmptyCV_;

    bool startedFlag = false;
    /**
     * 拿到所有任务
     * @return
     */
    std::vector<std::shared_ptr<AsyncItem>> takeAllTask()
    {
        std::unique_lock<std::mutex> locker(g_mtx);
        std::vector<std::shared_ptr<AsyncItem>> totalTaskVector;
        for (size_t i = 0; i < taskQueue.size(); i++) {
            totalTaskVector.push_back(taskQueue[i]);
        }
        auto it = taskMap.begin();
        while (it != taskMap.end()) {
            totalTaskVector.push_back(it->second);
            ++it;
        }
        taskQueue.clear();
        taskMap.clear();
        return totalTaskVector;
    }
public:
    /**
     * 添加任务；一些任务，延迟期间，后一个需要覆盖上一个，只执行一次，根据name覆盖
     */
    void put(std::string name, std::shared_ptr<AsyncItem> asyncTask)
    {
        std::unique_lock<std::mutex> locker(g_mtx);
        // 添加任务 HILOG_INFO("delaytaskque put task: %{public}s", name.c_str());
        // 添加后，通知不为空
        // notify后，被唤醒线程也需要本次操作完成，锁释放后，才能take元素，所以设置元素在notify前后，都可以拿到；
        // 但是从逻辑上说，先设置元素，再notify
        taskMap[name] = asyncTask;
        notEmptyCV_.notify_one();
    }

    void put(std::shared_ptr<AsyncItem> asyncTask)
    {
        std::unique_lock<std::mutex> locker(g_mtx);
        // 添加后，通知不为空
        // notify后，被唤醒线程也需要本次操作完成，锁释放后，才能take元素，所以设置元素在notify前后，都可以拿到；
        // 但是从逻辑上说，先设置元素，再notify
        taskQueue.push_back(asyncTask);
        notEmptyCV_.notify_one();
    }

public:

    void Start()
    {
        // 任务没启动，就启动
        {
            std::unique_lock<std::mutex> locker(g_mtx);
            if (startedFlag) {
                // 已经启动任务，直接返回，HILOG_INFO("delaytaskque delay task is started yijing");
                return;
            }
            startedFlag = true;
        }
        // 启动任务
        std::thread thread([this]() -> void {
            this->run();
        });
        thread.detach();
    }
private:
    void run()
    {
        // 开启任务 HILOG_INFO("delaytaskque start delay task");
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds (1));
            // 等待1s结束，从队列那所有任务，HILOG_INFO("delaytaskque wait 1s finish start delay task");
            auto taskVector = takeAllTask();
            // 没有任务，休眠，等待放入任务后，被唤醒
            if (taskVector.empty()) {
                // 队列为空，休眠，put任务后，会被唤醒，HILOG_INFO("delaytaskque task queue is empty");
                // 需要休眠
                {
                    std::unique_lock<std::mutex> locker(g_mtx);
                    HILOG_INFO("delaytaskque is empty, wait condition, ts = %{public}lld", (long long) time(NULL));
                    notEmptyCV_.wait(locker);
                }
            } else {
                // 拿到所有任务执行，HILOG_INFO("delaytaskque task queue has task: %{public}d", (int)taskVector.size());
                // 有任务，执行所有任务
                for (size_t i = 0; i < taskVector.size(); i++) {
                    taskVector[i]->Run();
                }
            }
        }
    }
};
} // namespace Contacts
} // namespace OHOS

#endif // CONTACTSDATAABILITY_DELAY_ASYNC_TASK_H