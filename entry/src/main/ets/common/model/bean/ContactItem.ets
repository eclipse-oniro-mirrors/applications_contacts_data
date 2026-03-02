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

import { ValuesBucket } from '@ohos.data.ValuesBucket';
import { Contacts } from '../../contract/Contacts';


/**
 * ContactItem bean to batch insert table contacts
 */
export default class ContactItem {
  private mId: string | undefined = undefined;

  private mNameRawContactId: string | undefined = undefined;

  private mPhotoId: string | undefined = undefined;

  private mPhotoFileId: string | undefined = undefined;

  private mPersonalRingtone: string | undefined = undefined;

  private mIsTransferVoicemail: string | undefined = undefined;

  private mCompany: string | undefined = undefined;

  private mPosition: string | undefined = undefined;

  private mHasPhoneNumber: string | undefined = undefined;

  private mContactLastUpdatedTimestamp: string | undefined = undefined;

  private mHasDisplayName: string | undefined = undefined;

  private mHasEmail: string | undefined = undefined;

  private mReadOnly: string | undefined = undefined;

  private mPersonalNotificationRingtone: string | undefined = undefined;

  /**
   * Set id in contact item for insert table contact
   *
   * @param id for contact
   */
  public setId(id: string) {
    this.mId = id;
    return this;
  }

  /**
   * Set name_raw_contact_id in contact item for insert table contact
   *
   * @param name_raw_contact_id for contact
   */
  public setNameRawContactId(nameRawContactId: string) {
    this.mNameRawContactId = nameRawContactId;
    return this;
  }

  /**
   * Set photo_id in contact item for insert table contact
   *
   * @param photo_id for contact
   */
  public setPhotoId(photoId: string) {
    this.mPhotoId = photoId;
    return this;
  }

  /**
   * Set photo_file_id in contact item for insert table contact
   *
   * @param photo_file_id for contact
   */
  public setPhotoFileId(photoFileId: string) {
    this.mPhotoFileId = photoFileId;
    return this;
  }

  /**
   * Set personal_ringtone in contact item for insert table contact
   *
   * @param personal_ringtone for contact
   */
  public setPersonalRingtone(personalRingtone: string) {
    this.mPersonalRingtone = personalRingtone;
    return this;
  }

  /**
   * Set is_transfer_voicemail in contact item for insert table contact
   *
   * @param is_transfer_voicemail for contact
   */
  public setIsTransferVoicemail(isTransferVoicemail: string) {
    this.mIsTransferVoicemail = isTransferVoicemail;
    return this;
  }

  /**
   * Set company in contact item for insert table contact
   *
   * @param company for contact
   */
  public setCompany(company: string) {
    this.mCompany = company;
    return this;
  }

  /**
   * Set position in contact item for insert table contact
   *
   * @param position for contact
   */
  public setPosition(position: string) {
    this.mPosition = position;
    return this;
  }

  /**
   * Set has_phone_number in contact item for insert table contact
   *
   * @param has_phone_number for contact
   */
  public setHasPhoneNumber(hasPhoneNumber: string) {
    this.mHasPhoneNumber = hasPhoneNumber;
    return this;
  }

  /**
   * Set contact_last_updated_timestamp in contact item for insert table contact
   *
   * @param contact_last_updated_timestamp for contact
   */
  public setContactLastUpdatedTimestamp(contactLastUpdatedTimestamp: string) {
    this.mContactLastUpdatedTimestamp = contactLastUpdatedTimestamp;
    return this;
  }

  /**
   * Set has_display_name in contact item for insert table contact
   *
   * @param has_display_name for contact
   */
  public setHasDisplayName(hasDisplayName: string) {
    this.mHasDisplayName = hasDisplayName;
    return this;
  }

  /**
   * Set has_email in contact item for insert table contact
   *
   * @param has_email for contact
   */
  public setHasEmail(hasEmail: string) {
    this.mHasEmail = hasEmail;
    return this;
  }

  /**
   * Set read_only in contact item for insert table contact
   *
   * @param read_only for contact
   */
  public setReadOnly(readOnly: string) {
    this.mReadOnly = readOnly;
    return this;
  }

  /**
   * Set personal_notification_ringtone in contact item for insert table contact
   *
   * @param personal_notification_ringtone for contact
   */
  public setPersonalNotificationRingtone(personalNotificationRingtone: string) {
    this.mPersonalNotificationRingtone = personalNotificationRingtone;
    return this;
  }

  /**
   * Create ValuesBucket object
   *
   * @returns ValuesBucket with raw contacts info
   */
  public createValuesBucket(): ValuesBucket {
    const valuesBucket: ValuesBucket = {};
    valuesBucket[Contacts.NEW_ID] = this.mId as string;
    valuesBucket[Contacts.NAME_RAW_CONTACT_ID] = this.mNameRawContactId as string;
    valuesBucket[Contacts.PHOTO_ID] = this.mPhotoId as string;
    valuesBucket[Contacts.PHOTO_FILE_ID] = this.mPhotoFileId as string;
    valuesBucket[Contacts.PERSONAL_RINGTONE] = this.mPersonalRingtone as string;
    valuesBucket[Contacts.IS_TRANSFER_VOICEMAIL] = this.mIsTransferVoicemail as string;
    valuesBucket[Contacts.COMPANY] = this.mCompany as string;
    valuesBucket[Contacts.POSITION] = this.mPosition as string;
    valuesBucket[Contacts.HAS_PHONE_NUMBER] = this.mHasPhoneNumber as string;
    valuesBucket[Contacts.CONTACT_LAST_UPDATED_TIMESTAMP] = this.mContactLastUpdatedTimestamp as string;
    valuesBucket[Contacts.HAS_DISPLAY_NAME] = this.mHasDisplayName as string;
    valuesBucket[Contacts.HAS_EMAIL] = this.mHasEmail as string;
    valuesBucket[Contacts.READ_ONLY] = this.mReadOnly as string;
    valuesBucket[Contacts.PERSONAL_NOTIFICATION_RINGTONE] = this.mPersonalNotificationRingtone as string;
    return valuesBucket;
  }
}