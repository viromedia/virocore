//
//  VROARFrameARCore.cpp
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARFrameARCore.h"
#include "VROARSessionARCore.h"
#include "VROARCameraARCore.h"
#include "VROTextureSubstrate.h"
#include "VROVector3f.h"
#include "VROARHitTestResult.h"
#include "VROPlatformUtil.h"
#include "VROLog.h"

VROARFrameARCore::VROARFrameARCore(jni::Object<arcore::Frame> frameJNI,
                                   VROViewport viewport,
                                   std::shared_ptr<VROARSessionARCore> session) :
    _session(session),
    _viewport(viewport) {

    _frameJNI = frameJNI.NewGlobalRef(*VROPlatformGetJNIEnv());
    _camera = std::make_shared<VROARCameraARCore>(frameJNI, session);
}

VROARFrameARCore::~VROARFrameARCore() {
    
}

double VROARFrameARCore::getTimestamp() const {
    return (double) arcore::frame::getTimestampNs(*_frameJNI.get());
}

const std::shared_ptr<VROARCamera> &VROARFrameARCore::getCamera() const {
    return _camera;
}

std::vector<VROARHitTestResult> VROARFrameARCore::hitTest(int x, int y, std::set<VROARHitTestResultType> types) {
    /*
     Convert from viewport space to camera image space.
     */
    VROMatrix4f viewportSpaceToCameraImageSpace = getViewportToCameraImageTransform();
    VROVector3f pointViewport(x / (float)_viewport.getWidth(), y / (float)_viewport.getHeight());
    VROVector3f pointCameraImage = viewportSpaceToCameraImageSpace.multiply(pointViewport);
    
    /*
     Perform the ARCore hit test.

     TODO VIRO-1895 Convert this to ARCore
     */
    std::vector<VROARHitTestResult> vResults;

    /*
    CGPoint point = CGPointMake(pointCameraImage.x, pointCameraImage.y);
    ARHitTestResultType typesAR = convertResultTypes(types);
    NSArray<ARHitTestResult *> *results = [_frame hitTest:point types:typesAR];

    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    for (ARHitTestResult *result in results) {
        std::shared_ptr<VROARAnchor> vAnchor;
        if (session && result.anchor) {
            vAnchor = session->getAnchorForNative(result.anchor);
        }
        
        VROARHitTestResult vResult(convertResultType(result.type), vAnchor, result.distance,
                                   VROConvert::toMatrix4f(result.worldTransform),
                                   VROConvert::toMatrix4f(result.localTransform));
        vResults.push_back(vResult);
    }
    */
    
    return vResults;
}

VROMatrix4f VROARFrameARCore::getViewportToCameraImageTransform() {
    pabort("Not supported on ARCore");
}

bool VROARFrameARCore::isDisplayRotationChanged() {
    return arcore::frame::isDisplayRotationChanged(*_frameJNI.get());
}

void VROARFrameARCore::getBackgroundTexcoords(VROVector3f *BL, VROVector3f *BR, VROVector3f *TL, VROVector3f *TR) {
    std::vector<float> texcoords = arcore::frame::getBackgroundTexcoords(*_frameJNI.get());
    BL->x = texcoords[0];
    BL->y = texcoords[1];
    TL->x = texcoords[2];
    TL->y = texcoords[3];
    BR->x = texcoords[4];
    BR->y = texcoords[5];
    TR->x = texcoords[6];
    TR->y = texcoords[7];
}

const std::vector<std::shared_ptr<VROARAnchor>> &VROARFrameARCore::getAnchors() const {
    return _anchors;
}

float VROARFrameARCore::getAmbientLightIntensity() const {
    jni::Object<arcore::LightEstimate> lightEstimate = arcore::frame::getLightEstimate(*_frameJNI.get());
    if (arcore::light_estimate::isValid(lightEstimate)) {
        return arcore::light_estimate::getPixelIntensity(lightEstimate);
    }
    else {
        return 1.0;
    }
}

float VROARFrameARCore::getAmbientLightColorTemperature() const {
    return 1.0;
}

