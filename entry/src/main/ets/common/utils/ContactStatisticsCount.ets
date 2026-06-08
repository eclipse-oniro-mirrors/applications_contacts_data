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
/**
 * 空间类型
 */
export enum SpaceType {
  MAIN = 100,
  PRIVACY = 101,
}

export class ContactStatisticsCountReport {
  // 应用包名
  public PNAMEID = 'com.ohos.contactsdataability';
  // 应用版本
  public PVERSIONID = '1000061';

  // 云表正常联系人数量
  public CLOUD_NORMAL_NUM = 0;
  // 云表回收站联系人数量
  public CLOUD_DELETE_NUM = 0;
  // 回收站字段为null数量
  public NULL_RECYCLED_NUM = 0;
  // 云表存在，本地表不存在的uuid数量
  public ONLY_IN_CLOUD_NUM = 0;
  // raw表contactId为空的数量
  public NULL_CONTACTID_NUM = 0;
  // raw表uuid为空的数量
  public NULL_UUID_NUM = 0;
  // 列表联系人数量
  public NORMAL_CONTACT_NUM = 0;
  // 回收站联系人数量
  public DELETED_CONTACT_NUM = 0;
  // 列表真实联系人数量
  public NORMAL_CONTACT_NUM_REAL = 0;
  // 异常的raw数量
  public BAD_RAW_CONTACT_NUM = 0;
  // 异常的contact数量
  public BAD_CONTACT_NUM = 0;
  // 关联异常的contact数量,即raw表中id与关联的contactId在contact表中关联的rawId不一致
  public INVALID_CONTACT_NUM = 0;
  // 三方联系人的数量
  public THIRD_CONTACT_NUM = 0;
  // 主空间还是隐私空间
  public USERID = SpaceType.MAIN;
  // 数据捐赠开关 开关打开1 关闭0
  public SEARCH_SWITCH = 1;
}

/**
 * 双升单/克隆失败原因上报
 */
export class ContactCountFailReport extends ContactStatisticsCountReport {
  public FAULT_INFO = '';
}

/**
 * 用户创建事件
 */
export class ContactUserCreateStatisticsCountReport extends ContactStatisticsCountReport {
  public CALLER_NAME = '';
}

/**
 * 用户创建事件
 */
export class ContactUserBatchCreateStatisticsCountReport extends ContactStatisticsCountReport {
  public CALLER_NAME = '';
  public INSERT_NUM = 0;
}

/**
 * 用户删除事件
 */
export class ContactUserDelStatisticsCountReport extends ContactStatisticsCountReport {
  public CALLER_NAME = '';
  public DELETE_NUM: number = 0;
}

/**
 * 用户删除回收站事件
 */
export class ContactUserDelRecentStatisticsCountReport extends ContactStatisticsCountReport {
  // 到期自动清理
  public static AUTO_TYPE = 0;
  // 手动清理
  public static MANUAL_TYPE = 1;
  public CALLER_NAME = '';
  public DELETE_NUM: number = 0;
  public TYPE: number = 0;
}

/**
 * 联系人恢复数量统计：双升单、双到单、单到单克隆、云备份恢复
 */
export class ContactRestoreCountReport extends ContactStatisticsCountReport {
  public static DOUBLE_TO_SINGLE_CLONE_TYPE = 0;
  // 云备份和单单克隆无法区分类型，默认都是用单单克隆标记
  public static SINGLE_TO_SINGLE_CLONE_TYPE = 1;
  public static CLOUD_BACKUP_RESTORE_TYPE = 2;

  public TYPE: number = 0;
  public CLONE_SUC_NUM: number = 0;
  public CLONE_FAIL_NUM: number = 0;

  constructor(type: number) {
    super();
    this.TYPE = type;
  }
}

/**
 * 云下行新增，联系人数量统计信息（新增回收站+新增列表）
 */
export class ContactCloudDownInsertCountStatics {
  // 回收站新增
  public restoreInsert = 0;
  // 回收站删除，硬删
  public restoreDel = 0;

  // 列表新增
  public contactInsert = 0;
  // 列表删除进回收站
  public contactSoftDel = 0;
  // 列表硬删
  public contactHardDel = 0;

  // 插入操作失败，不区分回收站还是列表
  public failInsert = 0;

  public updateUuidToList = 0;
  public updateUuidToRecycle = 0;
}

/**
 * 云下行上报信息
 */
export class ContactCloudDownloadCountInfoReport {
  // 更新处理成功，普通更新，仅更新内容
  public static CONTENT_UPDATE_TYPE = 0;

