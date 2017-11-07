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

package com.viromedia.releasetest.tests;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.viro.renderer.ARHitTestResult;
import com.viro.renderer.jni.ARScene;
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
import com.viromedia.releasetest.BuildConfig;
import com.viromedia.releasetest.ViroReleaseTestActivity;

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
import java.util.concurrent.atomic.AtomicInteger;

import static org.awaitility.Awaitility.await;
import static org.junit.Assert.assertEquals;

/**
 * Created by manish on 10/26/17.
 */

public abstract class ViroBaseTest {
    private static final String TAG = ViroBaseTest.class.getName();
    private static final String TEST_PASSED_TAG = "testPassed";
    private static final String TEST_FAILED_TAG = "testFailed";
    private static final Integer TEST_FAILED = 1;
    private static final Integer TEST_PASSED = 1;
    // TODO This is this large just for testing
    private static final Integer TEST_MAX_DURATION_SEC = 60;
    private final AtomicBoolean mTestButtonsClicked = new AtomicBoolean(false);
    private final AtomicInteger mTestResult = new AtomicInteger(-1);
    public ViroView mViroView;
    @Rule
    public ViroActivityTestRule<ViroReleaseTestActivity> mActivityTestRule
            = new ViroActivityTestRule(ViroReleaseTestActivity.class, true, true);
    protected MutableTestMethod mMutableTestMethod;
    protected Timer mTimer;
    protected Scene mScene;
    protected Node mYesButtonNode;
    protected Node mNoButtonNode;
    protected ViroReleaseTestActivity mActivity;
    private Node mTestClassNameNode;
    private Node mTestMethodNameNode;
    private Node mExpectedMessageNode;
    private GenericEventCallback callbackOne;
    private GenericEventCallback callbackTwo;

    @Before
    public void setUp() {
        mActivity = (ViroReleaseTestActivity) mActivityTestRule.getActivity();
        mViroView = mActivity.getViroView();
        mTimer = new Timer();
        await().until(glInitialized());

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mScene = new ARScene();
        } else {
            mScene = new Scene();
        }
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
        final Node rootNode = mScene.getRootNode();
        final EnumSet<Node.TransformBehavior> transformBehavior = EnumSet.of(Node.TransformBehavior.BILLBOARD_Y);

        // Add yes button
        mYesButtonNode = new Node();
        final Bitmap yesBitmap = getBitmapFromAssets(mActivity, "icon_thumb_up.png");
        final Texture yesTexture = new Texture(yesBitmap,
                Texture.TextureFormat.RGBA8, true, true);
        final Material yesMaterial = new Material();
        final Surface yesSurface = new Surface(2, 2, 0, 0, 1, 1);
        yesSurface.setMaterial(yesMaterial);
        yesSurface.setImageTexture(yesTexture);
        mYesButtonNode.setGeometry(yesSurface);
        final float[] yesPosition = {2.5f, -0.5f, -3.3f};
        mYesButtonNode.setPosition(new Vector(yesPosition));
        mYesButtonNode.setTransformBehaviors(transformBehavior);
        mYesButtonNode.setEventDelegate(getGenericDelegate(TEST_PASSED_TAG));
        rootNode.addChildNode(mYesButtonNode);

        // Add no button
        mNoButtonNode = new Node();
        final Bitmap noBitmap = getBitmapFromAssets(mActivity, "icon_thumb_down.png");
        final Texture noTexture = new Texture(noBitmap,
                Texture.TextureFormat.RGBA8, true, true);
        final Material noMaterial = new Material();
        final Surface noSurface = new Surface(2, 2, 0, 0, 1, 1);
        noSurface.setMaterial(noMaterial);
        noSurface.setImageTexture(noTexture);
        mNoButtonNode.setGeometry(noSurface);
        final float[] noPosition = {-2.5f, -0.5f, -3.3f};
        mNoButtonNode.setPosition(new Vector(noPosition));
        mNoButtonNode.setTransformBehaviors(transformBehavior);
        mNoButtonNode.setEventDelegate(getGenericDelegate(TEST_FAILED_TAG));
        rootNode.addChildNode(mNoButtonNode);

