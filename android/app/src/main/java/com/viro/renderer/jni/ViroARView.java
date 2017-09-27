/**
 * Copyright © 2017 Viro Media. All rights reserved.
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
import android.view.View;
import android.view.WindowManager;
import android.widget.Toast;

import com.google.ar.core.Config;
import com.google.ar.core.Frame;
import com.google.ar.core.Frame.TrackingState;
import com.google.ar.core.HitResult;
import com.google.ar.core.Plane;
import com.google.ar.core.PlaneHitResult;
import com.google.ar.core.Session;

import com.google.vr.ndk.base.AndroidCompat;
import com.viro.renderer.BuildInfo;
import com.viro.renderer.FrameListener;
import com.viro.renderer.GLSurfaceViewQueue;
import com.viro.renderer.keys.KeyValidationListener;
import com.viro.renderer.keys.KeyValidator;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Class for instantiating an AR Viro view. Integrates with AR Core.
 */
public class ViroARView extends GLSurfaceView implements VrView {

    private static final String TAG = "Viro";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("gvr");
        System.loadLibrary("gvr_audio");
        System.loadLibrary("native-lib");
    }

    private static class ViroARRenderer implements GLSurfaceView.Renderer {

        private WeakReference<ViroARView> mView;

        public ViroARRenderer(ViroARView view) {
            mView = new WeakReference<ViroARView>(view);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            ViroARView view = mView.get();
            if (view == null) {
                return;
            }

            view.mNativeRenderer.onSurfaceCreated(view.getHolder().getSurface());
            view.mNativeRenderer.initalizeGl();
            if (view.mGlListener != null) {
                Runnable myRunnable = new Runnable() {
                    @Override
                    public void run() {
                        ViroARView view = mView.get();
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
            ViroARView view = mView.get();
            if (view == null) {
                return;
            }
            view.mNativeRenderer.onSurfaceChanged(view.getHolder().getSurface(), width, height);

            // Notify ARCore session that the view size changed so that the perspective matrix and
            // the video background can be properly adjusted.
            view.mSession.setDisplayGeometry(width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            ViroARView view = mView.get();
            if (view == null) {
                return;
            }

            for (FrameListener listener : view.mFrameListeners) {
                listener.onDrawFrame();
            }
            view.mNativeRenderer.drawFrame();
        }
    }

    private GLSurfaceView.Renderer mRenderer;

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


    private Config mDefaultConfig;
    private Session mSession;

    public ViroARView(Context context, GlListener glListener) {
        super(context);

        final Context activityContext = getContext();
        final Activity activity = (Activity) getContext();

        mSystemVisibilityListener = new OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                    setImmersiveSticky();
                }
            }
        };


        // Initialize the native renderer.
        initSurfaceView();

        mAssetManager = getResources().getAssets();
        mPlatformUtil = new PlatformUtil(
                new GLSurfaceViewQueue(this),
                mFrameListeners,
                activityContext,
                mAssetManager);
        mNativeRenderer = new RendererJni(
                getClass().getClassLoader(),
                activityContext.getApplicationContext(), this, activity,
                mAssetManager, mPlatformUtil);
        mNativeRenderContext = new RenderContextJni(mNativeRenderer.mNativeRef);

        mGlListener = glListener;
        mKeyValidator = new KeyValidator(activityContext);

        mSavedSystemUIVisbility = activity.getWindow().getDecorView().getSystemUiVisibility();
        mSavedOrientation = activity.getRequestedOrientation();
        activity.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mSystemVisibilityListener);

        mWeakActivity = new WeakReference<Activity>(activity);
        setVrModeEnabled(false);

        mSession = new Session(activity);

        // Create default config, check is supported, create session from that config.
        mDefaultConfig = Config.createDefaultConfig();
        if (!mSession.isSupported(mDefaultConfig)) {
            Toast.makeText(activity, "This device does not support AR", Toast.LENGTH_LONG).show();
            activity.finish();
            return;
        }
    }

    /**
     * Initialize this {@link GLSurfaceView}.
     */
    private void initSurfaceView() {
        int colorBits = 8;
        int alphaBits = 8;
        int depthBits = 16;
        int stencilBits = 8;

        setEGLContextClientVersion(3);
        setEGLConfigChooser(colorBits, colorBits, colorBits, alphaBits, depthBits, stencilBits);
        setPreserveEGLContextOnPause(true);

        // TODO JIRA-1555 Uncomment this to test creating an SRGB framebuffer
        //glSurfaceView.setEGLWindowSurfaceFactory(new ViroEGLWindowSurfaceFactory());

        setRenderer(new ViroARRenderer(this));
        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        // setOnTouchListener(new ViroGvrLayout.ViroOnTouchListener(this));
    }

    // TODO If we request camera permissions we have to respond here somehow
    //      This is an activity callback so we have to hook into the parent activity
    /*
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        Activity activity = mWeakActivity.get();
        if (activity != null) {
            return;
        }

        if (!CameraPermissionHelper.hasCameraPermission(getContext())) {
            Toast.makeText(activity,
                    "Camera permission is needed to run this application", Toast.LENGTH_LONG).show();
            activity.finish();
        }
    }
    */

    /*
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // Create the texture and pass it to ARCore session to be filled during update().
        mBackgroundRenderer.createOnGlThread(this);
        mSession.setCameraTextureName(mBackgroundRenderer.getTextureId());

        // Prepare the other rendering objects.
        try {
            mVirtualObject.createOnGlThread(this, "andy.obj", "andy.png");
            mVirtualObject.setMaterialProperties(0.0f, 3.5f, 1.0f, 6.0f);

            mVirtualObjectShadow.createOnGlThread(this,
                    "andy_shadow.obj", "andy_shadow.png");
            mVirtualObjectShadow.setBlendMode(BlendMode.Shadow);
            mVirtualObjectShadow.setMaterialProperties(1.0f, 0.0f, 0.0f, 1.0f);
        } catch (IOException e) {
            Log.e(TAG, "Failed to read obj file");
        }
        try {
            mPlaneRenderer.createOnGlThread(this, "trigrid.png");
        } catch (IOException e) {
            Log.e(TAG, "Failed to read plane texture");
        }
        mPointCloud.createOnGlThread(this);
    }
    */

    /*
    @Override
    public void onDrawFrame(GL10 gl) {
        // Clear screen to notify driver it should not load any pixels from previous frame.
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        try {
            // Obtain the current frame from ARSession. When the configuration is set to
            // UpdateMode.BLOCKING (it is by default), this will throttle the rendering to the
            // camera framerate.
            Frame frame = mSession.update();

            // Handle taps. Handling only one tap per frame, as taps are usually low frequency
            // compared to frame rate.
            MotionEvent tap = mQueuedSingleTaps.poll();
            if (tap != null && frame.getTrackingState() == TrackingState.TRACKING) {
                for (HitResult hit : frame.hitTest(tap)) {
                    // Check if any plane was hit, and if it was hit inside the plane polygon.
                    if (hit instanceof PlaneHitResult && ((PlaneHitResult) hit).isHitInPolygon()) {
                        // Cap the number of objects created. This avoids overloading both the
                        // rendering system and ARCore.
                        if (mTouches.size() >= 16) {
                            mSession.removeAnchors(Arrays.asList(mTouches.get(0).getAnchor()));
                            mTouches.remove(0);
                        }
                        // Adding an Anchor tells ARCore that it should track this position in
                        // space. This anchor will be used in PlaneAttachment to place the 3d model
                        // in the correct position relative both to the world and to the plane.
                        mTouches.add(new PlaneAttachment(
                                ((PlaneHitResult) hit).getPlane(),
                                mSession.addAnchor(hit.getHitPose())));

                        // Hits are sorted by depth. Consider only closest hit on a plane.
                        break;
                    }
                }
            }

            // Draw background.
            mBackgroundRenderer.draw(frame);

            // If not tracking, don't draw 3d objects.
            if (frame.getTrackingState() == TrackingState.NOT_TRACKING) {
                return;
            }

            // Get projection matrix.
            float[] projmtx = new float[16];
            mSession.getProjectionMatrix(projmtx, 0, 0.1f, 100.0f);

            // Get camera matrix and draw.
            float[] viewmtx = new float[16];
            frame.getViewMatrix(viewmtx, 0);


        } catch (Throwable t) {
            // Avoid crashing the application due to unhandled exceptions.
            Log.e(TAG, "Exception on the OpenGL thread", t);
        }
    }
*/

    ///// START VIRO METHODS

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
    public RenderContextJni getRenderContextRef(){
        return mNativeRenderContext;
    }

    @Override
    public void setSceneController(SceneControllerJni scene) {
        mNativeRenderer.setSceneController(scene.mNativeRef, 1.0f);
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

        // Note that the order matters - GLSurfaceView is paused first so that it does not try
        // to query the session. If Session is paused before GLSurfaceView, GLSurfaceView may
        // still call mSession.update() and get a SessionPausedException.
        super.onPause();
        mSession.pause();
    }

    @Override
    public void onActivityResumed(Activity activity) {
        if (mWeakActivity.get() != activity){
            return;
        }

        setImmersiveSticky();
        mNativeRenderer.onResume();

        // ARCore requires camera permissions to operate. If we did not yet obtain runtime
        // permission on Android M and above, now is a good time to ask the user for it.
        if (CameraPermissionHelper.hasCameraPermission(activity)) {
            // Note that order matters - see the note in onPause(), the reverse applies here.
            mSession.resume(mDefaultConfig);
            super.onResume();
        } else {
            CameraPermissionHelper.requestCameraPermission(activity);
        }
    }

    @Override
    public void onActivityDestroyed(Activity activity) {
        // no-op, the vrview destruction should be controlled by the SceneNavigator.
    }

    @Override
    public void destroy() {
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
        // No-op
    }

    @Override
    public void recenterTracking() {
        // No-op
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
}
