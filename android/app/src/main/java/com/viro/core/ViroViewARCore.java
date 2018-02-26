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
import android.view.Surface;

import com.google.ar.core.ArCoreApk;
import com.google.ar.core.Session;
import com.google.ar.core.exceptions.UnavailableApkTooOldException;
import com.google.ar.core.exceptions.UnavailableArcoreNotInstalledException;
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException;
import com.google.ar.core.exceptions.UnavailableSdkTooOldException;
import com.google.ar.core.exceptions.UnavailableUserDeclinedInstallationException;
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
import java.util.concurrent.atomic.AtomicBoolean;

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

    private static final String TAG = "Viro";

    /**
     * Callback interface for responding to {@link ViroViewARCore} startup success or failure.
     */
    public interface StartupListener {

        /**
         * Callback invoked when {@link ViroViewARCore} has finished initialization, meaning the
         * rendering surface was succesfully created and ARCore was successfully installed and
         * initialized. When this is received, the ViroView is ready to begin rendering content.
         */
        void onSuccess();

        /**
         * Callback invoked when the {@link ViroViewARCore} failed to initialize.
         *
         * @param error        The error code.
         * @param errorMessage The reason for the failure as a string.
         */
        void onFailure(StartupError error, String errorMessage);

    }

    /**
     * Errors returned by the {@link StartupListener}, in response to Viro failing to
     * initialize.
     */
    public enum StartupError {

        /**
         * Indicates ARCore is not supported on the device.
         */
        ARCORE_NOT_SUPPORTED,

        /**
         * Indicates ARCore is supported by the device, but is not installed on the device and attempts
         * to install it failed.
         */
        ARCORE_NOT_INSTALLED,

        /**
         * Indicates ARCore is supported by the device, and an install was requested by the application
         * but declined by the user.
         */
        ARCORE_USER_DECLINED_INSTALL,

        /**
         * Indicates the version of ARCore on the user's device is out of date.
         */
        ARCORE_NEEDS_UPDATE,

        /**
         * Indicates the ARCore SDK used by this app is out of date.
         */
        ARCORE_SDK_NEEDS_UPDATE,

        /**
         * Indicates an unknown error detecting ARCore on the device.
         */
        ARCORE_UNKNOWN,

        /**
         * Indicates AR could not be started because camera permissions were not granted to the
         * application.
         */
        CAMERA_PERMISSIONS_NOT_GRANTED,

    };

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

    private static class ViroARRenderer implements GLSurfaceView.Renderer {

        private WeakReference<ViroViewARCore> mView;

        public ViroARRenderer(ViroViewARCore view) {
            mView = new WeakReference<ViroViewARCore>(view);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            final ViroViewARCore view = mView.get();
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
            view.mRendererSurfaceInitialized.set(true);
            if (view.mARCoreInstalled.get()) {
                Runnable myRunnable = new Runnable() {
                    @Override
                    public void run() {
                        final ViroViewARCore view = mView.get();
                        if (view == null) {
                            return;
                        }
                        view.notifyRendererStart();
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
            ((RendererARCore) view.mNativeRenderer).setARDisplayGeometry(view.mRotation, view.mWidth, view.mHeight);
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
    private ViroTouchGestureListener mViroTouchGestureListener;
    private ViroMediaRecorder mMediaRecorder;
    private StartupListener mStartupListener;

    // The renderer start listener is invoked when these are all true
    private AtomicBoolean mRendererSurfaceInitialized = new AtomicBoolean(false);
    private AtomicBoolean mARCoreInstalled = new AtomicBoolean(false);
    private boolean mRequestedCameraPermissions = false;

    private boolean mAppRequestedInstall = true;
    private boolean mUserRequestedInstall = false;

    /**
     * Create a new ViroViewARCore with the default {@link RendererConfiguration}. This constructor
     * will immediately throw an {@link UnavailableDeviceNotCompatibleException} if ARCore is not
     * compatible with this device. You can also check for this condition with the static {@link
     * #isDeviceCompatible(Context)} method.
     * <p>
     * If ARCore <i>is</i> compatible with the device, then Viro will check if it is installed and
     * prompt the user to install it if it isn't. Any error encountered during this process will be
     * reported as a {@link StartupError} to the provided {@link StartupListener}. Many of these
     * errors are asynchronous: for example, if the user does not have ARCore installed and declines
     * to install it when prompted, or if the install fails.
     * <p>
     *
     * @param context         The activity context.
     * @param startupListener Listener to respond to startup success or failure. Will be notified of
     *                        any errors encountered while initializing AR and the renderer.
     *                        Optional, may be null.
     * @throws UnavailableDeviceNotCompatibleException If ARCore is not compatible with the device.
     */
    public ViroViewARCore(@NonNull final Context context,
                          @Nullable final StartupListener startupListener) {
        this(context, startupListener, null);
    }

    /**
     * Create a new ViroViewARCore with the given {@link RendererConfiguration}, which determines
     * the rendering techniques and rendering fidelity to use for this View. This constructor
     * will immediately throw an {@link UnavailableDeviceNotCompatibleException} if ARCore is not
     * compatible with this device. You can also check for this condition with the static {@link
     * #isDeviceCompatible(Context)} method.
     * <p>
     * If ARCore <i>is</i> compatible with the device, then Viro will check if it is installed and
     * prompt the user to install it if it isn't. Any error encountered during this process will be
     * reported as a {@link StartupError} to the provided {@link StartupListener}. Many of these
     * errors are asynchronous: for example, if the user does not have ARCore installed and declines
     * to install it when prompted, or if the install fails.
     * <p>
     *
     * @param context         The activity context.
     * @param startupListener Listener to respond to startup success or failure. Will be notified of
     *                        any errors encountered while initializing AR and the renderer.
     *                        Optional, may be null.
     * @param config          The {@link RendererConfiguration} to use. May be null to use the
     *                        default configuration.
     * @throws UnavailableDeviceNotCompatibleException If ARCore is not compatible with the device.
     */
    public ViroViewARCore(@NonNull final Context context,
                          @Nullable final StartupListener startupListener,
                          @Nullable RendererConfiguration config) {
        super(context, config);
        init(context, startupListener);
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

    private void init(final Context context, final StartupListener startupListener) {
        mStartupListener = startupListener;
        if (!isDeviceCompatible(context)) {
            notifyRendererFailed(StartupError.ARCORE_NOT_SUPPORTED,
                    "This device is not compatible with ARCore");
            throw new UnavailableDeviceNotCompatibleException();
        }

        // We wait to load libraries until after the ARCore check, otherwise UnsatisfiedLinkErrors
        // may occur on devices that are using Android 23 or earlier (due to ARCore's dependency
        // on native camera)
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        try {
            System.loadLibrary("viro_arcore");
        } catch (UnsatisfiedLinkError e) {
            Log.i(TAG, "ViroCore not found, loading ViroReact");
            System.loadLibrary("viro_react");
        }

        mSurfaceView = new GLSurfaceView(context);
        addView(mSurfaceView);

        final Context activityContext = getContext();
        final Activity activity = (Activity) getContext();

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
        mNativeRenderer = new RendererARCore(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(),
                mAssetManager, mPlatformUtil, mRendererConfig);
        mNativeViroContext = new ViroContext(mNativeRenderer.mNativeRef);
        mViroTouchGestureListener = new ViroTouchGestureListener(activity, mNativeRenderer);
        setOnTouchListener(mViroTouchGestureListener);

        if (BuildConfig.FLAVOR.equalsIgnoreCase(FLAVOR_VIRO_CORE)) {
            validateAPIKeyFromManifest();
        }
    }

    /**
     * Used by release tests.
     * @hide
     * @param listener
     */
    public void setStartupListener(StartupListener listener) {
        mStartupListener = listener;
    }

    private void notifyRendererStart() {
        if (mStartupListener != null) {
            mStartupListener.onSuccess();
        }
    }

    private void notifyRendererFailed(StartupError error, String errorMessage) {
        if (mStartupListener != null) {
            mStartupListener.onFailure(error, errorMessage);
        }
    }

    private boolean requestARCoreInstall(Activity activity) {
        // Do nothing if we've already detected ARCore is installed
        if (mARCoreInstalled.get()) {
            return true;
        }

        StartupError error = StartupError.ARCORE_UNKNOWN;
        String message = null;
        try {
            switch (ArCoreApk.getInstance().requestInstall(activity, mAppRequestedInstall)) {
                case INSTALL_REQUESTED:
                    Log.i(TAG, "ARCore installation requested by application");

                    // Flip this to false, so we only ask the user once to install ARCore
                    mAppRequestedInstall = false;
                    return false;
                case INSTALLED:
                    Log.i(TAG, "ARCore installed on device");
                    break;
            }

            // Create a dummy session just to check if it's possible
            new Session(activity);
            mARCoreInstalled.set(true);
            ((RendererARCore) mNativeRenderer).onARCoreInstalled(getContext());
            ((RendererARCore) mNativeRenderer).setARDisplayGeometry(mRotation, mWidth, mHeight);

            if (mRendererSurfaceInitialized.get()) {
                notifyRendererStart();
            }
        } catch (UnavailableArcoreNotInstalledException e) {
            error = StartupError.ARCORE_NOT_INSTALLED;
            Log.i(TAG, "Error: ARCore not installed on device", e);
            message = "Please install ARCore on this device to use AR features";

        } catch (UnavailableUserDeclinedInstallationException e) {
            error = StartupError.ARCORE_USER_DECLINED_INSTALL;
            Log.i(TAG, "Error: User declined installing ARCore on device");
            message = "Please install ARCore on this device to use AR features";

        } catch (UnavailableApkTooOldException e) {
            error = StartupError.ARCORE_NEEDS_UPDATE;
            Log.i(TAG, "Error: ARCore on this device needs to be updated");
            message = "Please update ARCore on this device to use AR features";

        } catch (UnavailableSdkTooOldException e) {
            error = StartupError.ARCORE_SDK_NEEDS_UPDATE;
            Log.i(TAG, "Error: ARCore SDK used by this application needs to be updated");
            message = "Please update this app to support AR features";

        } catch (Exception e) {
            error = StartupError.ARCORE_UNKNOWN;
            Log.i(TAG, "Error: Unknown error when installing ARCore: " + e.getMessage());
            message = "This device does not support AR";
        }

        if (message != null) {
            notifyRendererFailed(error, message);
            return false;
        }
        return true;
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
        if (mNativeRenderer == null) {
            return;
        }

        if (mWeakActivity.get() != activity) {
            return;
        }

        if (!CameraPermissionHelper.hasCameraPermission(activity) && mRequestedCameraPermissions) {
            notifyRendererFailed(StartupError.CAMERA_PERMISSIONS_NOT_GRANTED, "Error: " +
                    "Attempted to resume an ARCore experience without the required camera permissions!");
            return;
        }

        if (!CameraPermissionHelper.hasCameraPermission(activity)) {
            CameraPermissionHelper.requestCameraPermission(activity);
            mRequestedCameraPermissions = true;
            return;
        }

        setImmersiveSticky();
        boolean arcoreInstalled = requestARCoreInstall(activity);
        if (!arcoreInstalled) {
            return;
        }

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

        if (mFrameListeners != null) {
            mFrameListeners.clear();
            mFrameListeners = null;
        }
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
        if (mNativeRenderer == null) {
            return;
        }
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
        if (mNativeRenderer == null) {
            return;
        }
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
            ((RendererARCore) mNativeRenderer).setAnchorDetectionTypes(types);
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
            ((RendererARCore) mNativeRenderer).performARHitTestWithRay(ray.toArray(), callback);
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
            ((RendererARCore) mNativeRenderer).performARHitTestWithPosition(position.toArray(), callback);
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
            ((RendererARCore) mNativeRenderer).performARHitTestWithPoint(point.x, point.y, callback);
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
        ((RendererARCore) mNativeRenderer).setARDisplayGeometry(mRotation, mWidth, mHeight);
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
        return ((RendererARCore) mNativeRenderer).getCameraTextureId();
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
     * Checks if ARCore is compatible with the target device. This does <i>not</i> indicate whether ARCore is
     * installed. If this method returns false, attempts to create {@link ViroViewARCore} will throw
     * an exception.
     *
     * @param context The {@link Context} of your application.
     */
    public static boolean isDeviceCompatible(Context context) {
        ArCoreApk.Availability availability = ArCoreApk.getInstance().checkAvailability(context);
        Log.i(TAG, "ARCore availability check returned [" + availability   + "]");
        if (availability == ArCoreApk.Availability.UNSUPPORTED_DEVICE_NOT_CAPABLE) {
            return false;
        }
        else {
            return true;
        }
    }
}
