package com.viro.renderer;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Vibrator;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;
import com.google.vr.ndk.base.AndroidCompat;
import com.google.vr.ndk.base.GvrLayout;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class ViroActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        System.loadLibrary("native-lib");
    }

    private GvrLayout mGVRLayout;
    private long mNativeRenderer;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners = new ArrayList<FrameListener>();
    private Map<Integer, VideoSink> mVideoSinks = new HashMap<Integer, VideoSink>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Ensure fullscreen immersion.
        setImmersiveSticky();
        getWindow()
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

        // Initialize GvrLayout and the native renderer.
        mGVRLayout = new GvrLayout(this);
        mAssetManager = getResources().getAssets();
        mNativeRenderer =
                nativeCreateRenderer(
                        getClass().getClassLoader(),
                        this,
                        this.getApplicationContext(),
                        mAssetManager,
                        mGVRLayout.getGvrApi().getNativeGvrContext());

        // Add the GLSurfaceView to the GvrLayout.
        GLSurfaceView glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.setEGLContextClientVersion(3);
        glSurfaceView.setEGLConfigChooser(8, 8, 8, 0, 0, 0);
        glSurfaceView.setPreserveEGLContextOnPause(true);
        glSurfaceView.setRenderer(
                new GLSurfaceView.Renderer() {
                    @Override
                    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                        nativeInitializeGl(mNativeRenderer);
                    }

                    @Override
                    public void onSurfaceChanged(GL10 gl, int width, int height) {
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
        glSurfaceView.setOnTouchListener(
                new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        if (event.getAction() == MotionEvent.ACTION_DOWN) {
                            // Give user feedback and signal a trigger event.
                            ((Vibrator) getSystemService(Context.VIBRATOR_SERVICE)).vibrate(50);
                            nativeOnTriggerEvent(mNativeRenderer);
                            return true;
                        }
                        return false;
                    }
                });
        mGVRLayout.setPresentationView(glSurfaceView);

        // Add the GvrLayout to the View hierarchy.
        setContentView(mGVRLayout);

        // Enable scan line racing.
        if (mGVRLayout.setAsyncReprojectionEnabled(true)) {
            // Scanline racing decouples the app framerate from the display framerate,
            // allowing immersive interaction even at the throttled clockrates set by
            // sustained performance mode.
            AndroidCompat.setSustainedPerformanceMode(this, true);
        }

        // Enable VR Mode.
        AndroidCompat.setVrModeEnabled(this, true);

        // Prevent screen from dimming/locking.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    protected void onPause() {
        super.onPause();
        nativeOnPause(mNativeRenderer);
        mGVRLayout.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        nativeOnResume(mNativeRenderer);
        mGVRLayout.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Destruction order is important; shutting down the GvrLayout will detach
        // the GLSurfaceView and stop the GL thread, allowing safe shutdown of
        // native resources from the UI thread.
        mGVRLayout.shutdown();
        nativeDestroyRenderer(mNativeRenderer);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            setImmersiveSticky();
        }
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

    private void setImmersiveSticky() {
        getWindow()
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
    public void removeVideoSink(int textureId) {
        VideoSink videoSink = mVideoSinks.remove(textureId);
        mFrameListeners.remove(videoSink);

        videoSink.releaseSurface();
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public int getAudioSampleRate() {
        AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        String nativeParam = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        return Integer.parseInt(nativeParam);
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public int getAudioBufferSize() {
        AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        String nativeParam = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        return Integer.parseInt(nativeParam);
    }

    private native long nativeCreateRenderer(
            ClassLoader appClassLoader, ViroActivity activity, Context context, AssetManager assets, long nativeGvrContext);
    private native void nativeDestroyRenderer(long nativeRenderer);
    private native void nativeInitializeGl(long nativeRenderer);
    private native long nativeDrawFrame(long nativeRenderer);
    private native void nativeOnTriggerEvent(long nativeRenderer);
    private native void nativeOnPause(long nativeRenderer);
    private native void nativeOnResume(long nativeRenderer);
}
