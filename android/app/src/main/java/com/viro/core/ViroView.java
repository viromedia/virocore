/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the 
LICENSE file in the
 * root directory of this source tree. An additional grant 
of patent rights can be found in
 * the PATENTS file in the same directory.
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
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.FrameLayout;

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
public abstract class ViroView extends FrameLayout implements Application.ActivityLifecycleCallbacks {

    private final static String TAG = ViroView.class.getSimpleName();
    private final static String API_KEY_METADATA_TAG = "com.viromedia.API_KEY";
    private int mSavedSystemUIVisbility;
    private int mSavedOrientation;
    Renderer mNativeRenderer;
    protected ViroContext mNativeViroContext;
    private KeyValidator mKeyValidator;
    private OnSystemUiVisibilityChangeListener mSystemVisibilityListener;
    protected boolean mDestroyed = false;
    protected WeakReference<Activity> mWeakActivity;
    private String mApiKey;
    protected RendererStartListener mRenderStartListener = null;


    /**
     * @hide
     */
    protected ViroView(final Context context) {
        super(context);
        init();
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
        init();
    }

    private void init() {
        final Activity activity = (Activity) getContext();
        mWeakActivity = new WeakReference<>(activity);
        mSavedSystemUIVisbility = activity.getWindow().getDecorView().getSystemUiVisibility();
        mSavedOrientation = activity.getRequestedOrientation();
        mSystemVisibilityListener = new OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(final int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                    setImmersiveSticky();
                }
            }
        };
        activity.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mSystemVisibilityListener);

        final Context activityContext = getContext();
        mKeyValidator = new KeyValidator(activityContext);
        try {
            final ApplicationInfo ai = activity.getPackageManager().getApplicationInfo(activity.getPackageName(), PackageManager.GET_META_DATA);
            final Bundle bundle = ai.metaData;
            final String viroApiKey = bundle.getString(API_KEY_METADATA_TAG);
            if (viroApiKey == null || viroApiKey.isEmpty()) {
                throw new IllegalArgumentException();
            }

            mApiKey = viroApiKey;
        } catch (final Exception e) {
            Log.e(TAG, "Dear developer. Don't forget to configure <meta-data android:name=\"com.viromedia.API_KEY\" android:value=\"YOUR_API_KEY\"/> in your AndroidManifest.xml file. Don't have an API KEY? Get one by signing up on https://viromedia.com/signup/");
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
        mDestroyed = true;
        mNativeViroContext.dispose();
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

    /**
     * Set the {@link Scene} this ViroView should render. {@link ARScene} should be used for augmented
     * reality applications.
     *
     * @param scene The {@link Scene} to render in this ViroView.
     */
    public abstract void setScene(Scene scene);

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
     * (Used by Java API, mApiKey picked up from Metadata in Android Manifest) Your Viro API key must be validated, via this method, prior to rendering content. Content
     * will render during the validation process; if the key fails validation, the screen will
     * deactivate and render black.
     *
     * @hide
     */
     final void validateAPIKey() {
        mNativeRenderer.setSuspended(false);
        // we actually care more about the headset than platform in this case.
        mKeyValidator.validateKey(mApiKey, getHeadset(), new KeyValidationListener() {
            @Override
            public void onResponse(boolean success) {
                if (!mDestroyed) {
                    mNativeRenderer.setSuspended(!success);
                }
            }
        });
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
     * Set the callback listener that will be invoked when the renderer has finished initializing.
     *
     * @param renderStartListener {@link RendererStartListener} callback.
     */
    public final void setRenderStartListener(final RendererStartListener renderStartListener) {
        mRenderStartListener = renderStartListener;
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
     * @hide
     */
    protected void setImmersiveSticky() {
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
