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

VROARFrameInertial::VROARFrameInertial(const std::shared_ptr<VROARCamera> &camera, VROViewport viewport) :
    _camera(camera),
    _viewport(viewport) {
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
    VROVector3f imageSize = _camera->getImageSize();
    
    // When the image is rendered, it's expanded to the viewport's size, which
    // may end up stretching the image as the scale is anisotropic. Here we return
    // texture coordinates to apply in order to undo this elongation.
    float scaleX = (float) _viewport.getWidth()  / imageSize.x;
    float scaleY = (float) _viewport.getHeight() / imageSize.y;
    
    VROMatrix4f matrix;
    matrix[0] = 1.0 / scaleY;
    matrix[5] = 1.0 / scaleX;
    matrix[12] = (1 - matrix[0]) / 2.0;
    matrix[13] = (1 - matrix[5]) / 2.0;
    return matrix;
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
    return camera->getSampleBuffer();
}

