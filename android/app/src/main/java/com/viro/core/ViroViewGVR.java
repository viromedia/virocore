/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.AttrRes;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.cardboard.ContextUtils;
import com.google.vr.ndk.base.AndroidCompat;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;
import com.viro.core.internal.BuildInfo;
import com.viro.core.internal.FrameListener;
import com.viro.core.internal.GLSurfaceViewQueue;
import com.viro.core.internal.PlatformUtil;
import com.viro.renderer.BuildConfig;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * ViroViewGVR is a {@link ViroView} for rendering content in stereo for VR headsets using
 * the Google GVR SDK. This includes both Google Cardboard and Daydream headsets. ViroViewGVR
 * handles all GVR initialization.
 */
public class ViroViewGVR extends ViroView {

    private static final String TAG = "Viro";

    // Used to load the native libraries on application startup.
    static {
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        System.loadLibrary("viro_renderer");
    }

    /**
     * Callback interface for responding to {@link ViroViewGVR} startup success or failure.
     */
    public interface StartupListener {

        /**
         * Callback invoked when {@link ViroViewGVR} has finished initialization, meaning the
         * rendering surface was succesfully created and GVR was successfully initialized. When
         * this is received, the ViroView is ready to begin rendering content.
         */
        void onSuccess();

        /**
         * Callback invoked when the {@link ViroViewGVR} failed to initialize.
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
         * Indicates an unknown error.
         */
        UNKNOWN,

    };

    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners = new ArrayList();
    private PlatformUtil mPlatformUtil;
    private GvrLayout mGVRLayout;
    private StartupListener mStartupListener;
    private boolean mVRModeEnabled;

    // Activity state to restore to before being modified by the renderer.

    private static class ViroSurfaceViewRenderer implements GLSurfaceView.Renderer {

        private WeakReference<ViroViewGVR> mView;
        private WeakReference<GLSurfaceView> mSurface;

        public ViroSurfaceViewRenderer(ViroViewGVR view, GLSurfaceView surface) {
            mView = new WeakReference<ViroViewGVR>(view);
            mSurface = new WeakReference<GLSurfaceView>(surface);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            ViroViewGVR view = mView.get();
            if (view == null) {
                return;
            }
            GLSurfaceView surface = mSurface.get();
            if (surface == null) {
                return;
            }

            view.mNativeRenderer.onSurfaceCreated(surface.getHolder().getSurface());
            // TODO VIRO-2278 Make this dynamically check for sRGB framebuffer support in Daydream
            view.mNativeRenderer.initializeGL(false);
            if (view.mStartupListener != null) {
                Runnable myRunnable = new Runnable() {
                    @Override
                    public void run() {
                        ViroViewGVR view = mView.get();
                        if (view == null || view.mDestroyed) {
                            return;
                        }
                        view.mStartupListener.onSuccess();
                    }
                };
                new Handler(Looper.getMainLooper()).post(myRunnable);
            }
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            ViroViewGVR view = mView.get();
            if (view == null) {
                return;
            }
            GLSurfaceView surface = mSurface.get();
            if (surface == null) {
                return;
            }

            /**
             * We have to manually notify the GVR surface presenter
             * if the dimensions of the surface has changed (for example
             * during a rotation).
             */
            final GvrApi gvr = view.mGVRLayout.getGvrApi();
            if (gvr != null) {
                gvr.refreshDisplayMetrics();
            }

            view.mNativeRenderer.onSurfaceChanged(surface.getHolder().getSurface(), width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            ViroViewGVR view = mView.get();
            if (view == null) {
                return;
            }

            for (FrameListener listener : view.mFrameListeners) {
                listener.onDrawFrame();
            }
            view.mNativeRenderer.drawFrame();
        }
    }

    private static class ViroOnTouchListener implements OnTouchListener {

        private WeakReference<ViroViewGVR> mView;

        public ViroOnTouchListener(ViroViewGVR view) {
            mView = new WeakReference<ViroViewGVR>(view);
        }

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            ViroViewGVR view = mView.get();
            if (view == null) {
                return false;
            }

