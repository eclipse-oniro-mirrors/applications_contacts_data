/**
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

import { ValuesBucket } from '@ohos.data.ValuesBucket';
import { Group } from '../../contract/Group';

// import { Group } from '../../database/contract/Group';

/**
 * GroupItem bean to batch insert table groups
 */
export default class GroupItem {
  private mGroupName: string = '';

  private mAccountId: number = -1;

  private mId: number = -1;

  private mGroupNotes: string = '';

  private mIsDeleted: string = '';

  private mNext2nextIsDeleted: number = 0;

  private mGroupRingtone: string = '';

  private mRingtoneModifyTime: number = -1;

  private mLastestModifyTime: number = -1;

  /**
   * Set group_name in groups item for insert
   *
   * @param groupName for groups table
   */
  public setGroupName(groupName: string) {
    this.mGroupName = groupName;
    return this;
  }

  /**
   * Set account_id in data item for batch insert
   *
   * @param account_id the id for group
   * @returns DataItem obj
   */
  public setAccountId(accountId: number) {
    this.mAccountId = accountId;
    return this;
  }

  /**
   * Set id in data item for batch insert
   *
   * @param id the id for group
   * @returns DataItem obj
   */
  public setId(id: number) {
    this.mId = id;
    return this;
  }

  /**
   * Set group_notes in groups item for insert
   *
   * @param groupNotes for groups table
   */
  public setGroupNotes(groupNotes: string) {
    this.mGroupNotes = groupNotes;
    return this;
  }

  /**
   * Set is_deleted in groups item for insert
   *
   * @param isDeleted for groups table
   */
  public setDeleted(isDeleted: string) {
    this.mIsDeleted = isDeleted;
    return this;
  }

  /**
   * Set next2next is_deleted in data item for batch insert
   *
   * @param next2nextIsDeleted the id for group
   * @returns GroupItem obj
   */
  public setNext2nextIsDeleted(next2nextIsDeleted: number) {
    this.mNext2nextIsDeleted = next2nextIsDeleted;
    return this;
  }

  /**
   * Set group_ringtone in groups item for insert
   *
   * @param groupRingtone for groups table
   */
  public setGroupRingtone(groupRingtone: string) {
    this.mGroupRingtone = groupRingtone;
    return this;
  }

  /**
   * Set ringtone_modify_time in groups item for insert
   *
   * @param ringtoneModifyTime for groups table
   */
  public setRingtoneModifyTime(ringtoneModifyTime: number) {
    this.mRingtoneModifyTime = ringtoneModifyTime;
    return this;
  }

  /**
   * Set lastest_modify_time in groups item for insert
   *
   * @param lastestModifyTime for groups table
   */
  public setLastestModifyTime(lastestModifyTime: number) {
    this.mLastestModifyTime = lastestModifyTime;
    return this;
  }

  /**
   * Create ValuesBucket object
   *
   * @returns ValuesBucket with GroupItem
   */
  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {};
    valuesBucket[Group.NEW_ID] = this.mId;
    valuesBucket[Group.ACCOUNT_ID] = this.mAccountId;
    valuesBucket[Group.GROUP_NAME] = this.mGroupName;
    valuesBucket[Group.GROUP_NOTES] = this.mGroupNotes;
    valuesBucket[Group.IS_DELETED] = this.mIsDeleted;
    return valuesBucket;
  }

  /**
   * Create ValuesBucket object
   *
   * @returns ValuesBucket with GroupItem
   */
  public createHmos2nextValuesBucket(): ValuesBucket {
    let value: ValuesBucket = {};
    value[Group.GROUP_NAME] = this.mGroupName;
    value[Group.GROUP_NOTES] = this.mGroupNotes;
    value[Group.IS_DELETED] = this.mIsDeleted;
    return value;
  }

  /**
   * Create ValuesBucket object
   *
   * @returns ValuesBucket with GroupItem
   */
  public createNext2nextValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {};
    valuesBucket[Group.GROUP_NAME] = this.mGroupName;
    valuesBucket[Group.ACCOUNT_ID] = this.mAccountId;
    valuesBucket[Group.GROUP_NOTES] = this.mGroupNotes;
    valuesBucket[Group.IS_DELETED] = this.mNext2nextIsDeleted;
    valuesBucket[Group.GROUP_RINGTONE] = this.mGroupRingtone;
    valuesBucket[Group.RINGTONE_MODIFY_TIME] = this.mRingtoneModifyTime;
    valuesBucket[Group.LASTEST_MODIFY_TIME] = this.mLastestModifyTime;
    return valuesBucket;
  }
}