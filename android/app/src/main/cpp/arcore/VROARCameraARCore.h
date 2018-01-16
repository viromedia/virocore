//
//  VROARCameraARCore.h
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARCameraARCore_h
#define VROARCameraARCore_h

#include "VROARCamera.h"
#include "ARCore_JNI.h"

class VROARSessionARCore;
enum class VROCameraOrientation;

class VROARCameraARCore : public VROARCamera {
public:
    
    VROARCameraARCore(jni::Object<arcore::Frame> frame);
    virtual ~VROARCameraARCore();
    
    VROARTrackingState getTrackingState() const;
    VROARTrackingStateReason getLimitedTrackingStateReason() const;
    
    VROMatrix4f getRotation() const;
    VROVector3f getPosition() const;
    VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) const;
    
    VROVector3f getImageSize() const;
    
private:
    
    jni::UniqueWeakObject <arcore::Frame> _frame;

    VROVector3f _position;
    VROMatrix4f _rotation;

};

#endif /* VROARCameraARCore_h */
