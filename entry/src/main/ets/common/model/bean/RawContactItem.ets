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
import { StringUtil } from '../../../common/utils/StringUtil';
import RawContactsColumns from '../../contract/RawContactsColumns';

/**
 * RawContactItem bean to batch insert table raw_contacts
 */
export default class RawContactItem {
  private mSendToVoicemail: string = '';
  private mDeleted: string = '';
  private mDisplayName: string = '';
  private mTimesContacted: number = 0;
  private mLastTimeContacted: number = 0;
  private mStarred: string = '';
  private mStarredOrder: string = '';
  private mPhoneticName: string = '';
  private mPhoneticNameStyle: string = '';
  private mRawContactIsReadOnly: string = '';
  private mPhonebookLabel: string = '';
  private mVibrationType: string = '';
  private mAggregationMode: string = '';
  private mDirty: string = '1';
  // add next2next property
  private mContactId: number = -1;

  public setContactId(value: number) {
    this.mContactId = value;
    return this;
  }

  private mPhotoId: number = -1;

  public setPhotoId(value: number) {
    this.mPhotoId = value;
    return this;
  }

  private mMergeMode: number = 0;

  public setMergeMode(value: number) {
    this.mMergeMode = value;
    return this;
  }

  private mFavorite: number = 0;

  public setFavorite(value: number) {
    this.mFavorite = value;
    return this;
  }

  private mContactedCount: number = 0;

  public setContactedCount(value: number) {
    this.mContactedCount = value;
    return this;
  }

  private mLastestContactedTime: number = 0;

  public setLastestContactedTime(value: number) {
    this.mLastestContactedTime = value;
    return this;
  }

  private mIsTransferVoicemail: number = 0;

  public setTransferVoicemail(value: number) {
    this.mIsTransferVoicemail = value;
    return this;
  }

  private mPhotoFileId: number = -1;

  public setPhotoFileId(value: number) {
    this.mPhotoFileId = value;
    return this;
  }

  private mPersonalRingtone: string = '';

  public setPersonalRingtone(value: string) {
    this.mPersonalRingtone = value;
    return this;
  }

  private mNext2nextIsDeleted: number = 0;

  public setNext2nextIsDeleted(value: number) {
    this.mNext2nextIsDeleted = value;
    return this;
  }

  private mPersonalNotificationRingtone: string = '';

  public setPersonalNotificationRingtone(value: string) {
    this.mPersonalNotificationRingtone = value;
    return this;
  }

  private mPhotoFirstName: string = '';

  public setPhotoFirstName(value: string) {
    this.mPhotoFirstName = value;
    return this;
  }

  private mAccountId: number = -1;

  public setAccountId(value: number) {
    this.mAccountId = value;
    return this;
  }

  private mVersion: number = 0;

  public setVersion(value: number) {
    this.mVersion = value;
    return this;
  }

  private mSort: string = '1';

  public setSort(value: string) {
    this.mSort = value;
    return this;
  }

  private mSortKey: string = '';

  public setSortKey(value: string) {
    this.mSortKey = value;
    return this;
  }

  private mFavoriteOrder: number = -1;

  public setFavoriteOrder(value: number) {
    this.mFavoriteOrder = value;
    return this;
  }

  private mPhoneticNameType: number = -1;

  public setPhoneticNameType(value: number) {
    this.mPhoneticNameType = value;
    return this;
  }

  private mCompany: string = '';

  public setCompany(value: string) {
    this.mCompany = value;
    return this;
  }

  private mPosition: string = '';

  public setPosition(value: string) {
    this.mPosition = value;
    return this;
  }

  private mReadOnly: number = 0;

  public setReadOnly(value: number) {
    this.mReadOnly = value;
    return this;
  }

  private mSortFirstLetter: string = '';

  public setSortFirstLetter(value: string) {
    this.mSortFirstLetter = value;
    return this;
  }

  private mIsNeedMerge: number = 1;

  public setNeedMerge(value: number) {
    this.mIsNeedMerge = value;
    return this;
  }

  private mMergeStatus: number = 1;

  public setMergeStatus(value: number) {
    this.mMergeStatus = value;
    return this;
  }

  private mIsMergeTarget: number = 0;

  public setMergeTarget(value: number) {
    this.mIsMergeTarget = value;
    return this;
  }

  private mVibrationSetting: number = 0;

  public setVibrationSetting(value: number) {
    this.mVibrationSetting = value;
    return this;
  }

  private mSyncId: number = -1;

  public setSync_id(value: number) {
    this.mSyncId = value;
    return this;
  }

  private mSyn_2: string = '';

  public setSyn_2(value: string) {
    this.mSyn_2 = value;
    return this;
  }

  private mPrimaryContact: number = 0;

  public setPrimaryContact(value: number) {
    this.mPrimaryContact = value;
    return this;
  }

  private mExtra1: string = '';

  public setExtra1(value: string) {
    this.mExtra1 = value;
    return this;
  }

  private mExtra2: string = '';

