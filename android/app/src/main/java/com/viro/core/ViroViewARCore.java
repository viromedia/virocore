/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.AttrRes;
import android.support.annotation.Keep;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.Display;
import android.widget.Toast;

import com.google.ar.core.Config;
import com.google.ar.core.Session;
import com.google.vr.cardboard.ContextUtils;
import com.viro.core.internal.ARTouchGestureListener;
import com.viro.core.internal.CameraPermissionHelper;
import com.viro.core.internal.FrameListener;
import com.viro.core.internal.GLSurfaceViewQueue;
import com.viro.core.internal.PlatformUtil;
import com.viro.renderer.BuildConfig;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * ViroViewARCore is a {@link ViroView} for rendering augmented reality scenes using Google's ARCore
 * API for tracking. When using this view, the camera's real-time video feed will be rendered to the
 * background of your {@link ARScene}, and the coordinate system of Viro's virtual content and the
 * real world will be fused.
 * <p>
 * The coordinate system's origin is the location of the device when it starts the application.
 * Units are in meters.
 */
public class ViroViewARCore extends ViroView {

    /**
     * ARCore, which underlies Viro's augmented reality system, can be set to detect various types
     * of real world features (anchors) by continually scanning the incoming video feed from the
     * device camera. This enum indicates what anchors ARCore should attempt to detect. The {@link
     * ARScene.Listener} will receive a callback whenever an {@link ARAnchor}
     * of one of these enabled types is found.
     */
    public enum AnchorDetectionType {

        /**
         * Anchor detection will be entirely disabled. This setting results in optimal performance
         * and should be used in applications where anchors are not required.
         */
        NONE("none"),

        /**
         * Horizontal planes will be detected by the AR tracking system.
         */
        PLANES_HORIZONTAL("planes_horizontal");

