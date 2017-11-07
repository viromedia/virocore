/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

import com.viro.renderer.FrameListener;
import com.viro.renderer.RenderCommandQueue;

import java.util.List;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * ViroViewOVR is a {@link ViroView} for rendering content in stereo for VR headsets using
 * the Oculus Mobile SDK.
 */
public class ViroViewOVR extends ViroView implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("vrapi");
        System.loadLibrary("native-lib");
    }

    /**
     * Executes events on the renderer thread by collecting them in a
     * queue and running them as a {@link FrameListener}.
     */
    static class OVRRenderCommandQueue implements RenderCommandQueue, FrameListener {

        private Queue<Runnable> mQueue = new ConcurrentLinkedQueue<Runnable>();

        @Override
        public void queueEvent(Runnable r) {
            mQueue.add(r);
        }

        @Override
        public void onDrawFrame() {
            while (!mQueue.isEmpty()) {
                Runnable r = mQueue.poll();
                if (r != null) {
                    r.run();
                }
            }
        }
    }

    private SurfaceView mSurfaceView;
    private AssetManager mAssetManager;
    private OVRRenderCommandQueue mRenderQueue = new OVRRenderCommandQueue();
    private List<FrameListener> mFrameListeners = new CopyOnWriteArrayList<FrameListener>();
    private RendererStartListener mRenderStartListener = null;
    private PlatformUtil mPlatformUtil;

    /**
     * Create a new ViroViewOVR.
     *
     * @param activity              The activity containing the view.
     * @param rendererStartListener Runnable to invoke when the renderer has finished initializing.
     *                              Optional, may be null.
     */
    public ViroViewOVR(Activity activity, RendererStartListener rendererStartListener) {
        super(activity);
        mSurfaceView = new SurfaceView(activity);
        mSurfaceView.getHolder().addCallback(this);
        addView(mSurfaceView);

        final Context activityContext = getContext();

        mAssetManager = getResources().getAssets();

        mPlatformUtil = new PlatformUtil(mRenderQueue, mFrameListeners, activityContext, mAssetManager);
        mFrameListeners.add(mRenderQueue);

        mNativeRenderer = new Renderer(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(),
                this, activity, mAssetManager, mPlatformUtil);

        mNativeViroContext = new ViroContext(mNativeRenderer.mNativeRef);
        mRenderStartListener = rendererStartListener;

        // Note: unlike GVR we don't have to worry about restoring these Activity settings because
        // OVR apps aren't hybrid 2D -> VR applications.
        // Prevent screen from dimming/locking.
        activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // Prevent screen from switching to portrait
        activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }

    @Override
    public void recenterTracking() {
        mNativeRenderer.recenterTracking();
    }

    @Override
    public ViroMediaRecorder getRecorder() {
        return null; // Recorder is not supported.
    }

    @Override
    public void setScene(Scene scene) {
        mNativeRenderer.setSceneController(scene.mNativeRef, 1.0f);
    }

    /**
     * @hide
     */
    @Override
    public void setDebug(boolean debug) {
        // no-op here
    }

    /**
     * @hide
     */
    @Override
    public void setVRModeEnabled(boolean vrModeEnabled) {

    }

    /**
     * @hide
     */
    @Override
    public String getPlatform() {
        return "ovr-mobile";
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
    public void onActivityPaused(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }
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
        mNativeRenderer.onResume();
    }

    /**
     * @hide
     */
    @Override
    public void onActivityDestroyed(Activity activity) {
        // No-op
    }

    /**
     * @hide
     */
    @Override
    public void dispose() {
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
        mNativeRenderer.onKeyEvent(event.getKeyCode(), event.getAction());
        return true;
    }

    /**
     * @hide
     */
    @Override
    public boolean onTouchEvent( MotionEvent event ){
        mNativeRenderer.onTouchEvent(event.getAction(), event.getX(), event.getY());
        return true;
    }

    /**
     * @hide
     */
    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {
        //No-op
    }

    /**
     * @hide
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mNativeRenderer.onSurfaceCreated(holder.getSurface());
        mRenderQueue.queueEvent(new Runnable() {
            @Override
            public void run() {
                mNativeRenderer.initalizeGl();
            }
        });
        if (mRenderStartListener != null) {
            mRenderStartListener.onRendererStart();
        }
    }

    /**
     * @hide
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mNativeRenderer.onSurfaceChanged(holder.getSurface(), width, height);
    }

    /**
     * @hide
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mNativeRenderer.onSurfaceDestroyed(holder.getSurface());
    }

    /**
     * Accessed by native code (VROSceneRendererOVR.cpp)
     * @hide
     */
    void onDrawFrame() {
        for (FrameListener listener : mFrameListeners) {
            listener.onDrawFrame();
        }
    }
}