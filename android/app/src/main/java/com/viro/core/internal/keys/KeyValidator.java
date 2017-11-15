/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal.keys;

import android.content.Context;
import android.util.Log;

import com.amazonaws.auth.CognitoCachingCredentialsProvider;
import com.amazonaws.mobileconnectors.dynamodbv2.dynamodbmapper.DynamoDBMapper;
import com.amazonaws.regions.Region;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.dynamodbv2.AmazonDynamoDBClient;
import com.viro.renderer.BuildConfig;

import java.lang.ref.WeakReference;

/**
 * Utility class to validate api keys passed in SceneNavigator
 */

public class KeyValidator {

    private static final String TAG = "Viro";
    private WeakReference<Context> mContextWeakRef;
    private DynamoDBMapper mDynamoDBMapper;
    private AmazonDynamoDBClient mDynamoClient;

    private static int MAX_WAIT_INTERVAL_MILLIS = 64000;

    public KeyValidator(Context context) {
        mContextWeakRef = new WeakReference<Context>(context);
        CognitoCachingCredentialsProvider credentialsProvider = new CognitoCachingCredentialsProvider(
                context,
                BuildConfig.COGNITO_IDENTITY_POOL_ID,
                Regions.US_WEST_2
        );

        mDynamoClient = new AmazonDynamoDBClient(credentialsProvider);
        mDynamoClient.setRegion(Region.getRegion(Regions.US_WEST_2));
        mDynamoDBMapper = new DynamoDBMapper(mDynamoClient);

    }

    /**
     * Validates the apiKey from Dynamo tables. Calls {@link KeyValidationListener#onResponse(boolean)}
     * based on response from Dynamo.
     * @param apiKey
     * @param listener
     */
    public void validateKey(final String apiKey, final String vrPlatform, final KeyValidationListener listener) {
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                int retries = 0;
                boolean retry;
                do {
                    retry = false;
                    try {
                        ApiKey keyFromDynamo = mDynamoDBMapper.load(ApiKey.class, apiKey);
                        // Our dynamodb table stores valid as String true or false. As a result, objectmapper loads it as String
                        if (keyFromDynamo != null && keyFromDynamo.getValid().equalsIgnoreCase("true")) {
                            listener.onResponse(true);
                            Context context = mContextWeakRef.get();
                            if(context != null) {
                                KeyMetricsRecorder recorder = new KeyMetricsRecorder(mDynamoClient, context);
                                recorder.record(apiKey, vrPlatform);
                            }
                        } else {
                            listener.onResponse(false);
                            Log.i(TAG, "The given API Key is either missing or invalid! If you " +
                                    "have not signed up for accessing Viro Media platform, please" +
                                    " do so at www.viromedia.com. Otherwise, contact " +
                                    "info@viromedia.com if you have a valid key and are " +
                                    "encountering this error.");
                        }
                    } catch (Exception e) {
                        retry = true;
                    }
                    if (retry == true) {
                        retries++;
                        long waitTime = getWaitTimeExp(retries);
                        Log.d(TAG, "Attempt #" + retries + " to fetch api keys failed." +
                                " Retrying in " + waitTime + " milliseconds");
                        try {
                            Thread.sleep(waitTime);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                } while (retry);
            }
        };
        Thread dynamoThread = new Thread(runnable);
        dynamoThread.start();
    }

    /**
     * Returns the next wait interval, in milliseconds, using an exponential
     * backoff algorithm.
     */
    private long getWaitTimeExp(int retryCount) {
        long waitTime = ((long) Math.pow(2, retryCount) * 1000L);
        return Math.min(waitTime, MAX_WAIT_INTERVAL_MILLIS);
    }
}
