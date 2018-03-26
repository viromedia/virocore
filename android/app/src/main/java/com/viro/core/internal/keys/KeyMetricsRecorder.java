/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal.keys;


import android.content.Context;
import android.util.Log;

import com.amazonaws.services.dynamodbv2.AmazonDynamoDBClient;
import com.amazonaws.services.dynamodbv2.model.AttributeAction;
import com.amazonaws.services.dynamodbv2.model.AttributeValue;
import com.amazonaws.services.dynamodbv2.model.AttributeValueUpdate;
import com.amazonaws.services.dynamodbv2.model.UpdateItemRequest;
import com.viro.core.ViroView;
import com.viro.core.internal.BuildInfo;
import com.viro.metrics.android.UniqueID;
import com.viro.metrics.android.ViroAndroidKeenClientBuilder;
import com.viro.metrics.java.ViroKeenClient;
import com.viro.metrics.java.ViroKeenLogging;
import com.viro.metrics.java.ViroKeenProject;
import com.viro.renderer.BuildConfig;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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
    private String mPackageName;
    private boolean isDebug;
    private Context mContext;
    private static final String TEST_BED_PACKAGE_NAME = "com.viromedia.viromedia";

    // Play Store (And Galaxy App Store) installer package names
    private static final String GOOGLE_PLAY_1 = "com.android.vending";
    private static final String GOOGLE_PLAY_2 = "com.google.android.feedback";
    private static final String GALAXY_STORE = "com.sec.android.app.samsungapps";
    private static final List<String> APP_STORES = new ArrayList<>(Arrays.asList(
            GOOGLE_PLAY_1,GOOGLE_PLAY_2,GALAXY_STORE));



    public KeyMetricsRecorder(AmazonDynamoDBClient client, Context context) {
        mDynamoClient = client;
        mPackageName = BuildInfo.getPackageName(context);
        isDebug = isDebugMetrics(context);
        mContext = context;
    }

    private static boolean isDebugMetrics(Context context) {
        // The package name of the app that installed this app
        final String installer = context.getPackageManager()
                .getInstallerPackageName(context.getPackageName());
        final String appPackageName = context.getPackageName();

        // Return true if the app is our own testbed or
        // negation -(if the installer has been downloaded from Play Store)
        return appPackageName.equalsIgnoreCase(TEST_BED_PACKAGE_NAME) ||
                !(installer != null && APP_STORES.contains(installer));
    }

    public void record(String key, String viewType, String platform) {
        /**
         * Keen IO Metrics recording
         */

        ViroKeenClient client = new ViroAndroidKeenClientBuilder(mContext).build();
        ViroKeenClient.initialize(client);
        ViroKeenProject project = new ViroKeenProject("5ab1966bc9e77c0001b45ba0", "715EDB702D9AD29A56E127F08864DBCED277F35D946AA4DD2D4BD712B2356CA9E2E17276E74A8B69E44A720D0F92AF3A0D81CAF61404ABA069EE794C3FBFC19F5D66CB32B192B0B43AEA6CA61CED029E564237DB7452DDDC6955103CFAC11320", null);
        client.setDefaultProject(project);

//        ViroKeenLogging.enableLogging();
//        client.setDebugMode(true);
        Map<String, Object> event = new HashMap<>();
        event.put("event", "renderer_init");
        event.put("app_id", mPackageName);
        event.put("os", "android");
        event.put("instance_id", UniqueID.id(mContext));
        event.put("build_type", isDebugMetrics(mContext) ? "debug" : "release");
        event.put("viro_product", BuildConfig.VIRO_PLATFORM);
        event.put("platform", platform);
        event.put("view_type", viewType);
        client.addEventAsync("ViroViewInit", event);

        /**
         * DynamoDB Metrics recording below
         */
        HashMap<String, AttributeValue> keyMap = new HashMap<>();
        // Add the primary key
        AttributeValue primaryKeyValue = new AttributeValue().withS(getDynamoKey(key, viewType));
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
        if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase(ViroView.PLATFORM_VIRO_CORE)) {
            // don't reuse FLAVOR_VIRO_CORE because our delimiter is an underscore...
            builder.append("virocore").append(DELIMITER);
        } else {
            builder.append("android").append(DELIMITER);
        }
        // Add the VR platform
        builder.append(vrPlatform).append(DELIMITER);
        // Add the Android package name
        builder.append(mPackageName).append(DELIMITER);
        // Add the build type (debug|release);
        builder.append(isDebug ? "debug" : "release");

        return builder.toString();
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