  // 云下行更新进入回收站+回收站数据云下行新增进入回收站+非回收站数据下行新增，本地去重软删除数据
  public static UPDATE_TO_RECYCLE = 1;

  // 更新：从回收站恢复
  public static UPDATE_TO_RESTORE = 2;

  // 更新至回收站，已经从回收站恢复
  public static UPDATE_TO_RECYCLE_ALREADY_IN_RECYCLE = 3;

  // 更新失败
  public static UPDATE_FAIL = 4;

  // 下行硬删除：云下行硬删 + 非回收站数据云下行新增去重硬删 + 回收站数据云下行去重删除
  public static HARD_DEL_TYPE = 5;
  // 回收站新增失败+列表新增失败
  public static INSERT_FAIL_TYPE = 6;
  // 新增成功：列表新增(不包括回收站)
  public static INSERT_SUCCESS = 7;
  // 退账号删除数量，cursor处理时，可能处理到退账号删除数据，与云下行产生的硬删区分
  public static LOGOUT_DEL_TYPE = 8;
  // 云下行数据有uuid，本地存在相同无uuid的数据，将uuid更新至本地（关闭云同步开关，本地云信息置空，再打开，会是这种）
  public static UPDATE_UUID_TO_LIST_TYPE = 9;
  public static UPDATE_UUID_TO_RECYCLE_TYPE = 10;

  public TYPE: number = 0;
  public COUNT: number = 0;

  constructor(type: number, count: number) {
    this.TYPE = type;
    this.COUNT = count;
  }

}

/**
 * 云同步下行联系人数量
 */
export class ContactCloudDownloadCountInfo {

  // 云下行更新
  // 更新处理成功，普通更新，仅更新内容
  public countUpdateSuccess: number = 0;
  // 云下行更新进入回收站+回收站数据云下行新增进入回收站+非回收站数据下行新增，本地去重软删除数据
  public countUpdateToRecycle: number = 0;
  // 云下行更新，从回收站恢复
  public countUpdateToRestore: number = 0;
  public countUpdateToRecycleAlreadyInRecycle: number = 0;
  // 更新处理失败
  public countUpdateFail: number = 0;

  // 云下行硬删 + 非回收站数据云下行新增去重硬删 + 回收站数据云下行去重删除
  public countHardDel: number = 0;
  // 回收站新增失败+列表新增失败
  public countInsertFail: number = 0;
  // 新增成功：列表新增(不包括回收站)
  public countInsertSuccess: number = 0;
  // 退账号删除数量，cursor处理时，可能处理到退账号删除数据，与云下行产生的硬删区分
  public countLogoutDel: number = 0;

  // 云下行，存在相同本地数据，且本地数据无uuid，将云下行uuid设置到本地数据数量
  public countUpdateUuidToList = 0;
  public countUpdateUuidToRecycle = 0;

  public resetCount() {
    this.countUpdateFail = 0;
    this.countUpdateSuccess = 0;
    this.countUpdateToRecycle = 0;
    this.countUpdateToRestore = 0;
    this.countUpdateToRecycleAlreadyInRecycle = 0;

    this.countHardDel = 0;
    this.countInsertFail = 0;
    this.countInsertSuccess = 0;
    this.countLogoutDel = 0;

    this.countUpdateUuidToList = 0;
    this.countUpdateUuidToRecycle = 0;
  }

  /**
   * 生成上报数量信息
   * @returns
   */
  public generateContactCloudDownloadReportCountInfoList(): ContactCloudDownloadCountInfoReport[] {
    let reportList: ContactCloudDownloadCountInfoReport[] = [];

    if (this.countUpdateSuccess > 0) {
      let countUpdateSuccess: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.CONTENT_UPDATE_TYPE, this.countUpdateSuccess);
      reportList.push(countUpdateSuccess);
    }

