/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
import { BusinessError } from '@kit.BasicServicesKit';
import { calendarManager } from '@kit.CalendarKit'
import common from '@ohos.app.ability.common';
import bundleManager from '@ohos.bundle.bundleManager';
import DataColumns from '../common/data/DataColumns';
import LogUtils from '../common/utils/LogUtils';
import RawContactsColumns from '../common/data/RawContactsColumns';
import relationalStore from '@ohos.data.relationalStore';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import { abilityAccessCtrl } from '@kit.AbilityKit';
import SettingsColumns from '../common/data/SettingsColumns';
import { StringUtil } from '../common/utils/StringUtil';
import { contactRdbHelper } from '../common/helperUtils/ContactsRdbHelperForCloudSync';
import DbConstants from '../common/DbConstants';

const TAG = 'BirthdayManager';
const EVENT_TYPE_ID: number = 11;
const BIRTHDAY_TYPE: string[] = ['1', '2', '1.0', '2.0'];
const BIRTHDAY_NOT_SYNC_DIRTY: number = 0;
const BIRTHDAY_NOT_SYNC_SUCCEED: number = 1;
const BIRTHDAY_NOT_SYNC_FAILED: number = 2;
const BIRTHDAY_NOT_SYNC_INVALID: number = 3;
const BIRTHDAY_TYPE_LUNAR: string = '2';
const BIRTHDAY_TYPE_LUNAR_POINT: string = '2.0';
const NORMAIL_CONTACT: number = 0;
const columns = [DataColumns.DETAIL_INFO, RawContactsColumns.DISPLAY_NAME, RawContactsColumns.CONTACT_ID,
    DataColumns.CUSTOM_DATA, DataColumns.CALENDAR_EVENT_ID, DataColumns.RAW_CONTACT_ID];
export default class BirthdayManager {
  private context: common.Context | undefined = undefined;
  private calendar: calendarManager.Calendar | undefined = undefined;
  private calendarManager: calendarManager.CalendarManager | null = null;
  private static singleInstance: BirthdayManager;
 
  /**
   *  单例
   * @returns BirthdayManager
   */
  public static getInstance(): BirthdayManager {
    if (BirthdayManager.singleInstance == null) {
      BirthdayManager.singleInstance = new BirthdayManager();
    }
    return BirthdayManager.singleInstance;
  }
 
  /**
   * 初始化方法
   * @param context context
   */
  public init(context: common.Context) {
    this.context = context;
  }
 
  /**
   * 查询是否有读写日历的权限
   */
  public async isHaveCalendarPermission(): Promise<Boolean> {
    let info = await bundleManager.getApplicationInfo('com.ohos.contactsdataability',
      bundleManager.ApplicationFlag.GET_APPLICATION_INFO_WITH_PERMISSION);
    if (info) {
      let hasPermission: number = abilityAccessCtrl.createAtManager().
        verifyAccessTokenSync(info.accessTokenId, 'ohos.permission.WRITE_CALENDAR');
      LogUtils.i(TAG, `isHaveCalendarPermission hasPermission = ${hasPermission}`);
      return hasPermission === 0;
    }
    return false;
  }
 
  /**
   * 生成calendar对象
   */
  public async createCalendar(): Promise<boolean> {
    let isHaveCalendarPermission = await this.isHaveCalendarPermission();
    if (!isHaveCalendarPermission) {
      LogUtils.i(TAG, `createCalendar failed, no calendar permission.`);
      return false;
    }
    this.calendarManager = calendarManager.getCalendarManager(this.context!);
    if (!this.calendarManager || this.calendarManager === undefined) {
      return false;
    }
    // 日历账号，联系人数据库的日历账号
    const calendarAccount: calendarManager.CalendarAccount = {
      name: bundleManager?.getBundleInfoForSelfSync(
        bundleManager?.BundleFlag.GET_BUNDLE_INFO_WITH_APPLICATION).name,
      type: calendarManager?.CalendarType?.BIRTHDAY
    };
    try {
      this.calendar = await this.calendarManager?.getCalendar(calendarAccount);
      return (this.calendar && this.calendar !== undefined);
    } catch (err) {
      LogUtils.e(TAG, `Failed to get calendar. Code: ${err.code}, message: ${err.message}`);
      if (err) {
        try {
          this.calendar = await this.calendarManager?.createCalendar(calendarAccount);
          LogUtils.i(TAG, `Succeeded in creating calendar`);
          if (this.calendar) {
            this.calendarSetConfig(this.calendar);
            return true;
          }
        } catch (error) {
          LogUtils.e(TAG, `Failed to create calendar. Code: ${error.code}, message: ${error.message}`);
        }
      }
    };
    return false;
  }
 
