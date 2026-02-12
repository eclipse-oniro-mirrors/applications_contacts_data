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
import { contextConstant } from '@kit.AbilityKit';
import LogUtils from '../../common/utils/LogUtils';
import fs from '@ohos.file.fs';
import { GroupFileInfo } from '../../common/data/GroupFileInfo';
import { ValuesBucket } from '@kit.ArkData';
import { BusinessError, systemDateTime } from '@kit.BasicServicesKit';
import { image } from '@kit.ImageKit';
import DataColumns from '../../common/data/DataColumns';
import rdb from '@ohos.data.relationalStore';
import taskPool from '@ohos.taskpool';
import { StringUtil } from '../../common/utils/StringUtil';
import DbConstants from '../DbConstants';
import { contactRdbHelper } from '../helperUtils/ContactsRdbHelperForCloudSync';

const TAG = 'GroupIdUtil';

const MIN_RESOLUTION = 409600;

enum BlobSource {
  DEFAULT = 0, // 历史数据，可能存在大头像，也可能是小头像，为了兼容先去查询大头像，如果不存在，则去取小头像
  CLONE_DEFAULT = 1, // 克隆、云同步处理头像原图时，会判断分辨率，大于640*640的会把blob_source置为3，否则置为1
  SMALL = 2, // 小头像，存储在blobdata的缩略图
  BIG = 3, // 大头像
}

export default class GroupIdUtil {
  // groupId用于联系人应用与联系人数据库应用之间数据共享，groupId为固定值，通过p7b申请，不可随意更改!
  public static groupId: string = 'group.1521646752317339712';
  private static instance: GroupIdUtil;
  private static groupDir: string;

  public static getInstance(): GroupIdUtil {
    if (!GroupIdUtil.instance) {
      GroupIdUtil.instance = new GroupIdUtil();
    }
    return GroupIdUtil.instance;
  }

  // 获取公共沙箱路径
  async getGroupDir(context: Context) {
    try {
      if (!GroupIdUtil.groupDir) {
        let ctx = context;
        ctx.area = contextConstant.AreaMode.EL2;
        GroupIdUtil.groupDir = await ctx.getGroupDir(GroupIdUtil.groupId);
      }
      return GroupIdUtil.groupDir;
    } catch (error) {
      LogUtils.e(TAG, 'getGroupId error: ' + error?.message + ', stack: ' + error?.stack);
      return GroupIdUtil.groupDir;
    }
  }

  // 双升单，临时存放旧头像文件
  async saveUpgradeFileDir(upgradeFileDir: string, context: Context) {
    const tagStr = TAG + ` saveUpgradeFileDir `;
    try {
      let groupDir = await this.getGroupDir(context);
      let resultFileDir = groupDir + '/oldPhoto/photos';
      let resultFileDirRes = await fs.access(resultFileDir);
      if (resultFileDirRes) {
        LogUtils.w(tagStr, `resultFileDir is exsit, delete it before`);
        await fs.rmdir(resultFileDir);
        LogUtils.w(tagStr, `resultFileDir delete success`);
      }
      let oldFileDir = groupDir + '/oldPhoto';
      let oldFileDirRes = await fs.access(oldFileDir);
      if (!oldFileDirRes) {
        // 创建旧头像文件夹
        await fs.mkdir(oldFileDir);
      }
      let upgradeFileDirRes = await fs.access(upgradeFileDir);
      if (!upgradeFileDirRes) {
        LogUtils.w(tagStr, `upgradeFileDir is not exsit`);
        return;
      }
      await fs.copyDir(upgradeFileDir, oldFileDir);
      LogUtils.w(tagStr, `copyDir success`);
    } catch (error) {
      LogUtils.e(tagStr, ' error: ' + error?.message + ', stack: ' + error?.stack);
    }
  }