  public setExtra2(value: string) {
    this.mExtra2 = value;
    return this;
  }

  private mExtra4: string = '';

  public setExtra4(value: string) {
    this.mExtra4 = value;
    return this;
  }

  private next2nextDirty: number = 0;

  public setNext2nextDirty(value: number) {
    this.next2nextDirty = value;
    return this;
  }


  /**
   * Set send_to_voicemail in raw contact item for insert table raw_contact
   *
   * @param sendToVoicemail for raw contact
   */
  public setSendToVoicemail(sendToVoicemail: string) {
    this.mSendToVoicemail = sendToVoicemail;
    return this;
  }

  /**
   * Set deleted in raw contact item for insert table raw_contact
   *
   * @param deleted for raw contact
   */
  public setDeleted(deleted: string) {
    this.mDeleted = deleted;
    return this;
  }

  /**
   * Set display_name in raw contact item for insert table raw_contact
   *
   * @param displayName for raw contact
   */
  public setDisplayName(displayName: string) {
    this.mDisplayName = displayName;
    return this;
  }

  /**
   * Set times_contacted in raw contact item for insert table raw_contact
   *
   * @param timesContacted for raw contact
   */
  public setTimesContacted(timesContacted: string) {
    this.mTimesContacted = StringUtil.isEmpty(timesContacted) ? 0 : Number(timesContacted);
    return this;
  }

  /**
   * Set last_time_contacted in raw contact item for insert table raw_contact
   *
   * @param lastTimeContacted for raw contact
   */
  public setLastTimeContacted(lastTimeContacted: string) {
    this.mLastTimeContacted = StringUtil.isEmpty(lastTimeContacted) ? 0 : Number(lastTimeContacted);
    return this;
  }

  /**
   * Set starred in raw contact item for insert table raw_contact
   *
   * @param starred for raw contact
   */
  public setStarred(starred: string) {
    this.mStarred = starred;
    return this;
  }

  /**
   * Set starred_order in raw contact item for insert table raw_contact
   *
   * @param starredOrder for raw contact
   */
  public setStarredOrder(starredOrder: string) {
    this.mStarredOrder = starredOrder;
    return this;
  }

  /**
   * Set phonetic_name in raw contact item for insert table raw_contact
   *
   * @param phoneticName for raw contact
   */
  public setPhoneticName(phoneticName: string) {
    this.mPhoneticName = phoneticName;
    return this;
  }

  /**
   * Set phonetic_name_style in raw contact item for insert table raw_contact
   *
   * @param phoneticNameStyle for raw contact
   */
  public setPhoneticNameStyle(phoneticNameStyle: string) {
    this.mPhoneticNameStyle = phoneticNameStyle;
    return this;
  }

  /**
   * Set raw_contact_is_read_only in raw contact item for insert table raw_contact
   *
   * @param rawContactIsReadOnly for raw contact
   */
  public setRawContactIsReadOnly(rawContactIsReadOnly: string) {
    this.mRawContactIsReadOnly = rawContactIsReadOnly;
    return this;
  }

  /**
   * Set phonebook_label in raw contact item for insert table raw_contact
   *
   * @param phonebookLabel for raw contact
   */
  public setPhonebookLabel(phonebookLabel: string) {
    this.mPhonebookLabel = phonebookLabel;
    return this;
  }

  /**
   * Set vibrationType in raw contact item for insert table raw_contact
   *
   * @param vibrationType for raw contact
   */
  public setVibrationType(vibrationType: string) {
    this.mVibrationType = vibrationType;
    return this;
  }

  /**
   * Set aggregation_mode in raw contact item for insert table raw_contact
   *
   * @param aggregationMode for raw contact
   */
  public setAggregationMode(aggregationMode: string) {
    this.mAggregationMode = aggregationMode;
    return this;
  }

  /**
   * Set dirty in raw contact item for insert table raw_contact
   *
   * @param dirty for raw contact
   */
  public setDirty(dirty: string) {
    this.mDirty = dirty;
    return this;
  }

