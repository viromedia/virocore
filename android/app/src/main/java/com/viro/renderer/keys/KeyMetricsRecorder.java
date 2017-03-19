/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.keys;


import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.util.Log;

import com.amazonaws.services.dynamodbv2.AmazonDynamoDBClient;
import com.amazonaws.services.dynamodbv2.model.AttributeAction;
import com.amazonaws.services.dynamodbv2.model.AttributeValue;
import com.amazonaws.services.dynamodbv2.model.AttributeValueUpdate;
import com.amazonaws.services.dynamodbv2.model.UpdateItemRequest;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.HashMap;

public class KeyMetricsRecorder {
    private static final String DELIMITER = "_";
    private static final String TAG = "Viro";
    private static final int MAX_WAIT_INTERVAL_MILLIS = 64000;

    // Dynamo table strings
    private static final String METRICS_TABLE_NAME = "ApiKey_Metrics_Alpha";
    private static final String METRICS_TABLE_PRIMARY_KEY = "ApiKey_BundleId_BuildType";
    private static final String METRICS_TABLE_SORT_KEY = "Date";
    private static final String METRICS_TABLE_COUNT_ATTR = "Count";

    private final AmazonDynamoDBClient mDynamoClient;
    private final Context mContext;

    public KeyMetricsRecorder(AmazonDynamoDBClient client, Context context) {
        mDynamoClient = client;
        mContext = context;
    }

    public void record(String key, String vrPlatform) {
        HashMap<String, AttributeValue> keyMap = new HashMap<>();
        // Add the primary key
        AttributeValue primaryKeyValue = new AttributeValue().withS(getDynamoKey(key, vrPlatform));
        keyMap.put(METRICS_TABLE_PRIMARY_KEY, primaryKeyValue);
        // Add the sort key (date)
        AttributeValue sortKeyValue = new AttributeValue().withS(getDate());
        keyMap.put(METRICS_TABLE_SORT_KEY, sortKeyValue);

        final UpdateItemRequest updateRequest = new UpdateItemRequest()
                .withTableName(METRICS_TABLE_NAME)
                .withKey(keyMap)
                .addAttributeUpdatesEntry(
                        METRICS_TABLE_COUNT_ATTR, new AttributeValueUpdate()
                                    .withAction(AttributeAction.ADD)
                                    .withValue(new AttributeValue().withN("1"))

                );

        Runnable updateItemRunnable = new Runnable() {
            @Override
            public void run() {
                int retryCount = 0;
                // Only try 10 times.
                while (retryCount <= 10) {
                    long waitTime = getWaitTimeExp(retryCount);
                    Log.d(TAG, "Attempt #" + retryCount + ", performing metrics recording in "
                            + getWaitTimeExp(retryCount) + " milliseconds");

                    try {
                        Thread.sleep(waitTime);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    try {
                        mDynamoClient.updateItem(updateRequest);
                        // Update item was successful so break the loop.
                        break;
                    } catch (Exception e) {
                        retryCount++;
                    }
                }
            }
        };
        Thread recordThread = new Thread(updateItemRunnable);
        recordThread.start();
    }

    /**
     * This function creates & returns the Dynamo key that we expect, in the form:
     *    ApiKey_OS_VrPlatform_BundleId_BuildType
     */
    private String getDynamoKey(String key, String vrPlatform) {

        StringBuilder builder = new StringBuilder();
        // Add the API key
        builder.append(key).append(DELIMITER);
        // Add the OS
        builder.append("android").append(DELIMITER);
        // Add the VR platform
        builder.append(vrPlatform).append(DELIMITER);
        // Add the Android package name
        builder.append(mContext.getApplicationContext().getPackageName()).append(DELIMITER);
        // Add the build type (debug|release);
        builder.append(isDebug() ? "debug" : "release");
        return builder.toString();
    }

    private boolean isDebug() {
        boolean debug = false;
        try {
            debug = (mContext.getPackageManager().getPackageInfo(
                    mContext.getPackageName(), 0).applicationInfo.flags &
                    ApplicationInfo.FLAG_DEBUGGABLE) != 0;
        } catch (PackageManager.NameNotFoundException e) {
            Log.w(TAG, "Couldn't find the debuggable flag, assuming release");
        }
        return debug;
    }

    /**
     * This function returns today's date in the format yyyyMMdd
     */
    private String getDate() {
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMdd");
        return dateFormat.format(Calendar.getInstance().getTime());
    }

    /**
     * Returns the next wait interval, in milliseconds, using an exponential
     * backoff algorithm.
     */
    private long getWaitTimeExp(int retryCount) {
        if (retryCount == 0) {
            return 0;
        }
        long waitTime = ((long) Math.pow(2, retryCount) * 1000L);
        return Math.min(waitTime, MAX_WAIT_INTERVAL_MILLIS);
    }
}
