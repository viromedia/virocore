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
#include "VROLight.h"
#include "VROARHitTestResultARCore.h"

VROARFrameARCore::VROARFrameARCore(arcore::Frame *frame,
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
    return (double) _frame->getTimestampNs();
}

const std::shared_ptr<VROARCamera> &VROARFrameARCore::getCamera() const {
    return _camera;
}

// TODO: VIRO-1940 filter results based on types. Right now, devs can't set this, so don't use filtering.
std::vector<std::shared_ptr<VROARHitTestResult>> VROARFrameARCore::hitTest(int x, int y, std::set<VROARHitTestResultType> types) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return {};
    }
    arcore::Session *session_arc = session->getSessionInternal();

    arcore::HitResultList *hitResultList = session_arc->createHitResultList();
    _frame->hitTest(x, y, hitResultList);

    int listSize = hitResultList->size();
    std::vector<std::shared_ptr<VROARHitTestResult>> toReturn;

    for (int i = 0; i < listSize; i++) {
        std::shared_ptr<arcore::HitResult> hitResult = std::shared_ptr<arcore::HitResult>(session_arc->createHitResult());
        hitResultList->getItem(i, hitResult.get());

        // Get the trackable associated with this hit result. Not all hit results have an
        // associated trackable. If a hit result does not have a trackable, we can still acquire
        // an anchor for it via hitResult->acquireAnchor(). This will create an anchor at the hit
        // result's pose. However, we don't immediately acquire this anchor because the user may
        // not even use the hit result. Instead we allow the user to manually acquire the anchor via
        // ARHitTestResult.createAnchoredNode().
        arcore::Trackable *trackable = hitResult->acquireTrackable();

        arcore::Pose *pose = session_arc->createPose();
        hitResult->getPose(pose);

        VROARHitTestResultType type;

        if (trackable != nullptr && trackable->getType() == arcore::TrackableType::Plane) {
            arcore::Plane *plane = (arcore::Plane *) trackable;
            bool inExtent  = plane->isPoseInExtents(pose);
            bool inPolygon = plane->isPoseInPolygon(pose);

            if (inExtent || inPolygon) {
                type = VROARHitTestResultType::ExistingPlaneUsingExtent;
            } else {
                type = VROARHitTestResultType::EstimatedHorizontalPlane;
            }
        } else {
            type = VROARHitTestResultType::FeaturePoint;
        }

        // Get the distance from the camera to the HitResult.
        float distance = hitResult->getDistance();

        // Get the transform to the HitResult.
        float worldTransformMtx[16];
        pose->toMatrix(worldTransformMtx);
        VROMatrix4f worldTransform(worldTransformMtx);
        VROMatrix4f localTransform = VROMatrix4f::identity();

        std::shared_ptr<VROARHitTestResult> vResult = std::make_shared<VROARHitTestResultARCore>(type, distance, hitResult,
                                                                                                 worldTransform, localTransform,
                                                                                                 session);
        toReturn.push_back(vResult);
        delete (pose);
        delete (trackable);
    }

    delete (hitResultList);
    return toReturn;
}

