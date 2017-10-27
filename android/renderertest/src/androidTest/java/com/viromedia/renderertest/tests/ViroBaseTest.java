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
import android.util.Log;

import com.viro.renderer.ARHitTestResult;
import com.viro.renderer.jni.EventDelegate;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Scene;
import com.viro.renderer.jni.Vector;
import com.viro.renderer.jni.ViroView;
import com.viro.renderer.jni.event.ClickState;
import com.viro.renderer.jni.event.ControllerStatus;
import com.viro.renderer.jni.event.PinchState;
import com.viro.renderer.jni.event.RotateState;
import com.viro.renderer.jni.event.SwipeState;
import com.viro.renderer.jni.event.TouchState;
import com.viromedia.renderertest.ViroReleaseTestActivity;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;

import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

import static org.awaitility.Awaitility.await;

/**
 * Created by manish on 10/26/17.
 */

public abstract class ViroBaseTest {
    private static final String TAG = ViroBaseTest.class.getName();
    public ViroView mViroView;
    @Rule
    public ActivityTestRule<ViroReleaseTestActivity> mActivityTestRule
            = new ActivityTestRule<>(ViroReleaseTestActivity.class, true, true);
    protected Timer mTimer;
    protected Scene mScene;
    protected ViroReleaseTestActivity mActivity;

    @Before
    public void setUp() {
        mActivity = mActivityTestRule.getActivity();
        mViroView = mActivity.getViroView();
        mTimer = new Timer();

        await().until(glInitialized());

        createBaseTestScene();
        configureTestScene();
        mViroView.setScene(mScene);
        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                callbackEverySecond();
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

    private void createBaseTestScene() {
        mScene = new Scene();
        // Add yes button
        // Add no button
        // Add instruction card
    }

    abstract void configureTestScene();


    abstract void callbackEverySecond();

    @After
    public void tearDown() throws InterruptedException {
        synchronized (this) {
            TimeUnit.SECONDS.sleep(10);
        }
    }

    private EventDelegate getGenericDelegate(String delegateTag) {
        EventDelegate delegateJni = new EventDelegate();
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_HOVER, false);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_FUSE, true);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_DRAG, true);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_CLICK, true);
        delegateJni.setEventDelegateCallback(new GenericEventCallback(delegateTag));

        return delegateJni;
    }

    private class GenericEventCallback implements EventDelegate.EventDelegateCallback {
        protected final String delegateTag;

        public GenericEventCallback(String tag) {
            delegateTag = tag;
        }

        @Override
        public void onHover(int source, Node node, boolean isHovering, float[] hitLoc) {
            Log.e(TAG, delegateTag + " onHover " + isHovering);
        }

        @Override
        public void onClick(int source, Node node, ClickState clickState, float[] hitLoc) {
            Log.e(TAG, delegateTag + " onClick " + clickState.toString() + " location " +
                    hitLoc[0] + ", " + hitLoc[1] + ", " + hitLoc[2]);
        }

        @Override
        public void onTouch(int source, Node node, TouchState touchState, float[] touchPadPos) {
            Log.e(TAG, delegateTag + "onTouch " + touchPadPos[0] + "," + touchPadPos[1]);
        }

        @Override
        public void onControllerStatus(int source, ControllerStatus status) {

        }

        @Override
        public void onSwipe(int source, Node node, SwipeState swipeState) {
            Log.e(TAG, delegateTag + " onSwipe " + swipeState.toString());
        }

        @Override
        public void onScroll(int source, Node node, float x, float y) {
            Log.e(TAG, delegateTag + " onScroll " + x + "," + y);

        }

        @Override
        public void onDrag(int source, Node node, float x, float y, float z) {
            Log.e(TAG, delegateTag + " On drag: " + x + ", " + y + ", " + z);

            Vector converted = node.convertLocalPositionToWorldSpace(new Vector(x, y, z));
            if (node.getParentNode() != null) {
                converted = node.getParentNode().convertLocalPositionToWorldSpace(new Vector(x, y, z));
                Log.e(TAG, delegateTag + " On CONV: " + converted.x + ", " + converted.y + ", " + converted.z);
            }
        }

        @Override
        public void onFuse(int source, Node node) {
            Log.e(TAG, delegateTag + " On fuse");
        }

        @Override
        public void onPinch(int source, Node node, float scaleFactor, PinchState pinchState) {
            Log.e(TAG, delegateTag + " On pinch");
        }

        @Override
        public void onRotate(int source, Node node, float rotateFactor, RotateState rotateState) {
            Log.e(TAG, delegateTag + " On rotate");
        }

        @Override
        public void onCameraARHitTest(int source, ARHitTestResult[] results) {
            Log.e(TAG, delegateTag + " On Camera AR Hit Test");
        }
    }
}
