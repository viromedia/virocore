//
//  VROInputControllerARAndroid.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROINPUTCONTROLLERARANDROID_H
#define ANDROID_VROINPUTCONTROLLERARANDROID_H

#include <VROInputControllerAR.h>

class VROInputControllerARAndroid : public VROInputControllerAR {

public:
    VROInputControllerARAndroid(float viewportWidth, float viewportHeight);
    virtual ~VROInputControllerARAndroid() {}

    void onTouchEvent(int action, float x, float y);
    void onPinchEvent(int pinchState, float scaleFactor, float x, float y);
    void onRotateEvent(int rotateState, float rotateDegrees, float x, float y);
};

#endif //ANDROID_VROINPUTCONTROLLERARANDROID_H
