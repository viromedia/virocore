/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

import com.viro.renderer.FrameListener;
import com.viro.renderer.RenderCommandQueue;
import com.viro.renderer.keys.KeyValidationListener;
import com.viro.renderer.keys.KeyValidator;

import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CopyOnWriteArrayList;

public class ViroOvrView extends SurfaceView implements ViroView, SurfaceHolder.Callback {

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

    private Renderer mNativeRenderer;
    private final ViroContext mNativeViroContext;
    private AssetManager mAssetManager;
    private OVRRenderCommandQueue mRenderQueue = new OVRRenderCommandQueue();
    private List<FrameListener> mFrameListeners = new CopyOnWriteArrayList<FrameListener>();
    private GLListener mGlListener = null;
    private PlatformUtil mPlatformUtil;
    private WeakReference<Activity> mWeakActivity;
    private KeyValidator mKeyValidator;
    private boolean mDestroyed = false;

    public ViroOvrView(Activity activity, GLListener glListener) {
        super(activity);
        getHolder().addCallback(this);

        final Context activityContext = getContext();

        mAssetManager = getResources().getAssets();

        mPlatformUtil = new PlatformUtil(mRenderQueue, mFrameListeners, activityContext, mAssetManager);
        mFrameListeners.add(mRenderQueue);

        mNativeRenderer = new Renderer(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(),
                this, activity, mAssetManager, mPlatformUtil);

        mNativeViroContext = new ViroContext(mNativeRenderer.mNativeRef);

        mGlListener = glListener;

        mKeyValidator = new KeyValidator(activityContext);

        // Note: unlike GVR we don't have to worry about restoring these Activity settings because
        // OVR apps aren't hybrid 2D -> VR applications.
        // Prevent screen from dimming/locking.
        activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // Prevent screen from switching to portrait
        activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        mWeakActivity = new WeakReference<Activity>((Activity)getContext());
    }

    @Override
    public void setDebug(boolean debug) {
        // no-op here
    }

    @Override
    public void setDebugHUDEnabled(boolean enabled) {
        mNativeRenderer.setDebugHUDEnabled(enabled);
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
    public ViroContext getViroContext(){
        return mNativeViroContext;
    }

    @Override
    public void setScene(Scene scene) {
        mNativeRenderer.setSceneController(scene.mNativeRef, 1.0f);
    }

    @Override
    public void setVrModeEnabled(boolean vrModeEnabled) {

    }

    @Override
    public void validateApiKey(String apiKey) {
        mNativeRenderer.setSuspended(false);
        // we actually care more about the headset than platform in this case.
        mKeyValidator.validateKey(apiKey, getHeadset(), new KeyValidationListener() {
            @Override
            public void onResponse(boolean success) {
                if (!mDestroyed) {
                    mNativeRenderer.setSuspended(!success);
                }
            }
        });
    }

    @Override
    public Renderer getRenderer() {
        return mNativeRenderer;
    }

    @Override
    public View getContentView() { return this; }

    @Override
    public String getPlatform() {
        return "ovr-mobile";
    }

    @Override
    public String getHeadset() {
        return mNativeRenderer.getHeadset();
    }

    @Override
    public String getController() {
        return mNativeRenderer.getController();
    }

    @Override
    public void onActivityCreated(Activity activity, Bundle bundle) {
        //No-op
    }

    @Override
    public void onActivityStarted(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }
        mNativeRenderer.onStart();
    }

    @Override
    public void onActivityStopped(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mNativeRenderer.onStop();
    }

    @Override
    public void onActivityPaused(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }
        mNativeRenderer.onPause();
    }

    @Override
    public void onActivityResumed(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }
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
        // No-op
    }

    @Override
    public void destroy() {
        mDestroyed = true;
        mNativeViroContext.dispose();
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
        mNativeRenderer.onSurfaceChanged(holder.getSurface(), width, height);
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
        refreshActivityLayout();
    }


    private void refreshActivityLayout(){
        Runnable myRunnable = new Runnable() {
            @Override
            public void run() {
                View view = ((Activity)getContext()).findViewById(android.R.id.content);
                if (view != null){
                    // The size of the activity's root view has changed since we have
                    // manually removed the toolbar to enter full screen. Thus, call
                    // requestLayout to re-measure the view sizes.
                    view.requestLayout();

                    // To be certain a relayout will result in a redraw, we call invalidate
                    view.invalidate();
                }
            }
        };
        new Handler(Looper.getMainLooper()).post(myRunnable);
    }
}