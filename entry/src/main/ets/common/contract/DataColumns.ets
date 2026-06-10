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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import BaseColumns from './BaseColumns';

/**
 * Field constants in the contact data table
 */
export default class DataColumns extends BaseColumns {
  /**
   * Column 'type_id' in table data
   */
  public static readonly TYPE_ID: string = 'type_id';

  /**
   * Column 'raw_contact_id' in table data
   */
  public static readonly RAW_CONTACT_ID: string = 'raw_contact_id';

  /**
   * Column 'read_only' in table data
   */
  public static readonly READ_ONLY: string = 'read_only';

  /**
   * Column 'version' in table data
   */
  public static readonly VERSION: string = 'version';

  /**
   * Column 'is_preferred_number' in table data
   */
  public static readonly IS_PREFERRED_NUMBER: string = 'is_preferred_number';

  /**
   * Column 'format_phone_number' in table data
   */
  public static readonly FORMAT_PHONE_NUMBER: string = 'format_phone_number';

  /**
   * Column 'detail_info' in table data
   */
  public static readonly DETAIL_INFO: string = 'detail_info';

  /**
  * Column 'location' in table data
  */
  public static readonly LOCATION: string = 'location';

  /**
   * Column 'family_name' in table data
   */
  public static readonly FAMILY_NAME: string = 'family_name';

  /**
   * Column 'middle_name_phonetic' in table data
   */
  public static readonly MIDDLE_NAME_PHONETIC: string = 'middle_name_phonetic';

  /**
   * Column 'given_name' in table data
   */
  public static readonly GIVEN_NAME: string = 'given_name';

  /**
   * Column 'alpha_name' in table data
   */
  public static readonly ALPHA_NAME: string = 'alpha_name';

  /**
   * Column 'other_lan_first_name' in table data
   */
  public static readonly OTHER_LAN_FIRST_NAME: string = 'other_lan_first_name';

  /**
   * Column 'other_lan_last_name' in table data
   */
  public static readonly OTHER_LAN_LAST_NAME: string = 'other_lan_last_name';

  /**
   * Column 'given_name_phonetic' in table data
   */
  public static readonly GIVEN_NAME_PHONETIC: string = 'given_name_phonetic';

  /**
   * Column 'name_phonetic' in table data
   */
  public static readonly NAME_PHONETIC: string = 'name_phonetic';
  /**
   * Column 'alias_detail_info' in table data
   */
  public static readonly ALIAS_DETAIL_INTO: string = 'alias_detail_info';

  /**
   * Column 'phonetic_name' in table data
   */
  public static readonly PHONETIC_NAME: string = 'phonetic_name';

  /**
   * Column 'lan_style' in table data
   */
  public static readonly LAN_STYLE: string = 'lan_style';

  /**
   * Column 'position' in table data
   */
  public static readonly POSITION: string = 'position';

  /**
   * Column 'custom_data' in table data
   */
  public static readonly CUSTOM_DATA: string = 'custom_data';

  /**
   * Column 'extend7' in table data
   */
  public static readonly EXTEND7: string = 'extend7';

  /**
   * Column 'street' in table data
   */
  public static readonly STREET: string = 'street';

  /**
   * Column 'pobox' in table data
   */
  public static readonly POBOX: string = 'pobox';

  /**
   * Column 'neighborhood' in table data
   */
  public static readonly NEIGHBORHOOD: string = 'neighborhood';

  /**
   * Column 'city' in table data
   */
  public static readonly CITY: string = 'city';

  /**
   * Column 'region' in table data
   */
  public static readonly REGION: string = 'region';

  /**
   * Column 'postcode' in table data
   */
  public static readonly POSTCODE: string = 'postcode';

  /**
   * Column 'country' in table data
   */
  public static readonly COUNTRY: string = 'country';

  /**
   * Column 'blob_data' in table data
   */
  public static readonly BLOB_DATA: string = 'blob_data';


  /**
   * Column 'mimetype_id' in table data
   */
  public static readonly MIMETYPE_ID: string = 'mimetype_id';

  /**
   * Column 'is_primary' in table data
   */
  public static readonly IS_PRIMARY: string = 'is_primary';

    /**
   * Column 'is_super_primary' in table data
   */
    public static readonly IS_SUPER_PRIMARY: string = 'is_super_primary';

  /**
   * Column 'data1' in table data
   */
  public static readonly DATA1: string = 'data1';

  /**
   * Column 'data2' in table data
   */
  public static readonly DATA2: string = 'data2';

  /**
   * Column 'data3' in table data
   */
  public static readonly DATA3: string = 'data3';

  /**
   * Column 'data4' in table data
   */
  public static readonly DATA4: string = 'data4';

  /**
   * Column 'data5' in table data
   */
  public static readonly DATA5: string = 'data5';

  /**
   * Column 'data6' in table data
   */
  public static readonly DATA6: string = 'data6';

  /**
   * Column 'data7' in table data
   */
  public static readonly DATA7: string = 'data7';

  /**
   * Column 'data8' in table data
   */
  public static readonly DATA8: string = 'data8';

  /**
   * Column 'data9' in table data
   */
  public static readonly DATA9: string = 'data9';

  /**
   * Column 'data10' in table data
   */
  public static readonly DATA10: string = 'data10';

  /**
   * Column 'data14' in table data
   */
  public static readonly DATA14: string = 'data14';
  /**
   * Column 'data15' in table data
   */
  public static readonly DATA15: string = 'data15';

  /**
   * Column 'mimetype' in clone table data
   */
  public static readonly MIMETYPE: string = 'mimetype';

  /**
   * Column 'photo_uri' in clone table data
   */
  public static readonly PHOTO_URI: string = 'photo_uri';

  /**
   * Column 'extend1' in table data
   */
  public static readonly EXTEND1: string = 'extend1';

  /**
   * Column 'extend2' in table data
   */
  public static readonly EXTEND2: string = 'extend2';

  /**
   * Column 'extend3' in table data
   */
  public static readonly EXTEND3: string = 'extend3';

  /**
   * Column 'extend4' in table data
   */
  public static readonly EXTEND4: string = 'extend4';

  /**
   * Column 'extend5' in table data
   */
  public static readonly EXTEND5: string = 'extend5';

  /**
   * Column 'extend6' in table data
   */
  public static readonly EXTEND6: string = 'extend6';

  /**
   * Column 'syn_1' in table data
   */
  public static readonly SYN_1: string = 'syn_1';

  /**
   * Column 'syn_2' in table data
   */
  public static readonly SYN_2: string = 'syn_2';

  /**
   * Column 'syn_3' in table data
   */
  public static readonly SYN_3: string = 'syn_3';

  /**
   * Column 'extend8' in table data
   */
  public static readonly EXTEND8: string = 'extend8';

  /**
   * Column 'extend9' in table data
   */
  public static readonly EXTEND9: string = 'extend9';

  /**
   * Column 'extend10' in table data
   */
  public static readonly EXTEND10: string = 'extend10';

  /**
   * Column 'extend11' in table data
   */
  public static readonly EXTEND11: string = 'extend11';
}