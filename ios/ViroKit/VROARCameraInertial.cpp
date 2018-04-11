//
//  VROARCameraInertial.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARCameraInertial.h"
#include "VROConvert.h"
#include "VROViewport.h"
#include "VROHeadTracker.h"
#include "VRORenderer.h"
#include "VROCameraTextureiOS.h"

VROARCameraInertial::VROARCameraInertial(std::shared_ptr<VRODriver> driver) {
    _headTracker = std::unique_ptr<VROHeadTracker>(new VROHeadTracker());
    
    VROCameraOrientation orientation = VROConvert::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]);
    _cameraTexture = std::make_shared<VROCameraTextureiOS>(VROTextureType::Texture2D);
    _cameraTexture->initCamera(VROCameraPosition::Back, orientation, driver);
}

VROARCameraInertial::~VROARCameraInertial() {
    
}

void VROARCameraInertial::run() {
    _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
    _cameraTexture->play();
}

void VROARCameraInertial::pause() {
    _headTracker->stopTracking();
    _cameraTexture->pause();
}

VROARTrackingState VROARCameraInertial::getTrackingState() const {
    return VROARTrackingState::Normal;
}

VROARTrackingStateReason VROARCameraInertial::getLimitedTrackingStateReason() const {
    return VROARTrackingStateReason::None;
}

VROMatrix4f VROARCameraInertial::getRotation() const {
    return _headTracker->getHeadRotation().invert(); 
}

VROVector3f VROARCameraInertial::getPosition() const {
    // Inertial camera does not yet support position
    return { 0, 0, 0 };
}

VROMatrix4f VROARCameraInertial::getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) {
    *outFOV = VRORenderer::computeFOVFromMinorAxis(_cameraTexture->getHorizontalFOV(), viewport.getWidth(), viewport.getHeight());
    
    VROVector3f cameraImageSize = getImageSize();
    float fx = fabs((float)cameraImageSize.x / (2 * tan(toRadians(outFOV->getLeft() + outFOV->getRight()) / 2.0)));
    float fy = fx;
    
    float s = 0;
    float x0 = viewport.getWidth() / 2.0;
    float y0 = viewport.getHeight() / 2.0;
    float X = near + far;
    float Y = near * far;
    
    float intrinsic[16] = {fx,   0,   0,  0,
                            s,  fy,   0,  0,
                          -x0, -y0,   X, -1,
                            0,   0,   Y,  0 };
    VROMatrix4f intrinsicMatrix(intrinsic);
    return viewport.getOrthographicProjection(near, far) * intrinsicMatrix;
}

VROVector3f VROARCameraInertial::getImageSize() {
    return _cameraTexture->getImageSize();
}

void VROARCameraInertial::updateCameraOrientation(VROCameraOrientation orientation) {
    UIInterfaceOrientation deviceOrientation = VROConvert::toDeviceOrientation(orientation);
    _headTracker->updateDeviceOrientation(deviceOrientation);
}

