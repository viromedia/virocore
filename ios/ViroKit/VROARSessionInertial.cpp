//
//  VROARSessionInertial.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARSessionInertial.h"
#include "VROARCameraInertial.h"
#include "VROARFrameInertial.h"
#include "VROCameraTexture.h"
#include "VRODriver.h"
#include "VROViewport.h"
#include "VROLog.h"

VROARSessionInertial::VROARSessionInertial(VROTrackingType trackingType, std::shared_ptr<VRODriver> driver) :
    VROARSession(trackingType) {
    _camera = std::make_shared<VROARCameraInertial>(driver);
}

VROARSessionInertial::~VROARSessionInertial() {

}

void VROARSessionInertial::run() {
    _camera->run();
    if (getTrackingType() == VROTrackingType::DOF3) {
        
    }
    else { // DOF6
        pabort("Inertial AR does not yet support DOF6");
    }
}

void VROARSessionInertial::pause() {
    _camera->pause();
}

bool VROARSessionInertial::isReady() const {
    return getScene() != nullptr;
}

void VROARSessionInertial::setAnchorDetection(std::set<VROAnchorDetection> types) {
    // Unsupported
}

void VROARSessionInertial::addAnchor(std::shared_ptr<VROARAnchor> anchor) {
    // Unsupported
}

void VROARSessionInertial::removeAnchor(std::shared_ptr<VROARAnchor> anchor) {
    // Unsupported
}

void VROARSessionInertial::updateAnchor(std::shared_ptr<VROARAnchor> anchor) {
    // Unsupported
}

std::shared_ptr<VROTexture> VROARSessionInertial::getCameraBackgroundTexture() {
    return _camera->getBackgroundTexture();
}

std::unique_ptr<VROARFrame> &VROARSessionInertial::updateFrame() {
    _currentFrame = std::unique_ptr<VROARFrame>(new VROARFrameInertial(_camera));
    return _currentFrame;
}

std::unique_ptr<VROARFrame> &VROARSessionInertial::getLastFrame() {
    return _currentFrame;
}

void VROARSessionInertial::setViewport(VROViewport viewport) {
    
}

void VROARSessionInertial::setOrientation(VROCameraOrientation orientation) {
    _camera->updateCameraOrientation(orientation);
}

void VROARSessionInertial::addAnchorNode(std::shared_ptr<VRONode> node) {
    // Unsupported
}


