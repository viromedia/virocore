//
//  VROARFrameiOS.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "Availability.h"
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARFrameiOS.h"
#include "VROARAnchoriOS.h"
#include "VROARCameraiOS.h"
#include "VROVideoTextureCache.h"
#include "VROTextureSubstrate.h"
#include "VROConvert.h"
#include "VROLog.h"

VROARFrameiOS::VROARFrameiOS(ARFrame *frame, VROViewport viewport, VROCameraOrientation orientation) :
    _frame(frame),
    _viewport(viewport),
    _orientation(orientation) {
        
    _camera = std::make_shared<VROARCameraiOS>(frame.camera, orientation);
    for (ARAnchor *anchor in frame.anchors) {
        _anchors.push_back(std::make_shared<VROARAnchoriOS>(anchor));
    }
}

VROARFrameiOS::~VROARFrameiOS() {
    
}

CVPixelBufferRef VROARFrameiOS::getImage() const {
    return _frame.capturedImage;
}

double VROARFrameiOS::getTimestamp() const {
    return _frame.timestamp;
}

const std::shared_ptr<VROARCamera> &VROARFrameiOS::getCamera() const {
    return _camera;
}

VROMatrix4f VROARFrameiOS::getBackgroundTexcoordTransform() {
    CGSize viewportSize = CGSizeMake(_viewport.getWidth()  / _viewport.getContentScaleFactor(),
                                     _viewport.getHeight() / _viewport.getContentScaleFactor());
    UIInterfaceOrientation orientation = VROConvert::toDeviceOrientation(_orientation);
    CGAffineTransform transform = CGAffineTransformInvert([_frame displayTransformWithViewportSize:viewportSize
                                                                                       orientation:orientation]);
    
    VROMatrix4f matrix;
    matrix[0] = transform.a;
    matrix[1] = transform.b;
    matrix[4] = transform.c;
    matrix[5] = transform.d;
    matrix[12] = transform.tx;
    matrix[13] = transform.ty;
    return matrix;
}

const std::vector<std::shared_ptr<VROARAnchor>> &VROARFrameiOS::getAnchors() const {
    return _anchors;
}

float VROARFrameiOS::getAmbientLightIntensity() const {
    return _frame.lightEstimate.ambientIntensity;
}

#endif
