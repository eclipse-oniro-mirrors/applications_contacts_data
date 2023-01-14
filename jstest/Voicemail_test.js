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
import common from './common.js';

const URI_VOICEMAIL = "datashare:///com.ohos.voicemailability";
const voicemailUri = "datashare:///com.ohos.voicemailability/calls/voicemail";

describe('VoicemailTest', function() {
    console.log("----------VoicemailTest is starting!----------");

    async function voicemailQuery(map, tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info(tag + ':calllogInsertQuery start ! dataShareHelper = ' + dataShareHelper);
        var resultColumns = [];
        let condition = new dataShare.DataSharePredicates();
        condition.equalTo("id", map.get("id"));
        try {
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            if (resultSet.rowCount > 0) {
                if (resultSet.goToFirstRow()) {
                    do {
                        for (var [key, value] of map) {
                            let dbresult = resultSet.getString(resultSet.getColumnIndex(key));
                            console.info(tag + ' :logMessage voicemailQuery key = ' + key + ' dbresult :' + dbresult +
                                         ' value : ' + value);
                            expect(value == dbresult).assertTrue();
                        }
                    } while (resultSet.goToNextRow());
                }
            }
            resultSet.close();
        } catch (error) {
            console.info(tag + ":logMessage voicemailQuery: error = " + error);
        }
    }

    async function voicemailQueryForDelete(map, tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info(tag + ': voicemailQueryForDelete start ! dataShareHelper = ' + dataShareHelper);
        let resultColumns = common.getCallLogResultColumns();
        let condition = new dataShare.DataSharePredicates();
        condition.equalTo("id", map.get("id"));
        try {
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            expect(resultSet.goToFirstRow() == false).assertTrue();
            console.info(tag + " :logMessage voicemailQueryForDelete: goToFirstRow " + resultSet.goToFirstRow());
            resultSet.close();
        } catch (error) {
            console.info(tag + " :logMessage voicemailQueryForDelete: error = " + error);
        }
    }

    async function voicemailQueryError(map, tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info(tag + ': voicemailQueryError start ! dataShareHelper = ' + dataShareHelper);
        let resultColumns = common.getCallLogResultColumns();
        let condition = new dataShare.DataSharePredicates();
        condition.equalTo("phone_number", map.get("phone_number"));
        try {
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            expect(resultSet.goToFirstRow() == false).assertTrue();
            console.info(tag + " :logMessage voicemailQueryError: goToFirstRow " + resultSet.goToFirstRow());
            resultSet.close();
        } catch (error) {
            console.info(tag + " :logMessage voicemailQueryError: error = " + error);
        }
    }

    async function voicemailQueryForBatchInsert(array, tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info(tag + ': voicemailQueryForBatchInsert start ! dataShareHelper = ' + dataShareHelper);
        let resultColumns = common.getCallLogResultColumns();
        let condition = new dataShare.DataSharePredicates();
        condition.equalTo("phone_number", array[0].get("phone_number"));
        try {
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            var size = array.size();
            console.info(tag + ' : logMessage voicemailQueryForBatchInsert: size' + size);
            expect(resultSet.rowCount == size).assertEqual(true);
            var i = 0;
            if (resultSet.rowCount > 0) {
                if (resultSet.goToFirstRow()) {
                    do {
                        for (var [key, value] of array[i]) {
                            let dbresult = resultSet.getString(resultSet.getColumnIndex(key));
                            console.info(tag + ' : logMessage voicemailQueryForBatchInsert dbresult :' + dbresult +
                                         ' value : ' + value);
                            console.info(
                                tag + ' : logMessage voicemailQueryForBatchInsert value ' + (value == dbresult));
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

    async function voiceMailDelete(tag)
    {
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        let condition = new dataShare.DataSharePredicates();
        condition.greaterThan("id", "0");
        var deleteCode = await dataShareHelper.delete(voicemailUri, condition);
        console.info(tag + ': voiceMailDelete deleteCode = ' + deleteCode);
        expect(deleteCode == 0).assertTrue();
    }

    /**
     * @tc.number  voicemail_insert_test_100
     * @tc.name    Insert voicemail data
     * @tc.desc    Function test
     */
    it("voicemail_insert_test_100", 0, async function(done) {
        console.info("---------logMessage voicemail_insert_test_100 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, common.getVoiceMail());
            console.info("logMessage voicemail_insert_test_100: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            var map = common.getVoiceMailMap();
            map.set("id", voicemailId.toString());
            await voicemailQuery(map, "voicemail_insert_test_100");
            await voiceMailDelete("voicemail_insert_test_100");
            done();
        } catch (error) {
            console.info("logMessage voicemail_insert_test_100: voicemail insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_insert_test_200
     * @tc.name    Insert voicemail status
     * @tc.desc    Function test
     */
    it("voicemail_insert_test_200", 0, async function(done) {
        console.info("---------logMessage voicemail_insert_test_200 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        var phoneNumber = randomNum(6);
        var stringValue = {"phone_number" : phoneNumber, "voice_status" : 1};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, stringValue)
            console.info("logMessage voicemail_insert_test_200: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            var map = new Map();
            map.set("id", voicemailId.toString());
            map.set("phone_number", phoneNumber);
            map.set("voice_status", "1");
            await voicemailQuery(map, "voicemail_insert_test_200");
            await voiceMailDelete("voicemail_insert_test_200");
            done();
        } catch (error) {
            console.info("logMessage voicemail_insert_test_200: voicemail insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_insert_test_100000
     * @tc.name    Insert voicemail status
     * @tc.desc    Function test
     */
    it("voicemail_insert_test_100000", 0, async function(done) {
        console.info("---------logMessage voicemail_insert_test_100000 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        var phoneNumber = randomNum(6);
        var stringValue = {"phone_number" : phoneNumber, "voice_status" : 0};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, stringValue)
            console.info("logMessage voicemail_insert_test_100000: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            var map = new Map();
            map.set("id", voicemailId.toString());
            map.set("phone_number", phoneNumber);
            map.set("voice_status", "0");
            await voicemailQuery(map, "voicemail_insert_test_100000");
            await voiceMailDelete("voicemail_insert_test_100000");
            done();
        } catch (error) {
            console.info("logMessage voicemail_insert_test_100000: voicemail insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_update_test_1000
     * @tc.name    Update voicemail data
     * @tc.desc    Function test
     */
    it("voicemail_update_test_1000", 0, async function(done) {
        console.info("------------logMessage voicemail_update_test_1000 is starting!----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);

        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, common.getVoiceMail());
            console.info("logMessage voicemail_update_test_1000: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await VoicemailUpdate();
            await voiceMailDelete("voicemail_update_test_1000");
            done();
        } catch (error) {
            console.info("logMessage voicemail_update_test_1000: voicemail insert error = " + error);
            done();
        }

        async function VoicemailUpdate()
        {
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            try {
                var updateCode = await dataShareHelper.update(voicemailUri, common.getVoiceMailUpdate(), condition);
                console.info("logMessage voicemail_update_test_1000: updateCode = " + updateCode);
                expect(updateCode == 0).assertTrue();
                var map = common.getVoiceMailUpdateMap();
                map.set("id", voicemailId.toString());
                await voicemailQuery(map, "voicemail_update_test_1000");
            } catch (error) {
                console.info("logMessage voicemail_update_test_1000: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  voicemail_update_test_900
     * @tc.name    Update voicemail status
     * @tc.desc    Function test
     */
    it("voicemail_update_test_900", 0, async function(done) {
        console.info("------------logMessage voicemail_update_test_900 is starting!----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(7);
        var insertValues = {"phone_number" : phoneNumber, "voice_status" : 0};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage voicemail_update_test_900: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await VoicemailUpdate();
            await voiceMailDelete("voicemail_update_test_900");
            done();
        } catch (error) {
            console.info("logMessage voicemail_update_test_900: voicemail insert error = " + error);
            done();
        }

        async function VoicemailUpdate()
        {
            var updateValues = {"voice_status" : 1};
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            try {
                var updateCode = await dataShareHelper.update(voicemailUri, updateValues, condition);
                console.info("logMessage voicemail_update_test_900: updateCode = " + updateCode);
                expect(updateCode == 0).assertTrue();
                var map = new Map();
                map.set("id", voicemailId.toString());
                map.set("voice_status", "1");
                await voicemailQuery(map, "voicemail_update_test_900");
            } catch (error) {
                console.info("logMessage voicemail_update_test_900: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  voicemail_delete_test_500
     * @tc.name    Delete voicemail data
     * @tc.desc    Function test
     */
    it("voicemail_delete_test_500", 0, async function(done) {
        console.info("-------------logMessage voicemail_delete_test_500 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(8);
        var insertValues = {"phone_number" : phoneNumber, "display_name" : "zhangming"};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage voicemail_delete_test_500: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await VoicemailDelete();
            await voiceMailDelete("voicemail_delete_test_500");
            done();
        } catch (error) {
            console.info("logMessage voicemail_delete_test_500: voicemail insert error = " + error);
            done();
        }

        async function VoicemailDelete()
        {
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            try {
                var deleteCode = await dataShareHelper.delete(voicemailUri, condition);
                console.info('logMessage voicemail_delete_test_500: deleteCode = ' + deleteCode);
                expect(deleteCode == 0).assertTrue();
                var map = new Map();
                map.set("id", voicemailId.toString());
                await voicemailQueryForDelete(map, "voicemail_delete_test_500");
            } catch (error) {
                console.info("logMessage voicemail_delete_test_500: delete error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  voicemail_update_test_600
     * @tc.name    Delete voicemail status
     * @tc.desc    Function test
     */
    it("voicemail_update_test_600", 0, async function(done) {
        console.info("-------------logMessage voicemail_update_test_600 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(8);
        var insertValues1 = {"phone_number" : phoneNumber, "voice_status" : 1};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues1);
            console.info("logMessage voicemail_update_test_600: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await VoicemailUpdate();
            await voiceMailDelete("voicemail_update_test_600");
            done();
        } catch (error) {
            console.info("logMessage voicemail_update_test_600: voicemail insert error = " + error);
            done();
        }

        async function VoicemailUpdate()
        {
            var updateValues = {"voice_status" : 0};
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            try {
                var updateCode = await dataShareHelper.update(voicemailUri, updateValues, condition);
                console.info('logMessage voicemail_update_test_600: deleteCode = ' + updateCode);
                expect(updateCode == 0).assertTrue();
                var map = new Map();
                map.set("id", voicemailId.toString());
                map.set("voice_status", "0");
                await voicemailQuery(map, "voicemail_update_test_600");
            } catch (error) {
                console.info("logMessage voicemail_update_test_600: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  voicemail_batchInsert_test_1800
     * @tc.name    Insert voicemail data in batches
     * @tc.desc    Function test
     */
    it("voicemail_batchInsert_test_1800", 0, async function(done) {
        console.info("-------------logMessage voicemail_batchInsert_test_1800 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(4);
        var insertValuesOne = {"phone_number" : phoneNumber, "display_name" : "xiaoming"};
        var insertValuesTwo = {"phone_number" : phoneNumber, "display_name" : "xiaohei"};
        var insertValuesThree = {"phone_number" : phoneNumber, "display_name" : "xiaohong"};
        var insertValuesFour = {"phone_number" : phoneNumber, "display_name" : "xiaohei"};
        var insertValuesFive = {"phone_number" : phoneNumber, "display_name" : "xiaofen"};
        var listAddBluk = [];
        listAddBluk[0] = insertValuesOne;
        listAddBluk[1] = insertValuesTwo;
        listAddBluk[2] = insertValuesThree;
        listAddBluk[3] = insertValuesFour;
        listAddBluk[4] = insertValuesFive;
        try {
            var batchInsertCode = await dataShareHelper.batchInsert(voicemailUri, listAddBluk);
            console.info("logMessage voicemail_batchInsert_test_1800: batchInsertCode = " + batchInsertCode);
            expect(batchInsertCode == 0).assertTrue();
            await voiceMailDelete("voicemail_batchInsert_test_1800");
            done();
        } catch (error) {
            console.info("logMessage voicemail_batchInsert_test_1800: batchInsert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_batchInsert_test_1700
     * @tc.name    Insert voicemail status in batches
     * @tc.desc    Function test
     */
    it("voicemail_batchInsert_test_1700", 0, async function(done) {
        console.info("-------------logMessage voicemail_batchInsert_test_1700 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(3);
        var insertValuesOne = {"phone_number" : phoneNumber, "display_name" : "xiaoming", "voice_status" : 0};
        var insertValuesTwo = {"phone_number" : phoneNumber, "display_name" : "xiaohei", "voice_status" : 1};
        var insertValuesThree = {"phone_number" : phoneNumber, "display_name" : "xiaohong", "voice_status" : 0};
        var insertValuesFour = {"phone_number" : phoneNumber, "display_name" : "xiaohei", "voice_status" : 0};
        var insertValuesFive = {"phone_number" : phoneNumber, "display_name" : "xiaofen", "voice_status" : 1};
        var listAddBluk = [];
        listAddBluk[0] = insertValuesOne;
        listAddBluk[1] = insertValuesTwo;
        listAddBluk[2] = insertValuesThree;
        listAddBluk[3] = insertValuesFour;
        listAddBluk[4] = insertValuesFive;
        try {
            var batchInsertCode = await dataShareHelper.batchInsert(voicemailUri, listAddBluk);
            console.info("logMessage voicemail_batchInsert_test_1700: batchInsertCode = " + batchInsertCode);
            expect(batchInsertCode == 0).assertTrue();
            var array = common.getVoiceMailBatchInsert();
            await voicemailQueryForBatchInsert(array, "voicemail_batchInsert_test_1700");
            await voiceMailDelete("voicemail_batchInsert_test_1700");
            done();
        } catch (error) {
            console.info("logMessage voicemail_batchInsert_test_1700: batchInsert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_query_test_1400
     * @tc.name    Query voicemail data
     * @tc.desc    Function test
     */
    it("voicemail_query_test_1400", 0, async function(done) {
        console.info("------------voicemail_query_test_1400 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(9);
        var insertValues = {"phone_number" : phoneNumber, "display_name" : "daming"};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage voicemail_query_test_1400: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await VoicemailQuery();
            await voiceMailDelete("voicemail_query_test_1400");
            done();
        } catch (error) {
            console.info("logMessage voicemail_query_test_1400: voicemial insert error = " + error);
            done();
        }
        async function VoicemailQuery()
        {
            var resultColumns = [ "id", "display_name", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            try {
                var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
                if (resultSet.goToFirstRow()) {
                    do {
                        console.info('voicemail_query_test_1400 id = ' + resultSet.getString(0));
                        expect(resultSet.getString(0) == voicemailId.toString()).assertTrue();
                        console.info('voicemail_query_test_1400 display_name = ' + resultSet.getString(1));
                        expect(resultSet.getString(1) == "daming").assertTrue();
                        console.info('voicemail_query_test_1400 phone_number = ' + resultSet.getString(2));
                        expect(resultSet.getString(2) == phoneNumber).assertTrue();
                    } while (resultSet.goToNextRow());
                }
                resultSet.close();
            } catch (error) {
                console.info("voicemail_query_test_1400 query error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  voicemail_query_test_1300
     * @tc.name    Query voicemail's status
     * @tc.desc    Function test
     */
    it("voicemail_query_test_1300", 0, async function(done) {
        console.info("------------voicemail_query_test_1300  is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(9);
        var insertValues = {"phone_number" : phoneNumber, "display_name" : "xiaoming", "voice_status" : 1};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage voicemail_query_test_1300: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await VoicemailQuery();
            await voiceMailDelete("voicemail_query_test_1300");
            done();
        } catch (error) {
            console.info("logMessage voicemail_query_test_1300: voicemial insert error = " + error);
            done();
        }
        async function VoicemailQuery()
        {
            var resultColumns = [ "id", "display_name", "phone_number" ];
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("voice_status", "1");
            condition.and();
            condition.equalTo("phone_number", phoneNumber);
            try {
                var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
                if (resultSet.goToFirstRow()) {
                    do {
                        console.info('voicemail_query_test_1300 id = ' + resultSet.getString(0));
                        expect(resultSet.getString(0) == voicemailId.toString()).assertTrue();
                        console.info('voicemail_query_test_1300 display_name = ' + resultSet.getString(1));
                        expect(resultSet.getString(1) == "xiaoming").assertTrue();
                        console.info('voicemail_query_test_1300 phone_number = ' + resultSet.getString(2));
                        expect(resultSet.getString(2) == phoneNumber).assertTrue();
                    } while (resultSet.goToNextRow());
                }
                resultSet.close();
            } catch (error) {
                console.info("voicemail_query_test_1300 query error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_voicemail_insert_test_300
     * @tc.name    Abnormal use case, an incorrect field is passed in when inserting data
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_insert_test_300", 0, async function(done) {
        console.info("------------abnormal_voicemail_insert_test_300 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(10);
        var insertValues = {"phone_numbers" : phoneNumber, "display_name" : "xiaoming"};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage abnormal_voicemail_insert_test_300: voicemialId = " + voicemailId);
            expect(voicemailId == -1).assertTrue();
            done();
        } catch (error) {
            console.info("abnormal_voicemail_insert_test_300 insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  abnormal_voicemail_insert_test_400
     * @tc.name    Exception use case, an incorrect table name is passed in when inserting data
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_insert_test_400", 0, async function(done) {
        console.info("------------abnormal_voicemail_insert_test_400 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var errorUri = "datashare:///com.ohos.calllogability/calls/voicemails";
        
        var phoneNumber = randomNum(9);
        var insertValues = {"phone_number" : phoneNumber, "display_name" : "xiaoming"};
        try {
            var voicemialId = await dataShareHelper.insert(errorUri, insertValues)
            console.info("logMessage abnormal_voicemail_insert_test_400: voicemialId = " + voicemialId);
            expect(voicemialId == -1).assertTrue();
            var map = new Map();
            map.set("phone_number", phoneNumber);
            await voicemailQueryError(map, "abnormal_voicemail_insert_test_400");
            done();
        } catch (error) {
            console.info("abnormal_voicemail_insert_test_400 insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  abnormal_voicemail_update_test_1100
     * @tc.name    Abnormal use case, passing in a wrong field when updating data
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_update_test_1100", 0, async function(done) {
        console.info("------------abnormal_voicemail_update_test_1100 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(10);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage abnormal_voicemail_update_test_1100: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await AbnormalUpdate();
            await voiceMailDelete("abnormal_voicemail_update_test_1100");
            done();
        } catch (error) {
            console.info("abnormal_voicemail_update_test_1100 insert error = " + error);
            done();
        }
        async function AbnormalUpdate()
        {
            var phoneNumber_Test = randomNum(15);
            var updateValues = {"phone_numbers" : phoneNumber_Test};
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("ids", voicemailId.toString());
            try {
                var updataCode = await dataShareHelper.update(voicemailUri, updateValues, condition);
                console.info("logMessage abnormal_voicemail_update_test_1100: updataCode = " + updataCode);
                expect(updataCode == -1).assertTrue();
                var map = new Map();
                map.set("phone_number", phoneNumber_Test);
                await voicemailQueryError(map, "abnormal_voicemail_update_test_1100");
            } catch (error) {
                console.info("logMessage abnormal_voicemail_update_test_1100: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_voicemail_update_test_1200
     * @tc.name    Abnormal use case, an incorrect table name is passed in when updating data
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_update_test_1200", 0, async function(done) {
        console.info("------------abnormal_voicemail_update_test_1200 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var errorUri = "datashare:///com.ohos.calllogability/calls/voicemails";
        var phoneNumber = randomNum(6);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage abnormal_voicemail_update_test_1200: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await AbnormalUpdate();
            await voiceMailDelete("abnormal_voicemail_update_test_1200");
            done();
        } catch (error) {
            console.info("logMessage abnormal_voicemail_update_test_1200: insert error = " + error);
            done();
        }

        async function AbnormalUpdate()
        {
            var phoneNumber_Test = randomNum(6);
            var updateValues = {"phone_number" : phoneNumber_Test};
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            try {
                var updataCode = await dataShareHelper.update(errorUri, updateValues, condition)
                console.info("logMessage abnormal_voicemail_update_test_1200: updataCode = " + updataCode);
                expect(updataCode == -1).assertTrue();
                var map = new Map();
                map.set("phone_number", phoneNumber_Test);
                await voicemailQueryError(map, "abnormal_voicemail_update_test_1200");
            } catch (error) {
                console.info("logMessage abnormal_voicemail_update_test_1200: update error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_voicemail_delete_test_700
     * @tc.name    Abnormal use case, passing in a wrong field when deleting data
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_delete_test_700", 0, async function(done) {
        console.info("------------abnormal_voicemail_delete_test_700 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(6);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage abnormal_voicemail_delete_test_700: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await AbnormalDelete();
            await voiceMailDelete("abnormal_voicemail_delete_test_700");
            done();
        } catch (error) {
            console.info("logMessage abnormal_voicemail_delete_test_700: insert error = " + error);
            done();
        }

        async function AbnormalDelete()
        {
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("ids", voicemailId.toString());
            try {
                var deleteCode = await dataShareHelper.delete(voicemailUri, condition);
                console.info("logMessage abnormal_voicemail_delete_test_700: deleteCode = " + deleteCode);
                expect(deleteCode == -1).assertTrue();
            } catch (error) {
                console.info("logMessage abnormal_voicemail_delete_test_700: delete error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_voicemail_delete_test_800
     * @tc.name    Abnormal use case, passing in an incorrect table name when deleting data
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_delete_test_800", 0, async function(done) {
        console.info("------------abnormal_voicemail_delete_test_800 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var errorUri = "datashare:///com.ohos.calllogability/calls/voicemails";
        var phoneNumber = randomNum(6);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage abnormal_voicemail_delete_test_800: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await AbnormalDelete();
            await voiceMailDelete("abnormal_voicemail_delete_test_800");
            done();
        } catch (error) {
            console.info("logMessage abnormal_voicemail_delete_test_800: insert error = " + error);
            done();
        }

        async function AbnormalDelete()
        {
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            try {
                var deleteCode = await dataShareHelper.delete(errorUri, condition);
                console.info("logMessage abnormal_voicemail_delete_test_800: deleteCode = " + deleteCode);
                expect(deleteCode == -1).assertTrue();
            } catch (error) {
                console.info("logMessage abnormal_voicemail_delete_test_800: delete error = " + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_voicemail_query_test_1500
     * @tc.name    Abnormal use case, an incorrect field is passed in when querying
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_query_test_1500", 0, async function(done) {
        console.info("------------abnormal_voicemail_query_test_1500 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        var phoneNumber = randomNum(6);
        var insertValues = {"phone_number" : phoneNumber};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            console.info("logMessage abnormal_voicemail_query_test_1500: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            await AbnormalQuery();
            await voiceMailDelete("abnormal_voicemail_query_test_1500");
            done();
        } catch (error) {
            console.info("logMessage abnormal_voicemail_query_test_1500: insert error = " + error);
            done();
        }

        async function AbnormalQuery()
        {
            var resultColumns = [ "id", "phone_number" ];

            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("ids", voicemailId.toString());
            try {
                var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition)
                console.info('logMessage abnormal_voicemail_query_test_1500: goToFirstRow' + resultSet.goToFirstRow());
                expect(resultSet.goToFirstRow()).assertEqual(false);
                resultSet.close();
            } catch (error) {
                console.info('logMessage abnormal_voicemail_query_test_1500: query error = ' + error);
                done();
            }
        }
    });

    /**
     * @tc.number  abnormal_voicemail_batchinsert_test_1900
     * @tc.name    Abnormal use cases, when inserting data in batches, some of them failed, check the processing logic
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_batchinsert_test_1900", 0, async function(done) {
        console.info("--------logMessage abnormal_voicemail_batchinsert_test_1900 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
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
            var batchInsertCode = await dataShareHelper.batchInsert(voicemailUri, listAddBluk)
            console.info("logMessage abnormal_voicemail_batchinsert_test_1900: batchInsertCode = " + batchInsertCode);
            expect(batchInsertCode == -1).assertTrue();
            done();
        } catch (error) {
            console.info('logMessage abnormal_voicemail_batchinsert_test_1900: batchInsert error = ' + error);
            done();
        }
    });

    /**
     * @tc.number  voiceMail_delete_test_2000
     * @tc.name    batchDelete
     * @tc.desc    Function test
     */
    it("voiceMail_delete_test_2000", 0, async function(done) {
        console.info("--------logMessage voiceMail_delete_test_2000 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        try {
            var voiceMailId = await dataShareHelper.insert(voicemailUri, common.getVoiceMail());
            console.info("logMessage voiceMail_delete_test_2000: voiceMailId = " + voiceMailId);
            expect(voiceMailId > 0).assertTrue();
            var map = common.getVoiceMailMap()
            map.set("id", voiceMailId.toString());
            await voicemailQuery(map, "voiceMail_delete_test_2000");
            await executeBatch();
        } catch (error) {
            console.info("logMessage voiceMail_delete_test_2000: voiceMailId insert error = " + error);
            done();
        }
        async function executeBatch()
        {
            console.info("logMessage voiceMail_delete_test_2000:  executeBatch start ");
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voiceMailId.toString());
            dataShareHelper.executeBatch(URI_VOICEMAIL, [ {
                uri : voicemailUri,
                type : featureAbility.DataAbilityOperationType.TYPE_DELETE,
                predicates : condition,
                expectedCount : 0,
                PredicatesBackReferences : {},
                interrupted : true,
            } ],
                (error, data) => {
                    console.info("logMessage voiceMail_delete_test_2000: executeBatch data = " + JSON.stringify(data));
                    console.info("logMessage voiceMail_delete_test_2000: data_3 = " + data);
                    console.info("logMessage voiceMail_delete_test_2000:  data_1= " + data[0].count);
                    expect(data[0].count == 0).assertTrue();
                    done();
                });
        }
    });

    /**
     * @tc.number  voiceMail_update_test_2100
     * @tc.name    batchUpdate
     * @tc.desc    Function test
     */
    it("voiceMail_update_test_2100", 0, async function(done) {
        console.info("--------logMessage voiceMail_update_test_2100 is starting!------------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        try {
            var voiceMailId = await dataShareHelper.insert(voicemailUri, common.getVoiceMail());
            console.info("logMessage voiceMail_update_test_2100: voiceMailId = " + voiceMailId);
            expect(voiceMailId > 0).assertTrue();
            var map = common.getVoiceMailMap();
            map.set("id", voiceMailId.toString());
            await voicemailQuery(map, "voiceMail_update_test_2100");
            await executeBatch();
        } catch (error) {
            console.info("logMessage voiceMail_update_test_2100: calllog insert error = " + error);
            done();
        }
        async function executeBatch()
        {
            var phoneNumber = randomNum(8);
            var updateValues = {"phone_number" : phoneNumber};
            console.info("logMessage voiceMail_update_test_2100:  executeBatch start ");
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voiceMailId.toString());
            dataShareHelper.executeBatch(URI_VOICEMAIL, [ {
                uri : voicemailUri,
                type : featureAbility.DataAbilityOperationType.TYPE_UPDATE,
                valuesBucket : updateValues,
                predicates : condition,
                expectedCount : 0,
                PredicatesBackReferences : {},
                interrupted : true,
            } ],
                (error, data) => {
                    console.info("logMessage voiceMail_update_test_2100: executeBatch data = " + JSON.stringify(data));
                    console.info("logMessage voiceMail_update_test_2100: data_3 = " + data);
                    console.info("logMessage voiceMail_update_test_2100:  data_1= " + data[0].count);
                    expect(data[0].count == 0).assertTrue();
                    done();
                });
        }
    });

    /**
     * @tc.number  voicemail_insert_test_2200
     * @tc.name    Insert voicemail status 0
     * @tc.desc    Function test
     */
    it("voicemail_insert_test_2200", 0, async function(done) {
        console.info("---------logMessage voicemail_insert_test_2200 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        var phoneNumber = randomNum(15);
        var stringValue = {"phone_number" : phoneNumber, "voice_status" : 0};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, stringValue)
            console.info("logMessage voicemail_insert_test_2200: voicemailId = " + voicemailId);
            expect(voicemailId > 0).assertTrue();
            var map = new Map();
            map.set("id", voicemailId.toString());
            map.set("phone_number", phoneNumber);
            map.set("voice_status", "0");
            await voicemailQuery(map, "voicemail_insert_test_2200");
            await voiceMailDelete("voicemail_insert_test_2200");
            done();
        } catch (error) {
            console.info("logMessage voicemail_insert_test_2200: voicemail insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_insert_test_2300
     * @tc.name    Insert 3 pieces of voice mail data first, then delete 2 pieces of data,
     *             and verify whether the deletion is successful
     * @tc.desc    Function test
     */
    it("voicemail_insert_test_2300", 0, async function(done) {
        console.info("---------logMessage voicemail_insert_test_2300 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        var phoneNumber = randomNum(11);
        var stringValue = {"phone_number" : phoneNumber, "voice_status" : 0};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, stringValue)
            var voicemailIdOne = await dataShareHelper.insert(voicemailUri, stringValue)
            var voicemailIdTwo = await dataShareHelper.insert(voicemailUri, stringValue)
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            condition.or();
            condition.equalTo("id", voicemailIdOne.toString());
            var deleteCode = await dataShareHelper.delete(voicemailUri, condition);
            console.info("logMessage voicemail_insert_test_2300: deleteCode = " + deleteCode);
            var resultColumns = [];
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            expect(resultSet.rowCount == 0).assertTrue();
            resultSet.close();
            await voiceMailDelete("voicemail_insert_test_2300");
            done();
        } catch (error) {
            console.info("logMessage voicemail_insert_test_2300: voicemail insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_insert_test_2400
     * @tc.name    Insert 3 pieces of voice mail data first, then update 2 pieces of data,
     *             and verify whether the deletion is successful
     * @tc.desc    Function test
     */
    it("voicemail_insert_test_2400", 0, async function(done) {
        console.info("---------logMessage voicemail_insert_test_2400 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        var phoneNumber_Test = randomNum(8);
        var stringValue = {"phone_number" : phoneNumber_Test, "voice_status" : 0};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, stringValue)
            var voicemailIdOne = await dataShareHelper.insert(voicemailUri, stringValue)
            var voicemailIdTwo = await dataShareHelper.insert(voicemailUri, stringValue)
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            condition.or();
            condition.equalTo("id", voicemailIdOne.toString());
            var phoneNumber = randomNum(13);
            var updateValue = {"phone_number" : phoneNumber, "voice_status" : 1};
            var updateCode = await dataShareHelper.update(voicemailUri, updateValue, condition);
            console.info("logMessage voicemail_insert_test_2400: deleteCode = " + updateCode);

            var map = new Map();
            map.set("id", voicemailId.toString());
            map.set("phone_number", phoneNumber);
            map.set("voice_status", "1");
            await voicemailQuery(map, "voicemail_insert_test_2400");

            var mapOne = new Map();
            mapOne.set("id", voicemailIdOne.toString());
            mapOne.set("phone_number", phoneNumber);
            mapOne.set("voice_status", "1");
            await voicemailQuery(mapOne, "voicemail_insert_test_2400");
            await voiceMailDelete("voicemail_insert_test_2400");
            done();
        } catch (error) {
            console.info("logMessage voicemail_insert_test_2400: voicemail insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_insert_test_2500
     * @tc.name    Query the voice mailbox with status 0 or id and return the query result
     * @tc.desc    Function test
     */
    it("voicemail_query_test_2500", 0, async function(done) {
        console.info("---------logMessage voicemail_query_test_2500 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        var phoneNumber = randomNum(10);
        var stringValue = {"phone_number" : phoneNumber, "voice_status" : 0};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, stringValue)
            var voicemailIdOne = await dataShareHelper.insert(voicemailUri, stringValue)
            let condition = new dataShare.DataSharePredicates();
            condition.beginWrap();
            condition.equalTo("id", voicemailId.toString());
            condition.or();
            condition.equalTo("id", voicemailIdOne.toString());
            condition.endWrap();
            condition.and();
            condition.equalTo("voice_status", "0");
            var resultColumns = [];
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            expect(resultSet.rowCount == 2).assertTrue();
            resultSet.close();
            await voiceMailDelete("voicemail_query_test_2500");
            done();
        } catch (error) {
            console.info("logMessage voicemail_query_test_2500: voicemail insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  voicemail_query_test_2600
     * @tc.name    Query the voice mailbox and return the query result
     * @tc.desc    Function test
     */
    it("voicemail_query_test_2600", 0, async function(done) {
        console.info("---------logMessage voicemail_query_test_2600 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        var phoneNumber = randomNum(12);
        var stringValue = {"phone_number" : phoneNumber, "voice_status" : 0};
        try {
            var voicemailId = await dataShareHelper.insert(voicemailUri, stringValue);
            console.info('voicemail_query_test_2600 voicemailId = ' + voicemailId);
            var voicemailIdOne = await dataShareHelper.insert(voicemailUri, stringValue);
            console.info('voicemail_query_test_2600  voicemailIdOne = ' + voicemailIdOne);
            let condition = new dataShare.DataSharePredicates();
            var resultColumns = [];
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            expect(resultSet.rowCount == 2).assertTrue();
            resultSet.close();
            await voiceMailDelete("voicemail_query_test_2600");
            done();
        } catch (error) {
            console.info("logMessage voicemail_query_test_2600: voicemail insert error = " + error);
            done();
        }
    });

    /**
     * @tc.number  abnormal_voicemail_insert_test_2700
     * @tc.name    When the voicemail status is added, a non-existent field is passed in
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_insert_test_2700", 0, async function(done) {
        console.info("---------logMessage abnormal_voicemail_insert_test_2700 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        try {
            var insertValues = {display_name : "2700name", voice_statuss : "1"};
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            expect(voicemailId == -1).assertTrue();
            let condition = new dataShare.DataSharePredicates();
            var resultColumns = [];
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            expect(resultSet.rowCount == 0).assertTrue();
            await voiceMailDelete("abnormal_voicemail_insert_test_2700");
            done();
        } catch (error) {
            done();
        }
    });

    /**
     * @tc.number  abnormal_voicemail_delete_test_2800
     * @tc.name    When deleting the voicemail status, pass in a non-existent field
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_delete_test_2800", 0, async function(done) {
        console.info("---------logMessage abnormal_voicemail_delete_test_2800 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        try {
            var insertValues = {display_name : "2800name", voice_status : "1"};
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            expect(voicemailId > 0).assertTrue();
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            var updateValues = {display_name : "2800name", voice_statuss : "0"};
            var updateCode = await dataShareHelper.update(voicemailUri, updateValues, condition);
            expect(updateCode == -1).assertTrue();
            var map = new Map();
            map.set("id", voicemailId.toString());
            map.set("display_name", "2800name");
            map.set("voice_status", "1");
            await voicemailQuery(map, "abnormal_voicemail_delete_test_2800");
            await voiceMailDelete("abnormal_voicemail_delete_test_2800");
            done();
        } catch (error) {
            done();
        }
    });

    /**
     * @tc.number  abnormal_voicemail_update_test_2900
     * @tc.name    When update the voicemail status, pass in a non-existent field
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_update_test_2900", 0, async function(done) {
        console.info("---------logMessage abnormal_voicemail_update_test_2900 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        try {
            var insertValues = {display_name : "2900name", voice_status : "1"};
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            expect(voicemailId > 0).assertTrue();
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("id", voicemailId.toString());
            var updateValues = {display_name : "2900name", voice_statuss : "0"};
            var updateCode = await dataShareHelper.update(voicemailUri, updateValues, condition);
            expect(updateCode == -1).assertTrue();
            var map = new Map();
            map.set("id", voicemailId.toString());
            map.set("display_name", "2900name");
            map.set("voice_status", "1");
            await voicemailQuery(map, "abnormal_voicemail_update_test_2900");
            await voiceMailDelete("abnormal_voicemail_update_test_2900");
            done();
        } catch (error) {
            done();
        }
    });

    /**
     * @tc.number  abnormal_voicemail_update_test_3000
     * @tc.name    When update the voicemail status, pass in a non-existent field
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_update_test_3000", 0, async function(done) {
        console.info("---------logMessage abnormal_voicemail_update_test_3000 is starting!---------")
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        console.info('logMessage uri = ' + URI_VOICEMAIL)
        try {
            var insertValues = {display_name : "2900name", voice_status : "1"};
            var voicemailId = await dataShareHelper.insert(voicemailUri, insertValues);
            expect(voicemailId > 0).assertTrue();
            let condition = new dataShare.DataSharePredicates();
            condition.equalTo("ids", voicemailId.toString());
            var resultColumns = [];
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            expect(resultSet.rowCount == -1).assertTrue();
            resultSet.close();
            await voiceMailDelete("abnormal_voicemail_update_test_3000");
            done();
        } catch (error) {
            done();
        }
    });

    /**
     * @tc.number   abnormal_voicemail_batchInsert_test_3100
     * @tc.name    Insert voicemail data in batches , pass in a non-existent field
     * @tc.desc    Function test
     */
    it("abnormal_voicemail_batchInsert_test_3100", 0, async function(done) {
        console.info("-------------logMessage abnormal_voicemail_batchInsert_test_3100 is starting!-----------");
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);

        var insertValuesOne = {"voice_status" : "0", "display_name" : "xiaoming"};
        var insertValuesTwo = {"voice_status" : "1", "display_name" : "xiaohei"};
        var insertValuesThree = {"voice_statusss" : "1", "display_name" : "xiaohong"};
        var insertValuesFour = {"voice_status" : "0", "display_name" : "xiaohei"};
        var insertValuesFive = {"voice_status" : "0", "display_name" : "xiaofen"};
        var listAddBluk = [];
        listAddBluk[0] = insertValuesOne;
        listAddBluk[1] = insertValuesTwo;
        listAddBluk[2] = insertValuesThree;
        listAddBluk[3] = insertValuesFour;
        listAddBluk[4] = insertValuesFive;
        try {
            var batchInsertCode = await dataShareHelper.batchInsert(voicemailUri, listAddBluk);
            console.info("logMessage abnormal_voicemail_batchInsert_test_3100: batchInsertCode = " + batchInsertCode);
            expect(batchInsertCode == -1).assertTrue();
            let condition = new dataShare.DataSharePredicates();
            var resultColumns = [];
            var resultSet = await dataShareHelper.query(voicemailUri, resultColumns, condition);
            expect(resultSet.rowCount == 0).assertTrue();
            resultSet.close();
            done();
        } catch (error) {
            console.info("logMessage abnormal_voicemail_batchInsert_test_3100: batchInsert error = " + error);
            done();
        }
    });

    afterAll(async function() {
        let dataShareHelper = dataShare.createDataShareHelper(URI_VOICEMAIL);
        console.info('voicemai afterAll logMessage get dataShareHelper success! dataShareHelper = ' + dataShareHelper);
        let condition = new dataShare.DataSharePredicates();
        condition.greaterThan("id", "0");
        var deleteCode = await dataShareHelper.delete(voicemailUri, condition);
        console.info('voicemai afterAll logMessage deleteCode = ' + deleteCode);
    });
})

function randomNum (num) {
    let number = toString(Math.floor(Math.random() * (9 * Math.pow(10, num))) + (1 * Math.pow(10, num)));
    return number ;
}
