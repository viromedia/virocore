/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest.tests;

import android.graphics.Color;
import android.support.test.rule.ActivityTestRule;

import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.Scene;
import com.viro.renderer.jni.ViroView;
import com.viromedia.renderertest.ViroReleaseTestActivity;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;

import java.util.TimerTask;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;
import java.util.Timer;

import static org.awaitility.Awaitility.await;

/**
 * Created by manish on 10/26/17.
 */

public abstract class ViroBaseTest {
    private static final String TAG = ViroBaseTest.class.getName();
    protected Scene mScene;
    protected Timer mTimer;
    protected ViroReleaseTestActivity mActivity;
    public ViroView mViroView;

    @Rule
    public ActivityTestRule<ViroReleaseTestActivity> mActivityTestRule
            = new ActivityTestRule<>(ViroReleaseTestActivity.class, true, true);

    @Before
    public void setUp() {
        mActivity = mActivityTestRule.getActivity();
        mViroView = mActivity.getViroView();
        mTimer = new Timer();


        await().until(glInitialized());
        mScene = createScene();

        mViroView.setScene(mScene);
        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                ViroBaseTest.this.callbackEverySecond();
            }
        }, 0, 1000);
    }

    private Callable<Boolean> glInitialized() {
        return new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mActivity.isGlInitialized();
            }
        };
    }
   Scene createScene() {
        Scene scene = new Scene();
        return scene;
    }


    void callbackEverySecond() {

    }

    @After
    public void tearDown() throws InterruptedException {
        synchronized (this) {
            TimeUnit.SECONDS.sleep(5);
        }

    }
}
