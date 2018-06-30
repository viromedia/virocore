/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.os.SystemClock;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;
import android.view.Surface;

/**
 * @hide
 * The AndroidViewSink is responsible for transferring canvas pixel data from an attached
 * Android view onto an AndroidViewTexture, so that it can be rendered in Viro. It is also
 * responsible for propagating and re-mapping Viro input events back onto the Android view.
 */

class AndroidViewSink extends FrameLayout {
    private final static String TAG = AndroidViewSink.class.getSimpleName();

    // The Android view we wish to render in our 3D scene.
    private View mAttachedView;

    // The Viro Texture containing the surface upon which to synchronize mAttachView's canvas on.
    private AndroidViewTexture mTexture;

    // True if this view should be hardware accelerated (enables bigger texture / canvas sizes).
    private boolean mUsesHardwareAcceleration;

    // Event handling states, used to propagate Viro events to the mAttachedView.
    private ClickState mPreviousClickEvent;
    private long mPreviousClickDownTimeStampMs;
    private boolean mShouldIgnoreInputEvents;

    public AndroidViewSink(@NonNull Context context, AndroidViewTexture texture, boolean hardwareAcceleration) {
        super(context);
        mTexture = texture;
        mUsesHardwareAcceleration = hardwareAcceleration;
        mPreviousClickEvent = ClickState.CLICK_UP;
        mShouldIgnoreInputEvents = true;
        mPreviousClickDownTimeStampMs = 0L;

        if (hardwareAcceleration) {
            setLayerType(View.LAYER_TYPE_HARDWARE, null);
        } else {
            setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        }

        setWillNotDraw(false);
    }

    public AndroidViewSink(@NonNull Context context) {
        this(context, null, 0, 0);
    }

    public AndroidViewSink(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0, 0);
    }

    public AndroidViewSink(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public AndroidViewSink(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        throw new IllegalArgumentException("Incorrect configuration of the Android View sink!");
    }

    public void attachView(View view) {
        addView(view);
        mAttachedView = view;
    }

    public void detachView() {
        removeAllViews();
        mAttachedView = null;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasureAllChildren(true);

        // Forces Android Views to be measured based on the set texture's spec, even if it's positioned offscreen.
        int newWidthSpec = View.MeasureSpec.makeMeasureSpec(mTexture.getWidth(), MeasureSpec.EXACTLY);
        int newHeightSpec = View.MeasureSpec.makeMeasureSpec(mTexture.getHeight(), MeasureSpec.EXACTLY);
        super.onMeasure(newWidthSpec, newHeightSpec);
    }

    @Override
    protected void dispatchDraw(Canvas canvas) {
        Surface surface = mTexture != null ? mTexture.getTextureRenderSurface() : null;
        if ( surface == null) {
            return;
        }

        // Clear and draw the attached view onto the AndroidViewTexture surface's canvas.
        try {
            final Canvas surfaceCanvas = surface.lockCanvas( null );
            surfaceCanvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
            super.dispatchDraw(surfaceCanvas);
            surface.unlockCanvasAndPost(surfaceCanvas);
        } catch (android.view.Surface.OutOfResourcesException excp) {
            Log.e(TAG, "Viro: Unable to draw on AndroidViewSink with insufficient memory!");
            excp.printStackTrace();
        }
    }

    public void dispatchClickStateEvent(ClickState state, float newX, float newY) {
        // Clicked events are not needed when mapping against Android Motion Events.
        if (state == ClickState.CLICKED) {
            return;
        }

        // Save the previous click down time stamp needed for constructing MotionEvents.
        if (mPreviousClickEvent == ClickState.CLICK_UP && state == ClickState.CLICK_DOWN) {
            mPreviousClickDownTimeStampMs = SystemClock.uptimeMillis();
        }

        // Map Viro's ClickState event to an Android Motion Event State.
        int action = MotionEvent.ACTION_UP;
        if (state == ClickState.CLICK_DOWN) {
            action = MotionEvent.ACTION_DOWN;
        }

        // Construct a new Android Motion event to be applied to this ViewSink's attached View.
        MotionEvent newEvent = MotionEvent.obtain(
                mPreviousClickDownTimeStampMs,
                SystemClock.uptimeMillis(),
                action,
                newX, newY, 0);

        // Finally, dispatch the new event to the attached android view.
        mShouldIgnoreInputEvents = false;
        boolean handledEvent = this.dispatchTouchEvent(newEvent);
        if (handledEvent && mUsesHardwareAcceleration) {
          requestLayout();
          invalidate();
        }
        mShouldIgnoreInputEvents = true;
        mPreviousClickEvent = state;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        // Usually ignore touch events and only process them via dispatchClickStateEvent().
        return mShouldIgnoreInputEvents;
    }
}