/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest;

import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.viro.core.RendererCloseListener;
import com.viro.core.RendererStartListener;
import com.viro.core.ViroView;
import com.viro.core.ViroViewGVR;

/**
 * Created by manish on 10/25/17.
 */

public class ViroReleaseTestActivity extends AppCompatActivity implements RendererStartListener, RendererCloseListener {
    private static final String TAG = ViroReleaseTestActivity.class.getSimpleName();

    private ViroViewGVR mViroView;
    private Handler mHandler;
    private boolean mGLInitialized = false;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        System.out.println("onCreate called");
//        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
//            mViroView = new ViroViewGVR(this, this, new Runnable() {
//                @Override
//                public void run() {
//                    Log.d(TAG, "On GVR userRequested exit");
//                }
//            });
//            mViroView.setVRModeEnabled(true);
//        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
//            mViroView = new ViroViewOVR(this, this);
//        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
//            mViroView = new ViroViewARCore(this, this);
//        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("Scene")) {
//            mViroView = new ViroViewScene(this, this);
//        }

        setContentView(R.layout.activity_main);
        mViroView = (ViroViewGVR) findViewById(R.id.viro_view);
        mViroView.setRenderStartListener(this);
        mViroView.setVRExitRunnable(() -> Log.d(TAG, "On GVR userRequested exit"));
        mHandler = new Handler(getMainLooper());
        // uncomment the below line to test AR.
        //testEdgeDetect();
        //testFindTarget();
    }

    @Override
    protected void onStart() {
        Log.i(TAG, "onStart");
        super.onStart();
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
        mViroView.onActivityStarted(this);
        mViroView.onActivityResumed(this);
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
        mViroView.onActivityPaused(this);
        mViroView.onActivityStopped(this);
        mViroView.onActivityDestroyed(this);
        mViroView.dispose();
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "onStop");
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "onDestroy");
        super.onDestroy();
    }

    public ViroView getViroView() {
        return mViroView;
    }

    @Override
    public void onRendererStart() {
        mGLInitialized = true;
    }

    public Boolean isGlInitialized() {
        return mGLInitialized;
    }

    @Override
    public void onRendererClosed() {
        Log.d(TAG, "On GVR userRequested exit");

    }
}
