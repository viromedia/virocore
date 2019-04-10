//
//  VROARCameraPrerecorded.h
//  ViroKit
//
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROARCameraPrerecorded_h
#define VROARCameraPrerecorded_h
#include "VROARCameraInertial.h"
#include "VROVideoTexture.h"
#import <AVFoundation/AVFoundation.h>
#include <memory>

/*
 An AR Camera that uses a video texture for rendering an ARScene's background.
 */
class VROARCameraPrerecorded : public VROARCameraInertial {
public:
    VROARCameraPrerecorded(VROTrackingType trackingType,
                           std::shared_ptr<VRODriver> driver);
    virtual ~VROARCameraPrerecorded();

    /*
     Returns the sample buffer representing last video frame, used by VROBodyTracker.
     */
    CMSampleBufferRef getSampleBuffer() const;

    /*
     Returns VROTexture representing the video output to be rendered on the background.
     */
    virtual std::shared_ptr<VROTexture> getBackgroundTexture() {
        return std::dynamic_pointer_cast<VROTexture>(_videoTexture);
    }

    /*
     Returns the size of loaded video for getViewportToCameraImageTransform()
     calculations.
     */
    VROVector3f getImageSize();

    // Transforms representing this camera
    VROMatrix4f getRotation() const;
    VROVector3f getPosition() const;
    VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV);

    // Internal methods as needed in VROARCameraIntertial.
    void run();
    void pause();
    void updateFrame();
private:
    std::shared_ptr<VROVideoTexture> _videoTexture;
};

#endif /* VROARCameraPrerecorded_h */
