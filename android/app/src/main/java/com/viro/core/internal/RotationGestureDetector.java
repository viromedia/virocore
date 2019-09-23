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

import android.graphics.PointF;
import android.view.MotionEvent;

public class RotationGestureDetector {

    public interface OnRotateGestureListener {
        public void onRotateBegin(RotationGestureDetector detector);
        public void onRotate(RotationGestureDetector detector);
        public void onRotateEnd(RotationGestureDetector detector);
    }

    private class Line {
        public float x1, y1, x2, y2;

        public Line(MotionEvent event, int pointerIndex1, int pointerIndex2) {
            x1 = event.getX(pointerIndex1);
            y1 = event.getY(pointerIndex1);
            x2 = event.getX(pointerIndex2);
            y2 = event.getY(pointerIndex2);
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
        int pointerIndex1 = 0;
        int pointerIndex2 = 0;
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                mPointer1 = event.getPointerId(event.getActionIndex());
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                mPointer2 = event.getPointerId(event.getActionIndex());
                pointerIndex1 = event.findPointerIndex(mPointer1);
                pointerIndex2 = event.findPointerIndex(mPointer2);
                if (pointerIndex1 == -1 || pointerIndex2 == -1) {
                    // Like below, this is for the S8 where for some reason, a "non-primary"
                    // pointer has touched down, while the "primary" (mPointer1) is not valid
                    break;
                }
                mFirstLine = new Line(event, pointerIndex1, pointerIndex2);
                break;
            case MotionEvent.ACTION_MOVE:
                if (mPointer1 != INVALID_POINTER && mPointer2 != INVALID_POINTER) {
                    pointerIndex1 = event.findPointerIndex(mPointer1);
                    pointerIndex2 = event.findPointerIndex(mPointer2);
                    if (pointerIndex1 == -1 || pointerIndex2 == -1) {
                        // saw this on S8, we get a "move" before an ACTION_POINTER_UP was
                        // called so mPointer2 isn't == INVALID_POINTER, but it really is.
                        break;
                    }
                    mLatestLine = new Line(event, pointerIndex1, pointerIndex2);
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
