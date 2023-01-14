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

import dataShare from '@ohos.data.dataShare';
import {afterAll, afterEach, beforeAll, beforeEach, describe, expect, it} from 'deccjsunit/index'

const URI_CONTACTS = "datashare:///com.ohos.contactsdataability";
const rawContactUri = "datashare:///com.ohos.contactsdataability/contacts/raw_contact";
const backup = "datashare:///com.ohos.contactsdataability/contacts/backup";
const recover = "datashare:///com.ohos.contactsdataability/contacts/recover";
const deletedUri = "datashare:///com.ohos.contactsdataability/contacts/deleted_raw_contact";

describe('RecoveryTest', function() {
    function sleep(numberMillis)
    {
        var now = new Date();
        var exitTime = now.getTime() + numberMillis;
        while (true) {
            now = new Date();
            if (now.getTime() > exitTime)
                return;
        }
    }

    async function ContactsQuery(tag, uri, size)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_CONTACTS);
        console.info(tag + ': ContactsQuery start ! dataShareHelper = ' + dataShareHelper);
        var resultColumns = [];
        let condition = new dataShare.DataSharePredicates();
        condition.greaterThan("id", "0");
        try {
            var resultSet = await dataShareHelper.query(uri, resultColumns, condition);
            console.info(tag + ' ContactsQuery resultSet.rowCount is = ' + resultSet.rowCount);
            console.info(tag + ' ContactsQuery size is = ' + size);
            expect(resultSet.rowCount == size).assertEqual(true);
            if (resultSet.rowCount > 0) {
                if (resultSet.goToFirstRow()) {
                    do {
                        var id = resultSet.getString(resultSet.getColumnIndex("id"));
                        console.info(tag + ':logMessage contactsQuery id = ' + id);
                        var name = resultSet.getString(resultSet.getColumnIndex("display_name"));
                        console.info(tag + ':logMessage contactsQuery name = ' + name);
                    } while (resultSet.goToNextRow())
                }
            }
            resultSet.close();
        } catch (error) {
            console.info(tag + "logMessage ContactsQuery: error = " + error);
        }
    }

    async function rawContactInsert(name)
    {
        console.info("------logMessage rawContactInsert is starting!-----");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CONTACTS);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var insertValue = {
            "display_name" : name,
        }
        try {
            var rawContactId = await dataShareHelper.insert(rawContactUri, insertValue);
            console.info('logMessage rawContactInsert: rawContactId = ' + rawContactId);
            expect(rawContactId > 0).assertTrue();
        } catch (error) {
            console.info('logMessage rawContactInsert: insert error = ' + error);
        }
        return rawContactId;
    }

    async function deleteRawContact(tag)
    {
        try {
            let dataShareHelper = dataShare.createDataShareHelper(URI_CONTACTS);
            let condition = new dataShare.DataSharePredicates();
            condition.greaterThan("id", "0");
            condition.and();
            condition.equalTo("is_deleted", "0");
            await dataShareHelper.delete(rawContactUri, condition);
            sleep(5000);
            var conditionAll = new dataShare.DataSharePredicates();
            conditionAll.greaterThan("id", "0");
            await dataShareHelper.delete(deletedUri, conditionAll);
        } catch (error) {
            console.info(tag + ': deleteRawContact error = ' + error);
        }
    }

    /**
     * @tc.number  recovery_test_100
     * @tc.name    backup database
     * @tc.desc    Function test
     */
    it("recovery_test_100", 0, async function(done) {
        await deleteRawContact("recovery_test_100");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CONTACTS);
        var updateValues = {};
        let condition = new dataShare.DataSharePredicates();
        try {
            var updateCode = await dataShareHelper.update(backup, updateValues, condition);
            sleep(2000);
            console.info('logMessage recovery_test_100: updateCode = ' + updateCode);
            expect(updateCode == 0).assertTrue();
            sleep(2000);
            await ContactsQuery("recovery_test_100", rawContactUri, 0);
            done();
        } catch (error) {
            console.info('logMessage recovery_test_100: raw_contact update error = ' + error);
            done();
        }
    });

    /**
     * @tc.number  recovery_test_200
     * @tc.n    ame    recovery database
     * @tc.desc    Function test
     */
    it("recovery_test_200", 0, async function(done) {
        await deleteRawContact("recovery_test_200");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CONTACTS);
        var updateValues = {};
        let condition = new dataShare.DataSharePredicates();
        rawContactInsert("liming");
        rawContactInsert("xiaolilili");
        sleep(2000);
        var updateCode = await dataShareHelper.update(backup, updateValues, condition);
        sleep(2000);
        console.info(' recovery_test_200: backup = ' + updateCode);
        expect(updateCode == 0).assertTrue();
        sleep(1000);
        try {
            var updateCode = await dataShareHelper.update(recover, updateValues, condition);
            sleep(3000);
            console.info('logMessage recovery_test_200: recover = ' + updateCode);
            expect(updateCode == 0).assertTrue();
            await ContactsQuery("recovery_test_200", rawContactUri, 2);
            await deleteRawContact("recovery_test_200");
            done();
        } catch (error) {
            console.info('logMessage recovery_test_200: raw_contact update error = ' + error);
            done();
        }
    });
});