/*
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

const featureAbility = requireNapi('ability.featureAbility');
const contact = requireInternal('Contact');
const deviceinfo = requireNapi('deviceInfo');
const ARGUMENTS_LEN_ONE = 1;
const ARGUMENTS_LEN_TWO = 2;
const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_CODE_DEVICE_NOTFOUD = 801;
const ERROR_SYSTEM = 16700001;
const ERROR_QUERY_VALUE_FAILED = 16700101;
const ERROR_SET_VALUE_FAILED = 16700102;
const ERROR_INVALID_PARAMETER = 16700002;
const ERROR_USER_CANCEL = 16700103;
const ERROR_OVER_LIMIT = 16700004;
const DATA_FIELD_EMAIL = 0;
const DATA_FIELD_PHONE = 1;
const DISPLAY_TYPE_EMAIL = 'EMAIL';
const DISPLAY_TYPE_PHONE = 'PHONE';
const DISPLAY_TYPE_DEFAULT = 'DEFAULT';

// 批量导入联系人最大数量限制
const IMPORT_CONTACTS_MAX_COUNT = 100;

// 批量导入联系人最小数量限制
const IMPORT_CONTACTS_MIN_COUNT = 1;

// 用户未选择的联系人返回ID
const USER_NOT_SELECT_CONTACT_ID = -2;

// picker处理成功
const PICKER_RESULTCODE_SUCCESS = 0;

// picker处理取消
const PICKER_RESULTCODE_CANCEL = 1;

const ERROR_MSG_INVALID_PARAM = 'Parameter error. Possible causes: Mandatory parameters are left unspecified';
const ERROR_MSG_SELECT_INVALID_PARAM = 'Parameter error. Possible causes: Parameter verification failed';
const ERROR_MSG_INVALID_PARAMETER = 'Invalid parameter value';
const ERROR_MSG_DEVICE_NOTFOUD = 'The specified SystemCapability name was not found.';
const ERROR_MSG_SYSTEM = 'General error.';
const ERROR_MSG_QUERY_VALUE_FAILED = 'Failed to get value from contacts data.';
const ERROR_MSG_SET_VALUE_FAILED = 'Failed to set value to contacts data.';
const ERROR_MSG_USER_CANCE = 'User cancel.';
const ERROR_MSG_OVER_LIMIT = 'The number of contacts exceeds the limit.';

const pushErrMap = new Map();
pushErrMap.set(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAMETER);
pushErrMap.set(ERROR_INVALID_PARAMETER, ERROR_MSG_INVALID_PARAM);
pushErrMap.set(ERROR_CODE_DEVICE_NOTFOUD, ERROR_MSG_DEVICE_NOTFOUD);
pushErrMap.set(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
pushErrMap.set(ERROR_QUERY_VALUE_FAILED, ERROR_MSG_QUERY_VALUE_FAILED);
pushErrMap.set(ERROR_SET_VALUE_FAILED, ERROR_MSG_SET_VALUE_FAILED);
pushErrMap.set(ERROR_USER_CANCEL, ERROR_MSG_USER_CANCE);
pushErrMap.set(ERROR_OVER_LIMIT, ERROR_MSG_OVER_LIMIT);

const contactKeyArr = new Set();
contactKeyArr.add('id').add('key').add('contactAttributes').add('emails')
    .add('events').add('groups').add('imAddresses').add('phoneNumbers')
    .add('portrait').add('postalAddresses').add('relations').add('sipAddresses')
    .add('websites').add('name').add('nickName').add('note').add('organization');
class BusinessError extends Error {
  constructor(code, desc) {
    let message = '';
    if (desc) {
      message = desc;
    } else if (pushErrMap.has(code)) {
      message = pushErrMap.get(code);
    } else {
      message = ERROR_MSG_SYSTEM;
    }
    super(message);
    this.code = code;
  }
}

let gContext = undefined;

function getDeviceType() {
  let result = '';
  try {
    result = deviceinfo.deviceType.toLowerCase();
    console.log('[picker] getDeviceType deviceType: ' + result);
    return result;
  } catch (error) {
    console.error('[picker] getDeviceType error');
    return result;
  }
}

function isSupportedDeviceType(currentDevice) {
  console.log('[picker] isSupportedDeviceType: ' + currentDevice);
  if (currentDevice === 'phone' || currentDevice === '2in1' || currentDevice === 'tablet') {
    return true;
  }
  return false;
}

function startContactsPicker(context, config, type) {
  console.log('[picker] startContactsPicker');
  if (context === undefined) {
    throw Error('[picker] contact gContext undefined');
  }
  if (config === undefined) {
    throw Error('[picker] contact config undefined');
  }
  gContext = context;
  let helper;
  if (type === 'select') {
    helper = contact.startContactsPicker(gContext, config);
    console.log('[picker] selectContactsPicker');
    if (helper === undefined) {
      console.error('[picker] contact startContactsPicker helper is undefined');
    }
  } else if (type === 'save') {
    console.log('[picker] saveContactsPicker');
    helper = contact.startSaveContactsPicker(gContext, config);
    if (helper === undefined) {
      console.error('[picker] contact startContactsPicker helper is undefined');
    }
  } else if (type === 'saveExist') {
    console.log('[picker] saveExistContactsPicker');
    helper = contact.startSaveExistContactsPicker(gContext, config);
    if (helper === undefined) {
      console.error('[picker] contact startContactsPicker helper is undefined');
    }
  } else if (type === 'import') {
    console.log('[picker] importContactsPicker');
    helper = contact.startImportContactsPicker(gContext, config);
    if (helper === undefined) {
      console.error('[picker] contact startImportContactsPicker helper is undefined');
    }
  }
  return helper;
}

function parseContactsPickerSelectOption(args) {
  console.log('[picker] parseContactsPickerSelectOption');
  let config = {
    parameters: {
      'pageFlag': 'page_flag_single_choose',
      'isContactMultiSelect': false,
      'isContactsPicker': true
    },
    bundleName: 'com.ohos.contacts',
    abilityName: 'ContactUiExtentionAbility'
  };

  if (args.length > 0 && typeof args[0] === 'object') {
    let option = args[0];
    if (option.isMultiSelect !== undefined) {
      config.parameters.pageFlag =
          option.isMultiSelect === true ? 'page_flag_multi_choose' : 'page_flag_single_choose';
      config.parameters.isContactMultiSelect = option.isMultiSelect;
    }
    if (option.maxSelectable !== undefined) {
      config.parameters.selectLimit = option.maxSelectable;
    } else if (option.selectLimit !== undefined) {
      config.parameters.selectLimit = option.selectLimit;
    }
    if (option.isDisplayedByName !== undefined) {
      config.parameters.isDisplayByName = option.isDisplayedByName;
    } else if (option.isDisplayByName !== undefined) {
      config.parameters.isDisplayByName = option.isDisplayByName;
    }
    if (option.filter !== undefined) {
      config.parameters.filter = option.filter;
      try {
        parseDisplayType(config, option);
      } catch (error) {
        console.error('[picker] parseDisplayType error: ' + error);
      }
    }
  }
  return config;
}

function parseDisplayType(config, option) {
  console.log('[picker] parseDisplayType start ');
  if (option.filter === undefined || option.filter.filterClause === undefined ||
    option.filter.filterClause.dataItem === undefined || option.filter.filterClause.dataItem.field === undefined)
  {
    console.log('[picker] parseDisplayType not have dataField');
    return;
  }
  let dataField = option.filter.filterClause.dataItem.field;
  console.log('[picker] parseDisplayType dataField ' + dataField);
  if (dataField === DATA_FIELD_EMAIL) {
    config.parameters.displayType = DISPLAY_TYPE_EMAIL;
  } else if (dataField === DATA_FIELD_PHONE) {
    config.parameters.displayType = DISPLAY_TYPE_PHONE;
  } else {
    config.parameters.displayType = DISPLAY_TYPE_DEFAULT;
  }
}

function parseContactsPickerSaveOption(args) {
  console.log('[picker] parseContactsPickerSaveOption');
  let config = {
    parameters: {
      'pageFlag': 'page_flag_save_contact',
      'isContactsPicker': true,
      'isSaveContact': true
    },
    bundleName: 'com.ohos.contacts',
    abilityName: 'ContactUiExtentionAbility'
  };

  if (args.length > 0 && typeof args[0] === 'object' && typeof args[1] === 'object') {
    let option = args[1];
    if (option) {
      config.parameters.contact = option;
    }
  }
  return config;
}

function parseContactsPickerSaveExistOption(args) {
  console.log('[picker] parseContactsPickerSaveExistOption');
  let config = {
    parameters: {
      'pageFlag': 'page_flag_save_exist_contact',
      'isContactsPicker': true,
      'isContactMultiSelect': false,
      'isSaveExistContact': true,
      'isDisplayByName': true
    },
    bundleName: 'com.ohos.contacts',
    abilityName: 'ContactUiExtentionAbility'
  };

  if (args.length > 0 && typeof args[0] === 'object' && typeof args[1] === 'object') {
    let option = args[1];
    if (option) {
      config.parameters.contact = option;
    }
  }
  return config;
}

function parseImportContactsPickerOption(args) {
  console.log('[picker] parseImportContactsPickerOption');
  let config = {
    parameters: {
      'tagetUrl': 'ImportContactsPage',
      'isContactsPicker': true,
      'isContactMultiSelect': true
    },
    bundleName: 'com.ohos.contacts',
    abilityName: 'ContactUiExtentionAbility'
  };

  if (args.length > 0 && typeof args[0] === 'object' && typeof args[1] === 'object') {
    let contacts = args[1];
    if (contacts) {
      config.parameters.importContacts = contacts;
    }
  }
  return config;
}

/**
 * 校验导入联系人参数
 * @throw ERROR_CODE_DEVICE_NOTFOUD/ERROR_INVALID_PARAMETER/ERROR_OVER_LIMIT
 */
