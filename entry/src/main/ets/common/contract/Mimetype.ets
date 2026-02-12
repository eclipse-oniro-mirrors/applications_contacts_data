/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import ContactsColumns from './ContactsColumns';

/**
 * To get table contact type
 */
export class Mimetype {
  /**
   * Column _id for table mimetype
   */
  public static readonly ID = '_id';

  /**
   * Column mimetype for table mimetype
   */
  public static readonly MIMETYPE = 'mimetype';

  /**
   * OpenHarmony frame mimetype email
   */
  public static readonly OPENHARMONY_EMAIL = 'vnd.openharmony.cursor.item/email_v2';

  /**
   * foundation frame mimetype email
   */
  public static readonly FOUNDATION_EMAIL = 'email';

  /**
   * OpenHarmony frame mimetype im
   */
  public static readonly OPENHARMONY_IM = 'vnd.openharmony.cursor.item/im';

  /**
   * foundation frame mimetype im
   */
  public static readonly FOUNDATION_IM = 'im';

  /**
   * OpenHarmony frame mimetype nickname
   */
  public static readonly OPENHARMONY_NICKNAME = 'vnd.openharmony.cursor.item/nickname';

  /**
   * foundation frame mimetype nickname
   */
  public static readonly FOUNDATION_NICKNAME = 'nickname';

  /**
   * OpenHarmony frame mimetype organization
   */
  public static readonly OPENHARMONY_ORGANIZATION = 'vnd.openharmony.cursor.item/organization';

  /**
   * foundation frame mimetype organization
   */
  public static readonly FOUNDATION_ORGANIZATION = 'organization';

  /**
   * OpenHarmony frame mimetype phone
   */
  public static readonly OPENHARMONY_PHONE = 'vnd.openharmony.cursor.item/phone_v2';

  /**
   * foundation frame mimetype phone
   */
  public static readonly FOUNDATION_PHONE = 'phone';

  /**
   * OpenHarmony frame mimetype sip_address
   */
  public static readonly OPENHARMONY_SIP_ADDRESS = 'vnd.openharmony.cursor.item/sip_address';

  /**
   * foundation frame mimetype sip_address
   */
  public static readonly FOUNDATION_SIP_ADDRESS = 'sip_address';

  /**
   * OpenHarmony frame mimetype name
   */
  public static readonly OPENHARMONY_NAME = 'vnd.openharmony.cursor.item/name';

  /**
   * foundation frame mimetype name
   */
  public static readonly FOUNDATION_NAME = 'name';

  /**
   * OpenHarmony frame mimetype postal_address
   */
  public static readonly OPENHARMONY_POSTAL_ADDRESS = 'vnd.openharmony.cursor.item/postal-address_v2';

  /**
   * foundation frame mimetype postal_address
   */
  public static readonly FOUNDATION_POSTAL_ADDRESS = 'postal_address';

  /**
   * OpenHarmony frame mimetype photo
   */
  public static readonly OPENHARMONY_PHOTO = 'vnd.openharmony.cursor.item/photo';

  /**
   * foundation frame mimetype photo
   */
  public static readonly FOUNDATION_PHOTO = 'photo';

  /**
   * OpenHarmony frame mimetype group_membership
   */
  public static readonly OPENHARMONY_GROUP_MEMBERSHIP = 'vnd.openharmony.cursor.item/group_membership';

  /**
   * foundation frame mimetype group_membership
   */
  public static readonly FOUNDATION_GROUP_MEMBERSHIP = 'group_membership';

  /**
   * OpenHarmony frame mimetype note
   */
  public static readonly OPENHARMONY_NOTE = 'vnd.openharmony.cursor.item/note';

  /**
   * foundation frame mimetype note
   */
  public static readonly FOUNDATION_NOTE = 'note';

  /**
   * OpenHarmony frame mimetype contact_event
   */
  public static readonly OPENHARMONY_CONTACT_EVENT = 'vnd.openharmony.cursor.item/contact_event';

  /**
   * foundation frame mimetype contact_event
   */
  public static readonly FOUNDATION_CONTACT_EVENT = 'contact_event';

  /**
   * OpenHarmony frame mimetype website
   */
  public static readonly OPENHARMONY_WEBSITE = 'vnd.openharmony.cursor.item/website';

  /**
   * foundation frame mimetype website
   */
  public static readonly FOUNDATION_WEBSITE = 'website';

  /**
   * OpenHarmony frame mimetype relation
   */
  public static readonly OPENHARMONY_RELATION = 'vnd.openharmony.cursor.item/relation';

  /**
   * foundation frame mimetype relation
   */
  public static readonly FOUNDATION_RELATION = 'relation';

  /**
   * OpenHarmony frame mimetype camcard
   */
  public static readonly OPENHARMONY_CAMCARD = 'vnd.openharmony.cursor.item/vnd.com.ohos.camcard.photo';

  /**
   * foundation frame mimetype camcard
   */
  public static readonly FOUNDATION_CAMCARD = 'camcard';

  /**
   * OpenHarmony frame mimetype camcard
   */
  public static readonly OPENHARMONY_CONTACT_MISC = 'vnd.com.openharmony.cursor.item/contact_misc';

  /**
   * foundation frame mimetype contact_misc
   */
  public static readonly FOUNDATION_CONTACT_MISC = 'contact_misc';

  /**
   * OpenHarmony frame mimetype camcard
   */
  public static readonly OPENHARMONY_HICALL_DEVICE = 'vnd.openharmony.cursor.item/hicall_device';

  /**
   * foundation frame mimetype hicall_device
   */
  public static readonly FOUNDATION_HICALL_DEVICE = 'hicall_device';
}