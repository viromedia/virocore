//
//  VROARSessionInertial.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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

#include "VROARSessionInertial.h"
#include "VROARCameraInertial.h"
#include "VROARFrameInertial.h"
#include "VROCameraTexture.h"
#include "VRODriver.h"
#include "VROViewport.h"
#include "VROLog.h"
#include "VROVisionModel.h"
#include "VROARCameraPrerecorded.h"

VROARSessionInertial::VROARSessionInertial(VROTrackingType trackingType,
                                           std::shared_ptr<VRODriver> driver) :
    VROARSession(trackingType, VROWorldAlignment::Gravity) {
        if (trackingType == VROTrackingType::PrerecordedVideo) {
            _camera = std::make_shared<VROARCameraPrerecorded>(trackingType, driver);
        } else {
            _camera = std::make_shared<VROARCameraInertial>(trackingType, driver);
        }
}

VROARSessionInertial::~VROARSessionInertial() {

}

void VROARSessionInertial::run() {
    _camera->run();
    if (getTrackingType() == VROTrackingType::DOF6) {
        pabort("Inertial AR does not yet support DOF6");
    }
}

void VROARSessionInertial::pause() {
    _camera->pause();
}

bool VROARSessionInertial::isReady() const {
    return getScene() != nullptr;
}

void VROARSessionInertial::resetSession(bool resetTracking, bool removeAnchors) {
    return; // does nothing in inertial.
}

bool VROARSessionInertial::setAnchorDetection(std::set<VROAnchorDetection> types) {
    // Unsupported
    return true;
}

void VROARSessionInertial::setCloudAnchorProvider(VROCloudAnchorProvider provider) {
    // Unsupported
}

void VROARSessionInertial::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    // Unsupported
}

void VROARSessionInertial::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {
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

void VROARSessionInertial::hostCloudAnchor(std::shared_ptr<VROARAnchor> anchor,
                                           std::function<void(std::shared_ptr<VROARAnchor>)> onSuccess,
                                           std::function<void(std::string error)> onFailure) {
    // Unsupproted
}
void VROARSessionInertial::resolveCloudAnchor(std::string anchorId,
                                              std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess,
                                              std::function<void(std::string error)> onFailure) {
    // Unsupported
}

std::shared_ptr<VROTexture> VROARSessionInertial::getCameraBackgroundTexture() {
    return _camera->getBackgroundTexture();
}

std::unique_ptr<VROARFrame> &VROARSessionInertial::updateFrame() {
    if (_trackingType == VROTrackingType::PrerecordedVideo) {
        std::static_pointer_cast<VROARCameraPrerecorded>(_camera)->updateFrame();
    }

    _currentFrame = std::unique_ptr<VROARFrame>(new VROARFrameInertial(_camera, _viewport));
    if (_visionModel) {
        _visionModel->update(_currentFrame.get());
    }
    return _currentFrame;
}

std::unique_ptr<VROARFrame> &VROARSessionInertial::getLastFrame() {
    return _currentFrame;
}

void VROARSessionInertial::setViewport(VROViewport viewport) {
    _viewport = viewport;
}

void VROARSessionInertial::setOrientation(VROCameraOrientation orientation) {
    _camera->updateCameraOrientation(orientation);
}

void VROARSessionInertial::addAnchorNode(std::shared_ptr<VRONode> node) {
    // Unsupported
}

void VROARSessionInertial::setVisionModel(std::shared_ptr<VROVisionModel> visionModel) {
    _visionModel = visionModel;
}

