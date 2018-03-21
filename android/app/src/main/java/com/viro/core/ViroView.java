/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;


import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.AttrRes;
import android.support.annotation.Keep;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.Toast;

import com.viro.core.internal.BuildInfo;
import com.viro.core.internal.keys.KeyValidationListener;
import com.viro.core.internal.keys.KeyValidator;

import java.lang.ref.WeakReference;

/**
 * ViroView is the entrypoint for Viro applications, it enables the rendering of 3D AR and VR
 * content to an Android View. To use ViroView, instantiate one of its subclasses:
 * <p>
 * <ul> <li>{@link ViroViewScene} renders content directly to an Android View.</li> <li>{@link
 * ViroViewGVR} renders content in stereo for virtual reality applications using Google's GVR SDK.
 * This supports Google Cardboard and Google Daydream.</li> <li>{@link ViroViewOVR} renders content
 * in stereo for virtual reality applications using Facebook's Oculus Mobile SDK. This supports
 * Samsung Gear and similar devices.</li> <li>{@link ViroViewARCore} renders augmented reality
 * content using Google's ARCore library for AR tracking.</li> </ul>
 * <p>
 * With the desired ViroView instantiated, it can then be used by creating a {@link Scene} (or
 * {@link ARScene} for AR), adding content to the Scene, and assigning it to the ViroView with
 * {@link #setScene(Scene)}.
 * <p>
 * Note that after instantiation, the ViroView will validate your Viro API Key. To do this, pass your
 * API key into AndroidManifest.xml metadata as <meta-data android:name="com.viromedia.API_KEY" android:value="YOUR_API_KEY"/>.
 */
@Keep
public abstract class ViroView extends FrameLayout implements Application.ActivityLifecycleCallbacks {

    private final static String TAG = ViroView.class.getSimpleName();
    private final static String API_KEY_METADATA_TAG = "com.viromedia.API_KEY";
    private final static String INVALID_KEY_MESSAGE = "Your API key is missing/invalid!";

    // Available platform types, can be accessed through com.viro.renderer.BuildConfig.VIRO_PLATFORM
    /**
     * @hide
     */
    public final static String PLATFORM_VIRO_REACT = "VIRO_REACT";
    /**
     * @hide
     */
    public final static String PLATFORM_VIRO_CORE = "VIRO_CORE";

    private int mSavedSystemUIVisbility;
    private int mSavedOrientation;
    Renderer mNativeRenderer;
    protected ViroContext mNativeViroContext;
    private KeyValidator mKeyValidator;
    private OnSystemUiVisibilityChangeListener mSystemVisibilityListener;
    protected boolean mDestroyed = false;
    protected WeakReference<Activity> mWeakActivity;
    private String mApiKey;
    protected Scene mCurrentScene;
    protected RendererConfiguration mRendererConfig;

    private boolean mShadowsEnabled = true;
    private boolean mHDREnabled = true;
    private boolean mPBREnabled = true;
    private boolean mBloomEnabled = true;

    /**
     * @hide
     */
    protected ViroView(final Context context, @Nullable RendererConfiguration config) {
        super(context);
        init(config != null ? config : new RendererConfiguration());
    }

    /**
     * @hide
     *
     * @param context
     * @param attrs
     * @param defStyleAttr
     */
    public ViroView(@NonNull final Context context, @Nullable final AttributeSet attrs, @AttrRes final int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(new RendererConfiguration());
    }

