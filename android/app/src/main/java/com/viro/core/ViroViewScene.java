/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.AttrRes;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;

import com.google.vr.cardboard.ContextUtils;
import com.viro.core.internal.FrameListener;
import com.viro.core.internal.GLSurfaceViewQueue;
import com.viro.core.internal.GLTextureView;
import com.viro.core.internal.PlatformUtil;
import com.viro.core.internal.ViroTouchGestureListener;
import com.viro.renderer.BuildConfig;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * ViroViewScene is a {@link ViroView} for rendering a {@link Scene} on a simple Android View.
 */
public class ViroViewScene extends ViroView {

    private static final String TAG = "Viro";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        System.loadLibrary("native-lib");
    }


    private static class ViroARRenderer implements GLTextureView.Renderer {
        private WeakReference<ViroViewScene> mView;

        public ViroARRenderer(ViroViewScene view) {
            mView = new WeakReference<ViroViewScene>(view);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            ViroViewScene view = mView.get();
            if (view == null) {
                return;
            }

            view.mNativeRenderer.onSurfaceCreated(null);
            view.mNativeRenderer.initalizeGl();
            if (view.mRenderStartListener != null) {
                Runnable myRunnable = new Runnable() {
                    @Override
                    public void run() {
                        ViroViewScene view = mView.get();
                        if (view == null || view.mDestroyed) {
                            return;
                        }
                        view.mRenderStartListener.onRendererStart();
                    }
                };
                new Handler(Looper.getMainLooper()).post(myRunnable);
            }
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            ViroViewScene view = mView.get();
            if (view == null) {
                return;
            }
            view.mNativeRenderer.onSurfaceChanged(null, width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            ViroViewScene view = mView.get();
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
    private GLTextureView mSurfaceView;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners = new ArrayList();
    private RendererStartListener mRenderStartListener = null;
    private PlatformUtil mPlatformUtil;
    private boolean mActivityPaused = true;
    private ViroMediaRecorder mMediaRecorder;
    private ViroTouchGestureListener mViroTouchGestureListener;

    /**
     * Create a new ViroViewScene.
     *
     * @param context               The activity context.
     * @param rendererStartListener Runnable to invoke when the renderer has finished initializing.
     *                              Optional, may be null.
     */
    public ViroViewScene(Context context, RendererStartListener rendererStartListener) {
        super(context);
        init(context, rendererStartListener);
    }

    /**
     * @hide
     *
     * @param context
     */
    public ViroViewScene(@NonNull final Context context) {
        this(context, (AttributeSet) null);
    }

    /**
     * @hide
     *
     * @param context
     * @param attrs
     */
    public ViroViewScene(@NonNull final Context context, @Nullable final AttributeSet attrs) {
        this(context, attrs, 0);

    }

    /**
     * @hide
     *
     * @param context
     * @param attrs
     * @param defStyleAttr
     */
    public ViroViewScene(@NonNull final Context context, @Nullable final AttributeSet attrs, @AttrRes final int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        if (ContextUtils.getActivity(context) == null) {
            throw new IllegalArgumentException("An Activity Context is required for Viro functionality.");
        } else {
            init(context, null);
        }
    }

    private void init(Context context, RendererStartListener rendererStartListener) {
        mSurfaceView = new GLTextureView(context);
        addView(mSurfaceView);

        final Context activityContext = getContext();
        final Activity activity = (Activity) getContext();
        initSurfaceView();

        mAssetManager = getResources().getAssets();
        mPlatformUtil = new PlatformUtil(
                new GLSurfaceViewQueue(mSurfaceView),
                mFrameListeners,
                activityContext,
                mAssetManager);
        mNativeRenderer = new Renderer(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(), this,
                mAssetManager, mPlatformUtil);
        mNativeViroContext = new ViroContext(mNativeRenderer.mNativeRef);
        mRenderStartListener = rendererStartListener;
        mViroTouchGestureListener = new ViroTouchGestureListener(activity, mNativeRenderer);
        setOnTouchListener(mViroTouchGestureListener);

        if (BuildConfig.FLAVOR.equalsIgnoreCase(FLAVOR_VIRO_CORE)) {
            validateAPIKeyFromManifest();
        }
    }
    private void initSurfaceView() {
        int colorBits = 8;
        int alphaBits = 8;
        int depthBits = 16;
        int stencilBits = 8;

        mSurfaceView.setEGLContextClientVersion(3);
        mSurfaceView.setEGLConfigChooser(colorBits, colorBits, colorBits, alphaBits, depthBits, stencilBits);
        mSurfaceView.setPreserveEGLContextOnPause(true);

        mSurfaceView.setRenderer(new ViroARRenderer(this));
        mSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    /**
     * Sets the background color of this Viro renderer Android view. This will be
     * the color that renders behind all Viro views.
     *
     * @param color The {@link android.graphics.Color}'s int value.
     */
    @Override
    public void setBackgroundColor(int color) {
        int alpha  = Color.alpha(color);
        mSurfaceView.setOpaque(alpha > 0);
        mNativeRenderer.setClearColor(color);
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
        super.setScene(scene);
        mNativeRenderer.setSceneController(scene.mNativeRef, 0.5f);
    }

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
    public void setVRModeEnabled(boolean vrModeEnabled) {
        //No-op
    }

    /**
     * @hide
     */
    @Override
    public String getPlatform() {
        return "scene";
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
        mSurfaceView.onPause();
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
        mSurfaceView.onResume();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityDestroyed(Activity activity) {
        this.dispose();
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

}