function validateImportContactsArgs(args) {
  let currentDevice = getDeviceType();
  if (!isSupportedDeviceType(currentDevice)) {
    console.error('[picker] device not found');
    throw new BusinessError(ERROR_CODE_DEVICE_NOTFOUD, ERROR_MSG_DEVICE_NOTFOUD);
  }
  if (arguments.length !== ARGUMENTS_LEN_TWO) {
    console.error('[picker] importContactViaUI Parameter error');
    throw new BusinessError(ERROR_INVALID_PARAMETER, ERROR_MSG_INVALID_PARAMETER);
  }
  if (!Array.isArray(args[1] || args[1].length < IMPORT_CONTACTS_MIN_COUNT)) {
    console.error('[picker] importContactViaUI contacts is not array or empty array');
    throw new BusinessError(ERROR_INVALID_PARAMETER, ERROR_MSG_INVALID_PARAMETER);
  }
  if (args[1].length > IMPORT_CONTACTS_MAX_COUNT) {
    console.error('[picker] importContactViaUI contacts count over limit:' + args[1].length);
    throw new BusinessError(ERROR_OVER_LIMIT, ERROR_MSG_OVER_LIMIT);
  }
  // 校验每个联系的属性是否合法
  for (let i = 0; i < args[1].length; i++) {
    let keyArr = Object.keys(args[1][i]);
    keyArr.forEach(item => {
      if (!contactKeyArr.has(item)) {
        console.error(`[picker] importContactViaUI, property: ${item} not exist`);
        throw new BusinessError(ERROR_INVALID_PARAMETER, ERROR_MSG_INVALID_PARAMETER);
      }
    });
  }
}

