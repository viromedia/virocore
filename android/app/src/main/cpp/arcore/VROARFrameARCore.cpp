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

VROARFrameARCore::VROARFrameARCore(ArFrame *frame,
                                   VROViewport viewport,
                                   std::shared_ptr<VROARSessionARCore> session) :
    _session(session),
    _viewport(viewport) {

    _frame = frame;
    _camera = std::make_shared<VROARCameraARCore>(frame, session);
}

VROARFrameARCore::~VROARFrameARCore() {

}

double VROARFrameARCore::getTimestamp() const {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return 0;
    }
    return (double) arcore::frame::getTimestampNs(_frame, session->getSessionInternal());
}

const std::shared_ptr<VROARCamera> &VROARFrameARCore::getCamera() const {
    return _camera;
}

// TODO: VIRO-1940 filter results based on types. Right now, devs can't set this, so don't use filtering.
std::vector<VROARHitTestResult> VROARFrameARCore::hitTest(int x, int y, std::set<VROARHitTestResultType> types) {
    std::shared_ptr<VROARSessionARCore> session_s = _session.lock();
    if (!session_s) {
        return {};
    }
    ArSession *session = session_s->getSessionInternal();

    ArHitResultList *hitResultList = arcore::hitresultlist::create(session);
    arcore::frame::hitTest(_frame, x, y, session, hitResultList);

    int listSize = arcore::hitresultlist::size(hitResultList, session);
    std::vector<VROARHitTestResult> toReturn;

    for (int i = 0; i < listSize; i++) {
        ArHitResult *hitResult = arcore::hitresult::create(session);
        arcore::hitresultlist::getItem(hitResultList, i, session, hitResult);
        ArTrackable *trackable = arcore::hitresult::acquireTrackable(hitResult, session);

        ArPose *pose = arcore::pose::create(session);
        arcore::hitresult::getPose(hitResult, session, pose);

        // Determine if the Trackable is a Plane
        bool isPlane = arcore::trackable::getType(trackable, session) == arcore::TrackableType::Plane;

        // Create the anchor only if the Trackable is a Plane
        std::shared_ptr<VROARAnchor> vAnchor = nullptr;
        VROARHitTestResultType type;
        if (isPlane) {
            ArPlane *plane = ArAsPlane(trackable);
            bool inExtent = arcore::plane::isPoseInExtents(plane, pose, session);
            bool inPolygon = arcore::plane::isPoseInPolygon(plane, pose, session);

            if (inExtent || inPolygon) {
                type = VROARHitTestResultType::ExistingPlaneUsingExtent;
            } else {
                type = VROARHitTestResultType::EstimatedHorizontalPlane;
            }

            ArAnchor *anchor = arcore::trackable::acquireAnchor(trackable, pose, session);
            vAnchor = session_s->getAnchorForNative(anchor);
            arcore::anchor::detach(anchor, session);
            arcore::anchor::release(anchor);
        } else {
            type = VROARHitTestResultType::FeaturePoint;
        }

        // Get the distance from the camera to the HitResult.
        float distance = arcore::hitresult::getDistance(hitResult, session);

        // Get the transform to the HitResult.
        VROMatrix4f worldTransform = arcore::pose::toMatrix(pose, session);

        // Calculate the local transform, relative to the anchor (if anchor available).
        // TODO: VIRO-1895 confirm this is correct. T(local) = T(world) x T(anchor)^-1
        VROMatrix4f localTransform = VROMatrix4f();
        if (vAnchor) {
            VROMatrix4f inverseAnchorTransform = vAnchor->getTransform().invert();
            localTransform = worldTransform.multiply(inverseAnchorTransform);
        }

        VROARHitTestResult vResult(type, vAnchor, distance, worldTransform, localTransform);
        toReturn.push_back(vResult);

        arcore::hitresult::destroy(hitResult);
        arcore::trackable::release(trackable);
    }

    arcore::hitresultlist::destroy(hitResultList);
    return toReturn;
}

VROMatrix4f VROARFrameARCore::getViewportToCameraImageTransform() {
    pabort("Not supported on ARCore");
}

bool VROARFrameARCore::hasDisplayGeometryChanged() {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return false;
    }
    return arcore::frame::hasDisplayGeometryChanged(_frame, session->getSessionInternal());
}

void VROARFrameARCore::getBackgroundTexcoords(VROVector3f *BL, VROVector3f *BR, VROVector3f *TL, VROVector3f *TR) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }
    std::vector<float> texcoords = arcore::frame::getBackgroundTexcoords(_frame, session->getSessionInternal());
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
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return 1.0;
    }

    float intensity = 0;
    ArLightEstimate *estimate = arcore::light_estimate::create(session->getSessionInternal());

    arcore::frame::getLightEstimate(_frame, session->getSessionInternal(), estimate);
    if (arcore::light_estimate::isValid(estimate, session->getSessionInternal())) {
        intensity = arcore::light_estimate::getPixelIntensity(estimate, session->getSessionInternal());
    }
    else {
        intensity = 1.0;
    }
    arcore::light_estimate::destroy(estimate);

    // multiply by 1000 because pixel intensity ranges from 0 to 1
    return intensity * 1000;
}

float VROARFrameARCore::getAmbientLightColorTemperature() const {
    return 1.0;
}

std::shared_ptr<VROARPointCloud> VROARFrameARCore::getPointCloud() {
    if (_pointCloud) {
        return _pointCloud;
    }
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return _pointCloud;
    }

    JNIEnv* env = VROPlatformGetJNIEnv();

    std::vector<VROVector4f> points;
    std::vector<uint64_t> identifiers; // Android doesn't have any identifiers with their point cloud!

    ArPointCloud *pointCloud = arcore::frame::acquirePointCloud(_frame, session->getSessionInternal());
    if (pointCloud != NULL) {
        const float *pointsArray = arcore::pointcloud::getPoints(pointCloud, session->getSessionInternal());
        int numPoints = arcore::pointcloud::getNumPoints(pointCloud, session->getSessionInternal());

        for (int i = 0; i < numPoints; i++) {
            // Only use points with > 0.0001. Average confidence when measured was around 0.001, so this
            // is lower than that. This is just meant to make the display of the points look good (if low
            // confidence points are used, we may end up with points very close to the camera).
            if (pointsArray[i * 4 + 3] > .0001) {
                VROVector4f point = VROVector4f(pointsArray[i * 4 + 0], pointsArray[i * 4 + 1],
                                                pointsArray[i * 4 + 2], pointsArray[i * 4+ 3]);
                points.push_back(point);
            }
        }
        arcore::pointcloud::release(pointCloud);
    }

    _pointCloud = std::make_shared<VROARPointCloud>(points, identifiers);
    return _pointCloud;
}