//
//  VROARCameraiOS.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARCameraiOS_h
#define VROARCameraiOS_h

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARCamera.h"
#include <ARKit/ARKit.h>

enum class VROCameraOrientation;

class VROARCameraiOS : public VROARCamera {
public:
    
    VROARCameraiOS(ARCamera *camera, VROCameraOrientation orientation);
    virtual ~VROARCameraiOS();
    
    VROARTrackingState getTrackingState() const;
    VROARTrackingStateReason getLimitedTrackingStateReason() const;
    
    VROMatrix4f getRotation() const;
    VROVector3f getPosition() const;
    VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) const;
    
    VROVector3f getImageSize() const;
    
private:
    
    ARCamera *_camera;
    VROCameraOrientation _orientation;
    
};

#endif
#endif /* VROARCameraiOS_h */
