/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viromedia.releasetest;

import android.content.Context;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;

import com.viro.core.RendererCloseListener;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import com.viro.core.ViroViewGVR;
import com.viro.core.ViroViewScene;

/**
 * Created by manish on 10/25/17.
 */

public class ViroReleaseTestActivity extends AppCompatActivity implements RendererCloseListener {
    private static final String TAG = ViroReleaseTestActivity.class.getSimpleName();

    private ViroView mViroView;
    private ImageView mThumbsUp;
    private ImageView mThumbsDown;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);

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

        if (BuildConfig.VR_ENABLED == 0) {
            mThumbsUp = (ImageView) findViewById(R.id.thumbsUp);
            mThumbsDown = (ImageView) findViewById(R.id.thumbsDown);
        }
    }

    public void startRenderer() {
        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            ((ViroViewGVR) mViroView).startTests();
            ((ViroViewGVR) mViroView).setVRExitRunnable(() -> Log.d(TAG, "On GVR userRequested exit"));
            mViroView.setVRModeEnabled(BuildConfig.VR_ENABLED == 1);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            ((ViroViewARCore) mViroView).startTests();

            ViroViewARCore arView = (ViroViewARCore) mViroView;
            Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
            arView.setCameraRotation(display.getRotation());

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("Scene")) {
            ((ViroViewScene) mViroView).startTests();
        }
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
