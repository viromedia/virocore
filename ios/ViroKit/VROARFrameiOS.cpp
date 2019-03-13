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
#include "VROARSessioniOS.h"
#include "VROARAnchoriOS.h"
#include "VROARCameraiOS.h"
#include "VROVideoTextureCache.h"
#include "VROTextureSubstrate.h"
#include "VROConvert.h"
#include "VROVector4f.h"
#include "VROARHitTestResult.h"
#include "VROLight.h"
#include "VROCameraTexture.h"

VROARFrameiOS::VROARFrameiOS(ARFrame *frame, VROViewport viewport, VROCameraOrientation orientation,
                             std::shared_ptr<VROARSessioniOS> session) :
    _frame(frame),
    _viewport(viewport),
    _orientation(orientation),
    _session(session) {
        
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

CGImagePropertyOrientation VROARFrameiOS::getImageOrientation() const {
    // Image orientation is determined by ARKit and is based on the camera orientation.
    // When in portrait mode, for example, ARKit returns its image rotated to the right.
    if (_orientation == VROCameraOrientation::Portrait) {
        return kCGImagePropertyOrientationRight;
    }
    else {
        // TODO Fill in proper image orientation types
        return kCGImagePropertyOrientationRight;
    }
}

double VROARFrameiOS::getTimestamp() const {
    return _frame.timestamp;
}

const std::shared_ptr<VROARCamera> &VROARFrameiOS::getCamera() const {
    return _camera;
}

ARHitTestResultType convertResultTypes(std::set<VROARHitTestResultType> types) {
    ARHitTestResultType type = 0;
    for (VROARHitTestResultType t : types) {
        if (t == VROARHitTestResultType::ExistingPlaneUsingExtent) {
            type |= ARHitTestResultTypeExistingPlaneUsingExtent;
        }
        else if (t == VROARHitTestResultType::ExistingPlane) {
            type |= ARHitTestResultTypeExistingPlane;
        }
        else if (t == VROARHitTestResultType::EstimatedHorizontalPlane) {
            type |= ARHitTestResultTypeEstimatedHorizontalPlane;
        }
        else if (t == VROARHitTestResultType::FeaturePoint) {
            type |= ARHitTestResultTypeFeaturePoint;
        }
    }
    
    return type;
}

VROARHitTestResultType convertResultType(ARHitTestResultType type) {
    if (type == ARHitTestResultTypeExistingPlaneUsingExtent) {
        return VROARHitTestResultType::ExistingPlaneUsingExtent;
    }
    else if (type == ARHitTestResultTypeExistingPlane) {
        return VROARHitTestResultType::ExistingPlane;
    }
    else if (type == ARHitTestResultTypeEstimatedHorizontalPlane) {
        return VROARHitTestResultType::EstimatedHorizontalPlane;
    }
    else if (type == ARHitTestResultTypeFeaturePoint) {
        return VROARHitTestResultType::FeaturePoint;
    }
    else {
        pabort();
    }
}

std::vector<std::shared_ptr<VROARHitTestResult>> VROARFrameiOS::hitTest(int x, int y, std::set<VROARHitTestResultType> types) {
    /*
     Convert from viewport space to camera image space.
     */
    VROMatrix4f viewportSpaceToCameraImageSpace = getViewportToCameraImageTransform();
    VROVector3f pointViewport(x / (float)_viewport.getWidth(), y / (float)_viewport.getHeight());
    VROVector3f pointCameraImage = viewportSpaceToCameraImageSpace.multiply(pointViewport);
    
    /*
     Perform the ARKit hit test.
     */
    CGPoint point = CGPointMake(pointCameraImage.x, pointCameraImage.y);
    ARHitTestResultType typesAR = convertResultTypes(types);
    NSArray<ARHitTestResult *> *results = [_frame hitTest:point types:typesAR];
    
    /*
     Convert the results to VROARHitTestResult objects.
     */
    std::shared_ptr<VROARSessioniOS> session = _session.lock();
    std::vector<std::shared_ptr<VROARHitTestResult>> vResults;
    for (ARHitTestResult *result in results) {
        std::shared_ptr<VROARAnchor> vAnchor;
        if (session && result.anchor) {
            vAnchor = session->getAnchorForNative(result.anchor);
        }
        
        std::shared_ptr<VROARHitTestResult> vResult = std::make_shared<VROARHitTestResult>(convertResultType(result.type), vAnchor, result.distance,
                                   VROConvert::toMatrix4f(result.worldTransform),
                                   VROConvert::toMatrix4f(result.localTransform));
        vResults.push_back(vResult);
    }
    
    return vResults;
}

VROMatrix4f VROARFrameiOS::getViewportToCameraImageTransform() const {
    CGSize viewportSize = CGSizeMake(_viewport.getWidth()  / _viewport.getContentScaleFactor(),
                                     _viewport.getHeight() / _viewport.getContentScaleFactor());
    UIInterfaceOrientation orientation = VROConvert::toDeviceOrientation(_orientation);
    
    /*
     The display transform converts from the camera's image space (normalized image space) to
     viewport space for the given orientation and viewport. We can either apply this transform
     to *vertices* of the camera background (by modifying its projection matrix) or apply the
     *inverse* of this transform to the *texture coordinates* of the camera background. The
     two are equivalent. We do the latter, since our camera background uses a fixed orthographic
     projection.
     */
    CGAffineTransform transform = CGAffineTransformInvert([_frame displayTransformForOrientation:orientation viewportSize:viewportSize]);
    
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

VROVector3f VROARFrameiOS::getAmbientLightColor() const {
    return VROLight::deriveRGBFromTemperature(_frame.lightEstimate.ambientColorTemperature);
}

std::shared_ptr<VROARPointCloud> VROARFrameiOS::getPointCloud() {
    if (_pointCloud) {
        return _pointCloud;
    }
    std::vector<VROVector4f> points;
    ARPointCloud *arPointCloud = _frame.rawFeaturePoints;
    for (int i = 0; i < arPointCloud.count; i++) {
        vector_float3 arPoint = arPointCloud.points[i];
        // the last value in the point is a "confidence" value from 0 to 1. Since
        // ARKit doesn't provide this, set it to 1.
        points.push_back(VROVector4f(arPoint.x, arPoint.y, arPoint.z, 1));
    }
    
    std::vector<uint64_t> identifiers(arPointCloud.identifiers, arPointCloud.identifiers + arPointCloud.count);
    
    _pointCloud = std::make_shared<VROARPointCloud>(points, identifiers);
    return _pointCloud;
}

#endif