  // 双升单 迁移联系人大头像文件并信息入库
  async upgradeBigPhotoFileInfo(groupFileInfoList: GroupFileInfo[], context: Context, isNext2next: boolean = false) {
    const tagStr = TAG + ` upgradeBigPhotoFileInfo `;
    try {
      const rdbStore = await contactRdbHelper.initDB(context);
      if (!rdbStore) {
        throw new Error(`GroupIdUtil initDb fail`);
      }
      let groupDir = await this.getGroupDir(context);
      // 新大头像所在文件夹
      let fileDir = groupDir + '/photo';
      let fileDirRes = await fs.access(fileDir);
      if (!fileDirRes) {
        await fs.mkdir(fileDir);
      }
      // 循环迁移大头像文件
      if (groupFileInfoList && groupFileInfoList.length) {
        LogUtils.w(tagStr, `groupFileInfoList.length: ${groupFileInfoList.length}`);
        for (const groupFileInfo of groupFileInfoList) {
          const contactId = groupFileInfo.contactId;
          const rawContactId = groupFileInfo.rawContactId;
          let oldFilePath = groupFileInfo.oldFilePath;
          const oldRawId = groupFileInfo.oldRawId;
          LogUtils.w(tagStr, `contactId: ${contactId}, rawContactId: ${rawContactId}, oldFilePath: ${
            StringUtil.maskSensitiveInfo(oldFilePath)}, oldRawId: ${oldRawId}`);
          let oldFilePathRes = await fs.access(oldFilePath);
          // 原大头像文件不存在，则不处理
          if (!oldFilePathRes) {
            LogUtils.w(tagStr, `oldFilePath is not exsit: ${StringUtil.maskSensitiveInfo(oldFilePath)}`);
            // 根据contactId拼接路径找不到头像文件，再拼接上rawId判断文件是否存在
            oldFilePath = oldFilePath + '_' + oldRawId;
            oldFilePathRes = await fs.access(oldFilePath);
            if (!oldFilePathRes) {
              LogUtils.w(tagStr, `oldFilePath is not exsit: ${StringUtil.maskSensitiveInfo(oldFilePath)}`);
            }
          }
          let newFilePath = fileDir + '/' + contactId + '_' + rawContactId;
          await this.deleteOldBigPhoto(newFilePath);
          const updateBucket: ValuesBucket = {};
          let blobSource: number = BlobSource.CLONE_DEFAULT;
          let blobDataPixel: PixelMap | null = null;
          if (oldFilePathRes) {
            blobSource = await this.checkFileScale(oldFilePath, 'upgradeBigPhotoFileInfo');
          } else {
            if (groupFileInfo.oldFileUint8Array !== null && groupFileInfo.oldFileUint8Array !== undefined) {
              let task: taskPool.Task =
                new taskPool.Task(getPixelMapFromFromUint8ArrayGroup, groupFileInfo.oldFileUint8Array);
              blobDataPixel = await taskPool.execute(task) as PixelMap | null;
            }
            if (blobDataPixel !== null && blobDataPixel !== undefined) {
              let imageInfo: image.ImageInfo = await blobDataPixel.getImageInfo();
              let height: number = imageInfo.size.height;
              let width: number = imageInfo.size.width;
              if (height * width > MIN_RESOLUTION) {
                blobSource = BlobSource.BIG;
              }
            }
          }
          updateBucket[DataColumns.DETAIL_INFO] = 'photo/' + contactId + '_' + rawContactId;
          updateBucket[DataColumns.BLOB_SOURCE] = blobSource;
          const predicates = new rdb.RdbPredicates(DbConstants.NEW_DATA_TABLE_NAME);
          predicates.equalTo(DataColumns.RAW_CONTACT_ID, rawContactId)
            .and()
            .equalTo(DataColumns.TYPE_ID, 8);
          let rdbRet = -1;
          rdbRet = await contactRdbHelper.updateData(updateBucket, predicates);
          if (rdbRet < 0 || rdbRet === 0) {
            LogUtils.e(tagStr, `update contact_data fail, ret:${rdbRet}`);
          } else {
            LogUtils.w(tagStr,
              `update contact_data success, ret:${rdbRet}, raw_contact_id: ${rawContactId}, blobSource: ${blobSource}`);
            // 4.当数据库更新成功，证明contact_data表有type_id为8的数据，代表联系人确实需要大头像文件，那么才复制头像文件至群组沙箱下
            // 如果是单单克隆，直接从旧图片路径拷贝到新图片路径
            if (isNext2next) {
              await this.copyOrSaveImage(oldFilePathRes, oldFilePath, newFilePath, blobDataPixel, tagStr + 'Next2next');
            } else {
              let task = new taskPool.Task(getPixelMapFromFile, oldFilePath);
              let pixelMap = await taskPool.execute(task) as PixelMap | null;
              if (pixelMap) {
                await this.getImagePackingData(pixelMap, newFilePath);
                pixelMap?.release().catch((error: BusinessError) => {
                  LogUtils.e(tagStr, `release resource error: ${error?.message}`);
                });
                LogUtils.w(tagStr, 'write attachmentPath success!!!');
              } else {
                await this.copyOrSaveImage(oldFilePathRes, oldFilePath, newFilePath, blobDataPixel, tagStr);
              }
            }
          }
        }
      }
    } catch (error) {
      LogUtils.e(tagStr, 'error: ' + error?.message + ', stack: ' + error?.stack);
    }
  }

