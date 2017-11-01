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
#include "VROARHitTestResult.h"
#include "VROPlatformUtil.h"
#include "VROVector4f.h"

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

// TODO: VIRO-1940 filter results based on types. Right now, devs can't set this, so no use filtering.
std::vector<VROARHitTestResult> VROARFrameARCore::hitTest(int x, int y, std::set<VROARHitTestResultType> types) {

    jni::Object<arcore::List> hitResultsJni = arcore::frame::hitTest(*_frameJNI.get(), x, y);

    int listSize = arcore::list::size(hitResultsJni);
    std::vector<VROARHitTestResult> toReturn;

    jni::JNIEnv &env = *VROPlatformGetJNIEnv();
    static auto PlaneHitResultClass = *jni::Class<arcore::PlaneHitResult>::Find(env).NewGlobalRef(env).release();

    for (int i = 0; i < listSize; i++) {
        jni::Object<arcore::HitResult> hitResult =
                (jni::Object<arcore::HitResult>) arcore::list::get(hitResultsJni, i);

        // Determine if the hitResult is a PlaneHitResult.
        bool isPlane = hitResult.IsInstanceOf(env, PlaneHitResultClass);


        // Get the anchor only if the result is of type PlaneHitResult
        std::shared_ptr<VROARSessionARCore> session = _session.lock();
        std::shared_ptr<VROARAnchor> vAnchor = nullptr;
        VROARHitTestResultType type;
        if (session && isPlane) {
            bool inExtent = arcore::planehitresult::isHitInExtents((jni::Object<arcore::PlaneHitResult>) hitResult);
            bool inPolygon = arcore::planehitresult::isHitInPolygon((jni::Object<arcore::PlaneHitResult>) hitResult);

            if (inExtent || inPolygon) {
                type = VROARHitTestResultType::ExistingPlaneUsingExtent;
            } else {
                type = VROARHitTestResultType::EstimatedHorizontalPlane;
            }

            jni::Object<arcore::Plane> plane =
                    arcore::planehitresult::getPlane((jni::Object<arcore::PlaneHitResult>) hitResult);

            vAnchor = session->getAnchorForNative(plane);
        } else {
            type = VROARHitTestResultType::FeaturePoint;
        }

        // Get the distance from the camera to the HitResult.
        float distance = arcore::hitresult::getDistance(hitResult);

        // Get the transform to the HitResult.
        VROMatrix4f worldTransform = arcore::hitresult::getPose(hitResult);

        // Calculate the local transform, relative to the anchor (if anchor available).
        // TODO: VIRO-1895 confirm this is correct. T(local) = T(world) x T(anchor)^-1
        VROMatrix4f localTransform = VROMatrix4f();
        if (vAnchor) {
            VROMatrix4f inverseAnchorTransform = vAnchor->getTransform().invert();
            localTransform = worldTransform.multiply(inverseAnchorTransform);
        }

        VROARHitTestResult vResult(type, vAnchor, distance, worldTransform, localTransform);
        toReturn.push_back(vResult);
    }
    
    return toReturn;
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

std::shared_ptr<VROARPointCloud> VROARFrameARCore::getPointCloud() {
    if (_pointCloud) {
        return _pointCloud;
    }

    JNIEnv* env = VROPlatformGetJNIEnv();

    std::vector<VROVector4f> points;
    std::vector<uint64_t> identifiers; // Android doesn't have any identifiers with their point cloud!

    jni::Object<arcore::PointCloud> pointCloud = arcore::frame::getPointCloud(*_frameJNI.get());
    if (pointCloud != NULL) {
        VROMatrix4f pointCloudTransform = arcore::frame::getPointCloudPose(*_frameJNI.get());

        std::vector<float> pointsVector = arcore::floatbuffer::toVector(arcore::pointcloud::getPoints(pointCloud));
        for (int i = 0; i < pointsVector.size(); i += 4) {
            // Only use points with > .1 confidence (scale of 0 -> 1)
            if (pointsVector[i + 3] > .1) {
                // The 4th value is a confidence value that we don't want to transform,
                // so transform the vector and then set the 4th value!
                VROVector4f point = VROVector4f(pointsVector[i], pointsVector[i + 1],
                                                pointsVector[i + 2], 1);

                point = pointCloudTransform.multiply(point);
                point.w = pointsVector[i + 3];
                points.push_back(point);
            }
        }
    }

    _pointCloud = std::make_shared<VROARPointCloud>(points, identifiers);
    return _pointCloud;
}