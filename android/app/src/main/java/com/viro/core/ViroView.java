/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
package com.viro.core;


import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
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
 * Note that after instantiation, the ViroView must validate your Viro API Key. To do this, pass your
 * API key into {@link #validateAPIKey(String)}.
 */
public abstract class ViroView extends FrameLayout implements Application.ActivityLifecycleCallbacks {

    private int mSavedSystemUIVisbility;
    private int mSavedOrientation;
    Renderer mNativeRenderer;
    protected ViroContext mNativeViroContext;
    private KeyValidator mKeyValidator;
    private OnSystemUiVisibilityChangeListener mSystemVisibilityListener;
    protected boolean mDestroyed = false;
    protected WeakReference<Activity> mWeakActivity;

    /**
     * @hide
     */
    protected ViroView(Context context) {
        super(context);

        final Activity activity = (Activity)getContext();
        mWeakActivity = new WeakReference<Activity>(activity);
        mSavedSystemUIVisbility = activity.getWindow().getDecorView().getSystemUiVisibility();
        mSavedOrientation = activity.getRequestedOrientation();
        mSystemVisibilityListener = new OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                    setImmersiveSticky();
                }
            }
        };
        activity.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(mSystemVisibilityListener);

        final Context activityContext = getContext();
        mKeyValidator = new KeyValidator(activityContext);
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
     * Your Viro API key must be validated, via this method, prior to rendering content. Content
     * will render during the validation process; if the key fails validation, the screen will
     * deactivate and render black.
     *
     * @param apiKey Your Viro API key.
     */
    public final void validateAPIKey(String apiKey) {
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
    public final String getController() {
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
