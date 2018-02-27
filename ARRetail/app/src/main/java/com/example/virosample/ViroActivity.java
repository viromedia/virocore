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
import android.os.Handler;
import android.util.Log;

import com.viro.core.RendererConfiguration;

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
 * Similar to {@link ProductARActivityComplete}, simply extend and
 * override onRendererStart() to start building your 3D scenes.
 */
public class ViroActivity extends Activity {
    private static final String TAG = ViroActivity.class.getSimpleName();
    protected ViroView mViroView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase("GVR")) {
            mViroView = createGVRView();

        } else if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase("OVR")) {
            mViroView = createOVRView();

        } else if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase("Scene")) {
            mViroView = createViroViewScene();
        } else if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase("ARCore")) {
            mViroView = createViroARCoreScene();
        }
        setContentView(mViroView);
    }

    private ViroView createGVRView() {
        ViroViewGVR viroView = new ViroViewGVR(this, new ViroViewGVR.StartupListener() {
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
        });
        return viroView;
    }

    private ViroView createOVRView() {
        ViroViewOVR viroView = new ViroViewOVR(this, new ViroViewOVR.StartupListener() {
            @Override
            public void onSuccess() {
                onRendererStart();
            }

            @Override
            public void onFailure(ViroViewOVR.StartupError error, String errorMessage) {
                onRendererFailed(error.toString(), errorMessage);
            }
        });
        return viroView;
    }

    private ViroView createViroViewScene() {
        ViroViewScene viroView = new ViroViewScene(this, new ViroViewScene.StartupListener() {
            @Override
            public void onSuccess() {
                onRendererStart();
            }

            @Override
            public void onFailure(ViroViewScene.StartupError error, String errorMessage) {
                onRendererFailed(error.toString(), errorMessage);
            }
        });
        return viroView;
    }

    private ViroView createViroARCoreScene() {
        RendererConfiguration config = new RendererConfiguration();
        config.setShadowsEnabled(true);
        config.setBloomEnabled(true);
        config.setHDREnabled(true);
        config.setPBREnabled(true);
        ViroViewARCore viroView = new ViroViewARCore(this, new ViroViewARCore.StartupListener() {
            @Override
            public void onSuccess() {
                onRendererStart();
            }

            @Override
            public void onFailure(ViroViewARCore.StartupError error, String errorMessage) {
                onRendererFailed(error.toString(), errorMessage);
            }
        }, config);
        return viroView;
    }

    public void onRendererStart() {
        // Override this function to start building your scene here!
    }

    public void onRendererFailed(String error, String errorMessage) {
        // Fail as you wish!
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
