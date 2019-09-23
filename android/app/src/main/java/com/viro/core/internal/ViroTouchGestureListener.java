//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.core.internal;

import android.content.Context;
import android.graphics.PointF;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.util.Log;

import com.viro.core.Renderer;

/*
 Class that handles and notifies RendererJni with touch and gesture events. This extends
 SimpleOnGestureListener, because we only want to listen to a subset of the gestures.
 */
public class ViroTouchGestureListener extends GestureDetector.SimpleOnGestureListener
        implements View.OnTouchListener, ScaleGestureDetector.OnScaleGestureListener,
        RotationGestureDetector.OnRotateGestureListener {

    private static final long MIN_FINGER_TOUCH_DURATION = 50; // milliseconds
    private static final long INVALID_FINGER_DOWN_TIME = -1;

    // we tie these with the MotionEvent values because we use them in the renderer.
    private static final int EVENT_BEGIN = MotionEvent.ACTION_DOWN;
    private static final int EVENT_END = MotionEvent.ACTION_UP;
    private static final int EVENT_MOVE = MotionEvent.ACTION_MOVE;

    private Renderer mNativeRenderer;
    // whether or not the renderer has been destroyed.
    private boolean mDestroyed = false;

    private ScaleGestureDetector mScaleDetector;
    private RotationGestureDetector mRotateDetector;

    private View.OnTouchListener mUserTouchListener;

    private long mFingerDownTime = 0;
    private boolean mIsTouching = false;
    private boolean mIsScaling = false;
    private boolean mIsRotating = false;
    private float mLatestScale = 1.0f;
    private float mLastTouchX = 0;
    private float mLastTouchY = 0;

    public ViroTouchGestureListener(Context context, Renderer rendererJni) {
        mNativeRenderer = rendererJni;
        mScaleDetector = new ScaleGestureDetector(context, this);
        mScaleDetector.setQuickScaleEnabled(false);
        mRotateDetector = new RotationGestureDetector(this);
    }

    public void destroy() {
        mDestroyed = true;
    }

    private void handleTap(float x, float y) {
        if (!mDestroyed) {
            mNativeRenderer.onTouchEvent(EVENT_BEGIN, x, y);
            mNativeRenderer.onTouchEvent(EVENT_END, x, y);
        }
    }

    public void setOnTouchListener(View.OnTouchListener listener) {
        mUserTouchListener = listener;
    }

    /*
     * -- OnTouchListener Implementation --
     */

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (mDestroyed) {
            return false;
        }

        if (mUserTouchListener != null && mUserTouchListener.onTouch(v, event)) {
            return true;
        }

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                mFingerDownTime = System.currentTimeMillis();
                break;
            case MotionEvent.ACTION_MOVE:
                if (mIsScaling || mIsRotating) {
                    mFingerDownTime = INVALID_FINGER_DOWN_TIME;
                    break;
                } else if (mIsTouching) {
                    // for some reason, even when the pixel positions aren't changing, android still
                    // calls this function... so just filter out exact same points.
                    if (mLastTouchX == event.getX() && mLastTouchY == event.getY()) {
                        break;
                    }

                    mNativeRenderer.onTouchEvent(EVENT_MOVE, event.getX(), event.getY());

                } else if (mFingerDownTime != INVALID_FINGER_DOWN_TIME
                        && (System.currentTimeMillis() - mFingerDownTime) > MIN_FINGER_TOUCH_DURATION) {
                    mIsTouching = true;
                    mNativeRenderer.onTouchEvent(EVENT_BEGIN, event.getX(), event.getY());
                }
                break;
            case MotionEvent.ACTION_UP:
                if (mIsTouching) {
                    mIsTouching = false;
                    mNativeRenderer.onTouchEvent(EVENT_END, event.getX(), event.getY());
                } else if ((System.currentTimeMillis() - mFingerDownTime) < MIN_FINGER_TOUCH_DURATION) {
                    // this is a "tap"
                    handleTap(event.getX(), event.getY());
                }
                break;
        }

        if (!mIsTouching) {
            mScaleDetector.onTouchEvent(event);
            mRotateDetector.onTouchEvent(event);
        }

        return true;
    }

    /*
     * -- OnScaleGestureListener Implementation --
     */
    @Override
    public boolean onScaleBegin(ScaleGestureDetector detector) {
        mIsScaling = true;
        mLatestScale = detector.getScaleFactor();
        if (!mDestroyed) {
            mNativeRenderer.onPinchEvent(EVENT_BEGIN, mLatestScale, detector.getFocusX(),
                    detector.getFocusY());
        }
        return true;
    }

    @Override
    public boolean onScale(ScaleGestureDetector detector) {
        mLatestScale *= detector.getScaleFactor();
        if (!mDestroyed) {
            mNativeRenderer.onPinchEvent(EVENT_MOVE, mLatestScale, detector.getFocusX(),
                    detector.getFocusY());
        }
        return true;
    }

    @Override
    public void onScaleEnd(ScaleGestureDetector detector) {
        mIsScaling = false;
        mLatestScale *= detector.getScaleFactor();
        if (!mDestroyed) {
            mNativeRenderer.onPinchEvent(EVENT_END, mLatestScale, detector.getFocusX(),
                    detector.getFocusY());
        }
    }

    /*
     * -- OnRotateGestureListener Implementation --
     */

    @Override
    public void onRotateBegin(RotationGestureDetector detector) {
        mIsRotating = true;
        if (!mDestroyed) {
            PointF center = detector.getOriginalCenterPoint();
            mNativeRenderer.onRotateEvent(EVENT_BEGIN, detector.getRotateRadians(), center.x, center.y);
        }
    }

    @Override
    public void onRotate(RotationGestureDetector detector) {
        if (!mDestroyed) {
            PointF center = detector.getCenterPoint();
            mNativeRenderer.onRotateEvent(EVENT_MOVE, detector.getRotateRadians(), center.x, center.y);
        }
    }

    @Override
    public void onRotateEnd(RotationGestureDetector detector) {
        mIsRotating = false;
        if (!mDestroyed) {
            PointF center = detector.getCenterPoint();
            mNativeRenderer.onRotateEvent(EVENT_END, detector.getRotateRadians(), center.x, center.y);
        }
    }
}