/**
 * 解析导入结果并按原始索引填充
 * @param  pickerData 解析应用侧的处理结果
 * @param  inputLength 传入的总数
 * @throws ERROR_SYSTEM
 * @returns 处理后的数组
 */
function parseImportResult(pickerData, inputLength) {
  let selectedArray = [];
  try {
    selectedArray = JSON.parse(pickerData);
  } catch (error) {
    console.error('[picker] importContactViaUI JSON.parse Error: ${error}');
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }
  let fullResult = new Array(inputLength).fill(USER_NOT_SELECT_CONTACT_ID);
  for (let i = 0; i < selectedArray.length; i++) {
    if (selectedArray[i]) {
      let originalIndex = selectedArray[i].index;
      let contactId = selectedArray[i].id;
      if (originalIndex >= 0 && originalIndex < inputLength) {
        fullResult[originalIndex] = contactId;
      }
    }
  }
  return fullResult;
}

/**
 * 处理导入结果
 * @throw ERROR_USER_CANNEL/ERROR_SYSTEM
 */
function handleImportResult(result, args) {
  if (result.resultCode === PICKER_RESULTCODE_SUCCESS) {
    return parseImportResult(result.pickerData, args[1].length);
  }
  if (result.resultCode === PICKER_RESULTCODE_CANCEL) {
    console.error('[picker] importContactViaUI Error: user cancel');
    throw new BusinessError(ERROR_USER_CANCEL, ERROR_MSG_USER_CANCE);
  }
  console.error('[picker] importContactViaUI Error: system');
  throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
}

/**
 * 导入联系人picker
 * @throw ERROR_SYSTEM 
 */
