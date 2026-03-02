/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
import { StringUtil } from '../../common/utils/StringUtil';

export namespace Contact {
  /**
   * Provides methods for contact information
   */

  export class Contact {
    /**
     * Indicates the contact ID.
     */
    public static readonly INVALID_CONTACT_ID: -1;
    /**
     * Indicates the contact ID.
     */
    public readonly id: number | null = -1;
    /**
     * Indicates the query key that identifies the contact.
     */
    public readonly key: string | null = null;
    /**
     * Indicates the contact attributes.
     */
    public contactAttributes: ContactAttributes | null = null;
    /**
     * Indicates an email address of the contact.
     */
    public emails: Email[] | null = null;
    /**
     * Indicates an event (special date) of the contact.
     */
    public events: Event[] | null = null;
    /**
     * Indicates a group of the contact.
     */
    public groups: Group[] | null = null;
    /**
     * Indicates an IM address of the contact.
     */
    public imAddresses: ImAddress[] | null = null;
    /**
     * Indicates a phone number of the contact.
     */
    public phoneNumbers: PhoneNumber[] | null = null;
    /**
     * Indicates the contact portrait.
     */
    public portrait: Portrait | null = null;
    /**
     * Indicates a postal address of the contact.
     */
    public postalAddresses: PostalAddress[] | null = null;
    /**
     * Indicates a relation of the contact.
     */
    public relations: Relation[] | null = null;
    /**
     * Indicates a Session Initiation Protocol (SIP) address of the contact.
     */
    public sipAddresses: SipAddress[] | null = null;
    /**
     * Indicates a website of the contact.
     */
    public websites: Website[] | null = null;
    /**
     * Indicates the contact name.
     */
    public name: Name | null = null;
    /**
     * Indicates the contact nickname.
     */
    public nickName: NickName | null = null;
    /**
     * Indicates the contact note.
     */
    public note: Note | null = null;
    /**
     * Indicates organization information about the contact.
     */
    public organization: Organization | null = null;
    /**
     * Indicates the favorite status.
     */
    public favorite: number | null = null;
    /**
     * Indicates favorite order.
     */
    public favoriteOrder: number | null = null;

    public bigPhotoUri: string | null = null;
    public bigPhotoCloudFileName: string | null = null;

    /**
     * Indicates the contact camcard.
     */
    public camcard: Camcard | null = null;

    public getLogStr(): string {
      return `phoneList:${
        this.phoneNumbers?.map((phone: Contact.PhoneNumber) => StringUtil.maskSensitiveInfo(phone?.phoneNumber!)).join(',')}${
          this.favorite ? `,favoriteOrder:${this.favoriteOrder}` : ''}`;
    }
  }

  /**
   * Provides methods for contact attributes information
   */
  export class ContactAttributes {
    /**
     * Indicates the contact attributes.
     */
    public attributes: Attribute[] | null = null;
  }

  /**
   * Provides methods for attribute information
   */
  enum Attribute {
    /**
     * Indicates the contact event.
     */
    ATTR_CONTACT_EVENT,

    /**
     * Indicates the email address.
     */
    ATTR_EMAIL,

    /**
     * Indicates the contact group.
     */
    ATTR_GROUP_MEMBERSHIP,

    /**
     * Indicates the instant messaging (IM) address.
     */
    ATTR_IM,

    /**
     * Indicates the name.
     */
    ATTR_NAME,

    /**
     * Indicates the nickname.
     */
    ATTR_NICKNAME,

    /**
     * Indicates the note.
     */
    ATTR_NOTE,

    /**
     * Indicates the organization.
     */
    ATTR_ORGANIZATION,

    /**
     * Indicates the phone number.
     */
    ATTR_PHONE,

    /**
     * Indicates the portrait.
     */
    ATTR_PORTRAIT,

    /**
     * Indicates the postal address.
     */
    ATTR_POSTAL_ADDRESS,

    /**
     * Indicates the relation.
     */
    ATTR_RELATION,

    /**
     * Indicates the Session Initiation Protocol (SIP) address.
     */
    ATTR_SIP_ADDRESS,

    /**
     * Indicates the website.
     */
    ATTR_WEBSITE
  }

