//
//  VROARCameraPrerecorded.cpp
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

#include "VROARCameraPrerecorded.h"
#include "VROViewport.h"
#include "VRORenderer.h"
#include "VROCameraTextureiOS.h"
#include "VROVideoTextureiOS.h"
static const double kFovMajorPrerecordedCamera = 70;

VROARCameraPrerecorded::VROARCameraPrerecorded(VROTrackingType trackingType,
                                               std::shared_ptr<VRODriver> driver):
                                                VROARCameraInertial(trackingType, driver) {
    // Configure the video texture to use CMSampleBufferring
    _videoTexture = std::make_shared<VROVideoTextureiOS>(VROStereoMode::None, true);
}

VROARCameraPrerecorded::~VROARCameraPrerecorded() {
}

void VROARCameraPrerecorded::updateFrame() {
    std::shared_ptr<VROVideoTextureiOS> vidTexture = std::static_pointer_cast<VROVideoTextureiOS>(_videoTexture);
    vidTexture->updateFrame();
}

void VROARCameraPrerecorded::run() {
    //No-op
}

void VROARCameraPrerecorded::pause() {
    //No-op
}

VROMatrix4f VROARCameraPrerecorded::getRotation() const {
    return VROMatrix4f::identity();
}

VROVector3f VROARCameraPrerecorded::getPosition() const {
    return { 0, 0, 0 };
}

VROMatrix4f VROARCameraPrerecorded::getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) {
    float viewportWidth = viewport.getWidth();
    float viewportHeight = viewport.getHeight();

    // Assume that the user cannot move the camera in VROTrackingType::Video.
    *outFOV = VRORenderer::computeFOVFromMajorAxis(kFovMajorPrerecordedCamera, viewportWidth, viewportHeight);

    // Use the computed fov to calculate projection.
    return outFOV->toPerspectiveProjection(near, far);
}

VROVector3f VROARCameraPrerecorded::getImageSize() {
    std::shared_ptr<VROVideoTextureiOS> vidTexture
                = std::dynamic_pointer_cast<VROVideoTextureiOS>(_videoTexture);
    return vidTexture->getVideoDimensions();
}

CMSampleBufferRef VROARCameraPrerecorded::getSampleBuffer() const {
    std::shared_ptr<VROVideoTextureiOS> vidTexture
                = std::dynamic_pointer_cast<VROVideoTextureiOS>(_videoTexture);
    return vidTexture->getSampleBuffer();
}
