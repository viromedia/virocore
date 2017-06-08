//
//  VROARCameraInertial.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARCameraInertial_h
#define VROARCameraInertial_h

#include "VROARCamera.h"
#include <memory>

class VROHeadTracker;
class VROCameraTexture;
class VRODriver;
enum class VROCameraOrientation;

class VROARCameraInertial : public VROARCamera {
public:
    
    VROARCameraInertial(std::shared_ptr<VRODriver> driver);
    virtual ~VROARCameraInertial();
    
    VROARTrackingState getTrackingState() const;
    VROARTrackingStateReason getLimitedTrackingStateReason() const;
    
    VROMatrix4f getRotation() const;
    VROVector3f getPosition() const;
    VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) const;
    
    VROVector3f getImageSize() const;
    
    /*
     Internal methods.
     */
    void run();
    void pause();
    void updateCameraOrientation(VROCameraOrientation orientation);

    std::shared_ptr<VROCameraTexture> getBackgroundTexture() {
        return _cameraTexture;
    }
    
private:
    
    /*
     Inertial sensor.
     */
    std::unique_ptr<VROHeadTracker> _headTracker;
    
    /*
     Background camera texture.
     */
    std::shared_ptr<VROCameraTexture> _cameraTexture;
    
};

#endif /* VROARCameraInertial_h */