  /**
   * Provides methods for email information
   */
  export class Email {
    /**
     * Indicates a custom label.
     */
    public static readonly CUSTOM_LABEL = 0;
    /**
     * Indicates a home email.
     */
    public static readonly EMAIL_HOME = 1;
    /**
     * Indicates a work email.
     */
    public static readonly EMAIL_WORK = 2;
    /**
     * Indicates an email of the OTHER type.
     */
    public static readonly EMAIL_OTHER = 3;
    /**
     * Indicates an invalid label ID.
     */
    public static readonly INVALID_LABEL_ID = -1;
    /**
     * Indicates the email address.
     */
    public email: string | null = null;
    /**
     * Indicates the label name of an attribute.
     */
    public labelName: string | null = null;
    /**
     * Indicates the displayed email name.
     */
    public displayName: string | null = null;
    /**
     * Indicates the label id.
     */
    public labelId: number | null = null;
  }

  /**
   * Provides methods for event information
   */
  export class Event {
    /**
     * Indicates a custom label.
     */
    public static readonly CUSTOM_LABEL = 2;
    /**
     * Indicates an anniversary event.
     */
    public static readonly EVENT_ANNIVERSARY = 3;
    /**
     * Indicates an event of the OTHER type.
     */
    public static readonly EVENT_OTHER = 0;
    /**
     * Indicates an birthday event.
     */
    public static readonly EVENT_BIRTHDAY = 1;
    /**
     * Indicates an invalid label ID.
     */
    public static readonly INVALID_LABEL_ID = -1;
    /**
     * Indicates the event date.
     */
    public eventDate: string | null = null;
    /**
     * Indicates the label name of an attribute.
     */
    public labelName: string | null = null;
    /**
     * Indicates the label id.
     */
    public labelId: number | null = null;
  }

  /**
   * Provides methods for group information
   */
  export class Group {
    /**
     * Indicates the contact group ID.
     */
    public groupId: number | null = null;
    /**
     * Indicates the contact group title.
     */
    public title: string | null = null;
  }

  /**
   * Provides methods for holder information
   */
  export class Holder {
    /**
     * Indicates the bundle name of a contact holder.
     */
    public bundleName: string | null = null;
    /**
     * Indicates the displayed name of a contact holder.
     */
    public displayName: string | null = null;
    /**
     * Indicates the holder ID.
     */
    public holderId: number | null = null;
  }

  /**
   * Provides methods for ImAddress information
   */
  export class ImAddress {
    /**
     * Indicates a custom label.
     */
    public static readonly CUSTOM_LABEL = -1;
    /**
     * Indicates an AIM instant message.
     */
    public static readonly IM_AIM = 1;
    /**
     * Indicates a Windows Live instant message.
     */
    public static readonly IM_MSN = 2;
    /**
     * Indicates a Yahoo instant message.
     */
    public static readonly IM_YAHOO = 3;
    /**
     * Indicates a Skype instant message.
     */
    public static readonly IM_SKYPE = 4;
    /**
     * Indicates a QQ instant message.
     */
    public static readonly IM_QQ = 5;
    /**
     * Indicates a QQ instant message.
     */
    public static readonly GOOGLE_TALK = 6;
    /**
     * Indicates an ICQ instant message.
     */
    public static readonly IM_ICQ = 7;
    /**
     * Indicates a Jabber instant message.
     */
    public static readonly IM_JABBER = 8;
    /**
     * Indicates an invalid label ID.
     */
    public static readonly INVALID_LABEL_ID = -2;
    /**
     * Indicates a custom label.
     */
    public static readonly CUSTOM_OTHER = 0;
    /**
     * Indicates the IM address.
     */
    public imAddress: string | null = null;
    /**
     * Indicates the label name of an attribute.
     */
    public labelName: string | null = null;
    /**
     * Indicates the label id.
     */
    public labelId: number | null = null;
  }

  /**
   * Provides methods for name information
   */
  export class Name {
    /**
     * Indicates the family name of the contact.
     */
    public familyName: string | null = null;
    /**
     * Indicates the phonetic family name of the contact.
     */
    public familyNamePhonetic: string | null = null;
    /**
     * Indicates the full name of the contact.
     */
    public fullName: string | null = null;
    /**
     * Indicates the given name of the contact.
     */
    public givenName: string | null = null;
    /**
     * Indicates the phonetic given name of the contact.
     */
    public givenNamePhonetic: string | null = null;
    /**
     * Indicates the middle name of the contact.
     */
    public middleName: string | null = null;
    /**
     * Indicates the phonetic middle name of the contact.
     */
    public middleNamePhonetic: string | null = null;
    /**
     * Indicates the prefix of the contact name.
     */
    public namePrefix: string | null = null;
    /**
     * Indicates the suffix of this contact name.
     */
    public nameSuffix: string | null = null;

