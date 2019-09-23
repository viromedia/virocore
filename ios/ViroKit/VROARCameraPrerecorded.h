//
//  VROARCameraPrerecorded.h
//  ViroKit
//
//  Copyright © 2019 Viro Media. All rights reserved.
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
