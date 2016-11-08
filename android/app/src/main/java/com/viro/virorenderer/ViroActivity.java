package com.viro.virorenderer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Vibrator;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import com.google.vr.ndk.base.AndroidCompat;
import com.google.vr.ndk.base.GvrLayout;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class ViroActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        System.loadLibrary("native-lib");
    }

    private GvrLayout gvrLayout;
    private long nativeTreasureHuntRenderer;

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
        gvrLayout = new GvrLayout(this);
        nativeTreasureHuntRenderer =
                nativeCreateRenderer(
                        getClass().getClassLoader(),
                        this.getApplicationContext(),
                        gvrLayout.getGvrApi().getNativeGvrContext());

        // Add the GLSurfaceView to the GvrLayout.
        GLSurfaceView glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.setEGLContextClientVersion(2);
        glSurfaceView.setEGLConfigChooser(8, 8, 8, 0, 0, 0);
        glSurfaceView.setPreserveEGLContextOnPause(true);
        glSurfaceView.setRenderer(
                new GLSurfaceView.Renderer() {
                    @Override
                    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                        nativeInitializeGl(nativeTreasureHuntRenderer);
                    }

                    @Override
                    public void onSurfaceChanged(GL10 gl, int width, int height) {}

                    @Override
                    public void onDrawFrame(GL10 gl) {
                        nativeDrawFrame(nativeTreasureHuntRenderer);
                    }
                });
        glSurfaceView.setOnTouchListener(
                new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        if (event.getAction() == MotionEvent.ACTION_DOWN) {
                            // Give user feedback and signal a trigger event.
                            ((Vibrator) getSystemService(Context.VIBRATOR_SERVICE)).vibrate(50);
                            nativeOnTriggerEvent(nativeTreasureHuntRenderer);
                            return true;
                        }
                        return false;
                    }
                });
        gvrLayout.setPresentationView(glSurfaceView);

        // Add the GvrLayout to the View hierarchy.
        setContentView(gvrLayout);

        // Enable scan line racing.
        if (gvrLayout.setAsyncReprojectionEnabled(true)) {
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
        nativeOnPause(nativeTreasureHuntRenderer);
        gvrLayout.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        nativeOnResume(nativeTreasureHuntRenderer);
        gvrLayout.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Destruction order is important; shutting down the GvrLayout will detach
        // the GLSurfaceView and stop the GL thread, allowing safe shutdown of
        // native resources from the UI thread.
        gvrLayout.shutdown();
        nativeDestroyRenderer(nativeTreasureHuntRenderer);
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

    private native long nativeCreateRenderer(
            ClassLoader appClassLoader, Context context, long nativeGvrContext);
    private native void nativeDestroyRenderer(long nativeTreasureHuntRenderer);
    private native void nativeInitializeGl(long nativeTreasureHuntRenderer);
    private native long nativeDrawFrame(long nativeTreasureHuntRenderer);
    private native void nativeOnTriggerEvent(long nativeTreasureHuntRenderer);
    private native void nativeOnPause(long nativeTreasureHuntRenderer);
    private native void nativeOnResume(long nativeTreasureHuntRenderer);
}
