/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.Point;
import android.opengl.EGL14;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.AttrRes;
import android.support.annotation.Keep;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Display;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.widget.Toast;

import com.google.vr.cardboard.ContextUtils;
import com.viro.core.internal.ViroTouchGestureListener;
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

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
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
        NONE("None"),

        /**
         * Horizontal planes will be detected by the AR tracking system.
         */
        PLANES_HORIZONTAL("PlanesHorizontal");

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

            final int EGL_GL_COLORSPACE_KHR = 0x309D;
            final int EGL_GL_COLORSPACE_SRGB_KHR = 0x3089;

            EGL10 egl = (EGL10) EGLContext.getEGL();
            EGLDisplay display = egl.eglGetCurrentDisplay();
            EGLSurface surface = egl.eglGetCurrentSurface(EGL14.EGL_DRAW);

            int[] value = new int[1];
            boolean sRGBFramebuffer = false;
            egl.eglQuerySurface(display, surface, EGL_GL_COLORSPACE_KHR, value);
            if (value[0] == EGL_GL_COLORSPACE_SRGB_KHR) {
                Log.i(TAG, "Acquired sRGB framebuffer");
                sRGBFramebuffer = true;
            }
            else {
                Log.i(TAG, "Did not acquire sRGB framebuffer [colorspace " + value[0] + "]");
            }

            view.mNativeRenderer.onSurfaceCreated(view.mSurfaceView.getHolder().getSurface());
            view.mNativeRenderer.initializeGL(sRGBFramebuffer);
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
            view.mWidth = width;
            view.mHeight = height;
            view.mNativeRenderer.setARDisplayGeometry(view.mRotation, view.mWidth, view.mHeight);
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

    private int mRotation = Surface.ROTATION_0;
    private int mWidth, mHeight;
    private GLSurfaceView mSurfaceView;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners = new ArrayList();
    private PlatformUtil mPlatformUtil;
    private boolean mActivityPaused = true;
    private ViroTouchGestureListener mViroTouchGestureListener;
    private ViroMediaRecorder mMediaRecorder;
    private OrientationEventListener mOrientationListener;

    /**
     * Create a new ViroViewARCore with the default {@link RendererConfiguration}.
     *
     * @param context               The activity context.
     * @param rendererStartListener Runnable to invoke when the renderer has finished initializing.
     *                              Optional, may be null.
     */
    public ViroViewARCore(@NonNull final Context context, @Nullable final RendererStartListener rendererStartListener) {
        super(context, null);
        init(context, rendererStartListener);
    }

    /**
     * Create a new ViroViewARCore with the given {@link RendererConfiguration}, which determines
     * the rendering techniques and rendering fidelity to use for this View.
     *
     * @param context               The activity context.
     * @param rendererStartListener Runnable to invoke when the renderer has finished initializing.
     *                              Optional, may be null.
     * @param config The {@link RendererConfiguration} to use.
     */
    public ViroViewARCore(@NonNull final Context context, @Nullable final RendererStartListener rendererStartListener,
                          @Nullable RendererConfiguration config) {
        super(context, config);
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
        // TODO VIRO-2906 Perform a static check if ARCore supported (call isSupported down below?)
        /*
        try {
            mSession = new Session(activity);
        } catch (UnsatisfiedLinkError error){
            Toast.makeText(activity, "Installed version of Android does not support AR" +
                    " (Currently supported on Android N, API 24 or later.", Toast.LENGTH_LONG).show();
            activity.finish();
            return;
        }
        */

        final Display display = activity.getWindowManager().getDefaultDisplay();
        final Point size = new Point();
        display.getSize(size);
        mWidth = size.x;
        mHeight = size.y;

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
                activityContext.getApplicationContext(),
                mAssetManager, mPlatformUtil, mRendererConfig);
        mNativeViroContext = new ViroContext(mNativeRenderer.mNativeRef);

        mRenderStartListener = rendererStartListener;
        mViroTouchGestureListener = new ViroTouchGestureListener(activity, mNativeRenderer);
        setOnTouchListener(mViroTouchGestureListener);

        // Create default config, check is supported, create session from that config.
        // TODO VIRO-2906 Ensure we have all support checks
        /*
        mConfig = new Config(mSession);
        if (!mSession.isSupported(mConfig)) {
            Toast.makeText(activity, "This device does not support AR", Toast.LENGTH_LONG).show();
            activity.finish();
            return;
        }
        mSession.configure(mConfig);
        */

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

    /**
     * @hide
     * @param listener
     */
    @Override
    public void setOnTouchListener(OnTouchListener listener) {
        // If we're adding our own ViroTouchGestureListener, then we add it as the actual
        // touch listener otherwise, we attach the listener to the ViroTouchGestureListener
        // which will forward the touches to the given listener before processing them itself.
        if (listener instanceof ViroTouchGestureListener) {
            super.setOnTouchListener(listener);
        } else if(mViroTouchGestureListener != null) {
            mViroTouchGestureListener.setOnTouchListener(listener);
        }
    }

    /**
     * @hide
     */
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

        // Note that the order matters - GLSurfaceView is paused first so that it does not try
        // to query the session. If Session is paused before GLSurfaceView, GLSurfaceView may
        // still call mSession.update() and get a SessionPausedException.
        mSurfaceView.onPause();
        mNativeRenderer.onPause();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityResumed(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        if (!CameraPermissionHelper.hasCameraPermission(activity)) {
            Log.e(TAG, "ERROR: Attempted to resume a Viro AR Core experience without " +
                    "the required Camera permissions!");
            return;
        }

        mActivityPaused = false;
        setImmersiveSticky();
        mNativeRenderer.onResume();
        mSurfaceView.onResume();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityDestroyed(Activity activity) {
        this.dispose();
        ARNode.nodeARMap.clear();
        mViroTouchGestureListener = null;
        mPlatformUtil = null;
        mAssetManager = null;
        mSurfaceView = null;
        mFrameListeners.clear();
        mFrameListeners = null;
    }

    @Override
    public void dispose() {
        if (mMediaRecorder != null) {
            mMediaRecorder.dispose();
        }
        if (mViroTouchGestureListener != null) {
            mViroTouchGestureListener.destroy();
        }
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
        if (!mDestroyed) {
            mNativeRenderer.setAnchorDetectionTypes(types);
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

    public void setCameraRotationAutomatic(boolean automatic) {
        Activity activity = mWeakActivity.get();
        if (activity == null) {
            return;
        }

        if (automatic) {
            mOrientationListener = new OrientationEventListener(activity) {
                @Override
                public void onOrientationChanged(int orientation) {

                }
            };
        }
        else {
            mOrientationListener = null;
        }
    }

    /**
     * Set the rotation of the background camera view. This is derived from one of the {@link android.view.Surface}
     * constants for rotation; e.g. {@link android.view.Surface#ROTATION_0}, {@link android.view.Surface#ROTATION_90},
     * {@link android.view.Surface#ROTATION_180}, or {@link android.view.Surface#ROTATION_270}.
     * <p>
     * Typically this value should be changed when the device orientation changes; e.g.,
     * in {@link android.app.Activity#onConfigurationChanged(Configuration)}. For example:
     * <p>
     * <tt>
     * <pre>
     *  public void onConfigurationChanged(Configuration newConfig) {
     *      super.onConfigurationChanged(newConfig);
     *      Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
     *      mViroViewARCore.setCameraRotation(display.getRotation());
     *  }
     * </pre>
     * </tt>
     *
     * @param rotation The rotation constant for the background camera view.
     */
    public void setCameraRotation(int rotation) {
        mRotation = rotation;
        mNativeRenderer.setARDisplayGeometry(mRotation, mWidth, mHeight);
    }

    /**
     * Get the OpenGL texture name (ID) that ViroCore uses to render the camera background each
     * frame. This may be used by advanced applications that want to process raw image data
     * received from the camera. The texture is of type GL_TEXTURE_EXTERNAL_OES, and uses a
     * <tt>samplerExternalOES</tt> in shaders. Returns 0 if the camera texture has not yet been
     * initialized.
     *
     * @return The camera texture ID. Returns 0 if the camera texture has not yet been initialized.
     */
    public int getCameraTextureName() {
        return mNativeRenderer.getCameraTextureId();
    }

    /**
     * Set the {@link ARHitTestListener}. When an ARHitTestListener is registered, it will be
     * continually notified of hit tests from the camera's forward vector into the AR environment.
     * This is an efficient way to be notified of real-world objects intersecting the center of
     * the user's AR screen.

     *
     * @param hitTestListener The callback that will receive the {@link ARHitTestResult}
     *                 results.
     */
    public void setCameraARHitTestListener(ARHitTestListener hitTestListener){
        if (mCurrentScene == null){
            return;
        }

        mCurrentScene.getRootNode().setARHitTestListener(hitTestListener);
    }

    /**
     * Checks if AR is supported on the target device and if the ARCore apk/sdk is present on the
     * device. If this method returns false, attempts to create {@link ViroViewARCore} will throw
     * an exception
     * @param context The {@link Context} of your app
     */

    // TODO VIRO-2906 Restore this check
    public static boolean isSupported(Context context) {
        return true;
        /*
        Session session;
        Config config;
        try {
            // Create a session with the given context
            session = new Session(context);
            // Create a default config
            config = new Config(session);
        } catch (UnsatisfiedLinkError error) {
            /**
             * TODO Need to catch this error due to
             * https://github.com/google-ar/arcore-android-sdk/issues/111
             * Remove this once Google fixes the above issue. As of 02/05/2018, the issue is being
             * marked as "fixed in an upcoming release.
             */
         //   return false;
       // }
        //return session.isSupported(config);
    }
}
