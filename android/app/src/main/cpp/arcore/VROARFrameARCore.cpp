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

// ARCore color corrections are generally returned in the range 0.5 to 1.5; since they
// go above 1.0, they are not compatible with with hexadecimal color representations
// used in React. To compensate for this, rebalance the color corrections by multiplying
// by this factor. Since doing this will reduce the intensity of the light, we also have
// to then multiply the estimated intensity by the *inverse* of this factor
static const float kLightEstimateIntensityRebalance = 0.5f;

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
std::vector<VROARHitTestResult> VROARFrameARCore::hitTest(int x, int y, std::set<VROARHitTestResultType> types) {
    std::shared_ptr<VROARSessionARCore> session_s = _session.lock();
    if (!session_s) {
        return {};
    }
    arcore::Session *session = session_s->getSessionInternal();

    arcore::HitResultList *hitResultList = session->createHitResultList();
    _frame->hitTest(x, y, hitResultList);

    int listSize = hitResultList->size();
    std::vector<VROARHitTestResult> toReturn;

    for (int i = 0; i < listSize; i++) {
        arcore::HitResult *hitResult = session->createHitResult();
        hitResultList->getItem(i, hitResult);

        arcore::Trackable *trackable = hitResult->acquireTrackable();
        if (trackable == nullptr) {
            delete (hitResult);
            continue;
        }

        arcore::Pose *pose = session->createPose();
        hitResult->getPose(pose);

        // Determine if the Trackable is a Plane
        bool isPlane = trackable->getType() == arcore::TrackableType::Plane;

        // Create the anchor only if the Trackable is a Plane
        std::shared_ptr<VROARAnchor> vAnchor = nullptr;
        VROARHitTestResultType type;
        if (isPlane) {
            arcore::Plane *plane = (arcore::Plane *) trackable;
            bool inExtent = plane->isPoseInExtents(pose);
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

        // Calculate the local transform, relative to the anchor (if anchor available).
        // TODO: VIRO-1895 confirm this is correct. T(local) = T(world) x T(anchor)^-1
        VROMatrix4f localTransform = VROMatrix4f();
        if (vAnchor) {
            VROMatrix4f inverseAnchorTransform = vAnchor->getTransform().invert();
            localTransform = worldTransform.multiply(inverseAnchorTransform);
        }

        VROARHitTestResult vResult(type, vAnchor, distance, worldTransform, localTransform);
        toReturn.push_back(vResult);

        delete (hitResult);
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
    return _anchors;
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
    // Multiply by 1000 because pixel intensity ranges from 0 to 1. Multiply by the inverse of
    // the rebalancing factor to compensate for the brightness reduction caused by rebalancing
    // color correction
    return intensity * 1000 * 1.0 / kLightEstimateIntensityRebalance;
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

    // ARCore returns light values in gamma space in the range 0.5 to 1.5. First convert
    // to linear color, then rebalance so the values do not breach 1.0. The brightness is
    // diminished but this is compensated by multiplying estimated intensity by the inverse
    // of the rebalance constant.
    VROVector3f gammaColor = { correction[0], correction[1], correction[2] };
    VROVector3f linearColor = VROLight::convertGammaToLinear(gammaColor);
    return linearColor.scale(kLightEstimateIntensityRebalance);
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