    /**
     * Indicates the phonetic of the contact name.
     */
    public phoneticName: string | null = null;
  }

  /**
   * Provides methods for nick name information
   */
  export class NickName {
    /**
     * Indicates the nickname of the contact.
     */
    public nickName: string | null = null;
  }

  /**
   * Provides methods for note information
   */
  export class Note {
    /**
     * Indicates the note content.
     */
    public noteContent: string | null = null;
  }

  /**
   * Provides methods for organization information
   */
  export class Organization {
    /**
     * Indicates the name of the organization to which the contact belongs.
     */
    public name: string | null = null;
    /**
     * Indicates the title of the contact.
     */
    public title: string | null = null;
  }

  /**
   * Provides methods for phone number information
   */
  export class PhoneNumber {
    /**
     * Indicates a custom label.
     */
    public static readonly CUSTOM_LABEL = 0;
    /**
     * Indicates a home number.
     */
    public static readonly NUM_MOBILE = 1;
    /**
     * Indicates a mobile phone number.
     */
    public static readonly NUM_HOME = 2;
    /**
     * Indicates a work number.
     */
    public static readonly NUM_WORK = 3;
    /**
     * Indicates a work fax number.
     */
    public static readonly NUM_FAX_WORK = 4;
    /**
     * Indicates a home fax number.
     */
    public static readonly NUM_FAX_HOME = 5;
    /**
     * Indicates a pager number.
     */
    public static readonly NUM_PAGER = 6;
    /**
     * Indicates a number of the OTHER type.
     */
    public static readonly NUM_OTHER = 7;
    /**
     * Indicates a callback number.
     */
    public static readonly NUM_CALLBACK = 8;
    /**
     * Indicates a car number.
     */
    public static readonly NUM_CAR = 9;
    /**
     * Indicates a company director number.
     */
    public static readonly NUM_COMPANY_MAIN = 10;
    /**
     * Indicates an Integrated Services Digital Network (ISDN) number.
     */
    public static readonly NUM_ISDN = 11;
    /**
     * Indicates a main number.
     */
    public static readonly NUM_MAIN = 12;
    /**
     * Indicates a number of the OTHER_FAX type.
     */
    public static readonly NUM_OTHER_FAX = 13;
    /**
     * Indicates a radio number.
     */
    public static readonly NUM_RADIO = 14;
    /**
     * Indicates a telex number.
     */
    public static readonly NUM_TELEX = 15;
    /**
     * Indicates a teletypewriter (TTY) or test-driven development (TDD) number.
     */
    public static readonly NUM_TTY_TDD = 16;
    /**
     * Indicates a work mobile phone number.
     */
    public static readonly NUM_WORK_MOBILE = 17;
    /**
     * Indicates a work pager number.
     */
    public static readonly NUM_WORK_PAGER = 18;
    /**
     * Indicates an assistant number.
     */
    public static readonly NUM_ASSISTANT = 19;
    /**
     * Indicates an MMS number.
     */
    public static readonly NUM_MMS = 20;
    /**
     * Indicates an invalid label ID.
     */
    public static readonly INVALID_LABEL_ID = -1;
    /**
     * customizel.
     */
    public static readonly CUSTOMIZE = 10000;
    /**
     * Indicates the label name of an attribute.
     */
    public labelName: string | null = null;
    /**
     * Indicates the phone number of the contact.
     */
    public phoneNumber: string | null = null;
    /**
     * Indicates the label id.
     */
    public labelId: number | null = null;

    public is_preferred_number: number = 0;

    public formatPhoneNumber?: string;

    public isDefaultNumber: string | null = null;
  }

  /**
   * Provides methods for portrait information
   */
  export class Portrait {
    /**
     * Indicates the uri of the contact portrait.
     */
    public uri: string | null | Uint8Array = null;
  }

