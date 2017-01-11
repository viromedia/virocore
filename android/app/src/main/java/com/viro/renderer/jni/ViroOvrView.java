/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

import com.viro.renderer.FrameListener;
import com.viro.renderer.RenderCommandQueue;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class ViroOvrView extends SurfaceView implements VrView, Application.ActivityLifecycleCallbacks, SurfaceHolder.Callback {

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
    private List<FrameListener> mFrameListeners = new ArrayList();

    private PlatformUtil mPlatformUtil;

    public ViroOvrView(Activity activity) {
        super(activity);
        getHolder().addCallback(this);

        final Context activityContext = getContext();

        mAssetManager = getResources().getAssets();

        OVRRenderCommandQueue commandQueue = new OVRRenderCommandQueue();
        mPlatformUtil = new PlatformUtil(commandQueue, mFrameListeners, activityContext, mAssetManager);
        mFrameListeners.add(commandQueue);

        mNativeRenderer = new RendererJni(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(),
                this, activity, mAssetManager, mPlatformUtil);

        mNativeRenderContext = new RenderContextJni(mNativeRenderer.mNativeRef);

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
        mNativeRenderer.setScene(scene.mNativeRef);
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
        mNativeRenderer.onPause();
    }

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
        return super.dispatchKeyEvent(event);
    }

    public void setImmersiveSticky() {
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

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {
        //No-op
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mNativeRenderer.onSurfaceCreated(holder.getSurface());
        mNativeRenderer.initalizeGl();
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
}