    if (this.countUpdateToRecycle > 0) {
      let countUpdateToRecycle: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.UPDATE_TO_RECYCLE, this.countUpdateToRecycle);
      reportList.push(countUpdateToRecycle);
    }

    if (this.countUpdateToRestore > 0) {
      let countUpdateToRestore: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.UPDATE_TO_RESTORE, this.countUpdateToRestore);
      reportList.push(countUpdateToRestore);
    }

    if (this.countUpdateToRecycleAlreadyInRecycle > 0) {
      let countUpdateToRecycleAlreadyInRecycle: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.UPDATE_TO_RECYCLE_ALREADY_IN_RECYCLE, this.countUpdateToRecycleAlreadyInRecycle);
      reportList.push(countUpdateToRecycleAlreadyInRecycle);
    }

    if (this.countUpdateFail > 0) {
      let countUpdateFail: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.UPDATE_FAIL, this.countUpdateFail);
      reportList.push(countUpdateFail);
    }

    if (this.countHardDel > 0) {
      let countHardDel: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.HARD_DEL_TYPE, this.countHardDel);
      reportList.push(countHardDel);
    }

    if (this.countInsertFail > 0) {
      let countInsertFail: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.INSERT_FAIL_TYPE, this.countInsertFail);
      reportList.push(countInsertFail);
    }

    if (this.countInsertSuccess > 0) {
      let countInsertSuccess: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.INSERT_SUCCESS, this.countInsertSuccess);
      reportList.push(countInsertSuccess);
    }

    if (this.countLogoutDel > 0) {
      let countLogoutDel: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.LOGOUT_DEL_TYPE, this.countLogoutDel);
      reportList.push(countLogoutDel);
    }

    if (this.countUpdateUuidToList > 0) {
      let updateUuidToList: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.UPDATE_UUID_TO_LIST_TYPE, this.countUpdateUuidToList);
      reportList.push(updateUuidToList);
    }

    if (this.countUpdateUuidToRecycle > 0) {
      let updateUuidToRecycle: ContactCloudDownloadCountInfoReport = new ContactCloudDownloadCountInfoReport(
        ContactCloudDownloadCountInfoReport.UPDATE_UUID_TO_RECYCLE_TYPE, this.countUpdateUuidToRecycle);
      reportList.push(updateUuidToRecycle);
    }

    return reportList;
  }

}

/**
 * 云同步上行数量
 */
export class ContactCloudUploadCount {
  //
  insertCount: number = 0;
  //
  updateCount: number = 0;
  // 删除至回收站
  deleteToRestoreCount: number = 0;

  /**
   * 重置数据
   */
  public resetCount() {
    this.insertCount = 0;
    this.updateCount = 0;
    this.deleteToRestoreCount = 0;
  }

  public generateUploadReportList() {
    let reportList: ContactCloudUploadCountInfoReport[] = [];
    if (this.insertCount > 0) {
      let insertCount: ContactCloudUploadCountInfoReport =
        new ContactCloudUploadCountInfoReport(ContactCloudUploadCountInfoReport.INSERT_TYPE, this.insertCount);
      reportList.push(insertCount);
    }

    if (this.updateCount > 0) {
      let updateCount: ContactCloudUploadCountInfoReport =
        new ContactCloudUploadCountInfoReport(ContactCloudUploadCountInfoReport.UPDATE_TYPE, this.updateCount);
      reportList.push(updateCount);
    }

    if (this.deleteToRestoreCount > 0) {
      let deleteToRestoreCount: ContactCloudUploadCountInfoReport =
        new ContactCloudUploadCountInfoReport(ContactCloudUploadCountInfoReport.UPDATE_TO_RECYCLE, this.deleteToRestoreCount);
      reportList.push(deleteToRestoreCount);
    }

    return reportList;
  }
}

/**
 * 云下行上报信息
 */
export class ContactCloudUploadCountInfoReport {
  // 新增
  public static INSERT_TYPE = 0;

  // 更新
  public static UPDATE_TYPE = 1;

  // 更新至回收站
  public static UPDATE_TO_RECYCLE = 2;

  public TYPE: number = 0;
  public COUNT: number = 0;

  constructor(type: number, count: number) {
    this.TYPE = type;
    this.COUNT = count;
  }

}

/**
 * 用同步开关状态变化上报
 */
export class CloudSyncSwitchStatusReport extends ContactStatisticsCountReport {
  public static OPEN_TYPE = 1;
  public static CLOSE_TYPE = 0;
  public static UNDEFINE_TYPE = 3;

  public SWITCH_STATUS = 0;
}

export class AutoMergeHandleCountReport extends ContactStatisticsCountReport {
  public static FIXED_TIME_TYPE = 0;
  public static ORGANIZE_CONTACTS_TYPE = 1;
  public static OPEN_SWITCH_TYPE = 2;

  public TYPE = 0;
  public HARD_DEL_NUM = 0;
  public SOFT_DEL_COUNT = 0;
}

export class ManualMergeHandleCountReport extends ContactStatisticsCountReport {
  public HARD_DEL_NUM = 0;
  public SOFT_DEL_NUM = 0;
}

/**
 * 恢复统计数量
 */
export class RestoreStaticsCountReport extends ContactStatisticsCountReport {
  RESTORE_NUM: number = 0;
  RESTORE_NUM_SUC: number = 0;
}

/**
 * 推账号删除联系人数量
 */
export class LogoutDelContactCountReport extends ContactStatisticsCountReport {
  HARD_DEL_NUM: number = 0;
}