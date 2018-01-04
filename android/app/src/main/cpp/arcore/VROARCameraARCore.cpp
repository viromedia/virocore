//
//  VROARCameraARCore.cpp
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARCameraARCore.h"
#include "VROViewport.h"
#include "VROMath.h"
#include "VROLog.h"
#include "VROCameraTexture.h"
#include "VROVector3f.h"
#include "VROFieldOfView.h"
#include "VROPlatformUtil.h"
#include "VROMatrix4f.h"
#include "VROARSessionARCore.h"

VROARCameraARCore::VROARCameraARCore(jni::Object<arcore::Frame> frame, std::shared_ptr<VROARSessionARCore> session) :
    _session(session) {
    _frame = frame.NewGlobalRef(*VROPlatformGetJNIEnv());

    VROMatrix4f viewMatrix = arcore::frame::getViewMatrix(frame);
    VROMatrix4f cameraMatrix = viewMatrix.invert();

    _position = { cameraMatrix[12], cameraMatrix[13], cameraMatrix[14] };

    // Remove the translation (this is returned via getPosition()) and we should be
    // left with rotation only
    _rotation = cameraMatrix;
    _rotation[12] = 0;
    _rotation[13] = 0;
    _rotation[14] = 0;

    // We have to compensate for the fact that our Viro axes rotate when the device
    // orientation changes, while the "real" axes used by ARKit do not.
    /*
    VROMatrix4f rotation;
    if (_orientation == VROCameraOrientation::Portrait) {
        rotation.rotateZ(M_PI / 2);
    }
    else if (_orientation == VROCameraOrientation::LandscapeLeft) {
        rotation.rotateZ(M_PI);
    }
    else if (_orientation == VROCameraOrientation::PortraitUpsideDown) {
        rotation.rotateZ(3 * M_PI / 2);
    }
     return matrix * rotation;
     */
}

VROARCameraARCore::~VROARCameraARCore() {
    
}

VROARTrackingState VROARCameraARCore::getTrackingState() const {
    arcore::TrackingState trackingState = arcore::frame::getTrackingState(*_frame.get());
    switch (trackingState) {
        case arcore::TrackingState::NotTracking:
            return VROARTrackingState::Unavailable;
        default:
            return VROARTrackingState::Normal;
    };
}

VROARTrackingStateReason VROARCameraARCore::getLimitedTrackingStateReason() const {
    return VROARTrackingStateReason::None;
}

VROMatrix4f VROARCameraARCore::getRotation() const {
    return _rotation;
}

VROVector3f VROARCameraARCore::getPosition() const {
    return _position;
}

VROMatrix4f VROARCameraARCore::getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) const {
    VROMatrix4f projection = arcore::frame::getProjectionMatrix(*_frame.get(), near, far);
    float fovX = toDegrees(atan(1.0f / projection[0]) * 2.0);
    float fovY = toDegrees(atan(1.0f / projection[5]) * 2.0);
    *outFOV = VROFieldOfView(fovX / 2.0, fovX / 2.0, fovY / 2.0, fovY / 2.0);

    return projection;
}

VROVector3f VROARCameraARCore::getImageSize() const {
    pabort("Image size not supported in ARCore");
    return {};
}