  /**
   * Provides methods for postal address information
   */
  export class PostalAddress {
    /**
     * Indicates a custom label.
     */
    public static readonly CUSTOM_LABEL = 0;
    /**
     * Indicates a home address.
     */
    public static readonly ADDR_HOME = 1;
    /**
     * Indicates a work address.
     */
    public static readonly ADDR_WORK = 2;
    /**
     * Indicates an address of the OTHER type.
     */
    public static readonly ADDR_OTHER = 3;
    /**
     * Indicates an invalid label ID.
     */
    public static readonly INVALID_LABEL_ID = -1;
    /**
     * Indicates the city where this contact is located.
     */
    public city: string | null = null;
    /**
     * Indicates the country/region where this contact is located.
     */
    public country: string | null = null;
    /**
     * Indicates the label name of an attribute.
     */
    public labelName: string | null = null;
    /**
     * Indicates the neighborhood where this contact is located.
     */
    public neighborhood: string | null = null;
    /**
     * Indicates the post box of this contact.
     */
    public pobox: string | null = null;
    /**
     * Indicates the postal address of this contact.
     */
    public postalAddress: string | null = null;
    /**
     * Indicates the postal code of this contact.
     */
    public postcode: string | null = null;
    /**
     * Indicates the area where this contact is located.
     */
    public region: string | null = null;
    /**
     * Indicates the street where this contact is located.
     */
    public street: string | null = null;
    /**
     * Indicates the label id.
     */
    public labelId: number | null = null;
    /**
     * Indicates the full address.
     */
    public fullAddr: string | null = null;
  }

  /**
   * Provides methods for relation information
   */
  export class Relation {
    /**
     * Indicates a custom label.
     */
    public static readonly CUSTOM_LABEL = 0;
    /**
     * Indicates an assistant.
     */
    public static readonly RELATION_ASSISTANT = 1;
    /**
     * Indicates a brother.
     */
    public static readonly RELATION_BROTHER = 2;
    /**
     * Indicates a child.
     */
    public static readonly RELATION_CHILD = 3;
    /**
     * Indicates a domestic partner.
     */
    public static readonly RELATION_DOMESTIC_PARTNER = 4;
    /**
     * Indicates a father.
     */
    public static readonly RELATION_FATHER = 5;
    /**
     * Indicates a friend.
     */
    public static readonly RELATION_FRIEND = 6;
    /**
     * Indicates a manager.
     */
    public static readonly RELATION_MANAGER = 7;
    /**
     * Indicates a mother.
     */
    public static readonly RELATION_MOTHER = 8;
    /**
     * Indicates a parent.
     */
    public static readonly RELATION_PARENT = 9;
    /**
     * Indicates a partner.
     */
    public static readonly RELATION_PARTNER = 10;
    /**
     * Indicates a referrer.
     */
    public static readonly RELATION_REFERRED_BY = 11;
    /**
     * Indicates a relative.
     */
    public static readonly RELATION_RELATIVE = 12;
    /**
     * Indicates a sister.
     */
    public static readonly RELATION_SISTER = 13;
    /**
     * Indicates a spouse.
     */
    public static readonly RELATION_SPOUSE = 14;
    /**
     * Indicates an invalid label ID.
     */
    public static readonly INVALID_LABEL_ID: -1;
    /**
     * Indicates the label name of an attribute.
     */
    public labelName: string | null = null;
    /**
     * Indicates the relation name.
     */
    public relationName: string | null = null;
    /**
     * Indicates the label id.
     */
    public labelId: number | null = null;
  }

  /**
   * Provides methods for sip address information
   */
  export class SipAddress {
    /**
     * Indicates a custom label.
     */
    public static readonly CUSTOM_LABEL = 0;
    /**
     * Indicates a home SIP address.
     */
    public static readonly SIP_HOME = 1;
    /**
     * Indicates a work SIP address.
     */
    public static readonly SIP_WORK = 2;
    /**
     * Indicates an SIP address of the OTHER type.
     */
    public static readonly SIP_OTHER = 3;
    /**
     * Indicates an invalid label ID.
     */
    public static readonly INVALID_LABEL_ID = -1;
    /**
     * Indicates the label name of an attribute.
     */
    public labelName: string | null = null;
    /**
     * Indicates the SIP address.
     */
    public sipAddress: string | null = null;
    /**
     * Indicates the label id.
     */
    public labelId: number | null = null;
  }

  /**
   * Provides methods for website information
   */
  export class Website {
    /**
     * Indicates the website.
     */
    public website: string | null = null;
  }

  /**
   * Provides methods for camcard information
   */
  export class Camcard {
    /**
     * Indicates the uri of the contact camcard.
     */
    public uri: string | null | Uint8Array = null;
  }
}
