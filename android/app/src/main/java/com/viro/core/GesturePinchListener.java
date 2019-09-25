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

package com.viro.core;

/**
 * Callback interface for responding to pinch events, which occur when the user pinches with two
 * fingers on the screen.
 */
public interface GesturePinchListener {

    /**
     * Callback when a two-finger pinch gesture is detected on the Controller screen. This is
     * used in AR.
     *
     * @param source      The platform specific source ID, which indicates what button or
     *                    component on the Controller triggered the event. See the Controller's
     *                    Guide for information.
     * @param node        The {@link Node} on which the gesture is being performed.
     * @param scaleFactor The scale factor change since the beginning of the event;
     *                    specifically, since receiving the PINCH_START state. Scale starts at
     *                    1.0 and increases if pinches outward and decreases if pinched inward.
     * @param pinchState  The state of the pinch event.
     */
    void onPinch(int source, Node node, float scaleFactor, PinchState pinchState);
}