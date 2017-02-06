/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

import com.viro.renderer.FrameListener;
import com.viro.renderer.RenderCommandQueue;

import java.util.List;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CopyOnWriteArrayList;

public class ViroOvrView extends SurfaceView implements VrView, SurfaceHolder.Callback {

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

    private RendererJni mNativeRenderer;
    private final RenderContextJni mNativeRenderContext;
    private AssetManager mAssetManager;
    private OVRRenderCommandQueue mRenderQueue = new OVRRenderCommandQueue();
    private List<FrameListener> mFrameListeners = new CopyOnWriteArrayList<FrameListener>();
    private GlListener mGlListener = null;
    private PlatformUtil mPlatformUtil;

    public ViroOvrView(Activity activity, GlListener glListener) {
        super(activity);
        getHolder().addCallback(this);

        final Context activityContext = getContext();

        mAssetManager = getResources().getAssets();

        mPlatformUtil = new PlatformUtil(mRenderQueue, mFrameListeners, activityContext, mAssetManager);
        mFrameListeners.add(mRenderQueue);

        mNativeRenderer = new RendererJni(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(),
                this, activity, mAssetManager, mPlatformUtil);

        mNativeRenderContext = new RenderContextJni(mNativeRenderer.mNativeRef);

        mGlListener = glListener;

        // Prevent screen from dimming/locking.
        activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        Application app = (Application)activityContext.getApplicationContext();
        app.registerActivityLifecycleCallbacks(this);
    }

    @Override
    public RenderContextJni getRenderContextRef(){
        return mNativeRenderContext;
    }

    @Override
    public void setScene(SceneJni scene) {
        mNativeRenderer.setScene(scene.mNativeRef, 1.0f);
    }

    @Override
    public void setVrModeEnabled(boolean vrModeEnabled) {

    }

    @Override
    public RendererJni getNativeRenderer() {
        return mNativeRenderer;
    }

    @Override
    public View getContentView() { return this; }

    @Override
    public void onActivityCreated(Activity activity, Bundle bundle) {
        //No-op
    }

    @Override
    public void onActivityStarted(Activity activity) {
        mNativeRenderer.onStart();
    }

    @Override
    public void onActivityStopped(Activity activity) {
        mNativeRenderer.onStop();
    }

    @Override
    public void onActivityPaused(Activity activity) {
        mNativeRenderer.onPause(); }

    @Override
    public void onActivityResumed(Activity activity) {
        mNativeRenderer.onResume();

        // Ensure fullscreen immersion.
        setImmersiveSticky();
        final Context activityContext = getContext();
        ((Activity)activityContext)
                .getWindow()
                .getDecorView()
                .setOnSystemUiVisibilityChangeListener(
                        new View.OnSystemUiVisibilityChangeListener() {
                            @Override
                            public void onSystemUiVisibilityChange(int visibility) {
                                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                                    setImmersiveSticky();
                                }
                            }
                        });
    }

    @Override
    public void onActivityDestroyed(Activity activity) {
        mNativeRenderContext.delete();
        mNativeRenderer.destroy();
    }

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

    @Override
    public boolean onTouchEvent( MotionEvent event ){
        mNativeRenderer.onTouchEvent(event.getAction(), event.getX(), event.getY());
        return true;
    }

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {
        //No-op
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mNativeRenderer.onSurfaceCreated(holder.getSurface());
        mRenderQueue.queueEvent(new Runnable() {
            @Override
            public void run() {
                mNativeRenderer.initalizeGl();
            }
        });
        if (mGlListener != null) {
            mGlListener.onGlInitialized();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mNativeRenderer.onSurfaceChanged(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mNativeRenderer.onSurfaceDestroyed(holder.getSurface());
    }

    // Accessed by Native code (VROSceneRendererOVR.cpp)
    public void onDrawFrame() {
        for (FrameListener listener : mFrameListeners) {
            listener.onDrawFrame();
        }
    }

    private void setImmersiveSticky() {
        ((Activity)getContext())
                .getWindow()
                .getDecorView()
                .setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }

}