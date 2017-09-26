/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.ndk.base.AndroidCompat;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;
import com.viro.renderer.BuildInfo;
import com.viro.renderer.FrameListener;
import com.viro.renderer.GLSurfaceViewQueue;
import com.viro.renderer.keys.KeyValidationListener;
import com.viro.renderer.keys.KeyValidator;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

/**
 * Custom Android view encapsulating all GvrLayout
 * setup and initialization for displaying a VR view.
 * Create this view during or post onCreate within
 * the activity lifecycle.
 */
public class ViroGvrLayout extends GvrLayout implements VrView {
    private static final String TAG = "Viro";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        System.loadLibrary("native-lib");
    }
    private RendererJni mNativeRenderer;
    private final RenderContextJni mNativeRenderContext;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners = new ArrayList();
    private GlListener mGlListener = null;
    private PlatformUtil mPlatformUtil;
    private WeakReference<Activity> mWeakActivity;
    private KeyValidator mKeyValidator;
    private boolean mDestroyed = false;

    // Activity state to restore to before being modified by the renderer.
    private int mSavedSystemUIVisbility;
    private int mSavedOrientation;
    private OnSystemUiVisibilityChangeListener mSystemVisibilityListener;

    private static class ViroEGLWindowSurfaceFactory implements GLSurfaceView.EGLWindowSurfaceFactory {
        public javax.microedition.khronos.egl.EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display,
                                                                             EGLConfig config, Object nativeWindow) {
            javax.microedition.khronos.egl.EGLSurface result = null;
            try {
                final int EGL_GL_COLORSPACE_KHR = 0x309D;
                final int EGL_GL_COLORSPACE_SRGB_KHR = 0x3089;
                final int[] surfaceAttribs = {
                        EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR,
                        EGL10.EGL_NONE
                };

                result = egl.eglCreateWindowSurface(display, config, nativeWindow, surfaceAttribs);
            } catch (IllegalArgumentException e) {
                // This exception indicates that the surface flinger surface
                // is not valid. This can happen if the surface flinger surface has
                // been torn down, but the application has not yet been
                // notified via SurfaceHolder.Callback.surfaceDestroyed.
                // In theory the application should be notified first,
                // but in practice sometimes it is not. See b/4588890
                Log.e(TAG, "eglCreateWindowSurface", e);
            }
            return result;
        }

        public void destroySurface(EGL10 egl, EGLDisplay display,
                                   javax.microedition.khronos.egl.EGLSurface surface) {
            egl.eglDestroySurface(display, surface);
        }
    }

    private static class ViroSurfaceViewRenderer implements GLSurfaceView.Renderer {

        private WeakReference<ViroGvrLayout> mView;
        private WeakReference<GLSurfaceView> mSurface;

        public ViroSurfaceViewRenderer(ViroGvrLayout view, GLSurfaceView surface) {
            mView = new WeakReference<ViroGvrLayout>(view);
            mSurface = new WeakReference<GLSurfaceView>(surface);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            ViroGvrLayout view = mView.get();
            if (view == null) {
                return;
            }
            GLSurfaceView surface = mSurface.get();
            if (surface == null) {
                return;
            }

            view.mNativeRenderer.onSurfaceCreated(surface.getHolder().getSurface());
            view.mNativeRenderer.initalizeGl();
            if (view.mGlListener != null) {
                Runnable myRunnable = new Runnable() {
                    @Override
                    public void run() {
                        ViroGvrLayout view = mView.get();
                        if (view == null) {
                            return;
                        }
                        view.mGlListener.onGlInitialized();
                    }
                };
                new Handler(Looper.getMainLooper()).post(myRunnable);
            }
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            ViroGvrLayout view = mView.get();
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
            final GvrApi gvr = view.getGvrApi();
            if (gvr != null){
                gvr.refreshDisplayMetrics();
            }

            view.mNativeRenderer.onSurfaceChanged(surface.getHolder().getSurface(), width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            ViroGvrLayout view = mView.get();
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

        private WeakReference<ViroGvrLayout> mView;

        public ViroOnTouchListener(ViroGvrLayout view) {
            mView = new WeakReference<ViroGvrLayout>(view);
        }

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            ViroGvrLayout view = mView.get();
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

    public ViroGvrLayout(Context context, GlListener glListener, Runnable vrExitListener) {
        super(context);

        final Context activityContext = getContext();
        mSystemVisibilityListener = new OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                    setImmersiveSticky();
                }
            }
        };

        // Initialize the native renderer.
        GLSurfaceView glSurfaceView = createSurfaceView();

        mAssetManager = getResources().getAssets();
        mPlatformUtil = new PlatformUtil(
                new GLSurfaceViewQueue(glSurfaceView),
                mFrameListeners,
                activityContext,
                mAssetManager);
        mNativeRenderer = new RendererJni(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(),
                mAssetManager, mPlatformUtil,
                getGvrApi().getNativeGvrContext());
        mNativeRenderContext = new RenderContextJni(mNativeRenderer.mNativeRef);

        mGlListener = glListener;
        mKeyValidator = new KeyValidator(activityContext);

        // Add the GLSurfaceView to the GvrLayout.
        setPresentationView(glSurfaceView);

        // We want Android's VR mode on as long as the app is in either release or the device
        // is using Daydream. The renderer (library) is always in release, so we use BuildInfo to
        // check the debug status of the app we're compiled into.
        if (!BuildInfo.isDebug(context) || !getHeadset().equalsIgnoreCase("cardboard")) {
            // According to the GVR documentation, this only sets the activity to "VR mode" and is only
            // supported on Android Nougat and up.
            // NOTE: this turns off "Draw over other apps" permissions that React Native needs in debug
            AndroidCompat.setVrModeEnabled((Activity) getContext(), true);
        }

        // While this is for VR mode, there doesn't seem to be a negative impact in Mono mode.
        if (setAsyncReprojectionEnabled(true)) {
            // Scanline racing decouples the app framerate from the display framerate,
            // allowing immersive interaction even at the throttled clockrates set by
            // sustained performance mode.
            AndroidCompat.setSustainedPerformanceMode((Activity) getContext(), true);
        }

        final Activity activity = (Activity)getContext();
        mSavedSystemUIVisbility = activity.getWindow().getDecorView().getSystemUiVisibility();
        mSavedOrientation = activity.getRequestedOrientation();

        // Prevent screen from dimming/locking.
        activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // Attach SystemUiVisibilityChangeListeners to enforce a full screen experience.
        activity.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mSystemVisibilityListener);
        mWeakActivity = new WeakReference<Activity>(activity);

        getUiLayout().setCloseButtonListener(vrExitListener);
        // default the mode to VR
        setVrModeEnabled(true);
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

        // TODO JIRA-1555 Uncomment this to test creating an SRGB framebuffer
        //glSurfaceView.setEGLWindowSurfaceFactory(new ViroEGLWindowSurfaceFactory());

        glSurfaceView.setRenderer(new ViroSurfaceViewRenderer(this, glSurfaceView));
        glSurfaceView.setOnTouchListener(new ViroOnTouchListener(this));

        return glSurfaceView;
    }

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
    public void setDebugHUDEnabled(boolean enabled) {
        mNativeRenderer.setDebugHUDEnabled(enabled);
    }

    @Override
    public void recenterTracking() {
        getGvrApi().recenterTracking();
    }

    @Override
    public RenderContextJni getRenderContextRef(){
        return mNativeRenderContext;
    }

    @Override
    public void setSceneController(SceneControllerJni sceneController) {
        mNativeRenderer.setSceneController(sceneController.mNativeRef, 1.0f);
    }

    /**
     * This function sets up the view for whichever mode is desired.
     *
     * @param vrModeEnabled - whether or not to use VR or 360 mode.
     */
    @Override
    public void setVrModeEnabled(boolean vrModeEnabled) {
        Activity activity = mWeakActivity.get();
        if (activity != null) {
            activity.setRequestedOrientation(vrModeEnabled ?
                    ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE : ActivityInfo.SCREEN_ORIENTATION_SENSOR);
        }

        setStereoModeEnabled(vrModeEnabled);
        getUiLayout().setEnabled(vrModeEnabled);
        mNativeRenderer.setVRModeEnabled(vrModeEnabled);
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
    public RendererJni getNativeRenderer() {
        return mNativeRenderer;
    }

    @Override
    public View getContentView() { return this; }

    @Override
    public String getPlatform() {
        return "gvr";
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
    public void onActivityPaused(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mNativeRenderer.onPause();
        super.onPause();
    }

    @Override
    public void onActivityResumed(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        mNativeRenderer.onResume();

        // Ensure fullscreen immersion.
        setImmersiveSticky();
        super.onResume();
    }

    @Override
    public void onActivityDestroyed(Activity activity) {
        // no-op, the vrview destruction should be controlled by the SceneNavigator.
    }

    @Override
    public void destroy() {
        super.shutdown();
        mDestroyed = true;

        mNativeRenderContext.delete();
        mNativeRenderer.destroy();

        Activity activity = mWeakActivity.get();
        if (activity == null) {
            return;
        }

        activity.getWindow().clearFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        activity.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(null);
        activity.setRequestedOrientation(mSavedOrientation);
        unSetImmersiveSticky();
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
        refreshActivityLayout();
    }

    public void unSetImmersiveSticky(){
        ((Activity)getContext())
                .getWindow()
                .getDecorView()
                .setSystemUiVisibility(mSavedSystemUIVisbility);
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
    public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {
        //No-op
    }

}
