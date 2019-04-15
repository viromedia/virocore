//
//  VROARCameraPrerecorded.cpp
//  ViroKit
//
//  Copyright © 2019 Viro Media. All rights reserved.
//

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
