//
//  VROARCameraInertial.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
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

#ifndef VROARCameraInertial_h
#define VROARCameraInertial_h

#include "VROARCamera.h"
#include <memory>
#import <AVFoundation/AVFoundation.h>

static bool kInertialRenderCameraUsingPreviewLayer = false;

class VROHeadTracker;
class VROCameraTexture;
class VROAVCaptureController;
class VRODriver;
class VROTexture;
enum class VROTrackingType;
enum class VROCameraOrientation;

class VROARCameraInertial : public VROARCamera {
public:
    
    VROARCameraInertial(VROTrackingType trackingType, std::shared_ptr<VRODriver> driver);
    virtual ~VROARCameraInertial();
    
    VROARTrackingState getTrackingState() const;
    VROARTrackingStateReason getLimitedTrackingStateReason() const;
    
    virtual VROMatrix4f getRotation() const;
    virtual VROVector3f getPosition() const;
    virtual VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV);
    
    virtual VROVector3f getImageSize();
    
    /*
     Internal methods.
     */
    virtual void run();
    virtual void pause();
    virtual void updateCameraOrientation(VROCameraOrientation orientation);

    virtual std::shared_ptr<VROTexture> getBackgroundTexture() {
        return std::dynamic_pointer_cast<VROTexture>(_cameraTexture);
    }
    
    virtual CMSampleBufferRef getSampleBuffer() const;
    
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
