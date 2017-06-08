//
//  VROARCameraiOS.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "Availability.h"
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARCameraiOS.h"
#include "VROMath.h"
#include "VROViewport.h"
#include "VROLog.h"

VROARCameraiOS::VROARCameraiOS(ARCamera *camera) :
    _camera(camera) {
    
}

VROARCameraiOS::~VROARCameraiOS() {
    
}

VROARTrackingState VROARCameraiOS::getTrackingState() const {
    switch (_camera.trackingState) {
        case ARTrackingStateNotAvailable:
            return VROARTrackingState::Unavailable;
        case ARTrackingStateLimited:
            return VROARTrackingState::Limited;
        case ARTrackingStateNormal:
            return VROARTrackingState::Normal;
        default:
            return VROARTrackingState::Unavailable;
    };
}

VROARTrackingStateReason VROARCameraiOS::getLimitedTrackingStateReason() const {
    switch (_camera.trackingStateReason) {
        case ARTrackingStateReasonNone:
            return VROARTrackingStateReason::None;
        case ARTrackingStateReasonExcessiveMotion:
            return VROARTrackingStateReason::ExcessiveMotion;
        case ARTrackingStateReasonInsufficientFeatures:
            return VROARTrackingStateReason::InsufficientFeatures;
        default:
            return VROARTrackingStateReason::None;
    }
}

VROMatrix4f VROARCameraiOS::getRotation() const {
    VROMatrix4f matrix = toMatrix4f(_camera.transform);
    
    // Remove the translation; this is returned via getPosition()
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    
    pinfo("Rotation matrix %s", matrix.toString().c_str());
    
    return matrix;
}

VROVector3f VROARCameraiOS::getPosition() const {
    VROMatrix4f matrix = toMatrix4f(_camera.transform);
    return { matrix[12], matrix[13], matrix[14] };
}

VROMatrix4f VROARCameraiOS::getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) const {
    // TODO Do we need to pass in an interface orientation here?
    // TODO Output the FOV!
    return toMatrix4f([_camera projectionMatrixWithViewportSize:CGSizeMake(viewport.getWidth() / viewport.getContentScaleFactor(),
                                                                           viewport.getHeight() / viewport.getContentScaleFactor())
                                                    orientation:[[UIApplication sharedApplication] statusBarOrientation]
                                                          zNear:near
                                                           zFar:far]);
}

VROVector3f VROARCameraiOS::getImageSize() const {
    CGSize size = _camera.imageResolution;
    return { (float) size.width, (float) size.height, 0 };
}

#endif