  /**
   * 检查calendar对象
   * @returns 
   */
  private async checkCalendarObject(): Promise<boolean> {
    if (!this.calendar || this.calendar === undefined) {
      return await this.createCalendar();
    }
    return true;
  }
 
  /**
   * 通过raw_contact_id列表查询view_contact_data表中生日信息，用于删除日历中生日
   * @param rawIdList 
   * @returns eventIds
   */
  public async queryEventId(rawIdList: number[]): Promise<number[]> {
    LogUtils.i(TAG, `queryEventId eventInfo ->${rawIdList}`);
    const eventIds: number[] = [];
    if (!rawIdList || rawIdList.length <= 0) {
      return eventIds;
    }
    let resultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        throw new Error(`initDb fail`);
      }
      const predicate: relationalStore.RdbPredicates = new relationalStore.RdbPredicates('view_contact_data');
      predicate.in(DataColumns.RAW_CONTACT_ID, rawIdList);
      predicate.and();
      predicate.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
      predicate.and();
      predicate.in(DataColumns.CUSTOM_DATA, BIRTHDAY_TYPE); //只有生日插入日历数据库
      const columns = [DataColumns.CALENDAR_EVENT_ID];
      resultSet = await rdbStore.query(predicate, columns);
      if (!resultSet || resultSet.rowCount <= 0) {
        return eventIds;
      }
      let calendarEventId: String = '';
      if (resultSet.goToFirstRow()) {
        do {
          calendarEventId = resultSet.getString(resultSet.getColumnIndex(DataColumns.CALENDAR_EVENT_ID));
          LogUtils.w(TAG, `buildEvents calendarEventId ->${calendarEventId}`);
          if (calendarEventId) {
            eventIds.push(Number(calendarEventId));
          }
        } while (resultSet.goToNextRow());
      }
    } catch (error) {
      LogUtils.e(TAG, `Failed to queryEventId. Code: ${error.code}, message: ${error.message}`);
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
    return eventIds;
  }
 
  /**
   * 查询view_contact_data视图中所有未同步到日历中的生日信息,即
   * IS_SYNC_BIRTHDAY_TO_CALENDAR = 0的
   * @returns 
   */
  private async handleInsertAndUpdateBirthday() {
    let resultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      LogUtils.i(TAG, `handleInsertAndUpdateBirthday start.`);
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        LogUtils.e(TAG, `handleInsertAndUpdateBirthday initDB failed.`);
        return;
      }
      const predicate: relationalStore.RdbPredicates = new relationalStore.RdbPredicates('view_contact_data');
      predicate.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
      predicate.and();
      predicate.in(DataColumns.CUSTOM_DATA, BIRTHDAY_TYPE); //只有生日插入日历数据库
      predicate.and();
 
      //只有IS_SYNC_BIRTHDAY_TO_CALENDAR = 0的都需要同步
      predicate.equalTo(DataColumns.IS_SYNC_BIRTHDAY_TO_CALENDAR, BIRTHDAY_NOT_SYNC_DIRTY);
      predicate.and();
      predicate.equalTo(RawContactsColumns.IS_DELETED, NORMAIL_CONTACT);
      resultSet = await rdbStore.query(predicate, columns);
      LogUtils.i(TAG, `handleInsertAndUpdateBirthday resultSet.rowCount ->${resultSet?.rowCount}`);
      if (!resultSet || resultSet.rowCount <= 0) {
        return;
      }
      await this.syncEventToCalendar(resultSet, true);
    } catch (error) {
      LogUtils.e(TAG, `Failed to handleInsertAndUpdateBirthday. Code: ${error.code}, message: ${error.message}`);
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
  }
 
  /**
   * 查询删除失败的生日，触发再次删除
   */
  private async handleDeletedBirthday() {
    let resultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      LogUtils.i(TAG, `handleDeletedBirthday start.`);
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        LogUtils.e(TAG, `handleDeletedBirthday initDB failed.`);
        return;
      }
      const predicate: relationalStore.RdbPredicates = new relationalStore.RdbPredicates('view_contact_data');
      predicate.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
      predicate.and();
      predicate.in(DataColumns.CUSTOM_DATA, BIRTHDAY_TYPE); //只有生日插入日历数据库
      predicate.and();
 
      //只有IS_SYNC_BIRTHDAY_TO_CALENDAR = 0的都需要同步
      predicate.equalTo(DataColumns.IS_SYNC_BIRTHDAY_TO_CALENDAR, BIRTHDAY_NOT_SYNC_FAILED);
      resultSet = await rdbStore.query(predicate, columns);
      LogUtils.i(TAG, `handleDeletedBirthday resultSet.rowCount ->${resultSet?.rowCount}`);
      if (!resultSet || resultSet.rowCount <= 0) {
        return;
      }
      let calendarEventId: string = '';
      let eventIds: number[] = [];
      if (resultSet.goToFirstRow()) {
        do {
          calendarEventId = resultSet.getString(resultSet.getColumnIndex(DataColumns.CALENDAR_EVENT_ID));
          LogUtils.i(TAG, `handleDeletedBirthday calendarEventId ->${calendarEventId}`);
          eventIds.push(Number(calendarEventId));
        } while (resultSet.goToNextRow());
      }
      if (eventIds.length > 0) {
        await this.deleteBirthdayEvent(eventIds);
      }
    } catch (error) {
      LogUtils.e(TAG, `Failed to handleDeletedBirthday. Code: ${error.code}, message: ${error.message}`);
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
  }
 
  /**
   * 进程启动时，处理没有同步到日历的生日，包括需要更新、插入、删除的
   */
  public async syncAllBirthdayToCalendar() {
    LogUtils.i(TAG, `syncAllBirthdayToCalendar start.`);
    let isCreateCalendar: boolean = await this.createCalendar();
    if (isCreateCalendar) {
      await this.handleDeletedBirthday();
      await this.handleInsertAndUpdateBirthday();
    }
  }

  /**
   * 刷新生日提醒 只执行一次,后面要删掉
   */
  public async refreshBirthReminder() {
    let resultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      LogUtils.i(TAG, `refreshBirthReminder start.`);
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        LogUtils.e(TAG, `refreshBirthReminder initDB failed.`);
        return;
      }

      // 刷新生日提醒，所有正常联系人生日都需要刷新。
      const predicate: relationalStore.RdbPredicates = new relationalStore.RdbPredicates('view_contact_data');
      predicate.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
      predicate.and();
      predicate.in(DataColumns.CUSTOM_DATA, BIRTHDAY_TYPE); //只有生日插入日历数据库
      predicate.and();
      predicate.equalTo(RawContactsColumns.IS_DELETED, NORMAIL_CONTACT);
      resultSet = await rdbStore.query(predicate, columns);
      LogUtils.i(TAG, `refreshBirthReminder resultSet.rowCount ->${resultSet?.rowCount}`);
      if (!resultSet || resultSet.rowCount <= 0) {
        return;
      }
      await this.syncEventToCalendar(resultSet, true);
    } catch (error) {
      LogUtils.e(TAG, `Failed to refreshBirthReminder. Code: ${error.code}, message: ${error.message}`);
    } finally {
      if (resultSet && !resultSet.isClosed) {
        resultSet.close();
      }
    }
  }

  /**
   * 更新 settings表中 refresh_contacts 值为13
   */
  private async updateSettingsRefreshContacts() {
    try {
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        throw new Error(`initDb fail`);
      }
      const predicateSettings: relationalStore.RdbPredicates = new relationalStore.RdbPredicates('settings');
      predicateSettings.equalTo(SettingsColumns.ID, 1);
      const updateBucket: ValuesBucket = {};
      updateBucket[SettingsColumns.REFRESH_CONTACTS] = 13;
      let rdbRet = await contactRdbHelper.updateData(updateBucket, predicateSettings);
      LogUtils.w(TAG, `updateSettingsRefreshContacts  update rdbRet: ${rdbRet}`);
    } catch (err) {
      LogUtils.e(TAG, `Failed to updateSettingsRefreshContacts Code: ${err.code}, message: ${err.message}`);
    }
  }
 
  /**
   * 根据rawContactId从view_contact_data视图中查询相关信息构建event对象
   * @param rawContactId
   * @param isNeedUpdate 通过联系人新建没有update操作，云同步等其他场景有更新的场景
   */
  public async handleSingleBirthday(rawContactId: string, isNeedUpdate: boolean) {
    let detailInfoResultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      LogUtils.i(TAG, `handleSingleBirthday rawContactId ->${rawContactId}`);
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        throw new Error(`initDb fail`);
      }
      const predicate: relationalStore.RdbPredicates = new relationalStore.RdbPredicates('view_contact_data');
      predicate.equalTo(DataColumns.RAW_CONTACT_ID, rawContactId);
      predicate.and();
      predicate.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
      predicate.and();
      predicate.in(DataColumns.CUSTOM_DATA, BIRTHDAY_TYPE); //只有生日插入日历数据库
      detailInfoResultSet = await rdbStore.query(predicate, columns);
      LogUtils.i(TAG, `handleSingleBirthday detailInfoResultSet.rowCount ->${detailInfoResultSet?.rowCount}`);
      await this.syncEventToCalendar(detailInfoResultSet, isNeedUpdate);
    } catch (error) {
      LogUtils.e(TAG, `Failed to handleSingleBirthday. Code: ${error.code}, message: ${error.message}`);
    } finally {
      if (detailInfoResultSet && !detailInfoResultSet.isClosed) {
        detailInfoResultSet.close();
      }
    }
  }
 
  /**
   * 根据rawContactId集合从view_contact_data视图中查询相关信息构建event对象
   * @param rawContactIds
   * @param isNeedUpdate 通过联系人新建没有update操作，云同步等其他场景有更新的场景
   */
  public async handleBatchBirthday(rawContactIds: number[], isNeedUpdate: boolean) {
    let detailInfoResultSet: relationalStore.ResultSet | null | undefined = null;
    try {
      LogUtils.i(TAG, `handleBatchBirthday rawContactIds ->${rawContactIds}`);
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        throw new Error(`initDb fail`);
      }
      const predicate: relationalStore.RdbPredicates = new relationalStore.RdbPredicates("view_contact_data");
      predicate.in(DataColumns.RAW_CONTACT_ID, rawContactIds);
      predicate.and();
      predicate.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
      predicate.and();
      predicate.in(DataColumns.CUSTOM_DATA, BIRTHDAY_TYPE); //只有生日插入日历数据库
      detailInfoResultSet = await rdbStore.query(predicate, columns);
      LogUtils.i(TAG, `handleBatchBirthday detailInfoResultSet.rowCount ->${detailInfoResultSet?.rowCount}`);
      await this.syncEventToCalendar(detailInfoResultSet, isNeedUpdate);
    } catch (error) {
      LogUtils.e(TAG, `Failed to handleBatchBirthday. Code: ${error.code}, message: ${error.message}`);
    } finally {
      if (detailInfoResultSet && !detailInfoResultSet.isClosed) {
        detailInfoResultSet.close();
      }
    }
  }

  /**
   * Process non-standard format dates.
   * @param date
   * @returns the result.
   */
  private async handleDate(date: string): Promise<string> {
    if (!date || !date.includes('-')) {
      return date;
    }
    const currentYear = new Date().getFullYear();
    const dateArray = date.split('-');
    switch (dateArray.length) {
      case 3:
        if (dateArray[0].length < 4) {
          const index = date.indexOf('-');
          const suffix = date.substring(index);
          return `${currentYear}${suffix}`;
        }
        return date;
      case 2:
        return `${currentYear}-${date}`;
      default:
        LogUtils.e(TAG, `syncEventToCalendar dateArray length is Invalid.`);
        return date;
    }
  }

  /**
   * 从result中查询信息，构建event同步到日历
   * @param resultSet 
   * @returns 
   */
  private async syncEventToCalendar(resultSet: relationalStore.ResultSet, isNeedUpdate: boolean): Promise<boolean> {
    if (!resultSet || resultSet.rowCount <= 0) {
      return true;
    }
    let result: boolean = true;
    let resultAddOrUpdate: boolean = true;
    let detailInfo: string = '';
    let displayName: string = '';
    let rawContactId: number = 0;
    let contactId: number = 0;
    let customData: string = '';
    let calendarEventId: string = '';
    const recurrenceRule: calendarManager.RecurrenceRule = {
      recurrenceFrequency: calendarManager?.RecurrenceFrequency.YEARLY
    }
    if (resultSet.goToFirstRow()) {
      do {
        detailInfo = resultSet.getString(resultSet.getColumnIndex(DataColumns.DETAIL_INFO));
        displayName = resultSet.getString(resultSet.getColumnIndex(RawContactsColumns.DISPLAY_NAME));
        contactId = resultSet.getLong(resultSet.getColumnIndex(RawContactsColumns.CONTACT_ID));
        rawContactId = resultSet.getLong(resultSet.getColumnIndex(DataColumns.RAW_CONTACT_ID));
        customData = resultSet.getString(resultSet.getColumnIndex(DataColumns.CUSTOM_DATA));
        calendarEventId = resultSet.getString(resultSet.getColumnIndex(DataColumns.CALENDAR_EVENT_ID));
        detailInfo = await this.handleDate(detailInfo);
        detailInfo = this.processDate(detailInfo);
        LogUtils.i(TAG, `syncEventToCalendar detailInfo ->${detailInfo}, calendarEventId ->${calendarEventId}`);
        const date = new Date(detailInfo + ' ' + '00:00:00');
        const isDateInvalid = Number.isNaN(date.valueOf());
        if (isDateInvalid) {
          LogUtils.w(TAG, `syncEventToCalendar date is Invalid.`);
          // 更新is_sync_calendar_to_calendar 到 data表
          await this.updateContactDataByRawConId('syncEventToCalendar', rawContactId, BIRTHDAY_NOT_SYNC_INVALID, 0,
            customData);
          return true;
        }
        const startTimeStamp = date.getTime() - date.getTimezoneOffset() * 60000;
        const endTimeStamp = startTimeStamp + 24 * 60 * 60 * 1000;
        let isLunar: boolean = false;
        let identifierValue: string = contactId.toString();
        LogUtils.w(TAG, `syncEventToCalendar customData ->${customData}`);
        if (customData === BIRTHDAY_TYPE_LUNAR || customData === BIRTHDAY_TYPE_LUNAR_POINT) {
          isLunar = true;
          identifierValue = identifierValue + '_' + 'lunar' + date.valueOf();
        } else {
          identifierValue = identifierValue + '_' + 'solar' + date.valueOf();
        }
        LogUtils.w(TAG, `syncEventToCalendar identifierValue ->${identifierValue}`);
        const event: calendarManager.Event = {
          id: Number(calendarEventId),
          title: displayName,
          startTime: startTimeStamp,
          endTime: endTimeStamp,
          isAllDay: true,
          identifier: identifierValue,
          reminderTime: [0, 10080],
          type: calendarManager.EventType.NORMAL,
          recurrenceRule: recurrenceRule,
          timeZone: 'UTC',
          isLunar: isLunar,
        };
 
        //需要更新数据库的场景如云同步，isNeedUpdate为true，新建联系人为先删除，再插入，不执行更新操作isNeedUpdate为false
        if (isNeedUpdate && calendarEventId) {
          resultAddOrUpdate = await this.updateBirthdayEvent(event, Number(rawContactId), customData);
        } else {
          resultAddOrUpdate = await this.addBirthdayEvent(event, Number(rawContactId), customData);
        }
        if (!resultAddOrUpdate) {
          result = resultAddOrUpdate;
        }
      } while (resultSet.goToNextRow());
    }
    LogUtils.w(TAG, `syncEventToCalendar result ->${result}`);
    return result;
  }

   /**
   * 1900年 shanghai时区从GMT+0805调整到了GMT+0800，时间有5分钟偏差
   * 和日历定的规格是没有年份或者小于1904年，全部按照1904年处理
   * @param dateStr
   * @returns
   */
  private processDate(dateStr: string): string {
    // 匹配无年份格式
    const noYearPattern = /^\d{1,2}-\d{1,2}$/;
    // 匹配有年份格式
    const withYearPattern = /^\d{4}-\d{1,2}-\d{1,2}$/;
    LogUtils.w(TAG, `birthday date is ${dateStr}.`);
    if (noYearPattern.test(dateStr)) {
        // 如果没有年份，添加1904年
        const parts = dateStr.split('-');
        const month = parts[0].padStart(2, '0');
        const day = parts[1].padStart(2, '0');
        return `1904-${month}-${day}`;
    } else if (withYearPattern.test(dateStr)) {
        const parts = dateStr.split('-');
        const year = parseInt(parts[0]);
        const month = parts[1].padStart(2, '0');
        const day = parts[2].padStart(2, '0');

        if (year < 1904) {
            LogUtils.w(TAG, `birthday date is less than 1904.`);
            return `1904-${month}-${day}`;
        } else {
            return `${year}-${month}-${day}`;
        }
    } else {
        // 如果格式不符合预期，返回原始字符串
        return dateStr;
    }
  }

  /**
   * 添加生日到日历数据库
   */
  private async addBirthdayEvent(event: calendarManager.Event, rawContactId: number,
      customData: string):
    Promise<boolean> {
    LogUtils.i(TAG, `addBirthdayEvent start`);
    let checkCalendarObjectResult = await this.checkCalendarObject();
    if (!checkCalendarObjectResult) {
      return false;
    }
    try {
      let result: number = -1;
      result = await this.calendar?.addEvent(event) as number;
      LogUtils.w(TAG, `addBirthdayEvent result ->${result}`);

      if (result > -1) {
        //TODO 更新calendar_event_id 到 data表
        await this.updateContactDataByRawConId('addBirthdayEvent', rawContactId, BIRTHDAY_NOT_SYNC_SUCCEED, result,
          customData);
        return true;
      }
    } catch (err) {
      LogUtils.e(TAG, `addBirthdayEvent Failed to addEvent. Code: ${err.code}, message: ${err.message}`);
      return false;
    }
    return false;
  }
 
  /**
   * 更新生日到日历数据库
   */
  private async updateBirthdayEvent(event: calendarManager.Event, rawContactId: number, customData: string):
    Promise<boolean> {
    let checkCalendarObjectResult = await this.checkCalendarObject();
    if (!checkCalendarObjectResult) {
      return false;
    }
    try {
      await this.calendar?.updateEvent(event);
      LogUtils.i(TAG, `Succeeded in updating event`);
      await this.updateContactDataByRawConId('updateBirthdayEvent', rawContactId, BIRTHDAY_NOT_SYNC_SUCCEED, -1,
        customData);
      return true;
    } catch (err) {
      await this.updateContactDataByRawConId('updateBirthdayEvent', rawContactId, BIRTHDAY_NOT_SYNC_DIRTY, -1,
        customData);
      LogUtils.e(TAG, `Failed to update event. Code: ${err.code}, message: ${err.message}`);
      return false;
    }
  }
 
  /**
   * 通过rawConId更新contact_data表中is_sync_calendar_to_calendar标志
   * @param methodName the method name
   * @param rawContactId the raw_contact_id
   * @param isSyncBirthdayToCalendar the is_sync_birthday_to_calendar
   */
  private async updateContactDataByRawConId(methodName: string, rawContactId: number,
    isSyncBirthdayToCalendar: number, calendarEventId: number, customData: string) {
    try {
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        throw new Error(`initDb fail`);
      }
      const updateBucket: ValuesBucket = {};
      if (calendarEventId > 0) {
        updateBucket[DataColumns.CALENDAR_EVENT_ID] = calendarEventId;
      }
      updateBucket[DataColumns.IS_SYNC_BIRTHDAY_TO_CALENDAR] = isSyncBirthdayToCalendar;
      const predicates: relationalStore.RdbPredicates =
        new relationalStore.RdbPredicates(DbConstants.CLONE_NEXT2NEXT_DATA_TABLE_NAME);
      predicates.equalTo(DataColumns.RAW_CONTACT_ID, rawContactId);
      predicates.and();
      predicates.equalTo(DataColumns.CUSTOM_DATA, customData);
      predicates.and();
      predicates.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
      let rdbRet = await contactRdbHelper.updateData(updateBucket, predicates);
      LogUtils.w(TAG, `${methodName} update table contact_data by rawConId, rdbRet: ${rdbRet}`);
    } catch (err) {
      LogUtils.e(TAG, `Failed to update table contact_data by rawConId in ${methodName}. Code: ${err.code}, message:
        ${err.message}`);
    }
  }
 
  /**
   *  更新contact_data表中is_sync_calendar_to_calendar标志
   * @param methodName the method name
   * @param eventIds the calendar_event_id
   * @param isSyncBirthdayToCalendar the is_sync_birthday_to_calendar
   */
  private async updateContactDataByEventId(methodName: string, eventIds: number[], isSyncBirthdayToCalendar: number) {
    try {
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        throw new Error(`initDb fail`);
      }
      const updateBucket: ValuesBucket = {};
      updateBucket[DataColumns.IS_SYNC_BIRTHDAY_TO_CALENDAR] = isSyncBirthdayToCalendar;
      if (isSyncBirthdayToCalendar === BIRTHDAY_NOT_SYNC_DIRTY) {
        updateBucket[DataColumns.CALENDAR_EVENT_ID] = '';
      }
      const predicates: relationalStore.RdbPredicates =
        new relationalStore.RdbPredicates(DbConstants.CLONE_NEXT2NEXT_DATA_TABLE_NAME);
      predicates.in(DataColumns.CALENDAR_EVENT_ID, eventIds);
      let rdbRet = await contactRdbHelper.updateData(updateBucket, predicates);
      LogUtils.w(TAG, `${methodName} update table contact_data by rawconId, rdbRet: ${rdbRet}`);
    } catch (err) {
      LogUtils.e(TAG, `Failed to update table contact_data by rawconId in ${methodName}. Code: ${err.code}, message:
        ${err.message}`);
    }
  }
 
  /**
   * 从日历数据库删除生日
   */
  public async deleteBirthdayEvent(eventIds: number[]) {
    LogUtils.i(TAG, `deleteBirthdayEvent start.`);
    let checkCalendarObjectResult = await this.checkCalendarObject();
    if (!checkCalendarObjectResult || eventIds.length <= 0) {
      return;
    }
    try {
      await this.calendar?.deleteEvents(eventIds);
      LogUtils.w(TAG, `Succeeded in deleting events`);
      await this.updateContactDataByEventId('deleteBirthdayEvent', eventIds, BIRTHDAY_NOT_SYNC_DIRTY);
    } catch (err) {
      await this.updateContactDataByEventId('deleteBirthdayEvent', eventIds, BIRTHDAY_NOT_SYNC_FAILED);
      LogUtils.e(TAG, `Failed to delete events. Code: ${err.code}, message: ${err.message}`);
    }
  }
 
  private calendarSetConfig(calendar: calendarManager.Calendar) {
    const config: calendarManager.CalendarConfig = {
      enableReminder: true,
      color: '#FFE64566'
    };
    calendar.setConfig(config).then(() => {
      LogUtils.i(TAG, `Succeeded in setting config, data->{enableReminder: ${config?.enableReminder}, color: ${config?.color}`);
    }).catch((err: BusinessError) => {
      LogUtils.e(TAG, `Failed to set config. Code: ${err.code}, message: ${err.message}`);
    });
  }

  /**
   * 标记生日为要删除状态，即is_sync_birthday_to_calendar字段值为2
   * @param contactDataIds contact_data中id列表
   */
  public async markedBirthdayToDelFlag(contactDataIds: number[]) {
    LogUtils.w(TAG, `markedBirthdayToDelFlag contactDataIds ->${contactDataIds}`);
    if (!contactDataIds || contactDataIds.length <= 0) {
      return;
    }
    try {
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        throw new Error(`initDb fail`);
      }
      const predicate: relationalStore.RdbPredicates =
        new relationalStore.RdbPredicates(DbConstants.CLONE_NEXT2NEXT_DATA_TABLE_NAME);
      predicate.in(DataColumns.ID, contactDataIds);
      const updateBucket: ValuesBucket = {};
      updateBucket[DataColumns.IS_SYNC_BIRTHDAY_TO_CALENDAR] = BIRTHDAY_NOT_SYNC_FAILED;
      let rdbRet = await contactRdbHelper.updateData(updateBucket, predicate);
      LogUtils.w(TAG, `markedBirthdayToDelFlag update table contact_data by contactDataIds, rdbRet: ${rdbRet}`);
    } catch (error) {
      LogUtils.e(TAG, `Failed to markedBirthdayToDelFlag. Code: ${error.code}, message: ${error.message}`);
    }
  }

  /**
   * 重置联系人数据库日程
   */
  public async resetAllCalendar(): Promise<boolean> {
    try {
      let checkCalendarObjectResult = await this.checkCalendarObject();
      if (!checkCalendarObjectResult) {
        return false;
      }

      // 删除账户下生日
      await this.calendarManager?.deleteCalendar(this.calendar);
      this.calendar = undefined;
      this.calendarManager = null;
      LogUtils.w(TAG, `deleteCalendar success`);

      // 将 contact_data 表的同步字段置为未同步
      const rdbStore = await contactRdbHelper.initDB(this.context!);
      if (!rdbStore) {
        LogUtils.w(TAG, 'resetAllCalendar rdb init fail!');
        return false;
      }
      // 更新信息
      const updateBucket: ValuesBucket = {};
      updateBucket[DataColumns.IS_SYNC_BIRTHDAY_TO_CALENDAR] = BIRTHDAY_NOT_SYNC_DIRTY;
      updateBucket[DataColumns.CALENDAR_EVENT_ID] = null;

      // 更新的条件
      const predicate: relationalStore.RdbPredicates =
        new relationalStore.RdbPredicates('contact_data');
      predicate.equalTo(DataColumns.TYPE_ID, EVENT_TYPE_ID);
      predicate.and();
      predicate.in(DataColumns.CUSTOM_DATA, BIRTHDAY_TYPE); //只有生日插入日历数据库，需要更新

      let rdbRet = await contactRdbHelper.updateData(updateBucket, predicate);
      LogUtils.w(TAG, `update contact_data BIRTHDAY_NOT_SYNC_DIRTY: ${rdbRet}`);
      if (rdbRet === -1) {
        LogUtils.e(TAG, `update contact_data BIRTHDAY_NOT_SYNC_DIRTY ret error: ${rdbRet}`);
        return false;
      }
    } catch (e) {
      LogUtils.e(TAG, 'resetAllCalendar delete old error: ' + e.code + '--' + e.message);
      return false;
    }

    try {
      // 重新同步账户下生日
      await this.syncAllBirthdayToCalendar();
      LogUtils.w(TAG, `syncAllBirthdayToCalendar: success`);
    } catch (e) {
      LogUtils.e(TAG, 'resetAllCalendar sync new error: ' + e.code + '--' + e.message);
      return false;
    }

    return true;
  }
   
  private async getEventString (event: calendarManager.Event, operation: string,
    customData: string): Promise<string> {
 
    const maskedTitle = StringUtil.maskSensitiveInfo(event?.title);
    const result = `${operation} {` +
    `displayName: ${maskedTitle},` +
    `identifier: ${event?.identifier},` +
    `isLunar: ${event?.isLunar},` +
    `customData: ${customData}}`;
    return result;
  }
}