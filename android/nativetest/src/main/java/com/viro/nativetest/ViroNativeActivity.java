/**
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
package com.viro.nativetest;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Color;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.Toast;

import com.viro.core.RendererConfiguration;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import com.viro.core.ViroViewGVR;
import com.viro.core.ViroViewOVR;
import com.viro.core.ViroViewScene;
import com.viromedia.nativetest.BuildConfig;

public class ViroNativeActivity extends AppCompatActivity {

    private static final String TAG = ViroNativeActivity.class.getSimpleName();
    private ViroView mViroView = null;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        RendererConfiguration config = new RendererConfiguration();
        config.setShadowsEnabled(true);
        config.setBloomEnabled(true);
        config.setHDREnabled(true);
        config.setPBREnabled(true);

        ViroView.NATIVE_TESTING_MODE = true;

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            mViroView = new ViroViewGVR(this, new ViroViewGVR.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewGVR.StartupError error, String errorMessage) {
                    onRendererFailed(error.toString(), errorMessage);
                }
            }, new Runnable() {
                @Override
                public void run() {
                    Log.e(TAG, "On GVR userRequested exit");
                }
            }, config);
            setContentView(mViroView);

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            mViroView = new ViroViewOVR(this, new ViroViewOVR.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewOVR.StartupError error, String errorMessage) {
                    onRendererFailed(error.toString(), errorMessage);
                }
            }, config);
            setContentView(mViroView);

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("Scene")) {
            mViroView = new ViroViewScene(this, new ViroViewScene.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewScene.StartupError error, String errorMessage) {
                    onRendererFailed(error.toString(), errorMessage);
                }
            }, config);


            mViroView.setPadding(60, 60, 60, 60);
            mViroView.setBackgroundColor(Color.argb(0, 0, 0, 0));

            FrameLayout frameLayout = new FrameLayout(this);
            frameLayout.addView(mViroView);
            frameLayout.setBackgroundColor(Color.BLUE);

            setContentView(frameLayout);

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mViroView = new ViroViewARCore(this, new ViroViewARCore.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewARCore.StartupError error, String errorMessage) {
                    onRendererFailed(error.toString(), errorMessage);
                }
            }, config);
            setContentView(mViroView);

            ViroViewARCore arView = (ViroViewARCore) mViroView;
            Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
            arView.setCameraRotation(display.getRotation());
        }
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

    @Override
    protected void onStart() {
        super.onStart();
        if (mViroView != null) {
            mViroView.onActivityStarted(this);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mViroView != null) {
            mViroView.onActivityResumed(this);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mViroView != null) {
            mViroView.onActivityPaused(this);
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mViroView != null) {
            mViroView.onActivityStopped(this);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mViroView != null) {
            mViroView.onActivityDestroyed(this);
        }
    }

    private void onRendererStart() {
        Log.e("Viro", "onRendererStart called");

        mViroView.setVRModeEnabled(true);
        mViroView.setDebugHUDEnabled(true);
        mViroView.validateAPIKey("7EEDCB99-2C3B-4681-AE17-17BC165BF792");
    }

    private void onRendererFailed(String error, String errorMessage) {
        Log.e("Viro", "onRendererFailed [error: " + error + "], message [" + errorMessage + "]");
        Toast.makeText(this, errorMessage, Toast.LENGTH_LONG).show();
    }
}