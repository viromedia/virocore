/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.keys;

import com.amazonaws.mobileconnectors.dynamodbv2.dynamodbmapper.DynamoDBAttribute;
import com.amazonaws.mobileconnectors.dynamodbv2.dynamodbmapper.DynamoDBHashKey;
import com.amazonaws.mobileconnectors.dynamodbv2.dynamodbmapper.DynamoDBTable;

/**
 * DynamoDb mapping class for table ApiKeys_Alpha
 */
@DynamoDBTable(tableName = "ApiKeys_Alpha")
public class ApiKey {
    private String apiKey;
    private String valid;
    @DynamoDBHashKey(attributeName = "ApiKey")
    public String getApiKey() {
        return apiKey;
    }

    public void setApiKey(String apiKey) {
        this.apiKey = apiKey;
    }

    /**
     * Note: Our dynamodb table stores valid as String true or false.
     * As a result, objectmapper loads it as String
     * @return String (true or false)
     */
    @DynamoDBAttribute(attributeName = "Valid")
    public String getValid() {
        return valid;
    }

    public void setValid(String valid) {
        this.valid = valid;
    }
}
