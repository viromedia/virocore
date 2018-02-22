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
#include "ARCore_Native.h"
#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include <memory>

class VROARSessionARCore;
class VROARFrameARCore;
enum class VROCameraOrientation;

class VROARCameraARCore : public VROARCamera {
public:
    
    VROARCameraARCore(ArFrame *frame,
                      std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARCameraARCore();
    
    VROARTrackingState getTrackingState() const;
    VROARTrackingStateReason getLimitedTrackingStateReason() const;
    
    VROMatrix4f getRotation() const;
    VROVector3f getPosition() const;
    VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) const;
    
    VROVector3f getImageSize() const;
    
private:

    ArFrame *_frame;
    std::weak_ptr<VROARSessionARCore> _session;

    VROVector3f _position;
    VROMatrix4f _rotation;

};

#endif /* VROARCameraARCore_h */
