/**
 * Copyright (c) 2015-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
package com.viro.renderer;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioManager;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.ndk.base.AndroidCompat;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Custom Android view encapsulating all GvrLayout
 * setup and initialization for displaying a VR view.
 * Create this view during or post onCreate within
 * the activity lifecycle.
 */
public class ViroGvrLayout extends GvrLayout implements Application.ActivityLifecycleCallbacks {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        System.loadLibrary("native-lib");
    }
    private long mNativeRenderer;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners = new ArrayList();
    private Map<Integer, VideoSink> mVideoSinks = new HashMap();
    private boolean mVrModeEnabled;

    public ViroGvrLayout(Context context, boolean vrModeEnabled) {
        super(context);
        mVrModeEnabled = vrModeEnabled;

        final Context activityContext = getContext();

        // Initialize the native renderer.
        mAssetManager = getResources().getAssets();
        mNativeRenderer = nativeCreateRenderer(
                getClass().getClassLoader(),
                this,
                activityContext.getApplicationContext(),
                mAssetManager,
                getGvrApi().getNativeGvrContext());

        // Add the GLSurfaceView to the GvrLayout.
        GLSurfaceView glSurfaceView = new GLSurfaceView(activityContext.getApplicationContext());
        glSurfaceView.setEGLContextClientVersion(3);
        glSurfaceView.setEGLConfigChooser(8, 8, 8, 0, 0, 0);
        glSurfaceView.setPreserveEGLContextOnPause(true);
        glSurfaceView.setRenderer(new Renderer() {
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                nativeInitializeGl(mNativeRenderer);
            }

            public void onSurfaceChanged(GL10 gl, int width, int height) {
                /**
                 * We have to manually notify the GVR surface presenter
                 * if the dimensions of the surface has changed (for example
                 * during a rotation).
                 */
                final GvrApi gvr = getGvrApi();
                if (gvr != null){
                    gvr.refreshDisplayMetrics();
                }
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                for (FrameListener listener : mFrameListeners) {
                    listener.onDrawFrame();
                    ;
                }
                nativeDrawFrame(mNativeRenderer);
            }
        });

        glSurfaceView.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    nativeOnTriggerEvent(mNativeRenderer);
                    return true;
                }
                return false;
            }
        });

        setPresentationView(glSurfaceView);

        // Enable scan line racing.
        if(setAsyncReprojectionEnabled(true)) {

            // Scanline racing decouples the app framerate from the display framerate,
            // allowing immersive interaction even at the throttled clockrates set by
            // sustained performance mode.
            AndroidCompat.setSustainedPerformanceMode((Activity)activityContext, true);
        }

        // Enable VR Mode.
        AndroidCompat.setVrModeEnabled((Activity)activityContext, mVrModeEnabled);
        if (mVrModeEnabled){
            ((Activity)activityContext).setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        // Prevent screen from dimming/locking.
        ((Activity)activityContext).getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        Application app = (Application)activityContext.getApplicationContext();
        app.registerActivityLifecycleCallbacks(this);
    }

    @Override
    public void onActivityPaused(Activity activity) {
        nativeOnPause(mNativeRenderer);
        super.onPause();
    }

    @Override
    public void onActivityResumed(Activity activity) {
        nativeOnResume(mNativeRenderer);

        // Ensure fullscreen immersion.
        setImmersiveSticky();
        final Context activityContext = getContext();
        ((Activity)activityContext)
                .getWindow()
                .getDecorView()
                .setOnSystemUiVisibilityChangeListener(
                        new OnSystemUiVisibilityChangeListener() {
                            @Override
                            public void onSystemUiVisibilityChange(int visibility) {
                                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                                    setImmersiveSticky();
                                }
                            }
                        });
        super.onResume();
    }

    @Override
    public void onActivityDestroyed(Activity activity) {
        super.shutdown();
        nativeDestroyRenderer(this.mNativeRenderer);
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

    // Accessed by Native code (VROPlatformUtil.cpp)
    public Bitmap loadBitmap(String assetPath) {
        InputStream in;
        Bitmap bitmap = null;
        try {
            in = mAssetManager.open(assetPath);
            bitmap = BitmapFactory.decodeStream(in);

            in.close();
        } catch (IOException e) {
        }

        return bitmap;
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public Surface createVideoSink(int textureId) {
        VideoSink videoSink = new VideoSink(textureId);
        mVideoSinks.put(textureId, videoSink);
        mFrameListeners.add(videoSink);

        return videoSink.getSurface();
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public void destroyVideoSink(int textureId) {
        VideoSink videoSink = mVideoSinks.remove(textureId);
        mFrameListeners.remove(videoSink);

        videoSink.releaseSurface();
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public int getAudioSampleRate() {
        AudioManager audioManager = (AudioManager) getContext().getSystemService(Context.AUDIO_SERVICE);
        String nativeParam = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        return Integer.parseInt(nativeParam);
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public int getAudioBufferSize() {
        AudioManager audioManager = (AudioManager) getContext().getSystemService(Context.AUDIO_SERVICE);
        String nativeParam = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        return Integer.parseInt(nativeParam);
    }

    @Override
    public void onActivityCreated(Activity activity, Bundle bundle) {
        //No-op
    }

    @Override
    public void onActivityStarted(Activity activity) {
        //No-op
    }

    @Override
    public void onActivityStopped(Activity activity) {
        //No-op
    }

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {
        //No-op
    }

    private native long nativeCreateRenderer(
            ClassLoader appClassLoader, ViroGvrLayout view, Context context, AssetManager assets, long nativeGvrContext);
    private native void nativeDestroyRenderer(long nativeRenderer);
    private native void nativeInitializeGl(long nativeRenderer);
    private native long nativeDrawFrame(long nativeRenderer);
    private native void nativeOnTriggerEvent(long nativeRenderer);
    private native void nativeOnPause(long nativeRenderer);
    private native void nativeOnResume(long nativeRenderer);
}
