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

import { Contact } from '../../components/PlatformInterface/Contact';
import { StringUtil } from '../utils/StringUtil'
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import rdb from '@ohos.data.relationalStore';

export class ContactInfo {
  public id: number = 0;
  public deleteId: number = -1;
  public contactId: number = 0;
  public isDeleted: number = 0;
  public recycledTime: string = '';
  public uuid: string | null = null;
  //todo to be deleted
  public birthday: string = '';
  public displayName: string = '';
  // 0 表示真名字，1 表示 displayName 是从其他信息取的假名字
  public updateFlag: number = 0;
  public nicknameList: Contact.NickName[] = [];  
  public notes: Contact.Note[] = [];
  public emails: Contact.Email[] = [];
  public position: string = '';
  public company: string = '';
  public aims: Contact.ImAddress[] = [];
  public websites: Contact.Website[] = [];
  //todo to be deleted
  public relationships: Contact.SipAddress[] = [];
  public events: Contact.Event[] = [];
  public groups: string[] = []; 
  public favorite: number | null = null;
  public favoriteOrder: number | null = null;
  public postalList: Contact.PostalAddress[] = [];
  public relationList: Contact.Relation[] = [];
  public phoneList: Contact.PhoneNumber[] = [];
  public photo: Contact.Portrait = new Contact.Portrait();
  // 联系人大头像文件名称
  public photoName: string = '';
  // 联系人铃音文件名
  public ringToneName: string = '';
  //todo to be deleted
  public sipAddress: Contact.SipAddress[] = [];
  //todo to be deleted
  public camcard: Contact.Camcard = new Contact.Camcard();
  public contactDataKeySet: Set<string> = new Set<string>();
  public dirty: number = 0;
  public syn_1: string = '';
  public uniqueKey: string = '';
  public formId: string | null = null;
  public insertValuesBucket: ValuesBucket[] = [];
  public familyName: string = '';
  public otherLanLastName: string = ''; // middleName
  public givenName: string = '';
  public alphaName: string = ''; // Prefix name
  public otherLanFirstName: string = ''; // Suffix name
  public middleNamePhonetic: string = '';
  public givenNamePhonetic: string = '';
  public phoneticName: string = ''; // familyNamePhonetic
  public fullNamePhonetic: string = ''; // raw_contacts表中phonetic_name
  public attachments: rdb.Assets | undefined = undefined;
  public contactedCount: number = 0;
  public lastestContactedTime: number = 0;
  public focusModeList: string | null = null;
  public oldRawId: number = 0;
  public oldContactId : number = 0;
  public photoFileName: string = '';
  public primaryContact : number = 0;

  public isEmpty() {
    return this.notes == null && this.emails == null && this.postalList == null && this.aims == null &&
      this.websites.length == 0 && this.nicknameList == null && this.relationList == null &&
      this.phoneList == null && this.groups == null && this.displayName == '' && this.events == null &&
      this.photo == null && this.sipAddress == null && this.camcard == null;
  }

  public getLogStr() {
    return `download contact info id:${this.id},contactId:${this.contactId},uuid:${StringUtil.getShortUuid(this.uuid)
      },isDeleted:${this.isDeleted},recycledTime:${this.recycledTime},displayName:${StringUtil.maskSensitiveInfo(this.displayName)
      },phoneList:${this.phoneList?.map((phone: Contact.PhoneNumber) => StringUtil.maskSensitiveInfo(phone?.phoneNumber!)).join(',')
      },attachments:${this.attachments?.map((attachment: rdb.Asset) => StringUtil.maskSensitiveInfo(attachment?.name)).join(',')
      },emails:${this.emails?.map((email: Contact.Email) => StringUtil.maskSensitiveInfo(email?.email!)).join(',')},updateFlag:${
      this.updateFlag}${this.favorite ? `,favoriteOrder:${this.favoriteOrder}` : ''}`;
  }
}