        // Add class name
        mTestClassNameNode = new Node();
        final Text testClassNameText = new Text(mViroView.getViroContext(), getClass().getSimpleName(),
                "Roboto", 25, Color.WHITE, 5f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final float[] classNamePosition = {0f, 3f, -3.3f};
        mTestClassNameNode.setPosition(new Vector(classNamePosition));
        mTestClassNameNode.setGeometry(testClassNameText);
        rootNode.addChildNode(mTestClassNameNode);

        // Add method name
        mTestMethodNameNode = new Node();
        final Text testMethodNameText = new Text(mViroView.getViroContext(),
                Thread.currentThread().getStackTrace()[1].getMethodName(),
                "Roboto", 25, Color.WHITE, 5f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final float[] methodNamePosition = {0f, 2.5f, -3.3f};
        mTestMethodNameNode.setPosition(new Vector(methodNamePosition));
        mTestMethodNameNode.setGeometry(testMethodNameText);
        rootNode.addChildNode(mTestMethodNameNode);

        // Add expected message card
        mExpectedMessageNode = new Node();
        final Text instructionCardText = new Text(mViroView.getViroContext(),
                "Test Text Here", "Roboto", 25, Color.WHITE, 5f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final float[] position = {0f, 2f, -3.3f};
        mExpectedMessageNode.setPosition(new Vector(position));
        mExpectedMessageNode.setGeometry(instructionCardText);
        rootNode.addChildNode(mExpectedMessageNode);
    }

    protected void assertPass(final String expectedMessage) {

        mTestButtonsClicked.set(false);
        mTestResult.set(-1);

        final Text methodNameText = (Text) mTestMethodNameNode.getGeometry();
        methodNameText.setText(Thread.currentThread().getStackTrace()[3].getMethodName());
        final Text instructionCardText = (Text) mExpectedMessageNode.getGeometry();
        instructionCardText.setText(expectedMessage);

        await().atMost(TEST_MAX_DURATION_SEC, TimeUnit.SECONDS).untilTrue(mTestButtonsClicked);
        assertEquals((long) TEST_PASSED, (long) mTestResult.get());
    }

    protected void assertPass(final String expectedMessage, final TestCleanUpMethod method) {
        assertPass(expectedMessage);

        if (method != null) {
            method.cleanUp();
        }
    }

    abstract void configureTestScene();

    void callbackEverySecond(final MutableTestMethod testMethod) {
        if (testMethod == null) {
            return;
        }

        testMethod.mutableTest();
    }

    @After
    public void tearDown() throws InterruptedException {

    }

    protected Bitmap getBitmapFromAssets(final Context context, final String filePath) {
        final AssetManager assetManager = context.getAssets();

        final InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = assetManager.open(filePath);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (final IOException e) {
            System.err.println("Error loading image from assets");
        }

        return bitmap;
    }

    protected EventDelegate getGenericDelegate(final String delegateTag) {
        final EventDelegate delegateJni = new EventDelegate();
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_HOVER, false);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_FUSE, true);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_DRAG, true);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_CLICK, true);
        if (delegateTag.equalsIgnoreCase(TEST_PASSED_TAG)) {
            callbackOne = new GenericEventCallback(delegateTag);
            delegateJni.setEventDelegateCallback(callbackOne);
        } else {
            callbackTwo = new GenericEventCallback(delegateTag);
            delegateJni.setEventDelegateCallback(callbackTwo);
        }


        return delegateJni;
    }

    public interface TestCleanUpMethod {
        void cleanUp();
    }

    public interface MutableTestMethod {
        void mutableTest();
    }

    private class GenericEventCallback implements EventDelegate.EventDelegateCallback {
        protected final String delegateTag;

        public GenericEventCallback(final String tag) {
            delegateTag = tag;
        }

        @Override
        public void onHover(final int source, final Node node, final boolean isHovering, final float[] hitLoc) {
            Log.e(TAG, delegateTag + " onHover " + isHovering);
        }

        @Override
        public void onClick(final int source, final Node node, final ClickState clickState, final float[] hitLoc) {
            Log.e(TAG, delegateTag + " onClick " + clickState.toString() + " location " +
                    hitLoc[0] + ", " + hitLoc[1] + ", " + hitLoc[2]);

            if (clickState.equals(ClickState.CLICKED)) {
                mTestResult.set(delegateTag.equalsIgnoreCase(TEST_PASSED_TAG) ? TEST_PASSED : TEST_FAILED);
                mTestButtonsClicked.set(true);
            }
        }

        @Override
        public void onTouch(final int source, final Node node, final TouchState touchState, final float[] touchPadPos) {
            Log.e(TAG, delegateTag + "onTouch " + touchPadPos[0] + "," + touchPadPos[1]);
        }

        @Override
        public void onControllerStatus(final int source, final ControllerStatus status) {

        }

        @Override
        public void onSwipe(final int source, final Node node, final SwipeState swipeState) {
            Log.e(TAG, delegateTag + " onSwipe " + swipeState.toString());
        }

        @Override
        public void onScroll(final int source, final Node node, final float x, final float y) {
            Log.e(TAG, delegateTag + " onScroll " + x + "," + y);

        }

        @Override
        public void onDrag(final int source, final Node node, final float x, final float y, final float z) {
            Log.e(TAG, delegateTag + " On drag: " + x + ", " + y + ", " + z);
            // TODO Differentiate between drag vs click if the user just wanted to move the object and not click it
            Vector converted = node.convertLocalPositionToWorldSpace(new Vector(x, y, z));
            if (node.getParentNode() != null) {
                converted = node.getParentNode().convertLocalPositionToWorldSpace(new Vector(x, y, z));
                Log.e(TAG, delegateTag + " On CONV: " + converted.x + ", " + converted.y + ", " + converted.z);
            }
        }

        @Override
        public void onFuse(final int source, final Node node) {
            Log.e(TAG, delegateTag + " On fuse");
        }

        @Override
        public void onPinch(final int source, final Node node, final float scaleFactor, final PinchState pinchState) {
            Log.e(TAG, delegateTag + " On pinch");
        }

        @Override
        public void onRotate(final int source, final Node node, final float rotateFactor, final RotateState rotateState) {
            Log.e(TAG, delegateTag + " On rotate");
        }

        @Override
        public void onCameraARHitTest(final int source, final ARHitTestResult[] results) {
            Log.e(TAG, delegateTag + " On Camera AR Hit Test");
        }
    }

    // TODO: Remove UI-Threaded patch once VIRO-2162 has been implemented.
    protected void runOnUiThread(Runnable runnable){
        AtomicBoolean finishedTest = new AtomicBoolean(false);
        Runnable myRunnable = new Runnable() {
            @Override
            public void run() {
                runnable.run();
                finishedTest.set(true);
            }
        };
        new Handler(Looper.getMainLooper()).post(myRunnable);
        await().until(() -> finishedTest.get());
    }
}