        private String mStringValue;
        private AnchorDetectionType(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, AnchorDetectionType> map = new HashMap<String, AnchorDetectionType>();
        static {
            for (AnchorDetectionType value : AnchorDetectionType.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         */
        public static AnchorDetectionType valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    private static final String TAG = "Viro";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        System.loadLibrary("native-lib");
    }

    private static class ViroARRenderer implements GLSurfaceView.Renderer {

        private WeakReference<ViroViewARCore> mView;

        public ViroARRenderer(ViroViewARCore view) {
            mView = new WeakReference<ViroViewARCore>(view);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            ViroViewARCore view = mView.get();
            if (view == null) {
                return;
            }

            view.mNativeRenderer.onSurfaceCreated(view.mSurfaceView.getHolder().getSurface());
            view.mNativeRenderer.initalizeGl();
            if (view.mRenderStartListener != null) {
                Runnable myRunnable = new Runnable() {
                    @Override
                    public void run() {
                        ViroViewARCore view = mView.get();
                        if (view == null  || view.mDestroyed) {
                            return;
                        }
                        view.mRenderStartListener.onRendererStart();
                    }
                };
                new Handler(Looper.getMainLooper()).post(myRunnable);
            }
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            ViroViewARCore view = mView.get();
            if (view == null) {
                return;
            }
            view.mNativeRenderer.onSurfaceChanged(view.mSurfaceView.getHolder().getSurface(), width, height);

            // Notify ARCore session that the view size changed so that the perspective matrix and
            // the video background can be properly adjusted.
            view.mSession.setDisplayGeometry(width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            ViroViewARCore view = mView.get();
            if (view == null) {
                return;
            }

            for (FrameListener listener : view.mFrameListeners) {
                listener.onDrawFrame();
            }
            view.mNativeRenderer.drawFrame();
        }
    }

    private Renderer mRenderer;
    private GLSurfaceView mSurfaceView;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners = new ArrayList();
    private PlatformUtil mPlatformUtil;
    private boolean mActivityPaused = true;
    private Config mConfig;
    private Session mSession;
    private ARTouchGestureListener mARTouchGestureListener;
    private ViroMediaRecorder mMediaRecorder;

    /**
     * Create a new ViroViewARCore.
     *
     * @param context               The activity context.
     * @param rendererStartListener Runnable to invoke when the renderer has finished initializing.
     *                              Optional, may be null.
     */
    public ViroViewARCore(@NonNull final Context context, @Nullable final RendererStartListener rendererStartListener) {
        super(context);
        init(context, rendererStartListener);
    }

    /**
     * @hide
     *
     * @param context
     */
    @Keep
    public ViroViewARCore(@NonNull final Context context) {
        this(context, (AttributeSet) null);
    }

    /**
     * @hide
     *
     * @param context
     * @param attrs
     */
    public ViroViewARCore(@NonNull final Context context, @Nullable final AttributeSet attrs) {
        this(context, attrs, 0);
    }

    /**
     * @hide
     *
     * @param context
     * @param attrs
     * @param defStyleAttr
     */
    public ViroViewARCore(@NonNull final Context context, @Nullable final AttributeSet attrs, @AttrRes final int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        if (ContextUtils.getActivity(context) == null) {
            throw new IllegalArgumentException("An Activity Context is required for Viro functionality.");
        } else {
            init(context, null);
        }
    }

    private void init(final Context context, final RendererStartListener rendererStartListener) {
        mSurfaceView = new GLSurfaceView(context);
        addView(mSurfaceView);

        final Context activityContext = getContext();
        final Activity activity = (Activity) getContext();

        // Initialize ARCore
        mSession = new Session(activity);

        // ARCore may crash unless we set some initial non-zero display geometry
        // (this will be resized by the the surface on the GL thread)
        final Display display = activity.getWindowManager().getDefaultDisplay();
        final Point size = new Point();
        display.getSize(size);
        mSession.setDisplayGeometry(size.x, size.y);

        // Initialize the native renderer.
        initSurfaceView();

        mAssetManager = getResources().getAssets();
        mPlatformUtil = new PlatformUtil(
                new GLSurfaceViewQueue(mSurfaceView),
                mFrameListeners,
                activityContext,
                mAssetManager);
        mNativeRenderer = new Renderer(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(), this, mSession,
                mAssetManager, mPlatformUtil);
        mNativeViroContext = new ViroContext(mNativeRenderer.mNativeRef);

        mRenderStartListener = rendererStartListener;
        mARTouchGestureListener = new ARTouchGestureListener(activity, mNativeRenderer);
        setOnTouchListener(mARTouchGestureListener);

        // Create default config, check is supported, create session from that config.
        mConfig = Config.createDefaultConfig();
        if (!mSession.isSupported(mConfig)) {
            Toast.makeText(activity, "This device does not support AR", Toast.LENGTH_LONG).show();
            activity.finish();
            return;
        }

        if (BuildConfig.FLAVOR.equalsIgnoreCase(FLAVOR_VIRO_CORE)) {
            validateAPIKeyFromManifest();
        }
    }


    /**
     * Initialize this {@link GLSurfaceView}.
     */
    private void initSurfaceView() {
        int colorBits = 8;
        int alphaBits = 8;
        int depthBits = 16;
        int stencilBits = 8;

        mSurfaceView.setEGLContextClientVersion(3);
        mSurfaceView.setEGLConfigChooser(colorBits, colorBits, colorBits, alphaBits, depthBits, stencilBits);
        mSurfaceView.setPreserveEGLContextOnPause(true);
        mSurfaceView.setEGLWindowSurfaceFactory(new ViroEGLWindowSurfaceFactory());

        mSurfaceView.setRenderer(new ViroARRenderer(this));
        mSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        /*
         Don't start the SurfaceView yet; we need to wait until the ARCore session
         is resumed (in onActivityResumed()).
         */
        mSurfaceView.onPause();
        // setOnTouchListener(new ViroGvrLayout.ViroOnTouchListener(this));
    }

    // TODO If we request camera permissions we have to respond here somehow
    //      This is an activity callback so we have to hook into the parent activity
    /*
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        Activity activity = mWeakActivity.get();
        if (activity != null) {
            return;
        }

        if (!CameraPermissionHelper.hasCameraPermission(getContext())) {
            Toast.makeText(activity,
                    "Camera permission is needed to run this application", Toast.LENGTH_LONG).show();
            activity.finish();
        }
    }
    */

    /**
     * @hide
     */
    @Override
    public void setDebug(boolean debug) {
        //No-op
    }

    @Override
    public void setScene(Scene scene) {
        if (scene == mCurrentScene) {
            return;
        }
        if (!(scene instanceof ARScene)) {
            throw new IllegalArgumentException("ViroViewARCore requires an ARScene");
        }
        super.setScene(scene);
        mNativeRenderer.setSceneController(scene.mNativeRef, 0.5f);
    }

    @Override
    public void setOnTouchListener(OnTouchListener listener) {
        // If we're adding our own ARTouchGestureListener, then we add it as the actual
        // touch listener otherwise, we attach the listener to the ARTouchGestureListener
        // which will forward the touches to the given listener before processing them itself.
        if (listener instanceof ARTouchGestureListener) {
            super.setOnTouchListener(listener);
        } else if(mARTouchGestureListener != null) {
            mARTouchGestureListener.setOnTouchListener(listener);
        }
    }

    @Override
    public void setPointOfView(Node node) {
        throw new IllegalStateException("Viro: Unable to set a different point of view in AR");
    }

    /**
     * @hide
     */
    @Override
    public void setVRModeEnabled(boolean vrModeEnabled) {
        //No-op
    }

    /**
     * @hide
     */
    @Override
    public String getPlatform() {
        return "arcore";
    }

    /**
     * @hide
     */
    @Override
    public void onActivityPaused(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mActivityPaused = true;
        mNativeRenderer.onPause();

        // Note that the order matters - GLSurfaceView is paused first so that it does not try
        // to query the session. If Session is paused before GLSurfaceView, GLSurfaceView may
        // still call mSession.update() and get a SessionPausedException.
        mSurfaceView.onPause();
        mSession.pause();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityResumed(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mActivityPaused = false;
        setImmersiveSticky();
        mNativeRenderer.onResume();

        // ARCore requires camera permissions to operate. If we did not yet obtain runtime
        // permission on Android M and above, now is a good time to ask the user for it.
        if (CameraPermissionHelper.hasCameraPermission(activity)) {
            // Note that order matters - see the note in onPause(), the reverse applies here.
            mSession.resume(mConfig);
            mSurfaceView.onResume();
        } else {
            CameraPermissionHelper.requestCameraPermission(activity);
        }
    }

    /**
     * @hide
     */
    @Override
    public void onActivityDestroyed(Activity activity) {
        this.dispose();
        ARNode.nodeARMap.clear();
        /*
          TODO VIRO-2280: Fix Tango Memory leak that holds onto ViroViewARCore.
          As a temporary patch, we null out our viro components here to free up
          most of our 3D controls in memory.
         */
        mARTouchGestureListener = null;
        mPlatformUtil = null;
        mAssetManager = null;
        mSurfaceView = null;
        mRenderer = null;
        mSession = null;
        mFrameListeners.clear();
        mFrameListeners = null;
        // End TODO VIRO-2280
    }

    @Override
    public void dispose() {
        if (mMediaRecorder != null) {
            mMediaRecorder.dispose();
        }
        mARTouchGestureListener.destroy();
        super.dispose();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityCreated(Activity activity, Bundle bundle) {
        //No-op
    }

    /**
     * @hide
     */
    @Override
    public void onActivityStarted(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mNativeRenderer.onStart();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityStopped(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mNativeRenderer.onStop();
    }

    /**
     * @hide
     */
    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {
        // No-op
    }

    /**
     * @hide
     */
    @Override
    public void recenterTracking() {
        // No-op
    }

    @Override
    public ViroMediaRecorder getRecorder() {
        if (mMediaRecorder == null) {
            mMediaRecorder = new ViroMediaRecorder(getContext(), mNativeRenderer,
                    getWidth(), getHeight());
        }
        return mMediaRecorder;
    }

    /**
     * Set the types of {@link ARAnchor}s the system should attempt to detect and track. The {@link
     * ARScene.Listener} will receive a callback whenever an {@link ARAnchor}
     * of one of these enabled types is found.
     *
     * @param types The set of anchor types that should be tracked.
     */
    public void setAnchorDetectionTypes(EnumSet<AnchorDetectionType> types) {
        if (types.size() == 0) {
            mConfig.setPlaneFindingMode(Config.PlaneFindingMode.DISABLED);
        }
        for (AnchorDetectionType type : types) {
            if (type == AnchorDetectionType.NONE) {
                mConfig.setPlaneFindingMode(Config.PlaneFindingMode.DISABLED);
            } else if (type == AnchorDetectionType.PLANES_HORIZONTAL) {
                mConfig.setPlaneFindingMode(Config.PlaneFindingMode.HORIZONTAL);
            }
        }
    }

    /**
     * Invoked by native.
     * @hide
     * @param config
     */
    public void setConfig(Config config) {
        if (mSession.isSupported(config) && !mActivityPaused) {
            mConfig = config;
            mSession.resume(mConfig);
        }
    }

    /**
     * Performs a hit-test from the camera's position in the direction of the given ray. The
     * hit-test returns all <i>real-world</i> features that are intersected by the ray.
     *
     * @param ray      Perform the hit-test from the camera's position in the direction of this
     *                 ray.
     * @param callback The callback that will receive the {@link ARHitTestResult}
     *                 results.
     */
    public void performARHitTestWithRay(Vector ray, ARHitTestListener callback) {
        if (!mDestroyed) {
            mNativeRenderer.performARHitTestWithRay(ray.toArray(), callback);
        }
    }

    /**
     * Performs a hit-test along the ray from the camera's current position <i>to</i> the given
     * position in world coordinates. The hit-test returns all <i>real-world</i> features that are
     * intersected by the ray.
     *
     * @param position Perform the hit-test along the ray from the camera's position to this
     *                 position.
     * @param callback The callback that will receive the {@link ARHitTestResult}
     *                 results.
     */
    public void performARHitTestWithPosition(Vector position, ARHitTestListener callback) {
        if (!mDestroyed) {
            mNativeRenderer.performARHitTestWithPosition(position.toArray(), callback);
        }
    }


    /**
     * Performs a hit-test searching for all <i>real-world</i> features at the given 2D point on the
     * view. Note that since a single 2D point on view corresponds to a 3D ray in the scene,
     * multiple results may be returned (each at a different depth).
     *
     * @param point    Perform the hit-test at this Point in the 2D view's coordinate system.
     * @param callback The callback that will receive the {@link ARHitTestResult}
     *                 results.
     */
    public void performARHitTest(Point point, ARHitTestListener callback) {
        if (!mDestroyed) {
            mNativeRenderer.performARHitTestWithPoint(point.x, point.y, callback);
        }
    }
}