async function importContactViaUI(...args) {
  console.log('[picker] importContactViaUI START');
  validateImportContactsArgs(args);
  const config = parseImportContactsPickerOption(args);
  let result;
  try {
    let context = getContext(this);
    result = await startContactsPicker(context, config, 'import');
  } catch (error) {
    console.error('[picker] importContactViaUI error: ' + error);
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }
  if (!result) {
    console.error(`[picker] Error importContactViaUI`);
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }
  console.warn(`[picker] importContactViaUI resultCode : ${result.resultCode}`);
  return handleImportResult(result, args);
}


async function contactsPickerSaveToExist(...args) {
  console.log('[picker] contactsPickerSaveToExist START');
  let currentDevice = getDeviceType();
  if (!isSupportedDeviceType(currentDevice)) {
    console.error('[picker] device not found');
    throw new BusinessError(ERROR_CODE_DEVICE_NOTFOUD, ERROR_MSG_DEVICE_NOTFOUD);
  }
  console.error(`[picker] contactsPickerSaveToExist`);
  if (arguments.length !== ARGUMENTS_LEN_TWO || JSON.stringify(args[1]).length <= 2) {
    console.error('[picker] contactsPickerSaveToExist Parameter error');
    throw new BusinessError(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
  }
  let jsonObj = JSON.parse(JSON.stringify(args[1]));
  let keyArr = Object.keys(jsonObj);
  keyArr.forEach(item => {
    if (!contactKeyArr.has(item)) {
      console.error(`[picker] contactsPickerSaveToExist, property: ${item} not exist`);
      throw new BusinessError(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
    }
  });
  const config = parseContactsPickerSaveExistOption(arguments);
  let result;
  try {
    let context = getContext(this);
    let pickerType = 'saveExist';
    result = await startContactsPicker(context, config, pickerType);
  } catch (error) {
    console.error('[picker] contactsPickerSaveToExist error: ' + error);
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }

  if (result === undefined) {
    console.error(`[picker] Error ContactsPickerSaveToExist`);
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }
  let jsContactId = result.jsContactId;
  console.warn(`[picker] save exist contact Id : ${jsContactId}`);
  console.warn(`[picker] save exist contact resultCode : ${result.resultCode}`);
  
  if (result.resultCode === 0) {
    return jsContactId;
  } else {
    if (result.resultCode === 1) {
      console.error(`[picker] contactsPickerSaveToExist Error: user cancel`);
      throw new BusinessError(ERROR_USER_CANCEL, ERROR_MSG_USER_CANCE);
    }
    console.error(`[picker] contactsPickerSaveToExist Error: system`);
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }
}

async function contactsPickerSave(...args) {
  console.log('[picker] contactsPickerSave START');
  let currentDevice = getDeviceType();
  if (!isSupportedDeviceType(currentDevice)) {
    console.error('[picker] device not found');
    throw new BusinessError(ERROR_CODE_DEVICE_NOTFOUD, ERROR_MSG_DEVICE_NOTFOUD);
  }
  console.error(`[picker] contactsPickerSave`);
  if (arguments.length !== ARGUMENTS_LEN_TWO || JSON.stringify(args[1]).length <= 2) {
    console.error('[picker] contactsPickerSave Parameter error');
    throw new BusinessError(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
  }
  let jsonObj = JSON.parse(JSON.stringify(args[1]));
  let keyArr = Object.keys(jsonObj);
  keyArr.forEach(item => {
    if (!contactKeyArr.has(item)) {
      console.error(`[picker] contactsPickerSave, property: ${item} not exist`);
      throw new BusinessError(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
    }
  });
  const config = parseContactsPickerSaveOption(arguments);
  let result;
  try {
    let context = getContext(this);
    let pickerType = 'save';
    result = await startContactsPicker(context, config, pickerType);
  } catch (error) {
    console.error('[picker] contactsPickerSave error: ' + error);
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }

  if (result === undefined) {
    console.error(`[picker] Error ContactsPickerSave`);
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }
  let jsContactId = result.jsContactId;
  console.warn(`[picker] save contact Id : ${jsContactId}`);
  console.warn(`[picker] save contact resultCode : ${result.resultCode}`);

  if (result.resultCode === 0) {
    return jsContactId;
  } else {
    if (result.resultCode === 1) {
      console.error(`[picker] contactsPickerSave Error: user cancel`);
      throw new BusinessError(ERROR_USER_CANCEL, ERROR_MSG_USER_CANCE);
    }
    console.error(`[picker] contactsPickerSave Error: system`);
    throw new BusinessError(ERROR_SYSTEM, ERROR_MSG_SYSTEM);
  }
}

function checkSelectParameter(config) {
  console.warn('[picker] checkSelectParameter config.parameters.selectLimit ' + config.parameters.selectLimit);
  if (config.parameters.selectLimit === undefined || config.parameters.selectLimit > 0) {
    return true;
  }
  return false;
}

function parseContacts(result) {
  let pickerData = result.pickerData;
  let contacts = [];
  try {
    contacts = JSON.parse(pickerData);
  } catch (Error) {
    console.error(`[picker] JSON.parse Error: ${Error}`);
  }
  if (contacts.length !== result.total) {
    console.error(`[picker] contacts.length : ${contacts.length} is not equal to total : ${result.total}`);
  }
  console.warn(`[picker] contacts.length : ${contacts.length}`);
  return contacts;
}

async function contactsPickerSelect(...args) {
  console.log('[picker] contactsPickerSelect START');
  if (arguments.length === ARGUMENTS_LEN_TWO && typeof arguments[1] !== 'function') {
    console.error('[picker] contactsPickerSelect callback invalid');
    throw Error('invalid callback');
  }
  const config = parseContactsPickerSelectOption(arguments);
  if (!checkSelectParameter(config)) {
    if (arguments.length === ARGUMENTS_LEN_TWO) {
      return args[1](new BusinessError(ERROR_CODE_INVALID_PARAM, ERROR_MSG_SELECT_INVALID_PARAM), undefined);
    } else if (arguments.length === ARGUMENTS_LEN_ONE) {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM, ERROR_MSG_SELECT_INVALID_PARAM);
    }
  }
  try {
    let context = getContext(this);
    let pickerType = 'select';
    let result = await startContactsPicker(context, config, pickerType);
    let contacts = parseContacts(result);
    if (arguments.length === ARGUMENTS_LEN_TWO && typeof arguments[1] === 'function') {
      if (result.resultCode === 0 || result.resultCode === 1) {
        return args[1](undefined, contacts);
      } else {
        return args[1](result.resultCode, undefined);
      }
    } else if (arguments.length === 1 && typeof arguments[0] === 'function') {
      if (result.resultCode === 0 || result.resultCode === 1) {
        return args[0](undefined, contacts);
      } else {
        return args[0](result.resultCode, undefined);
      }
    }
    return new Promise((resolve, reject) => {
      if (result.resultCode === 0 || result.resultCode === 1) {
        resolve(contacts);
      } else {
        reject(result.resultCode);
      }
    });
  } catch (error) {
    console.error('[picker] contactsPickerSelect error: ' + error);
    return undefined;
  }
}

export default {
  selectContact: contactsPickerSelect,
  selectContacts: contactsPickerSelect,
  saveToExistingContactViaUI: contactsPickerSaveToExist,
  addContactViaUI: contactsPickerSave,
  importContactsViaUI: importContactsViaUI,
  addContact: contact.addContact,
  addContacts: contact.addContacts,
  deleteContact: contact.deleteContact,
  queryContactsCount: contact.queryContactsCount,
  queryContact: contact.queryContact,
  queryContacts: contact.queryContacts,
  queryContactsByEmail: contact.queryContactsByEmail,
  queryContactsByPhoneNumber: contact.queryContactsByPhoneNumber,
  queryGroups: contact.queryGroups,
  queryHolders: contact.queryHolders,
  queryKey: contact.queryKey,
  queryMyCard: contact.queryMyCard,
  updateContact: contact.updateContact,
  isLocalContact: contact.isLocalContact,
  isMyCard: contact.isMyCard,
  hasMatchedCallLog: contact.hasMatchedCallLog,

  Contact: contact.Contact,
  ContactAttributes: contact.ContactAttributes,
  Attribute: contact.Attribute,
  Email: contact.Email,
  Event: contact.Event,
  Group: contact.Group,
  Holder: contact.Holder,
  ImAddress: contact.ImAddress,
  Name: contact.Name,
  NickName: contact.NickName,
  Note: contact.Note,
  Organization: contact.Organization,
  PhoneNumber: contact.PhoneNumber,
  Portrait: contact.Portrait,
  PostalAddress: contact.PostalAddress,
  Relation: contact.Relation,
  SipAddress: contact.SipAddress,
  Website: contact.Website,
  FilterCondition: contact.FilterCondition,
  DataField: contact.DataField,
  FilterType: contact.FilterType,
};
