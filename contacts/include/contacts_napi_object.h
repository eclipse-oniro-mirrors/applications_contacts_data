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

#ifndef CONTACT_NAPI_OBJECT_H
#define CONTACT_NAPI_OBJECT_H

#include <string>

namespace OHOS {
namespace ContactsApi {
enum class Attribute {
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
};

/**
 * Provides methods for contact attributes information
 */
class ContactAttributes {
public:
    /**
     * Indicates the contact attributes.
     */
    std::vector<int> attributes;
};

/**
 * Provides methods for email information
 */
class Email {
public:
    /**
     * Indicates a custom label.
     */
    int CUSTOM_LABEL = 0;

    /**
     * Indicates a home email.
     */
    int EMAIL_HOME = 1;

    /**
     * Indicates a work email.
     */
    int EMAIL_WORK = 2;

    /**
     * Indicates an email of the OTHER type.
     */
    int EMAIL_OTHER = 3;

    /**
     * Indicates an invalid label ID.
     */
    int INVALID_LABEL_ID = -1;

    /**
     * Indicates the email address.
     */
    std::string email;

    /**
     * Indicates the label name of an attribute.
     */
    std::string labelName;

    /**
     * Indicates the displayed email name.
     */
    std::string displayName;

    /**
     * Indicates the label id.
     */
    int labelId = -1;
};

/**
 * Provides methods for event information
 */
class Event {
public:
    /**
     * Indicates a custom label.
     */
    int CUSTOM_LABEL = 0;

    /**
     * Indicates an anniversary event.
     */
    int EVENT_ANNIVERSARY = 1;

    /**
     * Indicates an event of the OTHER type.
     */
    int EVENT_OTHER = 2;

    /**
     * Indicates an birthday event.
     */
    int EVENT_BIRTHDAY = 3;

    /**
     * Indicates an invalid label ID.
     */
    int INVALID_LABEL_ID = -1;

    /**
     * Indicates the event date.
     */
    std::string eventDate;

    /**
     * Indicates the label name of an attribute.
     */
    std::string labelName;

    /**
     * Indicates the label id.
     */
    int labelId = -1;
};

/**
 * Provides methods for group information
 */
class Group {
public:
    /**
     * Indicates the contact group ID.
     */
    int groupId = -1;

    /**
     * Indicates the contact group title.
     */
    std::string title;
};

/**
 * Provides methods for holder information
 */
class Holder {
public:
    /**
     * Indicates the bundle name of a contact holder.
     */
    std::string bundleName;

    /**
     * Indicates the displayed name of a contact holder.
     */
    std::string displayName;

    /**
     * Indicates the holder ID.
     */
    int holderId = -1;
};

/**
 * Provides methods for ImAddress information
 */
class ImAddress {
public:
    /**
     * Indicates a custom label.
     */
    int CUSTOM_LABEL = -1;

    /**
     * Indicates an AIM instant message.
     */
    int IM_AIM = 0;

    /**
     * Indicates a Windows Live instant message.
     */
    int IM_MSN = 1;

    /**
     * Indicates a Yahoo instant message.
     */
    int IM_YAHOO = 2;

    /**
     * Indicates a Skype instant message.
     */
    int IM_SKYPE = 3;

    /**
     * Indicates a QQ instant message.
     */
    int IM_QQ = 4;

    /**
     * Indicates an ICQ instant message.
     */
    int IM_ICQ = 6;

    /**
     * Indicates a Jabber instant message.
     */
    int IM_JABBER = 7;

    /**
     * Indicates an invalid label ID.
     */
    int INVALID_LABEL_ID = -2;

    /**
     * Indicates the IM address.
     */
    std::string imAddress;

    /**
     * Indicates the label name of an attribute.
     */
    std::string labelName;

    /**
     * Indicates the label id.
     */
    int labelId = -1;
};
/**
 * Provides methods for name information
 */
class Name {
public:
    /**
     * Indicates the family name of the contact.
     */
    std::string familyName;

    /**
     * Indicates the phonetic family name of the contact.
     */
    std::string familyNamePhonetic;

    /**
     * Indicates the full name of the contact.
     */
    std::string fullName;

    /**
     * Indicates the given name of the contact.
     */
    std::string givenName;

    /**
     * Indicates the phonetic given name of the contact.
     */
    std::string givenNamePhonetic;

    /**
     * Indicates the middle name of the contact.
     */
    std::string middleName;

    /**
     * Indicates the phonetic middle name of the contact.
     */
    std::string middleNamePhonetic;