            int action = event.getAction();
            if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_UP) {
                view.mNativeRenderer.onTouchEvent(action, event.getX(), event.getY());
                return true;
            }
            return false;
        }
    }

    /**
     * Create a new ViroViewGVR with the default {@link RendererConfiguration}.
     *
     * @param context         The activity context.
     * @param startupListener Listener to respond to startup success or failure. Will be notified of
     *                        any errors encountered while initializing Viro. Optional, may be null.
     * @param vrExitListener  Runnable to invoke when the user manually exits VR mode by tapping on
     *                        GVR's close button. Optional, may be null.
     */
    public ViroViewGVR(@NonNull final Context context, @Nullable final StartupListener startupListener, @Nullable final Runnable vrExitListener) {
        super(context, null);
        init(context, startupListener, vrExitListener, true);
    }

    /**
     * Create a new ViroViewGVR with the given {@link RendererConfiguration}, which determines
     * the rendering techniques and rendering fidelity to use for this View.
     *
     * @param context         The activity context.
     * @param startupListener Listener to respond to startup success or failure. Will be notified of
     *                        any errors encountered while initializing Viro. Optional, may be null.
     * @param vrExitListener  Runnable to invoke when the user manually exits VR mode by tapping on
     *                        GVR's close button. Optional, may be null.
     * @param config          The {@link RendererConfiguration} to use.
     */
    public ViroViewGVR(@NonNull final Context context, @Nullable final StartupListener startupListener,
                       @Nullable final Runnable vrExitListener, @Nullable RendererConfiguration config) {
        super(context, config);
        init(context, startupListener, vrExitListener, true);
    }

    /**
     * Create a new ViroViewGVR with the given {@link RendererConfiguration}, which determines
     * the rendering techniques and rendering fidelity to use for this View.
     *
     * @param context         The activity context.
     * @param startupListener Listener to respond to startup success or failure. Will be notified of
     *                        any errors encountered while initializing Viro. Optional, may be null.
     * @param vrExitListener  Runnable to invoke when the user manually exits VR mode by tapping on
     *                        GVR's close button. Optional, may be null.
     * @param config          The {@link RendererConfiguration} to use.
     * @param vrMode          Whether to initialize this view in VR (stereo) mode or 360 (mono) mode.
     */
    public ViroViewGVR(@NonNull final Context context, @Nullable final StartupListener startupListener,
                       @Nullable final Runnable vrExitListener, @Nullable RendererConfiguration config,
                       final boolean vrMode) {
        super(context, config);
        init(context, startupListener, vrExitListener, vrMode);
    }

    /**
     * @hide
     *
     * @param context
     */
    public ViroViewGVR(@NonNull final Context context) {
        this(context, null);
    }

    /**
     * @hide
     *
     * @param context
     * @param attrs
     */
    public ViroViewGVR(@NonNull final Context context, @Nullable final AttributeSet attrs) {
        this(context, attrs, 0);
    }

    /**
     * @hide
     *
     * @param context
     * @param attrs
     * @param defStyleAttr
     */
    public ViroViewGVR(@NonNull final Context context, @Nullable final AttributeSet attrs, @AttrRes final int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        if (ContextUtils.getActivity(context) == null) {
            throw new IllegalArgumentException("An Activity Context is required for Viro functionality.");
        } else {
            init(context, null, null, true);
        }

    }

    private void init(final Context context, final StartupListener startupListener, final Runnable vrExitListener, final boolean vrMode) {
        mGVRLayout = new GvrLayout(context);

        // Turn on async reprojection (which skews existing frames and fills them in when we're
        // dropping under 60 FPS). Async reprojection is only available on Daydream-enabled devices.
        // This must be set prior to GVR initialization.
        // TODO VIRO-2269 Async reprojection is causing shadowing issues, so it's disabled here
        //                by using if (false)
        if (false && mGVRLayout.setAsyncReprojectionEnabled(true)) {
            // Scanline racing decouples the app framerate from the display framerate,
            // allowing immersive interaction even at the throttled clockrates set by
            // sustained performance mode.
            AndroidCompat.setSustainedPerformanceMode((Activity) getContext(), true);
        }
        addView(mGVRLayout);

        final Context activityContext = getContext();
        final GLSurfaceView glSurfaceView = createSurfaceView();
        mAssetManager = getResources().getAssets();
        mPlatformUtil = new PlatformUtil(
                new GLSurfaceViewQueue(glSurfaceView),
                mFrameListeners,
                activityContext,
                mAssetManager);
        mNativeRenderer = new Renderer(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(),
                mAssetManager, mPlatformUtil,
                mGVRLayout.getGvrApi().getNativeGvrContext(), mRendererConfig);
        mNativeViroContext = new ViroContext(mNativeRenderer.mNativeRef);
        mStartupListener = startupListener;

        // Add the GLSurfaceView to the GvrLayout.
        mGVRLayout.setPresentationView(glSurfaceView);

        // Prevent screen from dimming/locking.
        final Activity activity = (Activity) getContext();
        activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (vrExitListener != null) {
            mGVRLayout.getUiLayout().setCloseButtonListener(vrExitListener);
        }

        // Default the mode to VR
        setVRModeEnabled(vrMode);

        if (BuildConfig.VIRO_PLATFORM.equalsIgnoreCase(PLATFORM_VIRO_CORE)) {
            validateAPIKeyFromManifest();
        }
    }

    @Override
    protected int getSystemUiVisibilityFlags() {
        if(mVRModeEnabled) {
            return (View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        } else {
            return 0;
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

    /**
     * Create (or update) the {@link GLSurfaceView} to be used by GVR. This view will be shared
     * between both VR and 360 modes.
     */
    private GLSurfaceView createSurfaceView() {
        int colorBits = 8;
        int alphaBits = 0;
        int depthBits = 16;
        int stencilBits = 8;

        GLSurfaceView glSurfaceView = new GLSurfaceView(getContext().getApplicationContext());
        glSurfaceView.setEGLContextClientVersion(3);
        glSurfaceView.setEGLConfigChooser(colorBits, colorBits, colorBits, alphaBits, depthBits, stencilBits);
        glSurfaceView.setPreserveEGLContextOnPause(true);

        glSurfaceView.setRenderer(new ViroSurfaceViewRenderer(this, glSurfaceView));
        glSurfaceView.setOnTouchListener(new ViroOnTouchListener(this));

        return glSurfaceView;
    }

    /**
     * @hide
     */
    @Override
    public void setDebug(boolean debug) {
        /**
         * Only override in the cardboard case because for Daydream it causes the
         * controller to no longer work.
         */
        if (getHeadset().equalsIgnoreCase("cardboard")) {
            AndroidCompat.setVrModeEnabled((Activity) getContext(), !debug);
        }
    }

    @Override
    public void recenterTracking() {
        mGVRLayout.getGvrApi().recenterTracking();
    }

    @Override
    public void setScene(Scene scene) {
        if (scene == mCurrentScene) {
            return;
        }
        super.setScene(scene);
        mNativeRenderer.setSceneController(scene.mNativeRef, 0.5f);
    }

    @Override
    public void setVRModeEnabled(boolean vrModeEnabled) {
        mVRModeEnabled = vrModeEnabled;
        changeSystemUiVisibility();
        Activity activity = mWeakActivity.get();
        if (activity != null) {
            activity.setRequestedOrientation(vrModeEnabled ?
                    ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE : ActivityInfo.SCREEN_ORIENTATION_SENSOR);
        }

        mGVRLayout.setStereoModeEnabled(vrModeEnabled);
        mGVRLayout.getUiLayout().setEnabled(vrModeEnabled);
        mNativeRenderer.setVRModeEnabled(vrModeEnabled);


        // We want Android's VR mode on when the app is in VR and release build OR always when
        // using Daydream. We don't want this on in Debug mode because it this option turns
        // off the "Draw over other apps" permission that React Native requires for the dev menu.
        // The renderer (library) is always in release, so we use BuildInfo to check the debug
        // status of the app we're compiled into.
        if ((vrModeEnabled && !BuildInfo.isDebug(getContext())) || getHeadset().equalsIgnoreCase("daydream")) {
            AndroidCompat.setVrModeEnabled((Activity) getContext(), true);
        } else {
            AndroidCompat.setVrModeEnabled((Activity) getContext(), false);

        }
    }

    /**
     * Runnable to invoke when the user manually exits VR mode by
     * tapping on GVR's close button.
     *
     * @param vrExitRunnable {@link Runnable}
     */
    public void setVRExitRunnable(final Runnable vrExitRunnable) {
        mGVRLayout.getUiLayout().setCloseButtonListener(vrExitRunnable);
    }

    /**
     * @hide
     */
    @Override
    public ViroMediaRecorder getRecorder() {
        return null;
    }

    /**
     * @hide
     */
    @Override
    public String getPlatform() {
        return "gvr";
    }

    /**
     * @hide
     */
    @Override
    public void onActivityPaused(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mNativeRenderer.onPause();
        mGVRLayout.onPause();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityResumed(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mNativeRenderer.onResume();

        // Ensure fullscreen immersion.
        changeSystemUiVisibility();
        mGVRLayout.onResume();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityDestroyed(Activity activity) {
        this.dispose();
    }

    /**
     * @hide
     */
    @Override
    public void dispose() {
        if (!mDestroyed) {
            mGVRLayout.shutdown();
            // once we dispose the view, we should turn off the VR specific stuff.
            AndroidCompat.setVrModeEnabled((Activity) getContext(), false);
            AndroidCompat.setSustainedPerformanceMode((Activity) getContext(), false);
        }

        super.dispose();
    }

    /**
     * @hide
     */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        // Avoid accidental volume key presses while the phone is in the VR headset.
        if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP
                || event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_DOWN) {
            return true;
        }
        return mGVRLayout.dispatchKeyEvent(event);
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
        //No-op
    }

}
