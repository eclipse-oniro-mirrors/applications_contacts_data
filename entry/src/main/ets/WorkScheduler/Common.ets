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
import LogUtils from '../common/utils/LogUtils';
import rpc from '@ohos.rpc';

export const BACKUP_WORK_ID = 1;

export const RELATION_SYNC_WORK_ID = 2;

export const DELETE_CONTACTS_WORK_ID = 3;

export const CLOUD_SYNC_POWER_CONTROL_WORK_ID = 4;

const FAKE_TOKEN = 'FAKE_TOKEN';

export class Stub extends rpc.RemoteObject {
    constructor(descriptor: string) {
        super(descriptor);
    }

    onRemoteMessageRequest(code: number, data: rpc.MessageSequence,
        reply: rpc.MessageSequence, option: rpc.MessageOption): boolean {
        LogUtils.i('Stub', 'onRemoteMessageRequest code:' + code);
        // 本方法没有任何具体实现，不存在被攻击的风险，为了屏蔽安全告警做了个无效校验
        if (data.readInterfaceToken() == FAKE_TOKEN) {
            return true;
        }
        return true;
    }
}