  private async copyOrSaveImage(hasOldFile: boolean, oldFilePath: string, newFilePath: string, pixel: PixelMap | null,
    tag: string): Promise<void> {
    if (hasOldFile) {
      await fs.copyFile(oldFilePath, newFilePath);
      LogUtils.w(tag, 'copy attachmentPath success!!!');
    } else {
      if (pixel) {
        await this.getImagePackingData(pixel, newFilePath);
        pixel.release().catch((error: BusinessError) => {
          LogUtils.e(tag, `pixel release err msg:${error.message}`);
        });
        LogUtils.w(tag, 'write attachmentPath success!!!');
      } else {
        LogUtils.e(tag, 'pixel is null');
      }
    }
  }

  // 双克单 迁移联系人大头像文件并信息入库
  async hmos2nextBigPhotoFileInfo(groupFileInfo: GroupFileInfo, context: Context) {
    const tagStr = TAG + ` hmos2nextBigPhotoFileInfo `;
    try {
      const rdbStore = await contactRdbHelper.initDB(context);
      if (!rdbStore) {
        throw new Error(`GroupIdUtil initDb fail`);
      }
      let groupDir = await this.getGroupDir(context);
      // 新大头像所在文件夹
      let fileDir = groupDir + '/photo';
      LogUtils.w(tagStr, 'fileDir: ' + fileDir);
      let fileDirRes = await fs.access(fileDir);
      if (!fileDirRes) {
        LogUtils.w(tagStr, 'fileDir is not exist, fileDir: ' + fileDir);
        // 创建当前文件夹
        await fs.mkdir(fileDir);
        LogUtils.w(tagStr, 'fileDir mkdir success, fileDir: ' + fileDir);
      }
      let pixelMap: PixelMap | null = null;
      try {
        if (groupFileInfo.oldFileUint8Array !== null && groupFileInfo.oldFileUint8Array !== undefined) {
          let task = new taskPool.Task(getPixelMapFromFromUint8ArrayGroup, groupFileInfo.oldFileUint8Array);
          pixelMap = await taskPool.execute(task) as PixelMap | null;
        }
        if (pixelMap !== null && pixelMap !== undefined) {
          const contactId = groupFileInfo.contactId;
          const rawContactId = groupFileInfo.rawContactId;
          let newFilePath = fileDir + '/' + contactId + '_' + rawContactId;
          await this.deleteOldBigPhoto(newFilePath);
          const updateBucket: ValuesBucket = {};
          updateBucket[DataColumns.DETAIL_INFO] = 'photo/' + contactId + '_' + rawContactId;
          let blobSource = 3;
          let imageInfo = await pixelMap.getImageInfo();
          let height = imageInfo.size.height;
          let width = imageInfo.size.width;
          if (height * width <= MIN_RESOLUTION) {
            blobSource = 1;
          }
          LogUtils.w(tagStr,
            `contactId: ${contactId}, rawContactId: ${rawContactId}, height: ${height}, width: ${width}`);
          updateBucket[DataColumns.BLOB_SOURCE] = blobSource;
          const predicates = new rdb.RdbPredicates(DbConstants.NEW_DATA_TABLE_NAME);
          predicates.equalTo(DataColumns.RAW_CONTACT_ID, rawContactId)
            .and()
            .equalTo(DataColumns.TYPE_ID, 8);
          let rdbRet = -1;
          rdbRet = await contactRdbHelper.updateData(updateBucket, predicates);
          if (rdbRet < 0 || rdbRet === 0) {
            LogUtils.e(tagStr, `update contact_data fail, ret:${rdbRet}`);
          } else {
            LogUtils.w(tagStr, `update contact_data success, ret:${rdbRet}, raw_contact_id: ${rawContactId}`);
            // 当数据库更新成功，证明contact_data表有type_id为8的数据，代表联系人确实需要大头像文件，那么才复制头像文件至群组沙箱下
            await this.getImagePackingData(pixelMap, newFilePath);
            LogUtils.w(tagStr, 'copy attachmentPath success!!!');
          }
        }
      } finally {
        if (pixelMap) {
          pixelMap.release().catch((error: BusinessError) => {
            LogUtils.e(tagStr, `pixelMap release err msg:${error.message}`);
          });
        }
      }
    } catch (error) {
      LogUtils.e(tagStr, 'error: ' + error?.message + ', stack: ' + error?.stack);
    }
  }

