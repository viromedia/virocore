//
//  VROInputControllerARAndroid.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <android/input.h>
#include "VROInputControllerARAndroid.h"

VROInputControllerARAndroid::VROInputControllerARAndroid(float viewportWidth,
                                                         float viewportHeight) :
    VROInputControllerAR(viewportHeight, viewportWidth) {
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

void VROInputControllerARAndroid::onRotateEvent(int rotateState, float rotateDegrees, float x, float y) {
    if (rotateState == AMOTION_EVENT_ACTION_DOWN) {
        onRotateStart(VROVector3f(x, y));
    } else if (rotateState == AMOTION_EVENT_ACTION_MOVE) {
        onRotate(rotateDegrees);
    } else if (rotateState == AMOTION_EVENT_ACTION_UP) {
        onRotateEnd();
    } else {
        pwarn("[Viro] onRotateEvent unknown action: %d", rotateState);
    }
}

void VROInputControllerARAndroid::processCenterCameraHitTest() {
    pinfo("Overridden VROInputControllerARAndroid processCenterCamereHitTest() executed.");
        VROInputControllerAR::processCenterCameraHitTest();

}
