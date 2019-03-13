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
#import <AVFoundation/AVFoundation.h>

class VROHeadTracker;
class VROCameraTexture;
class VROAVCaptureController;
class VRODriver;
enum class VROTrackingType;
enum class VROCameraOrientation;

class VROARCameraInertial : public VROARCamera {
public:
    
    VROARCameraInertial(VROTrackingType trackingType, std::shared_ptr<VRODriver> driver);
    virtual ~VROARCameraInertial();
    
    VROARTrackingState getTrackingState() const;
    VROARTrackingStateReason getLimitedTrackingStateReason() const;
    
    VROMatrix4f getRotation() const;
    VROVector3f getPosition() const;
    VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV);
    
    VROVector3f getImageSize();
    
    /*
     Internal methods.
     */
    void run();
    void pause();
    void updateCameraOrientation(VROCameraOrientation orientation);

    std::shared_ptr<VROCameraTexture> getBackgroundTexture() {
        return _cameraTexture;
    }
    
    CMSampleBufferRef getSampleBuffer() const;
    
private:
    
    VROTrackingType _trackingType;
    
    /*
     Inertial sensor.
     */
    std::unique_ptr<VROHeadTracker> _headTracker;
    
    /*
     Background capture controller or camera texture. We use the camera
     texture to render the background with Viro, and the capture controller
     directly if we want to render using an iOS video preview layer. We
     never use both.
     */
    std::shared_ptr<VROAVCaptureController> _captureController;
    std::shared_ptr<VROCameraTexture> _cameraTexture;
    
};

#endif /* VROARCameraInertial_h */
