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
#include "VROAVCaptureController.h"
#include "VROCameraTextureiOS.h"
#include "VROARSession.h"

static bool kCameraInertialUseFixedPerspectiveProjection = true;

VROARCameraInertial::VROARCameraInertial(VROTrackingType trackingType, std::shared_ptr<VRODriver> driver) {
    _trackingType = trackingType;
    _headTracker = std::unique_ptr<VROHeadTracker>(new VROHeadTracker());
    
    VROCameraOrientation cameraOrientation = VROConvert::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]);
    VROCameraPosition cameraPosition = (trackingType == VROTrackingType::Front) ? VROCameraPosition::Front : VROCameraPosition::Back;
    
    if (kInertialRenderCameraUsingPreviewLayer) {
        _captureController = std::make_shared<VROAVCaptureController>();
        _captureController->initCapture(cameraPosition, cameraOrientation, true, driver);
    } else {
        // Otherwise we use a camera texture that's rendered as a VROScene background
        _cameraTexture = std::make_shared<VROCameraTextureiOS>(VROTextureType::Texture2D);
        _cameraTexture->initCamera(cameraPosition, cameraOrientation, driver);
    }
}

VROARCameraInertial::~VROARCameraInertial() {
    
}

void VROARCameraInertial::run() {
    _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
    if (_cameraTexture) {
        _cameraTexture->play();
    } else {
        _captureController->play();
    }
}

void VROARCameraInertial::pause() {
    _headTracker->stopTracking();
    if (_cameraTexture) {
        _cameraTexture->pause();
    } else {
        _captureController->pause();
    }
}

VROARTrackingState VROARCameraInertial::getTrackingState() const {
    return VROARTrackingState::Normal;
}

VROARTrackingStateReason VROARCameraInertial::getLimitedTrackingStateReason() const {
    return VROARTrackingStateReason::None;
}

VROMatrix4f VROARCameraInertial::getRotation() const {
    if (_trackingType == VROTrackingType::Front) {
        /*
         For the front-facing camera we take the head tracker rotation and
         tweak it to fit the mirrored projection. There are two tweaks:
         
         1. The head rotation Y axis is reversed from what we're receiving from the
            head tracker.
         */
        VROMatrix4f rotation = _headTracker->getHeadRotation();
        VROVector3f scale = rotation.extractScale();
        VROVector3f eulerAngles = rotation.extractRotation(scale).toEuler();
        VROQuaternion invertedX(-eulerAngles.x, eulerAngles.y, eulerAngles.z);
        
        /*
         2. Before applying the rotation we first have to move our base forward
            vector from (0, 0, -1) to (0, 0, 1), so rotate about the Y axis by
            180 degrees, *then* apply the head rotation.
         */
        VROQuaternion flipForward(0, M_PI, 0);
        return invertedX.getMatrix() * flipForward.getMatrix();
    } else {
        return _headTracker->getHeadRotation().invert();
    }
}

VROVector3f VROARCameraInertial::getPosition() const {
    // Inertial camera does not yet support position
    return { 0, 0, 0 };
}

VROMatrix4f VROARCameraInertial::getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) {
    if (kCameraInertialUseFixedPerspectiveProjection) {
        /*
         iPhone XS front-facing (derived from ARKit face tracking config).
         
         3.85051, 0, -0.00236702, 0
         0, 1.77825, -8.47578e-05, 0
         0, 0, -1.0002, -0.010002
         0, 0, -1, 0
         */
        VROMatrix4f projection;
        projection[0] = 3.85051;
        projection[5] = 1.77825;
        projection[8] = -0.00236702;
        projection[9] = -8.47578e-05;
        projection[10] = -1.0002;
        projection[11] = -1;
        projection[14] = -0.010002;
        projection[15] = 0;
        
        float fovX = toDegrees(atan(1.0f / projection[0]) * 2.0);
        float fovY = toDegrees(atan(1.0f / projection[5]) * 2.0);
        *outFOV = VROFieldOfView(fovX / 2.0, fovX / 2.0, fovY / 2.0, fovY / 2.0);
        
        return projection;
    }
    else {
        if (_cameraTexture) {
            *outFOV = VRORenderer::computeFOVFromMinorAxis(_cameraTexture->getHorizontalFOV(), viewport.getWidth(), viewport.getHeight());
        } else {
            *outFOV = VRORenderer::computeFOVFromMinorAxis(_captureController->getHorizontalFOV(), viewport.getWidth(), viewport.getHeight());
        }
        
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
}

VROVector3f VROARCameraInertial::getImageSize() {
    if (_cameraTexture) {
        return _cameraTexture->getImageSize();
    } else {
        return _captureController->getImageSize();
    }
}

CMSampleBufferRef VROARCameraInertial::getSampleBuffer() const {
    if (_cameraTexture) {
        std::shared_ptr<VROCameraTextureiOS> texture = std::dynamic_pointer_cast<VROCameraTextureiOS>(_cameraTexture);
        return texture->getSampleBuffer();
    } else {
        return _captureController->getSampleBuffer();
    }
}

void VROARCameraInertial::updateCameraOrientation(VROCameraOrientation orientation) {
    UIInterfaceOrientation deviceOrientation = VROConvert::toDeviceOrientation(orientation);
    _headTracker->updateDeviceOrientation(deviceOrientation);
}