  /**
   * Create ValuesBucket object
   *
   * @returns ValuesBucket with raw contacts info
   */
  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {};
    valuesBucket[RawContactsColumns.IS_TRANSFER_VOICEMAIL] = this.mSendToVoicemail;
    valuesBucket[RawContactsColumns.IS_DELETED] = this.mDeleted;
    valuesBucket[RawContactsColumns.DISPLAY_NAME] = this.mDisplayName;
    valuesBucket[RawContactsColumns.CONTACTED_COUNT] = this.mTimesContacted;
    valuesBucket[RawContactsColumns.LASTEST_CONTACTED_TIME] = this.mLastTimeContacted;
    valuesBucket[RawContactsColumns.FAVORITE] = this.mStarred;
    valuesBucket[RawContactsColumns.FAVORITE_ORDER] = this.mStarredOrder;
    valuesBucket[RawContactsColumns.PHONETIC_NAME] = this.mPhoneticName;
    valuesBucket[RawContactsColumns.PHONETIC_NAME_TYPE] = this.mPhoneticNameStyle;
    valuesBucket[RawContactsColumns.READ_ONLY] = this.mRawContactIsReadOnly;
    valuesBucket[RawContactsColumns.SORT_FIRST_LETTER] = this.mPhonebookLabel;
    valuesBucket[RawContactsColumns.VIBRATION_SETTING] = this.mVibrationType;
    valuesBucket[RawContactsColumns.DIRTY] = this.mDirty;
    return valuesBucket;
  }

  /**
   * Create ValuesBucket object for clone
   *
   * @returns ValuesBucket with raw contacts info
   */
  public createValuesBucketForClone(): ValuesBucket {
    const valuesBucket: ValuesBucket = {};
    valuesBucket[RawContactsColumns.MERGE_MODE] = this.mAggregationMode;
    valuesBucket[RawContactsColumns.FAVORITE] = this.mStarred;
    valuesBucket[RawContactsColumns.CONTACTED_COUNT] = this.mTimesContacted;
    valuesBucket[RawContactsColumns.LASTEST_CONTACTED_TIME] = this.mLastTimeContacted;
    valuesBucket[RawContactsColumns.IS_TRANSFER_VOICEMAIL] = this.mSendToVoicemail;
    return valuesBucket;
  }

  /**
   * Create ValuesBucket object for clone
   *
   * @returns ValuesBucket with raw contacts info
   */
  public createValuesNext2nextBucketForClone(): ValuesBucket {
    const valuesBucket: ValuesBucket = {};
    valuesBucket[RawContactsColumns.CONTACT_ID] = this.mContactId;
    valuesBucket[RawContactsColumns.PHOTO_ID] = this.mPhotoId;
    valuesBucket[RawContactsColumns.PHOTO_FILE_ID] = this.mPhotoFileId;
    valuesBucket[RawContactsColumns.IS_TRANSFER_VOICEMAIL] = this.mIsTransferVoicemail;
    valuesBucket[RawContactsColumns.PERSONAL_RINGTONE] = this.mPersonalRingtone;
    valuesBucket[RawContactsColumns.IS_DELETED] = this.mNext2nextIsDeleted;
    valuesBucket[RawContactsColumns.PERSONAL_NOTIFICATION_RINGTONE] = this.mPersonalNotificationRingtone;
    valuesBucket[RawContactsColumns.PHOTO_FIRST_NAME] = this.mPhotoFirstName;
    valuesBucket[RawContactsColumns.ACCOUNT_ID] = this.mAccountId;
    valuesBucket[RawContactsColumns.VERSION] = this.mVersion;
    valuesBucket[RawContactsColumns.DISPLAY_NAME] = this.mDisplayName;
    valuesBucket[RawContactsColumns.SORT] = this.mSort;
    valuesBucket[RawContactsColumns.SORT_KEY] = this.mSortKey;
    valuesBucket[RawContactsColumns.CONTACTED_COUNT] = this.mContactedCount;
    valuesBucket[RawContactsColumns.LASTEST_CONTACTED_TIME] = this.mLastestContactedTime;
    valuesBucket[RawContactsColumns.FAVORITE] = this.mFavorite;
    valuesBucket[RawContactsColumns.FAVORITE_ORDER] = this.mFavoriteOrder;
    valuesBucket[RawContactsColumns.PHONETIC_NAME] = this.mPhoneticName;
    valuesBucket[RawContactsColumns.PHONETIC_NAME_TYPE] = this.mPhoneticNameType;
    valuesBucket[RawContactsColumns.COMPANY] = this.mCompany;
    valuesBucket[RawContactsColumns.POSITION] = this.mPosition;
    valuesBucket[RawContactsColumns.READ_ONLY] = this.mReadOnly;
    valuesBucket[RawContactsColumns.SORT_FIRST_LETTER] = this.mSortFirstLetter;
    valuesBucket[RawContactsColumns.MERGE_MODE] = this.mMergeMode;
    valuesBucket[RawContactsColumns.IS_NEED_MERGE] = this.mIsNeedMerge;
    valuesBucket[RawContactsColumns.MERGE_STATUS] = this.mMergeStatus;
    valuesBucket[RawContactsColumns.IS_MERGE_TARGET] = this.mIsMergeTarget;
    valuesBucket[RawContactsColumns.VIBRATION_SETTING] = this.mVibrationSetting;
    valuesBucket[RawContactsColumns.SYNC_ID] = this.mSyncId;
    valuesBucket[RawContactsColumns.SYN_2] = this.mSyn_2;
    valuesBucket[RawContactsColumns.PRIMARY_CONTACT] = this.mPrimaryContact;
    valuesBucket[RawContactsColumns.EXTRA1] = this.mExtra1;
    valuesBucket[RawContactsColumns.EXTRA2] = this.mExtra2;
    valuesBucket[RawContactsColumns.EXTRA4] = this.mExtra4;
    valuesBucket[RawContactsColumns.DIRTY] = this.next2nextDirty;
    return valuesBucket;
  }
}