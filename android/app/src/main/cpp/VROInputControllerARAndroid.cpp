//
//  VROInputControllerARAndroid.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
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

#include <android/input.h>
#include "VROInputControllerARAndroid.h"

VROInputControllerARAndroid::VROInputControllerARAndroid(float viewportWidth,
                                                         float viewportHeight,
                                                         std::shared_ptr<VRODriver> driver) :
    VROInputControllerAR(viewportHeight, viewportWidth, driver) {
    // no-op
}

void VROInputControllerARAndroid::onTouchEvent(int action, float x, float y) {
    if (action == AMOTION_EVENT_ACTION_DOWN) {
        onScreenTouchDown(VROVector3f(x, y));
    } else if (action == AMOTION_EVENT_ACTION_MOVE) {
        onScreenTouchMove(VROVector3f(x, y));
    } else if (action == AMOTION_EVENT_ACTION_UP) {
        onScreenTouchUp(VROVector3f(x, y));
    } else {
        pwarn("[Viro] onTouchEvent unknown action: %d", action);
    }
}

void VROInputControllerARAndroid::onPinchEvent(int pinchState, float scaleFactor, float x, float y) {
    if (pinchState == AMOTION_EVENT_ACTION_DOWN) {
        onPinchStart(VROVector3f(x, y));
    } else if (pinchState == AMOTION_EVENT_ACTION_MOVE) {
        onPinchScale(scaleFactor);
    } else if (pinchState == AMOTION_EVENT_ACTION_UP) {
        onPinchEnd();
    } else {
        pwarn("[Viro] onPinchEvent unknown action: %d", pinchState);
    }
}

void VROInputControllerARAndroid::onRotateEvent(int rotateState, float rotateRadians, float x, float y) {
    if (rotateState == AMOTION_EVENT_ACTION_DOWN) {
        onRotateStart(VROVector3f(x, y));
    } else if (rotateState == AMOTION_EVENT_ACTION_MOVE) {
        onRotate(rotateRadians);
    } else if (rotateState == AMOTION_EVENT_ACTION_UP) {
        onRotateEnd();
    } else {
        pwarn("[Viro] onRotateEvent unknown action: %d", rotateState);
    }
}