    /**
     * Indicates the prefix of the contact name.
     */
    std::string namePrefix;

    /**
     * Indicates the suffix of this contact name.
     */
    std::string nameSuffix;
};
/**
 * Provides methods for nick name information
 */
class NickName {
public:
    /**
     * Indicates the nickname of the contact.
     */
    std::string nickName;
};

/**
 * Provides methods for note information
 */
class Note {
public:
    /**
     * Indicates the note content.
     */
    std::string noteContent;
};
/**
 * Provides methods for organization information
 */
class Organization {
public:
    /**
     * Indicates the name of the organization to which the contact belongs.
     */
    std::string name;

    /**
     * Indicates the title of the contact.
     */
    std::string title;
};
/**
 * Provides methods for phone number information
 */
class PhoneNumber {
public:
    /**
     * Indicates a custom label.
     */
    int CUSTOM_LABEL = 0;

    /**
     * Indicates a home number.
     */
    int NUM_HOME = 1;

    /**
     * Indicates a mobile phone number.
     */
    int NUM_MOBILE = 2;

    /**
     * Indicates a work number.
     */
    int NUM_WORK = 3;

    /**
     * Indicates a work fax number.
     */
    int NUM_FAX_WORK = 4;

    /**
     * Indicates a home fax number.
     */
    int NUM_FAX_HOME = 5;

    /**
     * Indicates a pager number.
     */
    int NUM_PAGER = 6;

    /**
     * Indicates a number of the OTHER type.
     */
    int NUM_OTHER = 7;

    /**
     * Indicates a callback number.
     */
    int NUM_CALLBACK = 8;

    /**
     * Indicates a car number.
     */
    int NUM_CAR = 9;

    /**
     * Indicates a company director number.
     */
    int NUM_COMPANY_MAIN = 10;

    /**
     * Indicates an Integrated Services Digital Network (ISDN) number.
     */
    int NUM_ISDN = 11;

    /**
     * Indicates a main number.
     */
    int NUM_MAIN = 12;

    /**
     * Indicates a number of the OTHER_FAX type.
     */
    int NUM_OTHER_FAX = 13;

    /**
     * Indicates a radio number.
     */
    int NUM_RADIO = 14;

    /**
     * Indicates a telex number.
     */
    int NUM_TELEX = 15;

    /**
     * Indicates a teletypewriter (TTY) or test-driven development (TDD) number.
     */
    int NUM_TTY_TDD = 16;

    /**
     * Indicates a work mobile phone number.
     */
    int NUM_WORK_MOBILE = 17;

    /**
     * Indicates a work pager number.
     */
    int NUM_WORK_PAGER = 18;

    /**
     * Indicates an assistant number.
     */
    int NUM_ASSISTANT = 19;

    /**
     * Indicates an MMS number.
     */
    int NUM_MMS = 20;

    /**
     * Indicates an invalid label ID.
     */
    int INVALID_LABEL_ID = -1;

    /**
     * Indicates the label name of an attribute.
     */
    std::string labelName;

    /**
     * Indicates the phone number of the contact.
     */
    std::string phoneNumber;

    /**
     * Indicates the label id.
     */
    int labelId = -1;
};

/**
 * Provides methods for portrait information
 */
class Portrait {
public:
    /**
     * Indicates the uri of the contact portrait.
     */
    std::string uri;
};
/**
 * Provides methods for postal address information
 */
class PostalAddress {
public:
    /**
     * Indicates a custom label.
     */
    int CUSTOM_LABEL = 0;

    /**
     * Indicates a home address.
     */
    int ADDR_HOME = 1;

    /**
     * Indicates a work address.
     */
    int ADDR_WORK = 2;

    /**
     * Indicates an address of the OTHER type.
     */
    int ADDR_OTHER = 3;

    /**
     * Indicates an invalid label ID.
     */
    int INVALID_LABEL_ID = -1;

    /**
     * Indicates the city where this contact is located.
     */
    std::string city;

    /**
     * Indicates the country/region where this contact is located.
     */
    std::string country;

    /**
     * Indicates the label name of an attribute.
     */
    std::string labelName;

    /**
     * Indicates the neighborhood where this contact is located.
     */
    std::string neighborhood;

    /**
     * Indicates the post box of this contact.
     */
    std::string pobox;

    /**
     * Indicates the postal address of this contact.
     */
    std::string postalAddress;

    /**
     * Indicates the postal code of this contact.
     */
    std::string postcode;

