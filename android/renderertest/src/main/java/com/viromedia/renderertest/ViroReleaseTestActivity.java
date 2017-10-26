/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.viro.renderer.ARHitTestResult;
import com.viro.renderer.jni.EventDelegate;
import com.viro.renderer.jni.GLListener;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Scene;
import com.viro.renderer.jni.ViroGvrLayout;
import com.viro.renderer.jni.ViroOvrView;
import com.viro.renderer.jni.ViroView;
import com.viro.renderer.jni.ViroViewARCore;
import com.viro.renderer.jni.event.ClickState;
import com.viro.renderer.jni.event.ControllerStatus;
import com.viro.renderer.jni.event.PinchState;
import com.viro.renderer.jni.event.RotateState;
import com.viro.renderer.jni.event.SwipeState;
import com.viro.renderer.jni.event.TouchState;

/**
 * Created by manish on 10/25/17.
 */

public class ViroReleaseTestActivity extends AppCompatActivity implements GLListener {
    private static String TAG = ViroReleaseTestActivity.class.getSimpleName();

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
       return this.mViroView;
    }

    public EventDelegate getGenericDelegate(final String delegateTag){
        EventDelegate delegateJni = new EventDelegate();
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_HOVER, false);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_FUSE, true);

        delegateJni.setEventDelegateCallback(new GenericEventCallback(delegateTag));

        return delegateJni;
    }

    @Override
    public void onGlInitialized() {
        mGLInitialized = true;
    }

    public Boolean isGlInitialized() {
        return mGLInitialized;
    }
    public class GenericEventCallback implements EventDelegate.EventDelegateCallback {
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
            Log.e(TAG, delegateTag + " onClick " + clickState.toString());
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
            Log.e(TAG, delegateTag + " onScroll " + x + "," +y);

        }

        @Override
        public void onDrag(int source, Node node, float x, float y, float z) {
            Log.e(TAG, delegateTag +" On drag: " + x + ", " + y + ", " + z);
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
