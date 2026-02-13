/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
 
#ifndef CONTACT_CALLBACK_STUB_HELPER_H
#define CONTACT_CALLBACK_STUB_HELPER_H
 
#include <singleton.h>
#include "spam_call_adapter.h"
#include "spam_call_stub.h"
#include <memory>
 
namespace OHOS {
namespace Contacts {
class CallbackStubHelper : public SpamCallStub {
public:
    CallbackStubHelper(std::shared_ptr<SpamCallAdapter> spamCallAdapter);
    ~CallbackStubHelper();
    int32_t OnResult(int32_t &errCode, std::string &result) override;
 
private:
    std::shared_ptr<SpamCallAdapter> spamCallAdapter_ {nullptr};
};
}
}
 
#endif // CONTACT_CALLBACK_STUB_HELPER_H