  async getImagePackingData(pixelMap: PixelMap, filePath: string) {
    const tagStr = TAG + ` getImagePackingData `;
    try {
      let opt: image.PackingOption = {
        format: 'image/jpeg', quality: 100
      }
      const imagePackerApi = image.createImagePacker();
      let data = await imagePackerApi.packing(pixelMap, opt);
      await imagePackerApi.release();
      await this.writeImageFileData(data, filePath);
    } catch (error) {
      LogUtils.e(tagStr, ` err,msg:${error.message}`);
      return;
    }
  }

  async writeImageFileData(data: ArrayBuffer, filePath: string) {
    const tagStr = TAG + ` writeImageFileData `;
    try {
      let file = await fs.open(filePath, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      await fs.write(file.fd, data);
      await fs.close(file);
    } catch (error) {
      LogUtils.e(tagStr, ` err,msg:${error.message}`);
    }
  }

  // 删除文件
  async deleteOldBigPhoto(filePath: string) {
    try {
      await fs.unlink(filePath);
    } catch (e) {
      return;
    }
  }

  // 检查文件比例
  async checkFileScale(filePath: string, tag?: string) {
    const tagStr = TAG + ` bigPhotoFromFile checkFileScale tag: ${tag} `;
    // 默认使用海报显示大头像
    let blobSource = 3;
    try {
      let task = new taskPool.Task(checkFileScaleFromLocalFile, filePath);
      blobSource = await taskPool.execute(task) as number;
      return blobSource;
    } catch (e) {
      LogUtils.e(tagStr, `err,msg:${e.message}`);
      return blobSource;
    }
  }

  // 通过blobdata获取blobSource
  async getBlobSourceByBlobData(blobData: Uint8Array): Promise<number> {
    let blobSource: number = 3;
    let pixelMap = await getPixelMapFromFromUint8Array(blobData);
    try {
      if (pixelMap !== null && pixelMap !== undefined) {
        let imageInfo = await pixelMap.getImageInfo();
        let height = imageInfo.size.height;
        let width = imageInfo.size.width;
        if (height * width <= MIN_RESOLUTION) {
          blobSource = 1;
        }
      }
    } finally {
      if (pixelMap) {
        pixelMap.release().catch((error: BusinessError) => {
          LogUtils.e(TAG, `getBlobSourceByBlobData pixelMap release err msg:${error?.message}`);
        })
      }
    }
    return blobSource;
  }
}

@Concurrent
async function getPixelMapFromFile(filePath: string) {
  let imageSource: image.ImageSource | null = null;
  let pixelMap: image.PixelMap | null = null;
  try {
    //打印时间耗时
    imageSource = image.createImageSource(filePath);
    if (imageSource !== undefined) {
      pixelMap = await imageSource.createPixelMap();
      return pixelMap ? pixelMap : null;
    } else {
      LogUtils.w('GroupIdUtil', 'createImageSource filePath result empty');
    }
  } catch (error) {
    LogUtils.e('GroupIdUtil', `err,msg:${error.message}`);
  } finally {
    imageSource?.release().catch((error: BusinessError) => {
      LogUtils.e('GroupIdUtil', `release resource error: ${error?.message}`);
    });
  }
  return null;
}

@Concurrent
async function checkFileScaleFromLocalFile(filePath: string): Promise<number> {
  let imageSource: image.ImageSource | null = null;
  let blobSource = 3;
  try {
    imageSource = image.createImageSource(filePath);
    if (imageSource !== undefined) {
      let imageInfo = await imageSource.getImageInfo();
      if (imageInfo) {
        let height = imageInfo.size.height;
        let width = imageInfo.size.width;
        LogUtils.w('GroupIdUtil', `checkFileScaleFromLocalFile, height: ${height}, width: ${width}`);
        if (height * width <= 409600) {
          blobSource = 1;
        }
      }
      return blobSource;
    } else {
      LogUtils.w('GroupIdUtil', 'checkFileScaleFromLocalFile filePath result empty');
    }
  } catch (error) {
    LogUtils.e('GroupIdUtil', `checkFileScaleFromLocalFile err,msg:${error.message}`);
  } finally {
    imageSource?.release().catch((error: BusinessError) => {
      LogUtils.e('GroupIdUtil', `release resouce eroor: ${error?.message}`);
    });
  }
  return blobSource;
}

export async function getPixelMapFromFromUint8Array(oldFileUint8Array: Uint8Array): Promise<PixelMap | null> {
  let imageSource: image.ImageSource | null = null;
  let pixelMap: image.PixelMap | null = null;
  try {
    imageSource = image.createImageSource(oldFileUint8Array.buffer);
    if (imageSource !== undefined) {
      pixelMap = await imageSource.createPixelMap();
      return pixelMap ? pixelMap : null;
    } else {
      LogUtils.w('GroupIdUtil', 'getPixelMapFromFromUint8Array createImageSource filePath result empty');
    }
  } catch (error) {
    LogUtils.e('GroupIdUtil ', 'getPixelMapFromFromUint8Array error: ' + error?.message + ', stack: ' + error?.stack);
  } finally {
    [imageSource, pixelMap].forEach(resources => {
      resources?.release().catch((error: BusinessError) => {
        LogUtils.e('GroupIdUtil', `release resouce eroor: ${error?.message}`);
      });
    });
  }
  return null;
}

export async function getImageInfoFromUint8Array(oldFileUint8Array: Uint8Array): Promise<image.ImageInfo | null> {
  let imageSource: image.ImageSource | null = null;
  let imageInfo: image.ImageInfo | null = null;
  try {
    imageSource = image.createImageSource(oldFileUint8Array.buffer);
    if (imageSource !== undefined) {
      imageInfo = await imageSource.getImageInfo();
    } else {
      LogUtils.w('GroupIdUtil', 'getImageInfoFromFromUint8Array createImageSource filePath result empty');
    }
  } catch (error) {
    LogUtils.e('GroupIdUtil ', 'getImageInfoFromFromUint8Array error: ' + error?.message + ', stack: ' + error?.stack);
  } finally {
    imageSource?.release().catch((error: BusinessError) => {
      LogUtils.e('GroupIdUtil', `release resouce eroor: ${error?.message}`);
    });
  }
  return imageInfo;
}

@Concurrent
export async function getPixelMapFromFromUint8ArrayGroup(oldFileUint8Array: Uint8Array): Promise<PixelMap | null> {
  let imageSource: image.ImageSource | null = null;
  try {
    imageSource = image.createImageSource(oldFileUint8Array.buffer);
    if (imageSource !== undefined) {
      let pixelMap = await imageSource.createPixelMap().catch((error: BusinessError) => {
        LogUtils.w('GroupIdUtil',
          `getPixelMapFromFromUint8ArrayGroup createPixelMap error: ${error?.message}, stack: ${error?.stack}`);
      });
      return pixelMap ? pixelMap : null;
    } else {
      LogUtils.w('GroupIdUtil', 'getPixelMapFromFromUint8ArrayGroup createImageSource filePath result empty');
    }
  } catch (error) {
    LogUtils.e('GroupIdUtil ',
      'getPixelMapFromFromUint8ArrayGroup error: ' + error?.message + ', stack: ' + error?.stack);
  } finally {
    imageSource?.release().catch((error: BusinessError) => {
      LogUtils.e('GroupIdUtil', `release resouce eroor: ${error?.message}`);
    });
  }
  return null;
}