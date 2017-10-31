/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
LICENSE file in the
 * root directory of this source tree. An additional grant
of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest.tests;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.support.test.rule.ActivityTestRule;
import android.util.Log;

import com.viro.renderer.ARHitTestResult;
import com.viro.renderer.jni.EventDelegate;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Scene;
import com.viro.renderer.jni.Surface;
import com.viro.renderer.jni.Text;
import com.viro.renderer.jni.Texture;
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

import java.io.IOException;
import java.io.InputStream;
import java.util.EnumSet;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

import static junit.framework.Assert.assertTrue;
import static junit.framework.Assert.fail;
import static org.awaitility.Awaitility.await;

/**
 * Created by manish on 10/26/17.
 */

public abstract class ViroBaseTest {
    private static final String TAG = ViroBaseTest.class.getName();
    private static final String TEST_PASSED = "testPassed";
    private static final String TEST_FAILED = "testFailed";
    private final AtomicBoolean mTestButtonsClicked = new AtomicBoolean(false);
    public ViroView mViroView;
    @Rule
    public ActivityTestRule<ViroReleaseTestActivity> mActivityTestRule
            = new ActivityTestRule<>(ViroReleaseTestActivity.class, true, true);
    protected MutableTestMethod mMutableTestMethod;
    protected Timer mTimer;
    protected Scene mScene;
    protected ViroReleaseTestActivity mActivity;
    private Node mTestClassNameNode;
    private Node mTestMethodNameNode;
    private Node mExpectedMessageNode;

    @Before
    public void setUp() {
        mActivity = mActivityTestRule.getActivity();
        mViroView = mActivity.getViroView();
        mTimer = new Timer();

        await().until(glInitialized());

        mScene = new Scene();
        createBaseTestScene();
        configureTestScene();
        mViroView.setScene(mScene);

        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                callbackEverySecond(mMutableTestMethod);
            }
        }, 0, 1000);
    }

    private Callable<Boolean> glInitialized() {
        return () -> mActivity.isGlInitialized();
    }

    private void createBaseTestScene() {
        Node rootNode = mScene.getRootNode();
        EnumSet<Node.TransformBehavior> transformBehavior = EnumSet.of(Node.TransformBehavior.BILLBOARD_Y);

        // Add yes button
        Node yesButton = new Node();
        Bitmap yesBitmap = getBitmapFromAsset(mActivity, "icon_thumb_up.png");
        Texture yesTexture = new Texture(yesBitmap,
                Texture.TextureFormat.RGBA8, true, true);
        Material yesMaterial = new Material();
        Surface yesSurface = new Surface(2, 2, 0, 0, 1, 1);
        yesSurface.setMaterial(yesMaterial);
        yesSurface.setImageTexture(yesTexture);
        yesButton.setGeometry(yesSurface);
        float[] yesPosition = {-1.5f, -3f, -3.3f};
        yesButton.setPosition(new Vector(yesPosition));
        yesButton.setTransformBehaviors(transformBehavior);
        yesButton.setEventDelegate(getGenericDelegate(TEST_PASSED));
        rootNode.addChildNode(yesButton);

        // Add no button
        Node noButton = new Node();
        Bitmap noBitmap = getBitmapFromAsset(mActivity, "icon_thumb_down.png");
        Texture noTexture = new Texture(noBitmap,
                Texture.TextureFormat.RGBA8, true, true);
        Material noMaterial = new Material();
        Surface noSurface = new Surface(2, 2, 0, 0, 1, 1);
        noSurface.setMaterial(noMaterial);
        noSurface.setImageTexture(noTexture);
        noButton.setGeometry(noSurface);
        float[] noPosition = {1.5f, -3f, -3.3f};
        noButton.setPosition(new Vector(noPosition));
        noButton.setTransformBehaviors(transformBehavior);
        noButton.setEventDelegate(getGenericDelegate(TEST_FAILED));
        rootNode.addChildNode(noButton);

        // Add class name
        mTestClassNameNode = new Node();
        Text testClassNameText = new Text(mViroView.getViroContext(), getClass().getSimpleName(),
                "Roboto", 25, Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        float[] classNamePosition = {-1.5f, 3f, -3.3f};
        mTestClassNameNode.setPosition(new Vector(classNamePosition));
        mTestClassNameNode.setGeometry(testClassNameText);
        rootNode.addChildNode(mTestClassNameNode);

        // Add method name
        mTestMethodNameNode = new Node();
        Text testMethodNameText = new Text(mViroView.getViroContext(),
                Thread.currentThread().getStackTrace()[1].getMethodName(),
                "Roboto", 25, Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        float[] methodNamePosition = {-1.5f, 2.5f, -3.3f};
        mTestMethodNameNode.setPosition(new Vector(methodNamePosition));
        mTestMethodNameNode.setGeometry(testMethodNameText);
        rootNode.addChildNode(mTestMethodNameNode);

        // Add expected message card
        mExpectedMessageNode = new Node();
        Text instructionCardText = new Text(mViroView.getViroContext(),
                "Test Text Here", "Roboto", 25, Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        float[] position = {-1.5f, 2f, -3.3f};
        mExpectedMessageNode.setPosition(new Vector(position));
        mExpectedMessageNode.setGeometry(instructionCardText);
        rootNode.addChildNode(mExpectedMessageNode);
    }

    protected void assertPass(String expectedMessage) {

        mTestButtonsClicked.set(false);

        Text methodNameText = (Text) mTestMethodNameNode.getGeometry();
        methodNameText.setText(Thread.currentThread().getStackTrace()[3].getMethodName());
        Text instructionCardText = (Text) mExpectedMessageNode.getGeometry();
        instructionCardText.setText(expectedMessage);


        await().atMost(30, TimeUnit.SECONDS).untilTrue(mTestButtonsClicked);
    }

    abstract void configureTestScene();

    void callbackEverySecond(MutableTestMethod testMethod) {
        if(testMethod == null) {
            return;
        }

        testMethod.mutableTest();
    }

    @After
    public void tearDown() throws InterruptedException {

    }

    private Bitmap getBitmapFromAsset(Context context, String filePath) {
        AssetManager assetManager = context.getAssets();

        InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = assetManager.open(filePath);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (IOException e) {
            System.err.println("Error loading image from assets");
        }

        return bitmap;
    }

    protected EventDelegate getGenericDelegate(String delegateTag) {
        EventDelegate delegateJni = new EventDelegate();
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_HOVER, false);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_FUSE, true);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_DRAG, true);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_CLICK, true);
        delegateJni.setEventDelegateCallback(new GenericEventCallback(delegateTag));

        return delegateJni;
    }

    public interface MutableTestMethod {
        public void mutableTest();
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

            if (clickState.equals(ClickState.CLICKED)) {
                if (delegateTag.equalsIgnoreCase(TEST_PASSED)) {
                    assertTrue(delegateTag.equalsIgnoreCase(TEST_PASSED));
                } else {
                    fail();
                }
                mTestButtonsClicked.set(true);
            }
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
