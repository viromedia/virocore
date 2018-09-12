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
import android.view.View;
import android.widget.ImageView;

import com.viro.core.ARHitTestResult;
import com.viro.core.ARPointCloud;
import com.viro.core.ARScene;
import com.viro.core.EventDelegate;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Scene;
import com.viro.core.Surface;
import com.viro.core.Text;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.ViroView;
import com.viro.core.ClickState;
import com.viro.core.ControllerStatus;
import com.viro.core.PinchState;
import com.viro.core.RotateState;
import com.viro.core.SwipeState;
import com.viro.core.TouchState;
import com.viromedia.releasetest.BuildConfig;
import com.viromedia.releasetest.ViroReleaseTestActivity;

import org.junit.Before;
import org.junit.Rule;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.EnumSet;
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
    private static final String TAG = "Viro";
    private static final String TEST_PASSED_TAG = "testPassed";
    private static final String TEST_FAILED_TAG = "testFailed";
    private static final Integer TEST_FAILED = -1;
    private static final Integer TEST_PASSED = 1;
    // TODO This is this large just for testing
    private static final Integer TEST_MAX_DURATION_SEC = 120;
    private final AtomicBoolean mTestButtonsClicked = new AtomicBoolean(false);
    private final AtomicInteger mTestResult = new AtomicInteger(-1);
    public ViroView mViroView;
    @Rule
    public ViroActivityTestRule<ViroReleaseTestActivity> mActivityTestRule
            = new ViroActivityTestRule(ViroReleaseTestActivity.class, true, true);
    protected MutableTestMethod mMutableTestMethod;
    protected Scene mScene;
    protected Node mYesButtonNode;
    protected Node mNoButtonNode;
    protected Node mExpectedMessageNode;
    protected ViroReleaseTestActivity mActivity;
    private Node mTestClassNameNode;
    private Node mTestMethodNameNode;
    private GenericEventCallback callbackOne;
    private GenericEventCallback callbackTwo;
    private Handler mTestThreadHander;
    private Handler mUIThreadHandler;
    private TestCleanUpMethod mLastCleanupMethod;

    @Before
    public void setUp() {
        Log.i(TAG, "Setting up test");

        mActivity = (ViroReleaseTestActivity) mActivityTestRule.getActivity();
        mViroView = mActivity.getViroView();
        await().until(glInitialized());

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mScene = new ARScene();
        } else {
            mScene = new Scene();
        }

        Looper.prepare();
        mTestThreadHander = new Handler();
        mUIThreadHandler = new Handler(Looper.getMainLooper());

        Runnable setup = new Runnable() {
            @Override
            public void run() {
                createBaseTestScene();
                configureTestScene();
                mViroView.setScene(mScene);
            }
        };
        mUIThreadHandler.post(setup);

        mUIThreadHandler.postDelayed(new Runnable(){
            public void run() {
                callbackEverySecond(mMutableTestMethod);
                mUIThreadHandler.postDelayed(this, 1000);
            }
        }, 1000);
    }

    private Callable<Boolean> glInitialized() {
        Log.d(TAG, "glInitialized called - " + mActivity.isGlInitialized());
        return () -> mActivity.isGlInitialized();
    }

    private void createBaseTestScene() {
        final Node rootNode = mScene.getRootNode();
        final EnumSet<Node.TransformBehavior> transformBehavior = EnumSet.of(Node.TransformBehavior.BILLBOARD_Y);

        if (BuildConfig.VR_ENABLED == 1) {
            addThumbButtonsInVR();
        } else {
            addThumbButtonsOnGlass();
        }

        int textSize = 18;
        float textX = 0.58f;
        float textY = 2.82f;
        float textZ = -3.3f;

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            textX = 1.35f;
            textY = .92f;
            textSize = 16;
        }

        // Add class name
        mTestClassNameNode = new Node();
        final Text testClassNameText = new Text(mViroView.getViroContext(), getClass().getSimpleName(),
                "Roboto", textSize, Color.WHITE, 5f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        mTestClassNameNode.setPosition(new Vector(textX, textY, textZ));
        mTestClassNameNode.setGeometry(testClassNameText);
        rootNode.addChildNode(mTestClassNameNode);

        mTestMethodNameNode = new Node();
        final Text testMethodNameText = new Text(mViroView.getViroContext(),
                "Loading...",
                "Roboto", textSize - 2, Color.WHITE, 5f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        mTestMethodNameNode.setPosition(new Vector(textX, textY - 0.25f, textZ));
        mTestMethodNameNode.setGeometry(testMethodNameText);
        rootNode.addChildNode(mTestMethodNameNode);

        // Add expected message card
        mExpectedMessageNode = new Node();
        final Text instructionCardText = new Text(mViroView.getViroContext(),
                "Loading...", "Roboto", 16,
                Color.GREEN, 2.55f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        mExpectedMessageNode.setPosition(new Vector(0.025f, -3.06f, textZ)); //1.0, -3.0
        mExpectedMessageNode.setGeometry(instructionCardText);
        rootNode.addChildNode(mExpectedMessageNode);
    }

    private void addThumbButtonsInVR() {
        final Node rootNode = mScene.getRootNode();
        final EnumSet<Node.TransformBehavior> transformBehavior = EnumSet.of(Node.TransformBehavior.BILLBOARD_Y);

        // Add yes button
        mYesButtonNode = new Node();
        final Bitmap yesBitmap = getBitmapFromAssets(mActivity, "icon_thumb_up.png");
        final Texture yesTexture = new Texture(yesBitmap,
                Texture.Format.RGBA8, true, true);
        final Material yesMaterial = new Material();
        final Surface yesSurface = new Surface(2, 2);
        yesMaterial.setDiffuseTexture(yesTexture);
        yesSurface.setMaterials(Arrays.asList(yesMaterial));
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
                Texture.Format.RGBA8, true, true);
        final Material noMaterial = new Material();
        final Surface noSurface = new Surface(2, 2);
        noMaterial.setDiffuseTexture(noTexture);
        noSurface.setMaterials(Arrays.asList(noMaterial));
        mNoButtonNode.setGeometry(noSurface);
        final float[] noPosition = {-2.5f, -0.5f, -3.3f};
        mNoButtonNode.setPosition(new Vector(noPosition));
        mNoButtonNode.setTransformBehaviors(transformBehavior);
        mNoButtonNode.setEventDelegate(getGenericDelegate(TEST_FAILED_TAG));
        rootNode.addChildNode(mNoButtonNode);
    }

    private void addThumbButtonsOnGlass() {
        ImageView thumbsUp = (ImageView) mActivity.getThumbsUpView();
        thumbsUp.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mTestResult.set(TEST_PASSED);
                mTestButtonsClicked.set(true);
            }
        });

        ImageView thumbsDown = (ImageView) mActivity.getThumbsDownView();
        thumbsDown.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mTestResult.set(TEST_FAILED);
                mTestButtonsClicked.set(true);
            }
        });
    }

    protected void assertPass(final String expectedMessage) {
        // Display the scene when assertPass is invoked
        mScene.getRootNode().setVisible(true);
        mTestButtonsClicked.set(false);
        mTestResult.set(-1);

        String methodName = Thread.currentThread().getStackTrace()[3].getMethodName();
        if (methodName.equalsIgnoreCase("assertPass")) {
            methodName = Thread.currentThread().getStackTrace()[4].getMethodName();
        }

        final Text methodNameText = (Text) mTestMethodNameNode.getGeometry();
        methodNameText.setText(methodName);

        final Text instructionCardText = (Text) mExpectedMessageNode.getGeometry();
        instructionCardText.setText(expectedMessage);
    }

    protected void assertPass(final String expectedMessage, final TestCleanUpMethod method) {
        assertPass(expectedMessage);
    }

    abstract void configureTestScene();

    /**
     * This can be overriden to customize the reset of the state. The default behavior is we
     * destroy the scene entirely and recreate it.
     */
    void resetTestState() {
        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mScene = new ARScene();
        } else {
            mScene = new Scene();
        }

        createBaseTestScene();
        configureTestScene();

        // Hide the scene until assertPass is invoked (to prevent scene construction noise)
        mScene.getRootNode().setVisible(false);
        mViroView.setScene(mScene);
    };

    void callbackEverySecond(final MutableTestMethod testMethod) {
        if (testMethod == null) {
            return;
        }
        try {
            testMethod.mutableTest();
        } catch(Exception e) {
            Log.e(TAG, "Exception running mutable test method", e);
        }
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
        public void onRotate(final int source, final Node node, final float rotationRadians, final RotateState rotateState) {
            Log.e(TAG, delegateTag + " On rotate");
        }

        @Override
        public void onCameraARHitTest(ARHitTestResult[] results) {

        }

        @Override
        public void onARPointCloudUpdate(ARPointCloud pointCloud) {

        }

        @Override
        public void onCameraTransformUpdate(float posX, float poxY, float posZ, float rotEulerX, float rotEulerY, float rotEulerZ, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ) {

        }
    }

    protected void runUITest(Runnable runnable) {
        AtomicBoolean finishedTest = new AtomicBoolean(false);
        Runnable test = new Runnable() {
            @Override
            public void run() {
                mMutableTestMethod = null;
                runnable.run();
                finishedTest.set(true);
            }
        };
        mUIThreadHandler.post(test);

        await().until(() -> finishedTest.get());
        await().atMost(TEST_MAX_DURATION_SEC, TimeUnit.SECONDS).untilTrue(mTestButtonsClicked);
        assertEquals((long) TEST_PASSED, (long) mTestResult.get());

        AtomicBoolean finishedCleanup = new AtomicBoolean(false);
        Runnable cleanup = new Runnable() {
            @Override
            public void run() {
                mMutableTestMethod = null;
                resetTestState();
                finishedCleanup.set(true);
            }
        };
        mUIThreadHandler.post(cleanup);

        await().until(() -> finishedCleanup.get());

        // Wait an additional half second so that the Scene transition animation concludes,
        // before moving to the next test
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    @Deprecated
    protected void runOnUiThread(Runnable runnable) {
        AtomicBoolean finishedTest = new AtomicBoolean(false);
        Runnable myRunnable = new Runnable() {
            @Override
            public void run() {
                runnable.run();
                finishedTest.set(true);
            }
        };
        mUIThreadHandler.post(myRunnable);
        await().until(() -> finishedTest.get());
    }
}