    private void init(RendererConfiguration config) {
        final Activity activity = (Activity) getContext();
        mRendererConfig = config;
        mWeakActivity = new WeakReference<>(activity);
        mSavedSystemUIVisbility = activity.getWindow().getDecorView().getSystemUiVisibility();
        mSavedOrientation = activity.getRequestedOrientation();
        mSystemVisibilityListener = new OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(final int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                    changeSystemUiVisibility();
                }
            }
        };
        activity.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mSystemVisibilityListener);

        final Context activityContext = getContext();
        mKeyValidator = new KeyValidator(activityContext);
        try {
            final ApplicationInfo ai = activity.getPackageManager().getApplicationInfo(activity.getPackageName(), PackageManager.GET_META_DATA);
            final Bundle bundle = ai.metaData;
            if (bundle != null) {

                final String viroApiKey = bundle.getString(API_KEY_METADATA_TAG);
                mApiKey = viroApiKey;
            }
        } catch (final PackageManager.NameNotFoundException e) {
            Log.w("Viro", "Unable to find package name: " + e.getMessage());
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this ViroView.
     */
    public void dispose() {
        if (mDestroyed) {
            return;
        }

        /*
          TODO VIRO-2280: Fix Tango Memory leak that holds onto ViroViewARCore.
          As a temporary patch, we null out mCurrent scene here to release our 3D scene
          in memory.
         */
        mCurrentScene = null;
        mDestroyed = true;
        mNativeViroContext = null;
        if (mNativeRenderer != null) {
            mNativeRenderer.destroy();
        }

        Activity activity = mWeakActivity.get();
        if (activity == null) {
            return;
        }

        activity.getWindow().clearFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        activity.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(null);
        activity.setRequestedOrientation(mSavedOrientation);
        unSetImmersiveSticky();
    }

    /**
     * Set the {@link Scene} this ViroView should render. {@link ARScene} should be used for augmented
     * reality applications.
     *
     * @param scene The {@link Scene} to render in this ViroView.
     */
    public void setScene(Scene scene){
        mCurrentScene = scene;
    }

    /**
     * For VR applications, set this to true to render in stereo mode. Stereo renders two images:
     * one for the left eye and one for the right, each perturbed slightly to simulate depth. This
     * mode is meant to be used in a VR headset. Set to false to render to the entire screen, for
     * use when users do not have a headset.
     * <p>
     * Defaults to true.
     *
     * @param vrModeEnabled True to enable VR mode.
     */
    public abstract void setVRModeEnabled(boolean vrModeEnabled);

    /**
     * For VR applications, re-centers the head orientation. This resets the yaw to zero, but leaves
     * pitch and roll unmodified.
     */
    public abstract void recenterTracking();

    /**
     * Returns a {@link ViroMediaRecorder}, which can be used to take screenshots of the {@link
     * Scene} or record video. Currently only the {@link ViroViewARCore} supports the media
     * recorder.
     *
     * @return The {@link ViroMediaRecorder}, or null if not supported by the platform.
     */
    public abstract ViroMediaRecorder getRecorder();

    /**
     * Get the {@link ViroContext} for this ViroView. The ViroContext contains internal 3D state,
     * and is needed when constructing certain geometries and objects.
     *
     * @return The {@link ViroContext} for this ViroView.
     */
    public final ViroContext getViroContext() {
        return mNativeViroContext;
    }

    /**
     * Make the given {@link Node} the point of view for the user. The {@link Scene} in this
     * ViroView will be rendered from the perspective of the given Node's {@link Camera}. This
     * method is used to change the user's view of the scene. Note that when in AR or VR modes, the
     * point of view is automatically computed based on the device's rotation and position in the
     * world. This method is primarily for use with {@link ViroViewScene}.
     *
     * @param node The {@link Node} whose {@link Camera} will be used for the point of view.
     */
    public void setPointOfView(Node node) {
        getRenderer().setPointOfView(node);
    }

    /**
     * Get the field of view for this camera, along the major (larger) axis. Field of view is an
     * angle that determines how wide or narrow the camera lens is when rendering the scene.
     * <p>
     * The major axis is the axis with the larger dimension: the X axis in landscape mode, or the Y
     * axis in portrait mode. By specifying the field of view in terms of the major axis, Viro can
     * keep the field of view consistent upon orientation changes, when the major/minor axes swap.
     * <p>
     * You can set the field of view when using a {@link ViroViewScene} by creating a {@link
     * Camera}, setting the camera's field of view with {@link Camera#setFieldOfView(float)}, and
     * making that Camera's Node the point of view through {@link #setPointOfView(Node)}. The
     * default field of view for {@link ViroViewScene} is 90 degrees.
     * <p>
     * The field of view cannot be set for {@link ViroViewARCore} or {@link ViroViewGVR}: on those
     * platforms, field of view is fixed by the VR headset or AR camera.
     * <p>
     *
     * @return The field of view across the major axis, in degrees.
     */
    public float getFieldOfView() {
        return getRenderer().getFieldOfView();
    }

    /**
     * Get the real-time <i>world</i> position from the current point of view (Camera). The
     * position returned is real-time in that it is the position the Camera currently appears on-screen.
     *
     * @return The real-time position as a {@link Vector}.
     */
    public Vector getLastCameraPositionRealtime() {
        return getRenderer().getLastCameraPositionRealtime();
    }

    /**
     * Get the real-time <i>local</i> orientation from the current point of view (Camera), expressed as
     * three Euler angles. Specifically, the X component is rotation about the X axis (pitch), the
     * Y component is rotation about the node's Y axis (yaw), and Z is the rotation around the node's Z axis
     * (roll). This returns the real-time orientation, meaning the orientation of the Camera as it
     * currently appears on-screen.
     *
     * The returned orientation is the Camera's <i>world</i> orientation.
     *
     * @return The rotation in Euler form as a {@link Vector} in radians.
     */
    public Vector getLastCameraRotationEulerRealtime() {
        return getRenderer().getLastCameraRotationRealtime();
    }

    /**
     * Get the direction in which the Camera is facing (e.g. in the direction of the reticle).
     *
     * @return The direction the Camera is facing.
     */
    public Vector getLastCameraForwardRealtime() {
        return getRenderer().getLastCameraForwardRealtime();
    }

    /**
     * Set a {@link CameraListener} to receive updates whenever the camera's position or orientation
     * changes. The camera is the current point of view of the user.
     *
     * @param listener The {@link CameraListener} to receive updates.
     */
    public void setCameraListener(CameraListener listener) {
        getRenderer().setCameraListener(listener);
    }

    /**
     * (Used by Java API, mApiKey picked up from Metadata in Android Manifest) Your Viro API key must be validated, via this method, prior to rendering content. Content
     * will render during the validation process; if the key fails validation, the screen will
     * deactivate and render black.
     *
     * @hide
     */
     final void validateAPIKeyFromManifest() {
         mNativeRenderer.setSuspended(false);
         // we actually care more about the headset than platform in this case.
         validateAPIKey(mApiKey);
    }

    /**
     * (Used by React Viro, apiKey passed through react)Your Viro API key must be validated, via this method, prior to rendering content. Content
     * will render during the validation process; if the key fails validation, the screen will
     * deactivate and render black.
     *
     * @hide
     */
    public final void validateAPIKey(String apiKey) {
        mNativeRenderer.setSuspended(false);
        // we actually care more about the headset than platform in this case.
        final WeakReference<Renderer> weakRenderer = new WeakReference<>(mNativeRenderer);
        mKeyValidator.validateKey(apiKey, getHeadset(), new KeyValidationListener() {
            @Override
            public void onResponse(boolean success) {
                if (!mDestroyed) {
                    Renderer renderer = weakRenderer.get();
                    if(renderer != null ) {
                        renderer.setSuspended(!success);
                    }
                }

                // If the key was invalid in a debug build, then let's show a toast!
                if (!success && BuildInfo.isDebug(getContext())) {

                    Handler mainHandler = new Handler(getContext().getMainLooper());
                    Runnable showToastRunnable = new Runnable() {
                        @Override
                        public void run() {
                            Toast toast = Toast.makeText(getContext().getApplicationContext(),
                                    INVALID_KEY_MESSAGE, Toast.LENGTH_LONG);
                            toast.show();
                        }
                    };
                    mainHandler.post(showToastRunnable);
                }
            }
        });
    }

    /**
     * Retrieve the {@link Controller} for this view. Controller represents the user interface
     * through which the user interacts with the {@link Scene}. The exact form of Controller depends
     * on the underlying platform. For example, for Daydream this represents the Daydream controller
     * (the laser pointer). For Cardboard and GearVR, Controller is effectively the head-mounted
     * display itself (and its tap button).
     *
     * @return The {@link Controller} for this ViroView.
     */
    public final Controller getController() {
        return mNativeViroContext.getController();
    }

    /**
     * Enable or disable rendering dynamic shadows. If shadows are disabled here, shadow
     * casting {@link Light}s will simply not cast a shadow.
     * <p>
     * Shadows are not supported on all devices.
     *
     * @param enabled True to enable dynamic shadows for this ViroView.
     */
    public void setShadowsEnabled(boolean enabled) {
        mShadowsEnabled = enabled;
        mNativeRenderer.setShadowsEnabled(enabled);
    }

    /**
     * Returns true if dynamic shadows are enabled for this ViroView.
     *
     * @return True if shadows are enabled.
     */
    public boolean isShadowsEnabled() {
        return mShadowsEnabled;
    }

    /**
     * When HDR rendering is enabled, Viro uses a deeper color space and renders to a floating point
     * texture, where a tone-mapping algorithm is applied to preserve fine details in both bright
     * and dark regions of the scene. If HDR is disabled, then features like Bloom and PBR will not
     * work, and tone-mapping will be disabled.
     * <p>
     * HDR is not supported on all devices.
     *
     * @param enabled True to enable HDR for this ViroView.
     */
    public void setHDREnabled(boolean enabled) {
        mHDREnabled = enabled;
        mNativeRenderer.setHDREnabled(enabled);
    }

    /**
     * Returns true if HDR rendering is enabled for this ViroView.
     *
     * @return True if HDR is enabled.
     */
    public boolean isHDREnabled() {
        return mHDREnabled;
    }

    /**
     * Enable or disable physically-based rendering. Physically based rendering, or PBR, produces
     * more realistic lighting results for your scenes, and provides a more intuitive workflow for
     * artists. To use PBR, this property must be enabled, and {@link Material}s must use the {@link
     * com.viro.core.Material.LightingModel#PHYSICALLY_BASED} lighting model. PBR is controlled by a
     * variety of properties, see {@link com.viro.core.Material.LightingModel#PHYSICALLY_BASED} for
     * details.
     *
     * @param enabled True to enable PBR for this ViroView.
     */
    public void setPBREnabled(boolean enabled) {
        mPBREnabled = enabled;
        mNativeRenderer.setPBREnabled(enabled);
    }

    /**
     * Returns true if physically-based rendering is enabled for this ViroView.
     *
     * @return True if PBR is enabled.
     */
    public boolean isPBREnabled() {
        return mPBREnabled;
    }

    /**
     * Enable or disable bloom. Bloom adds a soft glow to bright areas in scene, simulating the way
     * bright highlights appear to the human eye. To make an object bloom, this property must be
     * enabled, and the objects's threshold for bloom must be set via {@link Material#setBloomThreshold(float)}.
     *
     * @param enabled True to enable bloom for this ViroView.
     */
    public void setBloomEnabled(boolean enabled) {
        mBloomEnabled = enabled;
        mNativeRenderer.setBloomEnabled(enabled);
    }

    /**
     * Returns true if bloom is enabled for this ViroView.
     *
     * @return True if bloom is enabled.
     */
    public boolean isBloomEnabled() {
        return mBloomEnabled;
    }

    /**
     * Converts the given 3D point in the world into its corresponding 2D point on the screen.
     * Note that each 2D point on the view corresponds to a continuous segment of 3D points in the world, each at
     * a different depth between the near and far clipping planes. The Z coordinate in the returned
     * {@link Vector} identifies the depth of the projected point, where 0.0 coincides with the
     * near clipping plane, 1.0 coincides with the far clipping plane, and all other values are
     * linear interpolations between the two.
     *
     * @param point Get the 2D point corresponding to this 3D world coordinate.
     * @return The 2D point in the view's coordinate system. The Z coordinate represents the depth
     * from 0.0 to 1.0.
     */
    public Vector projectPoint(Vector point) {
        return mNativeRenderer.projectPoint(point.x, point.y, point.z);
    }

    /**
     * Converts the given 2D point on the view into its corresponding 3D point in world coordinates.
     * Note that each 2D point on the view corresponds to a continuous segment of 3D points in the
     * world, each at a different depth between the near and far clipping planes. The Z coordinate
     * in the given {@link Vector} identifies the depth to return, where 0.0 coincides with the near
     * clipping plane, 1.0 coincides with the far clipping plane, and all other values are linear
     * interpolations between the two.
     * <p>
     * This method can be used for advanced hit-testing against real-world or virtual objects.
     *
     * @param point Get the 3D ray corresponding to this point in the 2D view's coordinate system.
     *              The Z coordinate represents the depth from 0.0 to 1.0.
     * @return The 3D point in world coordinates.
     */
    public Vector unprojectPoint(Vector point) {
        return mNativeRenderer.unprojectPoint(point.x, point.y, point.z);
    }

    /**
     * @hide
     */
    public abstract String getPlatform();

    /**
     * @hide
     */
    public final String getHeadset() {
        return mNativeRenderer.getHeadset();
    }
    /**
     * @hide
     */
    public final String getControllerType() {
        return mNativeRenderer.getController();
    }
    /**
     * @hide
     * @return
     */
    public final Renderer getRenderer() {
        return mNativeRenderer;
    }
    /**
     * Debug mode enables drawing over the view. Bridge only.
     * @hide
     */
    public abstract void setDebug(boolean debug);
    /**
     * @hide
     * @param enabled
     */
    public final void setDebugHUDEnabled(boolean enabled) {
        mNativeRenderer.setDebugHUDEnabled(enabled);
    }

    /**
     * Returns a set of bitwise-or flags to determine system UI visibility for a {@link ViroView}.
     * Return 0 if desire is to not to change the system UI Visibility. Otherwise return
     * bitwise-or flags of the following:
     * <br>
     * {@link #SYSTEM_UI_FLAG_LOW_PROFILE},
     * {@link #SYSTEM_UI_FLAG_HIDE_NAVIGATION}, {@link #SYSTEM_UI_FLAG_FULLSCREEN},
     * {@link #SYSTEM_UI_FLAG_LAYOUT_STABLE}, {@link #SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION},
     * {@link #SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN}, {@link #SYSTEM_UI_FLAG_IMMERSIVE},
     * and {@link #SYSTEM_UI_FLAG_IMMERSIVE_STICKY}.
     */
    abstract protected int getSystemUiVisibilityFlags();

    /**
     * @hide
     *
     * Invoked to customize the system UI.
     */
    protected void changeSystemUiVisibility() {
        if(getSystemUiVisibilityFlags() != 0) {
            ((Activity) getContext())
                    .getWindow()
                    .getDecorView()
                    .setSystemUiVisibility(
                            getSystemUiVisibilityFlags());
            refreshActivityLayout();
        }
    }

    /**
     * @hide
     */
    protected void unSetImmersiveSticky(){
        ((Activity)getContext())
                .getWindow()
                .getDecorView()
                .setSystemUiVisibility(mSavedSystemUIVisbility);
        refreshActivityLayout();
    }

    protected void refreshActivityLayout(){
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
