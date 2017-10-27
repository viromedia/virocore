/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest;

import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.viro.renderer.jni.GLListener;
import com.viro.renderer.jni.ViroGvrLayout;
import com.viro.renderer.jni.ViroOvrView;
import com.viro.renderer.jni.ViroView;
import com.viro.renderer.jni.ViroViewARCore;

/**
 * Created by manish on 10/25/17.
 */

public class ViroReleaseTestActivity extends AppCompatActivity implements GLListener {
    private static final String TAG = ViroReleaseTestActivity.class.getSimpleName();

    private ViroView mViroView;
    private Handler mHandler;
    private boolean mGLInitialized = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        System.out.println("onCreate called");
        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            mViroView = new ViroGvrLayout(this, this, new Runnable(){
                @Override
                public void run() {
                    Log.d(TAG, "On GVR userRequested exit");
                }
            });
            mViroView.setVrModeEnabled(false);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            mViroView = new ViroOvrView(this, this);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mViroView = new ViroViewARCore(this, this);
        }

        mViroView.validateApiKey("7EEDCB99-2C3B-4681-AE17-17BC165BF792");
        setContentView(mViroView.getContentView());

        mHandler = new Handler(getMainLooper());
        // uncomment the below line to test AR.
        //testEdgeDetect();
        //testFindTarget();
    }

    @Override
    protected void onStart(){
        super.onStart();
        mViroView.onActivityStarted(this);
    }

    @Override
    protected void onResume(){
        super.onResume();
        mViroView.onActivityResumed(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        mViroView.onActivityPaused(this);
    }

    @Override
    protected void onStop(){
        super.onStop();
        mViroView.onActivityStopped(this);
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mViroView.onActivityDestroyed(this);
    }

    public ViroView getViroView() {
        return mViroView;
    }

    @Override
    public void onGlInitialized() {
        mGLInitialized = true;
    }

    public Boolean isGlInitialized() {
        return mGLInitialized;
    }


}
