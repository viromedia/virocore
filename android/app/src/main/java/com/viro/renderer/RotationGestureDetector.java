/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer;

import android.graphics.PointF;
import android.util.Log;
import android.view.MotionEvent;

public class RotationGestureDetector {

    public interface OnRotateGestureListener {
        public void onRotateBegin(RotationGestureDetector detector);
        public void onRotate(RotationGestureDetector detector);
        public void onRotateEnd(RotationGestureDetector detector);
    }

    private class Line {
        public float x1, y1, x2, y2;

        public Line(MotionEvent event, int pointer1, int pointer2) {
            x1 = event.getX(event.findPointerIndex(pointer1));
            y1 = event.getY(event.findPointerIndex(pointer1));
            x2 = event.getX(event.findPointerIndex(pointer2));
            y2 = event.getY(event.findPointerIndex(pointer2));
        }
    }

    private static final int INVALID_POINTER = -1;
    private OnRotateGestureListener mListener;
    private int mPointer1 = INVALID_POINTER;
    private int mPointer2 = INVALID_POINTER;
    private Line mFirstLine = null;
    private Line mLatestLine = null;
    private float mCurrentRotateDegrees = 0;
    private boolean mHasRotationBegan = false;

    public RotationGestureDetector(OnRotateGestureListener listener) {
        mListener = listener;
    }

    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                mPointer1 = event.getPointerId(event.getActionIndex());
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                mPointer2 = event.getPointerId(event.getActionIndex());
                mFirstLine = new Line(event, mPointer1, mPointer2);
                break;
            case MotionEvent.ACTION_MOVE:
                if (mPointer1 != INVALID_POINTER && mPointer2 != INVALID_POINTER) {
                    mLatestLine = new Line(event, mPointer1, mPointer2);
                    mCurrentRotateDegrees = calcAngleBetweenLines(mFirstLine, mLatestLine);

                    // TODO: if rotation hasn't begun, should we consider a minimum angle?
                    if (mListener != null) {
                        if (!mHasRotationBegan) {
                            mHasRotationBegan = true;
                            mListener.onRotateBegin(this);
                        } else {
                            mListener.onRotate(this);
                        }
                    }
                }
                break;
            case MotionEvent.ACTION_UP:
                mPointer1 = INVALID_POINTER;
                endRotate();
                break;
            case MotionEvent.ACTION_POINTER_UP:
                mPointer2 = INVALID_POINTER;
                endRotate();
                break;
            case MotionEvent.ACTION_CANCEL:
                mPointer1 = INVALID_POINTER;
                mPointer2 = INVALID_POINTER;
                endRotate();
                break;
        }
        return true;
    }

    /*
     Returns the original center point of rotation if available.
     */
    public PointF getOriginalCenterPoint() {
        return getMidpoint(mFirstLine);
    }

    /*
     Returns the latest center point if available.
     */
    public PointF getCenterPoint() {
        return getMidpoint(mLatestLine);
    }

    public float getRotateRadians() {
        return (float) Math.toRadians(mCurrentRotateDegrees);
    }

    /*
     This method contains all the logic to end rotating (if it began).

     Note: we only reset mHasRotationBegan here because the listener might want to access
     the last degree and center points.
     */
    private void endRotate() {
        if (mListener != null && mHasRotationBegan) {
            mHasRotationBegan = false;

            // notify the listener last!
            mListener.onRotateEnd(this);
        }
    }

    private PointF getMidpoint(Line line) {
        if (line != null) {
            float x = (line.x1 + line.x2) / 2;
            float y = (line.y1 + line.y2) / 2;
            return new PointF(x, y);
        }
        return new PointF(0,0);
    }

    private float calcAngleBetweenLines(Line line1, Line line2) {
        float angle1 = (float) Math.atan2( (line1.y1 - line1.y2), (line1.x1 - line1.x2));
        float angle2 = (float) Math.atan2( (line2.y1 - line2.y2), (line2.x1 - line2.x2));

        return ((float)Math.toDegrees(angle1 - angle2)) % 360;
    }
}
