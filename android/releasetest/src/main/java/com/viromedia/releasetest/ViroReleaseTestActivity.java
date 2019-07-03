/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest;

import android.content.Context;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;

import com.viro.core.RendererCloseListener;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import com.viro.core.ViroViewGVR;
import com.crashlytics.android.Crashlytics;
import com.viro.core.ViroViewOVR;
import com.viro.core.ViroViewScene;

import io.fabric.sdk.android.Fabric;
/**
 * Created by manish on 10/25/17.
 */

public class ViroReleaseTestActivity extends AppCompatActivity implements RendererCloseListener {
    private static final String TAG = ViroReleaseTestActivity.class.getSimpleName();

    private ViroView mViroView;
    private Handler mHandler;
    private ImageView mThumbsUp;
    private ImageView mThumbsDown;
    private boolean mGLInitialized = false;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        Fabric.with(this, new Crashlytics());

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            if (BuildConfig.VR_ENABLED == 1) {
                setContentView(R.layout.activity_main_gvr_vr_enabled);
            }
            if (BuildConfig.VR_ENABLED == 0) {
                setContentView(R.layout.activity_main_gvr_vr_disabled);
            }
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            setContentView(R.layout.activity_main_ovr);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            setContentView(R.layout.activity_main_arcore);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("Scene")) {
            setContentView(R.layout.activity_main_scene);
        }

        mViroView = (ViroView) findViewById(R.id.viro_view);

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            ((ViroViewGVR) mViroView).setStartupListener(new ViroViewGVR.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewGVR.StartupError error, String errorMessage) {

                }
            });
            ((ViroViewGVR) mViroView).setVRExitRunnable(() -> Log.d(TAG, "On GVR userRequested exit"));
            mViroView.setVRModeEnabled(BuildConfig.VR_ENABLED == 1);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            ((ViroViewOVR) mViroView).setStartupListener(new ViroViewOVR.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewOVR.StartupError error, String errorMessage) {

                }
            });

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            ((ViroViewARCore) mViroView).setStartupListener(new ViroViewARCore.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewARCore.StartupError error, String errorMessage) {

                }
            });

            ViroViewARCore arView = (ViroViewARCore) mViroView;
            Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
            arView.setCameraRotation(display.getRotation());

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("Scene")) {
            ((ViroViewScene) mViroView).setStartupListener(new ViroViewScene.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewScene.StartupError error, String errorMessage) {

                }
            });
        }

        if (BuildConfig.VR_ENABLED == 0) {
            mThumbsUp = (ImageView) findViewById(R.id.thumbsUp);
            mThumbsDown = (ImageView) findViewById(R.id.thumbsDown);
        }
        mHandler = new Handler(getMainLooper());
    }

    public View getThumbsUpView() {
        return mThumbsUp;
    }

    public View getThumbsDownView() {
        return mThumbsDown;
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

    public void onRendererStart() {
        Log.d(TAG, "onRendererStart called");
        mGLInitialized = true;
    }

    public Boolean isGlInitialized() {
        return mGLInitialized;
    }

    @Override
    public void onRendererClosed() {
        Log.d(TAG, "On GVR userRequested exit");

    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        if (mViroView instanceof ViroViewARCore) {
            ViroViewARCore arView = (ViroViewARCore) mViroView;
            Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
            arView.setCameraRotation(display.getRotation());
        }
    }
}
