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

VROARCameraInertial::VROARCameraInertial(VROTrackingType trackingType, std::shared_ptr<VRODriver> driver) {
    _trackingType = trackingType;
    
    // Head tracking is not enabled for Pre-recorded cameras for now.
    if (trackingType != VROTrackingType::PrerecordedVideo) {
        _headTracker = std::unique_ptr<VROHeadTracker>(new VROHeadTracker());
    }
    
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
    if (_headTracker) {
        _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
    }

    if (_cameraTexture) {
        _cameraTexture->play();
    } else {
        _captureController->play();
    }
}

void VROARCameraInertial::pause() {
    if (_headTracker) {
        _headTracker->stopTracking();
    }
    
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
    if (!_headTracker) {
        return VROMatrix4f::identity();
    }
    
    if (_trackingType == VROTrackingType::Front) {
        /*
         For the front-facing camera we take the head tracker rotation use it
         directly.
         */
        VROMatrix4f headRotation = _headTracker->getHeadRotation();
        
        /*
         Before applying the rotation we first have to move our base forward
         vector from (0, 0, -1) to (0, 0, 1), so rotate about the Y axis by
         180 degrees, *then* apply the head rotation.
         */
        VROQuaternion rotateForward180(0, M_PI, 0);
        return headRotation * rotateForward180.getMatrix();
    } else {
        return _headTracker->getHeadRotation().invert();
    }
}

VROVector3f VROARCameraInertial::getPosition() const {
    // Inertial camera does not yet support position
    return { 0, 0, 0 };
}

VROMatrix4f VROARCameraInertial::getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) {
    std::vector<float> intrinsics;
    if (_cameraTexture) {
        intrinsics = _cameraTexture->getCameraIntrinsics();
    } else {
        intrinsics = _captureController->getCameraIntrinsics();
    }
    
    // Fallback if no intrinsics are given
    if (intrinsics.empty()) {
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
    } else {
        float fx = intrinsics[0];
        float fy = intrinsics[5];
        float cx = intrinsics[8];
        float cy = intrinsics[9];
        
        float width  = (float) viewport.getWidth();
        float height = (float) viewport.getHeight();
        
        VROMatrix4f projection;
        projection[0] = 2.0 * fx / width;
        projection[5] = 2.0 * fy / height;
        projection[8] = 1.0 - 2.0 * cx / width;
        projection[9] = 2.0 * cy / height - 1.0;
        projection[10] = (near + far) / (near - far);
        projection[11] = -1.0;
        projection[14] = 2.0 * far * near / (near - far);
        projection[15] = 0;
       
        return projection;
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
    if (!_headTracker) {
        return;
    }
    
    UIInterfaceOrientation deviceOrientation = VROConvert::toDeviceOrientation(orientation);
    _headTracker->updateDeviceOrientation(deviceOrientation);
}