    /**
     * Indicates the area where this contact is located.
     */
    std::string region;

    /**
     * Indicates the street where this contact is located.
     */
    std::string street;

    /**
     * Indicates the label id.
     */
    int labelId = -1;
};

class Relation {
public:
    /**
     * Indicates a custom label.
     */
    int CUSTOM_LABEL = 0;

    /**
     * Indicates an assistant.
     */
    int RELATION_ASSISTANT = 1;

    /**
     * Indicates a brother.
     */
    int RELATION_BROTHER = 2;

    /**
     * Indicates a child.
     */
    int RELATION_CHILD = 3;

    /**
     * Indicates a domestic partner.
     */
    int RELATION_DOMESTIC_PARTNER = 4;

    /**
     * Indicates a father.
     */
    int RELATION_FATHER = 5;

    /**
     * Indicates a friend.
     */
    int RELATION_FRIEND = 6;

    /**
     * Indicates a manager.
     */
    int RELATION_MANAGER = 7;

    /**
     * Indicates a mother.
     */
    int RELATION_MOTHER = 8;

    /**
     * Indicates a parent.
     */
    int RELATION_PARENT = 9;

    /**
     * Indicates a partner.
     */
    int RELATION_PARTNER = 10;

    /**
     * Indicates a referrer.
     */
    int RELATION_REFERRED_BY = 11;

    /**
     * Indicates a relative.
     */
    int RELATION_RELATIVE = 12;

    /**
     * Indicates a sister.
     */
    int RELATION_SISTER = 13;

    /**
     * Indicates a spouse.
     */
    int RELATION_SPOUSE = 14;

    /**
     * Indicates an invalid label ID.
     */
    int INVALID_LABEL_ID = -1;

    /**
     * Indicates the label name of an attribute.
     */
    std::string labelName;

    /**
     * Indicates the relation name.
     */
    std::string relationName;

    /**
     * Indicates the label id.
     */
    int labelId = -1;
};

class SipAddress {
public:
    /**
     * Indicates a custom label.
     */
    int CUSTOM_LABEL = 0;

    /**
     * Indicates a home SIP address.
     */
    int SIP_HOME = 1;

    /**
     * Indicates a work SIP address.
     */
    int SIP_WORK = 2;

    /**
     * Indicates an SIP address of the OTHER type.
     */
    int SIP_OTHER = 3;

    /**
     * Indicates an invalid label ID.
     */
    int INVALID_LABEL_ID = -1;

    /**
     * Indicates the label name of an attribute.
     */
    std::string labelName;

    /**
     * Indicates the SIP address.
     */
    std::string sipAddress;

    /**
     * Indicates the label id.
     */
    int labelId = -1;
};

class Website {
public:
    std::string website;
};

/**
 * Provides methods for contact information
 */
class Contacts {
public:
    /**
     * Indicates the contact ID.
     */
    static int INVALID_CONTACT_ID;

    /**
     * Indicates the contact ID.
     */
    int id;

    /**
     * Indicates the query key that identifies the contact.
     */
    std::string key;

    /**
     * Indicates the contact attributes.
     */
    ContactAttributes contactAttributes;

    /**
     * Indicates an email address of the contact.
     */
    std::vector<Email> emails;

    /**
     * Indicates an event (special date) of the contact.
     */
    std::vector<Event> events;

    /**
     * Indicates a group of the contact.
     */
    std::vector<Group> groups;

    /**
     * Indicates an IM address of the contact.
     */
    std::vector<ImAddress> imAddresses;

    /**
     * Indicates a phone number of the contact.
     */
    std::vector<PhoneNumber> phoneNumbers;

    /**
     * Indicates the contact portrait.
     */
    Portrait portrait;

    /**
     * Indicates a postal address of the contact.
     */
    std::vector<PostalAddress> postalAddresses;

    /**
     * Indicates a relation of the contact.
     */
    std::vector<Relation> relations;

    /**
     * Indicates a Session Initiation Protocol (SIP) address of the contact.
     */
    std::vector<SipAddress> sipAddresses;

    /**
     * Indicates a website of the contact.
     */
    std::vector<Website> websites;

    /**
     * Indicates the contact name.
     */
    Name name;

    /**
     * Indicates the contact nickname.
     */
    NickName nickName;
    /**
     * Indicates the contact note.
     */
    Note note;
    /**
     * Indicates organization information about the contact.
     */
    Organization organization;
};
} // namespace ContactsApi
} // namespace OHOS
#endif // CONTACT_NAPI_OBJECT_H
