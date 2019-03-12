//
//  VROARFrameInertial.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARFrameInertial.h"
#include "VROARAnchorInertial.h"
#include "VROARCameraInertial.h"
#include "VROTextureSubstrate.h"
#include "VROARHitTestResult.h"
#include "VROTime.h"
#include "VROViewport.h"
#include "VROCameraTextureiOS.h"

VROARFrameInertial::VROARFrameInertial(const std::shared_ptr<VROARCamera> &camera) :
    _camera(camera) {
    _timestamp = VROTimeCurrentSeconds();
}

VROARFrameInertial::~VROARFrameInertial() {
    
}

double VROARFrameInertial::getTimestamp() const {
    return _timestamp;
}

const std::shared_ptr<VROARCamera> &VROARFrameInertial::getCamera() const {
    return _camera;
}

std::vector<std::shared_ptr<VROARHitTestResult>> VROARFrameInertial::hitTest(int x, int y, std::set<VROARHitTestResultType> types) {
    // Unsupported
    return {};
}

VROMatrix4f VROARFrameInertial::getViewportToCameraImageTransform() const {
    return {}; // Identity
}

const std::vector<std::shared_ptr<VROARAnchor>> &VROARFrameInertial::getAnchors() const {
    return _anchors;
}

float VROARFrameInertial::getAmbientLightIntensity() const {
    // Light intensity is not supported by the Inertial AR engine
    return 1.0;
}

VROVector3f VROARFrameInertial::getAmbientLightColor() const {
    // Light color is not supported by the Inertial AR engine
    return { 1.0, 1.0, 1.0 };
}

std::shared_ptr<VROARPointCloud> VROARFrameInertial::getPointCloud() {
    return std::make_shared<VROARPointCloud>();
}

CMSampleBufferRef VROARFrameInertial::getImage() const {
    std::shared_ptr<VROARCameraInertial> camera = std::dynamic_pointer_cast<VROARCameraInertial>(_camera);
    std::shared_ptr<VROCameraTextureiOS> texture = std::dynamic_pointer_cast<VROCameraTextureiOS>(camera->getBackgroundTexture());
    return texture->getSampleBuffer();
}

