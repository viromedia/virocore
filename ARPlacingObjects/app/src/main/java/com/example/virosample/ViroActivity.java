/*
 * Copyright (c) 2017-present, Viro, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.virosample;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.viro.core.RendererStartListener;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import com.viro.core.ViroViewGVR;
import com.viro.core.ViroViewOVR;
import com.viro.core.ViroViewScene;

/**
 * A sample base Android activity containing the ViroView renderer.
 * <p>
 * This activity automatically handles the creation of an AR or VR renderer based on
 * the currently selected build variant in Android studio, and it's lifecycle requirements.
 * <p>
 * Similar to {@link ViroARPlanesDemoActivity}, simply extend and
 * override onRendererStart() to start building your 3D scenes.
 */
public class ViroActivity extends Activity implements RendererStartListener{
    private static final String TAG = ViroActivity.class.getSimpleName();
    protected ViroView mViroView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase("GVR")) {
            mViroView = new ViroViewGVR(this, this, new Runnable() {
                @Override
                public void run() {
                    // Handle existing GVR Here
                    Log.d(TAG, "On GVR userRequested exit");
                }
            });
            mViroView.setVRModeEnabled(true);
        } else if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase("OVR")) {
            mViroView = new ViroViewOVR(this, this);
        } else if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase("ARCore")) {
            mViroView = new ViroViewARCore(this, this);
        } else if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase("Scene")) {
            mViroView = new ViroViewScene(this, this);
        }

        setContentView(mViroView);
    }

    @Override
    public void onRendererStart() {
        // Override this function to start building your scene here!
    }

    @Override
    protected void onStart() {
        super.onStart();
        mViroView.onActivityStarted(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mViroView.onActivityResumed(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        mViroView.onActivityPaused(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        mViroView.onActivityStopped(this);
    }
}