std::vector<std::shared_ptr<VROARHitTestResult>> VROARFrameARCore::hitTestRay(VROVector3f *origin, VROVector3f *destination , std::set<VROARHitTestResultType> types) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return {};
    }
    arcore::Session *session_arc = session->getSessionInternal();

    arcore::HitResultList *hitResultList = session_arc->createHitResultList();
    _frame->hitTest(origin->x, origin->y, origin->z, destination->x, destination->y, destination->z, hitResultList);

    int listSize = hitResultList->size();
    std::vector<std::shared_ptr<VROARHitTestResult>> toReturn;

    for (int i = 0; i < listSize; i++) {
        std::shared_ptr<arcore::HitResult> hitResult = std::shared_ptr<arcore::HitResult>(session_arc->createHitResult());
        hitResultList->getItem(i, hitResult.get());

        // Get the trackable associated with this hit result. Not all hit results have an
        // associated trackable. If a hit result does not have a trackable, we can still acquire
        // an anchor for it via hitResult->acquireAnchor(). This will create an anchor at the hit
        // result's pose. However, we don't immediately acquire this anchor because the user may
        // not even use the hit result. Instead we allow the user to manually acquire the anchor via
        // ARHitTestResult.createAnchoredNode().
        arcore::Trackable *trackable = hitResult->acquireTrackable();

        arcore::Pose *pose = session_arc->createPose();
        hitResult->getPose(pose);

        VROARHitTestResultType type;

        if (trackable != nullptr && trackable->getType() == arcore::TrackableType::Plane) {
            arcore::Plane *plane = (arcore::Plane *) trackable;
            bool inExtent  = plane->isPoseInExtents(pose);
            bool inPolygon = plane->isPoseInPolygon(pose);

            if (inExtent || inPolygon) {
                type = VROARHitTestResultType::ExistingPlaneUsingExtent;
            } else {
                type = VROARHitTestResultType::EstimatedHorizontalPlane;
            }
        } else {
            type = VROARHitTestResultType::FeaturePoint;
        }

        // Get the distance from the camera to the HitResult.
        float distance = hitResult->getDistance();

        // Get the transform to the HitResult.
        float worldTransformMtx[16];
        pose->toMatrix(worldTransformMtx);
        VROMatrix4f worldTransform(worldTransformMtx);
        VROMatrix4f localTransform = VROMatrix4f::identity();

        std::shared_ptr<VROARHitTestResult> vResult = std::make_shared<VROARHitTestResultARCore>(type, distance, hitResult,
                                                                                                 worldTransform, localTransform,
                                                                                                 session);
        toReturn.push_back(vResult);
        delete (pose);
        delete (trackable);
    }

    delete (hitResultList);
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
    return _frame->hasDisplayGeometryChanged();
}

void VROARFrameARCore::getBackgroundTexcoords(VROVector3f *BL, VROVector3f *BR, VROVector3f *TL, VROVector3f *TR) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }

    float texcoords[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    _frame->getBackgroundTexcoords(texcoords);
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
    return _anchors; // Always empty; unused in ARCore
}

float VROARFrameARCore::getAmbientLightIntensity() const {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return 1.0;
    }

    float intensity = 0;
    arcore::LightEstimate *estimate = session->getSessionInternal()->createLightEstimate();

    _frame->getLightEstimate(estimate);
    if (estimate->isValid()) {
        intensity = estimate->getPixelIntensity();
    } else {
        intensity = 1.0;
    }
    delete (estimate);

    // Multiply by 1000 to get into lumen range
    return intensity * 1000;
}

VROVector3f VROARFrameARCore::getAmbientLightColor() const {
    VROVector3f color = { 1, 1, 1 };

    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return color;
    }

    arcore::LightEstimate *estimate = session->getSessionInternal()->createLightEstimate();
    _frame->getLightEstimate(estimate);

    float correction[4];
    if (estimate->isValid()) {
        estimate->getColorCorrection(correction);
    }
    delete (estimate);

    VROVector3f gammaColor = { correction[0], correction[1], correction[2] };
    return VROLight::convertGammaToLinear(gammaColor);
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

    arcore::PointCloud *pointCloud = _frame->acquirePointCloud();
    if (pointCloud != NULL) {
        const float *pointsArray = pointCloud->getPoints();
        int numPoints = pointCloud->getNumPoints();

        for (int i = 0; i < numPoints; i++) {
            // Only use points with > 0.1. This is just meant to make the display of the points
            // look good (if low confidence points are used, we may end up with points very close
            // to the camera).
            if (pointsArray[i * 4 + 3] > .1) {
                VROVector4f point = VROVector4f(pointsArray[i * 4 + 0], pointsArray[i * 4 + 1],
                                                pointsArray[i * 4 + 2], pointsArray[i * 4 + 3]);
                points.push_back(point);
            }
        }
        delete (pointCloud);
    }
    _pointCloud = std::make_shared<VROARPointCloud>(points, identifiers);
    return _pointCloud;
}