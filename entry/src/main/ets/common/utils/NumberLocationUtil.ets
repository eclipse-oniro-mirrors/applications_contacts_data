/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
import numberLookup from '@hms.telephony.numberLookup';
import LogUtils from './LogUtils';

const TAG = 'NumberLocationUtil';

export async function getNumberLocation(context: Context | undefined, number: string | undefined,
  isExactMatch?: boolean): Promise<string | null> {

  if (!number || number.length === 0 || !context) {
    LogUtils.e(TAG, `GETNumberLocation failed`);
    return null;
  }
  try {
    let location = await numberLookup.getNumberLocation(context, number, isExactMatch);
    return location;
  } catch (err) {
    LogUtils.e(TAG, `getNumberLocation failed, error code: ${err?.code}, message: ${err?.message}`);
    return null;
  }
}