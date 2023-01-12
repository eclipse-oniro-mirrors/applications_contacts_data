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
import {afterAll, describe, expect, it} from 'deccjsunit/index'

const URI_CALLLOG = "datashare:///com.ohos.calllogability";
const calllogUri = "datashare:///com.ohos.calllogability/calls/calllog";
import common from './common.js';

describe('CalllogTest', function() {
    console.log("----------CalllogTest is starting!----------");

    async function calllogQueryForALL(map, tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info(tag + ': calllogQueryByInsert start ! dataShareHelper = ' + dataShareHelper);
        let resultColumns = common.getCallLogResultColumns();
        let condition = new dataShare.DataSharePredicates();
        condition.equalTo("id", map.get("id"));
        try {
            let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
            if (resultSet.rowCount > 0) {
                if (resultSet.goToFirstRow()) {
                    do {
                        for (var [key, value] of map) {
                            let dbresult = resultSet.getString(resultSet.getColumnIndex(key));
                            console.info(tag + ' : logMessage calllogQueryByInsert key = ' + key +
                                         ' dbResult :' + dbresult + ' value : ' + value);
                            console.info(tag + ' : logMessage calllogQueryByInsert value ' + (value == dbresult));
                            expect(value == dbresult).assertEqual(true);
                        }
                    } while (resultSet.goToNextRow());
                }
            }
            console.info(tag + " :logMessage calllogQueryByInsert: end");
            resultSet.close();
        } catch (error) {
            console.info(tag + " :logMessage calllogQueryByInsert: error = " + error);
        }
    }

    async function calllogQueryForDelete(map, tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info(tag + ': calllogQueryForDelete start ! dataShareHelper = ' + dataShareHelper);
        let resultColumns = common.getCallLogResultColumns();
        let condition = new dataShare.DataSharePredicates();
        condition.equalTo("id", map.get("id"));
        try {
            let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
            expect(resultSet.goToFirstRow() == false).assertTrue();
            console.info(tag + " :logMessage calllogQueryForDelete: goToFirstRow " + resultSet.goToFirstRow());
            resultSet.close();
        } catch (error) {
            console.info(tag + " :logMessage calllogQueryForDelete: error = " + error);
        }
    }

    async function calllogQueryForBatchInsert(array, tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info(tag + ': calllogQueryForBatchInsert start ! dataShareHelper = ' + dataShareHelper);
        let resultColumns = common.getCallLogResultColumns();
        let condition = new dataShare.DataSharePredicates();
        condition.equalTo("phone_number", array[0].get("phone_number"));
        try {
            let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
            var size = array.size();
            console.info(tag + ' : logMessage calllogQueryForBatchInsert: size' + size);
            expect(resultSet.rowCount == size).assertEqual(true);
            var i = 0;
            if (resultSet.rowCount > 0) {
                if (resultSet.goToFirstRow()) {
                    do {
                        for (var [key, value] of array[i]) {
                            let dbresult = resultSet.getString(resultSet.getColumnIndex(key));
                            console.info(tag + ' : logMessage calllogQueryForBatchInsert dbresult :' + dbresult +
                                         ' value : ' + value);
                            console.info(tag + ' : logMessage calllogQueryForBatchInsert value ' + (value == dbresult));
                            expect(value == dbresult).assertTrue();
                        }
                        i++;
                    } while (resultSet.goToNextRow());
                }
            }
            resultSet.close();
        } catch (error) {
            console.info(tag + " :logMessage calllogQueryByInsert: error = " + error);
        }
    }

    async function calllogDelete(tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        let condition = new dataShare.DataSharePredicates();
        condition.greaterThan("id", "0");
        var deleteCode = await dataShareHelper.delete(calllogUri, condition);
        console.info(tag + ': calllogDelete deleteCode = ' + deleteCode);
        expect(deleteCode == 0).assertTrue();
    }

    /**
     * @tc.number  calllog_insert_test_100
     * @tc.name    Insert call log
     * @tc.desc    Function test
     */
    it("calllog_insert_test_100", 0, async function(done) {
        console.info("--------logMessage calllog_insert_test_100 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, common.getCallLogInsert());
            console.info("logMessage calllog_insert_test_100: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            var map = common.getCallLogInsertMap()
            map.set("id", calllogId.toString());
            await calllogQueryForALL(map, "calllog_insert_test_100");
            await calllogDelete("calllog_insert_test_100");
            done();
        } catch (error) {
            console.info("logMessage calllog_insert_test_100: calllog insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  calllog_update_test_400
     * @tc.name    Single call log update
     * @tc.desc    Function test
     */
    it("calllog_update_test_400", 0, async function(done) {
        console.info("--------logMessage calllog_update_test_400 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);

        try {
            var calllogId = await dataShareHelper.insert(calllogUri, common.getCallLogInsert());
            console.info("logMessage calllog_update_test_400: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await UpdateOneCalllog();
            done();
        } catch (error) {
            console.info("logMessage calllog_update_test_400: calllog insert error = " + error);
            done();
        }

        async function UpdateOneCalllog()
        {
            try {
                let condition = new dataShare.DataSharePredicates();
                condition.equalTo("id", calllogId.toString());
                var updateCode = await dataShareHelper.update(calllogUri, common.getCallLogUpdate(), condition);
                console.info("logMessage calllog_update_test_400: updateCode = " + updateCode);
                expect(updateCode == 0).assertTrue();
                var map = common.getCallLogUpdateMap();
                map.set("id", calllogId.toString());
                await calllogQueryForALL(map, "calllog_update_test_400");
                await calllogDelete("calllog_update_test_400");
            } catch (error) {
                console.info("logMessage calllog_update_test_400: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  calllog_update_test_500
     * @tc.name    Update all call logs
     * @tc.desc    Function test
     */
    it("calllog_update_test_500", 0, async function(done) {
        console.info("--------logMessage calllog_update_test_500 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(12);
        var insertValues = {"phone_number" : phoneNumber, "ring_duration" : "500"};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_update_test_500: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await UpdateAllCalllog();
            done();
        } catch (error) {
            console.info("logMessage calllog_update_test_500: calllog insert error = " + error);
            done();
        }
        async function UpdateAllCalllog()
        {
            var updateValues = {"answer_state" : "1"};
            let condition = new dataShare.DataSharePredicates();
            condition.greaterThan("id", "0");
            try {
                var updateCode = await dataShareHelper.update(calllogUri, updateValues, condition)
                console.info("logMessage calllog_update_test_500: updateCode = " + updateCode);
                expect(updateCode == 0).assertTrue();
                var map = new Map([ [ "phone_number", phoneNumber ], [ "ring_duration", "500" ] ])
                map.set("id", calllogId.toString());
                map.set("answer_state", "1");
                await calllogQueryForALL(map, "calllog_update_test_500");
                await calllogDelete("calllog_update_test_500");
            } catch (error) {
                console.info("logMessage calllog_update_test_500: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  calllog_delete_test_1300
     * @tc.name    Delete call log
     * @tc.desc    Function test
     */
    it("calllog_delete_test_1300", 0, async function(done) {
        console.info("--------logMessage calllog_delete_test_1300 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(5);
        var insertValues = {"phone_number" : phoneNumber, "ring_duration" : "200"};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_delete_test_1300: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await DeleteOneCalllog();
            done();
        } catch (error) {
            console.info("logMessage calllog_delete_test_1300: calllog insert error = " + error);
            done();
        }

        async function DeleteOneCalllog()
        {
            try {
                let condition = new dataShare.DataSharePredicates();
                condition.equalTo("id", calllogId.toString());
                var deleteCode = await dataShareHelper.delete(calllogUri, condition);
                console.info("logMessage calllog_delete_test_1300: deleteCode = " + deleteCode);
                expect(deleteCode == 0).assertTrue();
                var map = new Map();
                map.set("id", calllogId.toString());
                await calllogQueryForDelete(map, "calllog_delete_test_1300");
            } catch (error) {
                console.info("logMessage calllog_delete_test_1300: delete error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  calllog_batchInset_test_1600
     * @tc.name    Insert call logs in batches
     * @tc.desc    Function test
     */
    it("calllog_batchInset_test_1600", 0, async function(done) {
        console.info("--------logMessage calllog_batchInset_test_1600 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(13);
        var addBulk_value1 = {"phone_number" : phoneNumber, "ring_duration" : "333"};
        var addBulk_value2 = {"phone_number" : phoneNumber, "ring_duration" : "600"};
        var addBulk_value3 = {"phone_number" : phoneNumber, "ring_duration" : "600"};
        var addBulk_value4 = {"phone_number" : phoneNumber, "ring_duration" : "600"};
        var addBulk_value5 = {"phone_number" : phoneNumber, "ring_duration" : "600"};
        var listAddBluk = [];
        listAddBluk[0] = addBulk_value1;
        listAddBluk[1] = addBulk_value2;
        listAddBluk[2] = addBulk_value3;
        listAddBluk[3] = addBulk_value4;
        listAddBluk[4] = addBulk_value5;
        try {
            var batchInsertCode = await dataShareHelper.batchInsert(calllogUri, listAddBluk);
            console.info("logMessage calllog_batchInset_test_1600: batchInsertCode = " + batchInsertCode);
            expect(batchInsertCode == 0).assertTrue();
            await calllogQueryForBatchInsert(common.getCallLogBatchInsert(), "calllog_batchInset_test_1600");
            await calllogDelete("calllog_batchInset_test_1600");
            done();
        } catch (error) {
            console.info("logMessage calllog_batchInset_test_1600: error = " + error);
            done();
        }
    });

    /**
     * @tc.number  calllog_query_test_1200
     * @tc.name    Query all call logs
     * @tc.desc    Function test
     */
    it("calllog_query_test_1200", 0, async function(done) {
        console.info("------------calllog_query_test_1200  is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(5);
        var insertValues = {"phone_number" : phoneNumber, "ring_duration" : "200"};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_query_test_1200: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            var map = new Map([ [ "phone_number", phoneNumber ], [ "ring_duration", "200" ] ]);
            map.set("id", calllogId.toString());
            await calllogQueryForALL(map, "calllog_query_test_1200");
            await calllogDelete("calllog_query_test_1200");
            done();
        } catch (error) {
            console.info("logMessage calllog_query_test_1200: calllog insert error = " + error);
        }
    });

    /**
     * @tc.number  calllog_query_test_1100
     * @tc.name    Query call logs of combined conditions
     * @tc.desc    Function test
     */
    it("calllog_query_test_1100", 0, async function(done) {
        console.info("------------calllog_query_test_1100 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(7);
        var insertValues = {"phone_number" : phoneNumber, "ring_duration" : "100"};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_query_test_1100: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await QueryCalllog();
            await calllogDelete("calllog_query_test_1100");
            done()
        } catch (error) {
            console.info("logMessage calllog_query_test_1100: calllog insert error = " + error);
            done();
        }

        async function QueryCalllog()
        {
            var resultColumns = [ "id", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.greaterThan("id", "0");
            condition.and();
            condition.lessThan("ring_duration", "200").orderByAsc("id");
            try {
                let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
                if (resultSet.goToFirstRow()) {
                    do {
                        console.info("logMessage calllog_query_test_1100: columnNames:" + resultSet.columnNames);
                        console.info("logMessage calllog_query_test_1100: columnCount:" + resultSet.columnCount);
                        console.info('logMessage calllog_query_test_1100: id = ' + resultSet.getString(0));
                        expect(resultSet.getString(0) == calllogId.toString()).assertTrue();
                        console.info('logMessage calllog_query_test_1100: phone_number = ' + resultSet.getString(1));
                        expect(resultSet.getString(1) == phoneNumber).assertTrue();
                    } while (resultSet.goToNextRow());
                }
                resultSet.close();
            } catch (error) {
                console.info("logMessage calllog_query_test_1100: query error:" + error);
                done();
            }
        }
    });

    /**
     * @tc.number  calllog_query_test_800
     * @tc.name    Query a single call log
     * @tc.desc    Function test
     */
    it("calllog_query_test_800", 0, async function(done) {
        console.info("------------calllog_query_test_800 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(10);
        var insertValues = {"phone_number" : phoneNumber, "ring_duration" : "100"};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_query_test_800: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await QueryOneCalllog();
            await calllogDelete("calllog_query_test_800");
            done();
        } catch (error) {
            console.info("logMessage calllog_query_test_800: calllog insert error = " + error);
            done();
        }

        async function QueryOneCalllog()
        {
            var resultColumns = [ "id", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", calllogId.toString());
            try {
                let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
                if (resultSet.goToFirstRow()) {
                    do {
                        console.info("logMessage calllog_query_test_800: columnNames:" + resultSet.columnNames);
                        console.info("logMessage calllog_query_test_800: columnCount:" + resultSet.columnCount);
                        console.info('logMessage calllog_query_test_800: id = ' + resultSet.getString(0));
                        expect(resultSet.getString(0) == calllogId.toString()).assertTrue();
                        console.info('logMessage calllog_query_test_800: phone_number = ' + resultSet.getString(1));
                        expect(resultSet.getString(1) == phoneNumber).assertTrue();
                    } while (resultSet.goToNextRow());
                }
                resultSet.close();
            } catch (error) {
                console.info("logMessage calllog_query_test_800: query error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  calllog_fuzzyquery_test_100
     * @tc.name    Fuzzy query ability based on mobile phone number (beginning of a certain field)
     * @tc.desc    Function test
     */
    it("calllog_fuzzyquery_test_100", 0, async function(done) {
        console.info("------------calllog_fuzzyquery_test_100 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(8);
        var phoneNumber_Test = phoneNumber.substring(0,3);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_fuzzyquery_test_100: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await QueryByStartsWithPhoneNumber();
            await calllogDelete("calllog_fuzzyquery_test_100");
            done();
        } catch (error) {
            console.info("logMessage calllog_fuzzyquery_test_100: calllog insert error = " + error);
            done();
        }

        async function QueryByStartsWithPhoneNumber()
        {
            var resultColumns = [ "id", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.like("phone_number", phoneNumber_Test + "%");
            try {
                let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
                if (resultSet.goToFirstRow()) {
                    do {
                        console.info("logMessage calllog_fuzzyquery_test_100: columnNames:" + resultSet.columnNames);
                        console.info("logMessage calllog_fuzzyquery_test_100: columnCount:" + resultSet.columnCount);
                        console.info('logMessage calllog_fuzzyquery_test_100: id = ' + resultSet.getString(0));
                        expect(resultSet.getString(0) == calllogId.toString()).assertTrue();
                        console.info(
                            'logMessage calllog_fuzzyquery_test_100: phone_number = ' + resultSet.getString(1));
                        expect(resultSet.getString(1) == phoneNumber).assertTrue();
                    } while (resultSet.goToNextRow());
                }
                resultSet.close();
            } catch (error) {
                console.info("logMessage calllog_fuzzyquery_test_100: query error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  calllog_fuzzyquery_test_200
     * @tc.name    Fuzzy query ability based on mobile phone number (at the end of a certain field)
     * @tc.desc    Function test
     */
    it("calllog_fuzzyquery_test_200", 0, async function(done) {
        console.info("------------calllog_fuzzyquery_test_200 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(8);
        var phoneNumber_Test = phoneNumber.substring(6,9);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_fuzzyquery_test_200: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await QueryByEndWithPhoneNumber();
            await calllogDelete("calllog_fuzzyquery_test_200");
            done();
        } catch (error) {
            console.info("logMessage calllog_fuzzyquery_test_200: calllog insert error = " + error);
            done();
        }

        async function QueryByEndWithPhoneNumber()
        {
            var resultColumns = [ "id", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.like("phone_number", "%" + phoneNumber_Test);
            try {
                let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
                if (resultSet.goToFirstRow()) {
                    do {
                        console.info("logMessage calllog_fuzzyquery_test_200: columnNames:" + resultSet.columnNames);
                        console.info("logMessage calllog_fuzzyquery_test_200: columnCount:" + resultSet.columnCount);
                        console.info('logMessage calllog_fuzzyquery_test_200: id = ' + resultSet.getString(0));
                        expect(resultSet.getString(0) == calllogId.toString()).assertTrue();
                        console.info(
                            'logMessage calllog_fuzzyquery_test_200: phone_number = ' + resultSet.getString(1));
                        expect(resultSet.getString(1) == phoneNumber).assertTrue();
                    } while (resultSet.goToNextRow());
                }
                resultSet.close();
            } catch (error) {
                console.info("logMessage calllog_fuzzyquery_test_200: query error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  calllog_fuzzyquery_test_300
     * @tc.name    Fuzzy query ability based on mobile phone number (including a certain field)
     * @tc.desc    Function test
     */
    it("calllog_fuzzyquery_test_300", 0, async function(done) {
        console.info("------------calllog_fuzzyquery_test_300 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(15);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_fuzzyquery_test_300: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await QueryByPhoneNumberContainsField();
            await calllogDelete("calllog_fuzzyquery_test_300");
            done();
        } catch (error) {
            console.info("logMessage calllog_fuzzyquery_test_300: calllog insert error = " + error);
            done();
        }

        async function QueryByPhoneNumberContainsField()
        {
            var resultColumns = [ "id", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            var phoneNumber_Test = phoneNumber.substring(7,10);
            condition.like("phone_number", "%" + phoneNumber_Test + "%");
            try {
                let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
                if (resultSet.goToFirstRow()) {
                    do {
                        console.info("logMessage calllog_fuzzyquery_test_300: columnNames:" + resultSet.columnNames);
                        console.info("logMessage calllog_fuzzyquery_test_300: columnCount:" + resultSet.columnCount);
                        console.info('logMessage calllog_fuzzyquery_test_300: id = ' + resultSet.getString(0));
                        expect(resultSet.getString(0) == calllogId.toString()).assertTrue();
                        console.info(
                            'logMessage calllog_fuzzyquery_test_300: phone_number = ' + resultSet.getString(1));
                        expect(resultSet.getString(1) == phoneNumber).assertTrue();
                    } while (resultSet.goToNextRow());
                }
                resultSet.close();
            } catch (error) {
                console.info("logMessage calllog_fuzzyquery_test_300: query error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_calllog_insert_test_200
     * @tc.name    Abnormal use case, an incorrect field is passed in when inserting data
     * @tc.desc    Function test
     */
    it("abnormal_calllog_insert_test_200", 0, async function(done) {
        console.info("------------abnormal_calllog_insert_test_200 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(9);
        var insertValues = {"phone_numbers" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage abnormal_calllog_insert_test_200: calllogId = " + calllogId);
            expect(calllogId == -1).assertTrue();
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_insert_test_200: calllog insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  abnormal_calllog_insert_test_300
     * @tc.name    Abnormal use case, an incorrect table name is passed in when inserting data
     * @tc.desc    Function test
     */
    it("abnormal_calllog_insert_test_300", 0, async function(done) {
        console.info("------------abnormal_calllog_insert_test_300 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var errorUri = "datashare:///com.ohos.calllogability/calls/calllogs";
        var phoneNumber = randomNum(8);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(errorUri, insertValues);
            console.info("logMessage abnormal_calllog_insert_test_300: calllogId = " + calllogId);
            expect(calllogId == -1).assertTrue();
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_insert_test_300: calllog insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  abnormal_calllog_update_test_600
     * @tc.name    Abnormal use case, passing in a wrong field when updating data
     * @tc.desc    Function test
     */
    it("abnormal_calllog_update_test_600", 0, async function(done) {
        console.info("------------abnormal_calllog_update_test_600 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(9);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage abnormal_calllog_update_test_600: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await abnormalUpdate();
            await calllogDelete("abnormal_calllog_update_test_600");
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_update_test_600: calllog insert error = " + error);
            done();
        }

        async function abnormalUpdate()
        {
            var phoneNumber_Test = randomNum(7);
            var updateValues = {"phone_numbers" : phoneNumber_Test};
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("ids", calllogId.toString());
            try {
                var updataCode = await dataShareHelper.update(calllogUri, updateValues, condition);
                console.info("logMessage abnormal_calllog_update_test_600: updataCode = " + updataCode);
                expect(updataCode == -1).assertTrue();
                var map = new Map();
                map.set("id", calllogId.toString());
                map.set("phone_number", phoneNumber);
                await calllogQueryForALL(map, "abnormal_calllog_update_test_600");
            } catch (error) {
                console.info("logMessage abnormal_calllog_update_test_600: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_calllog_update_test_700
     * @tc.name    Abnormal use case, an incorrect table name is passed in when updating data
     * @tc.desc    Function test
     */
    it("abnormal_calllog_update_test_700", 0, async function(done) {
        console.info("------------abnormal_calllog_update_test_700 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var errorUri = "datashare:///com.ohos.calllogability/calls/calllogs";
        var phoneNumber = randomNum(6);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage abnormal_calllog_update_test_700: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await abnormalUpdate();
            await calllogDelete("abnormal_calllog_update_test_700");
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_update_test_700: calllog insert error = " + error);
            done();
        }

        async function abnormalUpdate()
        {
            var phoneNumber_Test = randomNum(7);
            var updateValues = {"phone_numbers" : phoneNumber_Test};
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", calllogId.toString());
            try {
                var updataCode = await dataShareHelper.update(errorUri, updateValues, condition);
                console.info("logMessage abnormal_calllog_update_test_700: updataCode = " + updataCode);
                expect(updataCode == -1).assertTrue();
                var map = new Map();
                map.set("id", calllogId.toString());
                map.set("phone_number", phoneNumber);
                await calllogQueryForALL(map, "abnormal_calllog_update_test_700");
                done();
            } catch (error) {
                console.info("logMessage abnormal_calllog_update_test_700: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_calllog_delete_test_1400
     * @tc.name    Abnormal use case, passing in a wrong field when deleting data
     * @tc.desc    Function test
     */
    it("abnormal_calllog_delete_test_1400", 0, async function(done) {
        console.info("------------abnormal_calllog_delete_test_1400 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(5);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage abnormal_calllog_delete_test_1400: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await abnormalDelete();
            await calllogDelete("abnormal_calllog_delete_test_1400");
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_delete_test_1400: calllog insert error = " + error);
            done();
        }

        async function abnormalDelete()
        {
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("ids", calllogId.toString());
            try {
                var deleteCode = await dataShareHelper.delete(calllogUri, condition);
                console.info("logMessage abnormal_calllog_delete_test_1400: deleteCode = " + deleteCode);
                expect(deleteCode == -1).assertTrue();
                var map = new Map();
                map.set("id", calllogId.toString());
                map.set("phone_number", phoneNumber);
                await calllogQueryForALL(map, "abnormal_calllog_delete_test_1400");
            } catch (error) {
                console.info("logMessage abnormal_calllog_delete_test_1400: delete error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_calllog_delete_test_1500
     * @tc.name    Abnormal use case, passing in an incorrect table name when deleting data
     * @tc.desc    Function test
     */
    it("abnormal_calllog_delete_test_1500", 0, async function(done) {
        console.info("------------abnormal_calllog_delete_test_1500 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var errorUri = "datashare:///com.ohos.calllogability/calls/calllogs";
        var phoneNumber = randomNum(6);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage abnormal_calllog_delete_test_1500: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await abnormalDelete();
            await calllogDelete("abnormal_calllog_delete_test_1500");
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_delete_test_1500: calllog insert error = " + error);
            done();
        }

        async function abnormalDelete()
        {
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", calllogId.toString());
            try {
                var deleteCode = await dataShareHelper.delete(errorUri, condition);
                console.info("logMessage abnormal_calllog_delete_test_1500: deleteCode = " + deleteCode);
                expect(deleteCode == -1).assertTrue();
                done()
                var map = new Map();
                map.set("id", calllogId.toString());
                map.set("phone_number", phoneNumber);
                await calllogQueryForALL(map, "abnormal_calllog_delete_test_1500");
            } catch (error) {
                console.info("logMessage abnormal_calllog_delete_test_1500: delete error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_calllog_query_test_900
     * @tc.name    Abnormal use case, an incorrect field is passed in when querying
     * @tc.desc    Function test
     */
    it("abnormal_calllog_query_test_900", 0, async function(done) {
        console.info("------------abnormal_calllog_query_test_900 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(6);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage abnormal_calllog_query_test_900: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await abnormalQuery();
            await calllogDelete("abnormal_calllog_query_test_900");
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_query_test_900: calllog insert error = " + error);
            done();
        }

        async function abnormalQuery()
        {
            var resultColumns = [ "id", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("ids", calllogId.toString());
            try {
                let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
                console.info('logMessage abnormal_calllog_query_test_900: goToFirstRow' + resultSet.goToFirstRow());
                expect(resultSet.goToFirstRow()).assertEqual(false);
                resultSet.close();
            } catch (error) {
                console.info("logMessage abnormal_calllog_query_test_900:query error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_calllog_batchinsert_test_1700
     * @tc.name    abnormal_batchinsert
     * @tc.desc    Function test
     */
    it("abnormal_calllog_batchinsert_test_1700", 0, async function(done) {
        console.info("--------logMessage abnormal_calllog_batchinsert_test_1700 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(9);
        var phoneNumber_Test = randomNum(6);
        var addBulk_value1 = {"phone_number" : phoneNumber, "ring_duration" : "500"};
        var addBulk_value2 = {"phone_number" : phoneNumber_Test, "ring_duration" : "100"};
        var addBulk_value3 = {"phone_numbers" : phoneNumber_Test, "ring_duration" : "100"};
        var addBulk_value4 = {"phone_number" : phoneNumber_Test, "ring_durations" : "100"};
        var addBulk_value5 = {"phone_number" : phoneNumber_Test, "ring_duration" : "100"};
        var listAddBluk = [];
        listAddBluk[0] = addBulk_value1;
        listAddBluk[1] = addBulk_value2;
        listAddBluk[2] = addBulk_value3;
        listAddBluk[3] = addBulk_value4;
        listAddBluk[4] = addBulk_value5;
        try {
            var batchInsertCode = await dataShareHelper.batchInsert(calllogUri, listAddBluk);
            console.info("logMessage abnormal_calllog_batchinsert_test_1700: batchInsertCode = " + batchInsertCode);
            expect(batchInsertCode == -1).assertTrue();
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_batchinsert_test_1700: batchInsert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  calllog_delete_test_1800
     * @tc.name    batchDelete
     * @tc.desc    Function test
     */
    it("calllog_delete_test_1800", 0, async function(done) {
        console.info("--------logMessage calllog_delete_test_1800 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, common.getCallLogInsert());
            console.info("logMessage calllog_delete_test_1800: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            var map = common.getCallLogInsertMap()
            map.set("id", calllogId.toString());
            await calllogQueryForALL(map, "calllog_delete_test_1800");
            await executeBatch();
        } catch (error) {
            console.info("logMessage calllog_delete_test_1800: calllog insert error = " + error);
            done();
        }

        async function executeBatch()
        {
            console.info("logMessage calllog_delete_test_1800:  executeBatch start ");
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", calllogId.toString());
            dataShareHelper.executeBatch(URI_CALLLOG, [ {
                uri : calllogUri,
                type : featureAbility.DataAbilityOperationType.TYPE_DELETE,
                predicates : condition,
                expectedCount : 0,
                PredicatesBackReferences : {},
                interrupted : true,
            } ],
                (error, data) => {
                    console.info("logMessage calllog_delete_test_1800: executeBatch data = " + JSON.stringify(data));
                    console.info("logMessage calllog_delete_test_1800: data_3 = " + data);
                    console.info("logMessage calllog_delete_test_1800:  data_1= " + data[0].count);
                    expect(data[0].count == 0).assertTrue();
                    done();
                });
        }
    });

    /**
     * @tc.number  calllog_update_test_1900
     * @tc.name    batchUpdate
     * @tc.desc    Function test
     */
    it("calllog_update_test_1900", 0, async function(done) {
        console.info("--------logMessage calllog_update_test_1900 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, common.getCallLogInsert());
            console.info("logMessage calllog_update_test_1900: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            var map = common.getCallLogInsertMap()
            map.set("id", calllogId.toString());
            await calllogQueryForALL(map, "calllog_update_test_1900");
            await executeBatch();
        } catch (error) {
            console.info("logMessage calllog_update_test_1900: calllog insert error = " + error);
            done();
        }

        async function executeBatch()
        {
            var phoneNumber = randomNum(8);
            var updateValues = {"phone_number" : phoneNumber};
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", calllogId.toString());
            dataShareHelper.executeBatch(URI_CALLLOG, [ {
                uri : calllogUri,
                type : featureAbility.DataAbilityOperationType.TYPE_UPDATE,
                valuesBucket : updateValues,
                predicates : condition,
                expectedCount : 0,
                PredicatesBackReferences : {},
                interrupted : true,
            } ],
                (error, data) => {
                    console.info("logMessage calllog_update_test_1900: executeBatch data = " + JSON.stringify(data));
                    console.info("logMessage calllog_update_test_1900: data_3 = " + data);
                    console.info("logMessage calllog_update_test_1900:  data_1= " + data[0].count);
                    expect(data[0].count == 0).assertTrue();
                    done();
                });
        }
    });

    /**
     * @tc.number  calllog_Delete_test_2000
     * @tc.name    calllog Delete
     * @tc.desc    Function test
     */
    it("calllog_Delete_test_2000", 0, async function(done) {
        console.info("--------logMessage calllog_Delete_test_2000 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(5);
        var insertValues = {"phone_number" : phoneNumber, "ring_duration" : "200"};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            var calllogIdTwo = await dataShareHelper.insert(calllogUri, insertValues);
            var calllogIdThree = await dataShareHelper.insert(calllogUri, insertValues);
            expect(calllogId > 0).assertTrue();
            expect(calllogIdTwo > 0).assertTrue();
            expect(calllogIdThree > 0).assertTrue();
            await DeleteCalllog();
            done();
        } catch (error) {
            console.info("logMessage calllog_Delete_test_2000: calllog insert error = " + error);
            done();
        }

        async function DeleteCalllog()
        {
            try {
                let condition = new dataShare.DataSharePredicates();
                condition.equalTo("id", calllogId.toString());
                condition.or();
                condition.equalTo("id", calllogIdTwo.toString());
                condition.or();
                condition.equalTo("id", calllogIdThree.toString());
                var deleteCode = await dataShareHelper.delete(calllogUri, condition);
                console.info("logMessage calllog_Delete_test_2000: deleteCode = " + deleteCode);
                expect(deleteCode == 0).assertTrue();
                var resultColumns = [];
                let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
                expect(resultSet.rowCount == 0).assertTrue();
                resultSet.close();
            } catch (error) {
                console.info("logMessage calllog_Delete_test_2000: delete error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  calllog_Delete_test_2100
     * @tc.name    calllog Delete
     * @tc.desc    Function test
     */
    it("calllog_Delete_test_2100", 0, async function(done) {
        console.info("--------logMessage calllog_Delete_test_2100 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, common.getCallLogInsert());
            console.info("logMessage calllog_Delete_test_2100: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            var map = common.getCallLogInsertMap()
            map.set("id", calllogId.toString());
            await calllogQueryForALL(map, "calllog_Delete_test_2100");
            await calllogDelete("calllog_Delete_test_2100");
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", calllogId);
            var resultColumns = [];
            let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
            expect(resultSet.rowCount == 0).assertTrue();
            resultSet.close();
            done();
        } catch (error) {
            console.info("logMessage calllog_Delete_test_2100: calllog insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  calllog_queryContains_test_2200
     * @tc.name    calllog Delete
     * @tc.desc    Function test
     */
    it("calllog_queryContains_test_2200", 0, async function(done) {
        console.info("--------logMessage calllog_queryContains_test_2200 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(17);
        var phoneNumber_Test = phoneNumber.substring(13,18);
        var insertValues = {"phone_number" : phoneNumber}
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage calllog_queryContains_test_2200: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();

            var resultColumns = [ "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.contains("phone_number", phoneNumber_Test);
            let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
            if (resultSet.rowCount > 0) {
                if (resultSet.goToFirstRow()) {
                    do {
                        var phone = resultSet.getString(resultSet.getColumnIndex("phone_number"));
                        console.info("logMessage calllog_queryContains_test_2200: phone is = " + phone);
                        expect(phone == phoneNumber).assertEqual(true);
                    } while (resultSet.goToNextRow());
                }
            }
            resultSet.close();
            await calllogDelete();
            done();
        } catch (error) {
            console.info("logMessage calllog_queryContains_test_2200: calllog insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  abnormal_calllog_update_test_2300
     * @tc.name    Update all call records, pass in a field that does not exist
     * @tc.desc    Function test
     */
    it("abnormal_calllog_update_test_2300", 0, async function(done) {
        console.info("--------logMessage abnormal_calllog_update_test_2300 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(10);
        var insertValues = {phone_number : phoneNumber, display_name : "name2300"}
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            var calllogIdTwo = await dataShareHelper.insert(calllogUri, insertValues);
            expect(calllogId > 0).assertTrue();
            expect(calllogIdTwo > 0).assertTrue();
            await UpdateOneCalllog();
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_update_test_2300: calllog insert error = " + error);
            done();
        }
        async function UpdateOneCalllog()
        {
            try {
                var insertValues = {phone_number : phoneNumber, display_names : "nameUpdateError2300"};
                 var condition =
                    new dataShare.DataSharePredicates();
                var updateCode = await dataShareHelper.update(calllogUri, insertValues, condition);
                console.info("logMessage abnormal_calllog_update_test_2300: updateCode = " + updateCode);
                expect(updateCode == -1).assertTrue();
                var map = new Map();
                map.set("id", calllogId.toString());
                map.set("display_name", "name2300");
                await calllogQueryForALL(map, "abnormal_calllog_update_test_2300");
                map.set("id", calllogIdTwo.toString());
                await calllogQueryForALL(map, "abnormal_calllog_update_test_2300");
                await calllogDelete("calllog_update_test_2300");
            } catch (error) {
                console.info("logMessage abnormal_calllog_update_test_2300: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_calllog_query_test_2400
     * @tc.name    When querying call records based on combined conditions, incorrect field
     *             names and non-existent data are passed in
     * @tc.desc    Function test
     */
    it("abnormal_calllog_query_test_2400", 0, async function(done) {
        console.info("--------logMessage abnormal_calllog_query_test_2400 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(10);
        var insertValues = {phone_number : phoneNumber, display_name : "name2300"}
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            var calllogIdTwo = await dataShareHelper.insert(calllogUri, insertValues);
            expect(calllogId > 0).assertTrue();
            expect(calllogIdTwo > 0).assertTrue();
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", calllogId.toString() + "ksks");
            var resultColumns = [ "display_names" ];
            let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
            console.info("abnormal_calllog_query_test_2400: resultSet.rowCount = " + resultSet.rowCount);
            expect(resultSet.rowCount == -1).assertTrue();
            resultSet.close();
            await calllogDelete("abnormal_calllog_query_test_2400");
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_query_test_2400: calllog insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  abnormal_calllog_query_test_2500
     * @tc.name    When querying all call records based on combined conditions, incorrect field
     *             names and non-existent data are passed in
     * @tc.desc    Function test
     */
    it("abnormal_calllog_query_test_2500", 0, async function(done) {
        console.info("--------logMessage abnormal_calllog_query_test_2500 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(10);
        var insertValues = {phone_number : phoneNumber, display_name : "name2300"}
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info('abnormal_calllog_query_test_2500 calllogId = ' + calllogId);
            var calllogIdTwo = await dataShareHelper.insert(calllogUri, insertValues);
            console.info('abnormal_calllog_query_test_2500 calllogIdTwo = ' + calllogIdTwo);
            expect(calllogId > 0).assertTrue();
            expect(calllogIdTwo > 0).assertTrue();
            let condition = new dataShare.DataSharePredicates();
            var resultColumns = [ "display_names" ];
            let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
            console.info('abnormal_calllog_query_test_2500 resultSet.goToFirstRow() = ' + resultSet.goToFirstRow());
            console.info('abnormal_calllog_query_test_2500 resultSet.rowCount = ' + resultSet.rowCount);
            expect(resultSet.goToFirstRow() == false).assertTrue();
            resultSet.close();
            await calllogDelete("abnormal_calllog_query_test_2500");
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_query_test_2500: calllog insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  abnormal_calllog_delete_test_2600
     * @tc.name    Batch call records are added, and some of them fail, depending on the processing logic
     * @tc.desc    Function test
     */
    it("abnormal_calllog_delete_test_2600", 0, async function(done) {
        console.info("--------logMessage abnormal_calllog_delete_test_2600 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(10);
        var insertValues = {phone_number : phoneNumber, display_name : "name2300"}
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            var calllogIdTwo = await dataShareHelper.insert(calllogUri, insertValues);
            expect(calllogId > 0).assertTrue();
            expect(calllogIdTwo > 0).assertTrue();
        } catch (error) {
            console.info("logMessage abnormal_calllog_delete_test_2600: calllog insert error = " + error);
            done();
        }
        let condition = new dataShare.DataSharePredicates();
        condition.equalTo("ids", calllogIdTwo.toString());
        condition.or();
        condition.equalTo("id", calllogId.toString());
        var code = await dataShareHelper.delete(calllogUri, condition);
        console.info("logMessage abnormal_calllog_delete_test_2600: code = " + code);
        expect(code == -1).assertTrue();
        condition.clear();
        var resultColumns = [];
        let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
        var count = 2;
        expect(resultSet.rowCount == count).assertTrue();
        resultSet.close();
        await calllogDelete("abnormal_calllog_delete_test_2600");
        done();
    });

    /**
     * @tc.number  abnormal_calllog_delete_test_2700
     * @tc.name    When batch call records are modified, the wrong field or table name is passed in
     * @tc.desc    Function test
     */
    it("abnormal_calllog_delete_test_2700", 0, async function(done) {
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(14);
        var insertValues = {phone_number : phoneNumber, display_name : "name2700"}
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            var calllogIdTwo = await dataShareHelper.insert(calllogUri, insertValues);
            expect(calllogId > 0).assertTrue();
            expect(calllogIdTwo > 0).assertTrue();
            await UpdateOneCalllog();
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_delete_test_2700: calllog insert error = " + error);
            done();
        }
        async function UpdateOneCalllog()
        {
            try {
                var insertValues = {phone_number : phoneNumber, display_names : "nameUpdateError2700"};
                let condition = new dataShare.DataSharePredicates();
                condition.equalTo("ids", calllogIdTwo.toString());
                condition.or();
                condition.equalTo("id", calllogId.toString());
                var updateCode = await dataShareHelper.update(calllogUri, insertValues, condition);
                console.info("logMessage abnormal_calllog_delete_test_2700: updateCode = " + updateCode);
                expect(updateCode == -1).assertTrue();
                var map = new Map();
                map.set("id", calllogId.toString());
                map.set("display_name", "name2700");
                await calllogQueryForALL(map, "abnormal_calllog_delete_test_2700");
                map.set("id", calllogIdTwo.toString());
                await calllogQueryForALL(map, "abnormal_calllog_delete_test_2700");
                await calllogDelete("abnormal_calllog_delete_test_2700");
            } catch (error) {
                console.info("logMessage abnormal_calllog_delete_test_2700: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_calllog_query_test_2800
     * @tc.name    Fuzzy search query based on mobile phone number
     * @tc.desc    Function test
     */
    it("abnormal_calllog_query_test_2800", 0, async function(done) {
        console.info("------------abnormal_calllog_query_test_2800 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(8);
        var phoneNumber_Test = phoneNumber.substring(0,3);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var calllogId = await dataShareHelper.insert(calllogUri, insertValues);
            console.info("logMessage abnormal_calllog_query_test_2800: calllogId = " + calllogId);
            expect(calllogId > 0).assertTrue();
            await QueryByStartsWithPhoneNumberError();
            await calllogDelete("abnormal_calllog_query_test_2800");
            done();
        } catch (error) {
            console.info("logMessage abnormal_calllog_query_test_2800: calllog insert error = " + error);
            done();
        }

        async function QueryByStartsWithPhoneNumberError()
        {
            var resultColumns = [ "id", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.like("phone_numbers", phoneNumber_Test + "%");
            try {
                let resultSet = await dataShareHelper.query(calllogUri, resultColumns, condition);
                console.info("logMessage abnormal_calllog_query_test_2800: resultSet.rowCount = " + resultSet.rowCount);
                expect(resultSet.rowCount == -1).assertTrue();
                resultSet.close();
            } catch (error) {
                console.info("logMessage abnormal_calllog_query_test_2800: query error = " + error);
                done();
            }
        }
    });

    afterAll(async function() {
        let dataShareHelper = dataShare.createDataShareHelper(URI_CALLLOG);
        console.info('callllog afterAll logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        let condition = new dataShare.DataSharePredicates();
        condition.greaterThan("id", "0");
        var deleteCode = await dataShareHelper.delete(calllogUri, condition);
        console.info('callllog afterAll end logMessage deleteCode = ' + deleteCode);
    });

    function randomNum (num) {
        let number = toString(Math.floor(Math.random() * (9 * Math.pow(10, num))) + (1 * Math.pow(10, num)));
        return number ;
    }
})
