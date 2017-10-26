/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest.tests;

import android.support.test.rule.ActivityTestRule;

import com.viro.renderer.jni.Scene;
import com.viro.renderer.jni.ViroView;
import com.viromedia.renderertest.ViroReleaseTestActivity;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;

import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

import static org.awaitility.Awaitility.await;

/**
 * Created by manish on 10/26/17.
 */

public abstract class ViroBaseTest {
    private static final String TAG = ViroBaseTest.class.getName();
    private Scene mScene;
    protected ViroReleaseTestActivity mActivity;
    public ViroView mViroView;
    @Rule
    public ActivityTestRule<ViroReleaseTestActivity> mActivityTestRule
            = new ActivityTestRule<>(ViroReleaseTestActivity.class, true, true);

    @Before
    public void setUp() {
        mActivity = mActivityTestRule.getActivity();
        mViroView = mActivity.getViroView();

        await().until(glInitialized());
        Scene mScene = createScene();

        mViroView.setScene(mScene);
    }

    private Callable<Boolean> glInitialized() {
        return new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mActivity.isGlInitialized();
            }
        };
    }
    abstract Scene createScene();

    @After
    public void tearDown() throws InterruptedException {
        synchronized (this) {
            TimeUnit.SECONDS.sleep(5);
        }

    }
}
