//
//  ARCore_Native_JNI.cpp
//  Viro
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "ARCore_Native.h"
#include "VROMatrix4f.h"

namespace arcore {

#pragma mark - Config

    ConfigNative::~ConfigNative() {
        ArConfig_destroy(_config);
    }

#pragma mark - Pose

    PoseNative::~PoseNative() {
        ArPose_destroy(_pose);
    }

    void PoseNative::toMatrix(float *outMatrix) {
        ArPose_getMatrix(_session, _pose, outMatrix);
    }

#pragma mark - AnchorList

    AnchorListNative::~AnchorListNative() {
        ArAnchorList_destroy(_anchorList);
    }

    Anchor *AnchorListNative::acquireItem(int index) {
        ArAnchor *anchor;
        ArAnchorList_acquireItem(_session, _anchorList, index, &anchor);
        return new AnchorNative(anchor, _session);
    }

    int AnchorListNative::size() {
        int size;
        ArAnchorList_getSize(_session, _anchorList, &size);
        return size;
    }

#pragma mark - Anchor

    AnchorNative::~AnchorNative() {
        ArAnchor_release(_anchor);
    }

    uint64_t AnchorNative::getHashCode() {
        return reinterpret_cast<uint64_t>(_anchor);
    }

    uint64_t AnchorNative::getId() {
        return reinterpret_cast<uint64_t>(_anchor);
    }

    void AnchorNative::getPose(Pose *outPose) {
        ArAnchor_getPose(_session, _anchor, ((PoseNative *)outPose)->_pose);
    }

    TrackingState AnchorNative::getTrackingState() {
        ArTrackingState trackingState;
        ArAnchor_getTrackingState(_session, _anchor, &trackingState);

        if (trackingState == AR_TRACKING_STATE_TRACKING) {
            return TrackingState::Tracking;
        } else {
            return TrackingState::NotTracking;
        }
    }

    void AnchorNative::detach() {
        ArAnchor_detach(_session, _anchor);
    }

#pragma mark - TrackableList

    TrackableListNative::~TrackableListNative() {
        ArTrackableList_destroy(_trackableList);
    }

    Trackable *TrackableListNative::acquireItem(int index) {
        ArTrackable *trackable;
        ArTrackableList_acquireItem(_session, _trackableList, index, &trackable);

        ArTrackableType type;
        ArTrackable_getType(_session, trackable, &type);
        if (type == AR_TRACKABLE_PLANE) {
            return new PlaneNative(ArAsPlane(trackable), _session);
        }
        else {
            return nullptr;
        }
    }

    int TrackableListNative::size() {
        int size;
        ArTrackableList_getSize(_session, _trackableList, &size);
        return size;
    }

#pragma mark - Plane

    PlaneNative::PlaneNative(ArPlane *plane, ArSession *session) :
        _trackable(ArAsTrackable(plane)), _plane(plane), _session(session) {

    }

    PlaneNative::~PlaneNative() {
        ArTrackable_release(_trackable);
    }

    Anchor *PlaneNative::acquireAnchor(Pose *pose) {
        ArAnchor *anchor;
        ArTrackable_acquireNewAnchor(_session, _trackable, ((PoseNative *)pose)->_pose, &anchor);
        return new AnchorNative(anchor, _session);
    }

    TrackingState PlaneNative::getTrackingState() {
        ArTrackingState trackingState;
        ArTrackable_getTrackingState(_session, _trackable, &trackingState);

        if (trackingState == AR_TRACKING_STATE_TRACKING) {
            return TrackingState::Tracking;
        } else {
            return TrackingState::NotTracking;
        }
    }

    TrackableType PlaneNative::getType() {
        ArTrackableType type;
        ArTrackable_getType(_session, _trackable, &type);

        if (type == AR_TRACKABLE_PLANE) {
            return TrackableType::Plane;
        } else {
            return TrackableType::Point;
        }
    }

    uint64_t PlaneNative::getHashCode() {
        return reinterpret_cast<uint64_t>(_plane);
    }

    void PlaneNative::getCenterPose(Pose *outPose) {
        return ArPlane_getCenterPose(_session, _plane, ((PoseNative *) outPose)->_pose);
    }

    float PlaneNative::getExtentX() {
        float extent;
        ArPlane_getExtentX(_session, _plane, &extent);
        return extent;
    }

    float PlaneNative::getExtentZ() {
        float extent;
        ArPlane_getExtentZ(_session, _plane, &extent);
        return extent;
    }

    Plane *PlaneNative::acquireSubsumedBy() {
        ArPlane *subsumedBy;
        ArPlane_acquireSubsumedBy(_session, _plane, &subsumedBy);
        return subsumedBy != NULL ? new PlaneNative(subsumedBy, _session) : NULL;
    }

    PlaneType PlaneNative::getPlaneType() {
        ArPlaneType type;
        ArPlane_getType(_session, _plane, &type);

        if (type == AR_PLANE_HORIZONTAL_DOWNWARD_FACING) {
            return PlaneType::HorizontalDownward;
        } else if (type == AR_PLANE_HORIZONTAL_UPWARD_FACING) {
            return PlaneType::HorizontalUpward;
        } else {
            return PlaneType::NonHorizontal;
        }
    }

    bool PlaneNative::isPoseInExtents(const Pose *pose) {
        int result;
        ArPlane_isPoseInExtents(_session, _plane, ((PoseNative *)pose)->_pose, &result);
        return (bool) result;
    }

    bool PlaneNative::isPoseInPolygon(const Pose *pose) {
        int result;
        ArPlane_isPoseInPolygon(_session, _plane, ((PoseNative *)pose)->_pose, &result);
        return (bool) result;
    }

#pragma mark - LightEstimate

    LightEstimateNative::~LightEstimateNative() {
            ArLightEstimate_destroy(_lightEstimate);
     }

    float LightEstimateNative::getPixelIntensity() {
        float intensity;
        ArLightEstimate_getPixelIntensity(_session, _lightEstimate, &intensity);
        return intensity;
    }

    bool LightEstimateNative::isValid() {
        ArLightEstimateState state;
        ArLightEstimate_getState(_session, _lightEstimate, &state);
        if (state == AR_LIGHT_ESTIMATE_STATE_NOT_VALID) {
            return false;
        } else {
            return true;
        }
    }

#pragma mark - Frame

    FrameNative::~FrameNative() {
        ArFrame_destroy(_frame);
    }

    void FrameNative::getViewMatrix(float *outMatrix) {
        ArCamera *camera;
        ArFrame_acquireCamera(_session, _frame, &camera);
        float matrix[16];
        ArCamera_getViewMatrix(_session, camera, outMatrix);
        ArCamera_release(camera);
    }

    void FrameNative::getProjectionMatrix(float near, float far, float *outMatrix) {
        ArCamera *camera;
        ArFrame_acquireCamera(_session, _frame, &camera);
        float matrix[16];
        ArCamera_getProjectionMatrix(_session, camera, near, far, outMatrix);
        ArCamera_release(camera);
    }

    TrackingState FrameNative::getTrackingState() {
        ArCamera *camera;
        ArFrame_acquireCamera(_session, _frame, &camera);

        ArTrackingState trackingState;
        ArCamera_getTrackingState(_session, camera, &trackingState);
        ArCamera_release(camera);

        if (trackingState == AR_TRACKING_STATE_PAUSED ||
            trackingState == AR_TRACKING_STATE_STOPPED) {
            return TrackingState::NotTracking;
        } else {
            return TrackingState::Tracking;
        }
    }

    void FrameNative::getLightEstimate(LightEstimate *outLightEstimate) {
        ArFrame_getLightEstimate(_session, _frame,
                                 ((LightEstimateNative *) outLightEstimate)->_lightEstimate);
    }

    int64_t FrameNative::getTimestampNs() {
        int64_t timestamp;
        ArFrame_getTimestamp(_session, _frame, &timestamp);
        return timestamp;
    }

    void FrameNative::getUpdatedAnchors(AnchorList *outList) {
        ArFrame_getUpdatedAnchors(_session, _frame, ((AnchorListNative *) outList)->_anchorList);
    }

    void FrameNative::getUpdatedPlanes(TrackableList *outList) {
        ArFrame_getUpdatedTrackables(_session, _frame, AR_TRACKABLE_PLANE,
                                     ((TrackableListNative *) outList)->_trackableList);
    }

    bool FrameNative::hasDisplayGeometryChanged() {
        int changed;
        ArFrame_getDisplayGeometryChanged(_session, _frame, &changed);
        return (bool) changed;
    }

    void FrameNative::hitTest(float x, float y, HitResultList *outList) {
        ArFrame_hitTest(_session, _frame, x, y, ((HitResultListNative *) outList)->_hitResultList);
    }

    void FrameNative::getBackgroundTexcoords(float *outTexcoords) {
        // BL, TL, BR, TR
        const float source[8] = {0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0};
        float dest[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        ArFrame_transformDisplayUvCoords(_session, _frame, 8, source, outTexcoords);
    }

    PointCloud *FrameNative::acquirePointCloud() {
        ArPointCloud *cloud;
        ArFrame_acquirePointCloud(_session, _frame, &cloud);
        return new PointCloudNative(cloud, _session);
    }

#pragma mark - PointCloud

    PointCloudNative::~PointCloudNative() {
        ArPointCloud_release(_pointCloud);
    }

    const float *PointCloudNative::getPoints() {
        const float *points;
        ArPointCloud_getData(_session, _pointCloud, &points);
        return points;
    }

    int PointCloudNative::getNumPoints() {
        int numPoints;
        ArPointCloud_getNumberOfPoints(_session, _pointCloud, &numPoints);
        return numPoints;
    }

#pragma mark - HitResultList

    HitResultListNative::~HitResultListNative() {
        ArHitResultList_destroy(_hitResultList);
    }

    void HitResultListNative::getItem(int index, HitResult *outResult) {
        ArHitResultList_getItem(_session, _hitResultList, index,
                                ((HitResultNative *) outResult)->_hitResult);
    }

    int HitResultListNative::size() {
        int size;
        ArHitResultList_getSize(_session, _hitResultList, &size);
        return size;
    }

#pragma mark - HitResult

    HitResultNative::~HitResultNative() {
        ArHitResult_destroy(_hitResult);
    }

    float HitResultNative::getDistance() {
        float distance;
        ArHitResult_getDistance(_session, _hitResult, &distance);
        return distance;
    }

    void HitResultNative::getPose(Pose *outPose) {
        ArHitResult_getHitPose(_session, _hitResult, ((PoseNative *) outPose)->_pose);
    }

    Trackable *HitResultNative::acquireTrackable() {
        ArTrackable *trackable;
        ArHitResult_acquireTrackable(_session, _hitResult, &trackable);

        ArTrackableType type;
        ArTrackable_getType(_session, trackable, &type);
        if (type == AR_TRACKABLE_PLANE) {
            return new PlaneNative(ArAsPlane(trackable), _session);
        }
        else {
            return nullptr;
        }
    }

    Anchor *HitResultNative::acquireAnchor() {
        ArAnchor *anchor;
        ArHitResult_acquireNewAnchor(_session, _hitResult, &anchor);
        return new AnchorNative(anchor, _session);
    }

#pragma mark - Session

    SessionNative::SessionNative(void *applicationContext, JNIEnv *env) {
            ArSession_create(env, applicationContext, &_session);
    }

    SessionNative::~SessionNative() {
        ArSession_destroy(_session);
    }

    ConfigStatus SessionNative::configure(Config *config) {
        ArStatus status = ArSession_configure(_session, ((ConfigNative *) config)->_config);
        if (status == AR_SUCCESS) {
            return ConfigStatus::Success;
        } else if (status == AR_ERROR_UNSUPPORTED_CONFIGURATION) {
            return ConfigStatus::UnsupportedConfiguration;
        } else if (status == AR_ERROR_SESSION_NOT_PAUSED) {
            return ConfigStatus::SessionNotPaused;
        }
        return ConfigStatus::UnsupportedConfiguration;
    }

    bool SessionNative::checkSupported(Config *config) {
        return ArSession_checkSupported(_session, ((ConfigNative *)config)->_config) == AR_SUCCESS;
    }

    void
    SessionNative::setDisplayGeometry(int rotation, int width, int height) {
        ArSession_setDisplayGeometry(_session, rotation, width, height);
    }

    void SessionNative::setCameraTextureName(int32_t textureId) {
        ArSession_setCameraTextureName(_session, textureId);
    }

    void SessionNative::pause() {
        ArSession_pause(_session);
    }

    void SessionNative::resume() {
        ArSession_resume(_session);
    }

    void SessionNative::update(Frame *frame) {
        ArSession_update(_session, ((FrameNative *)frame)->_frame);
    }

    Config *SessionNative::createConfig(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                                        UpdateMode updateMode) {
        ArConfig *config;
        ArConfig_create(_session, &config);

        // Set light estimation mode
        ArLightEstimationMode arLightingMode;
        switch (lightingMode) {
            case LightingMode::Disabled: {
                arLightingMode = AR_LIGHT_ESTIMATION_MODE_DISABLED;
                break;
            }
            case LightingMode::AmbientIntensity: {
                arLightingMode = AR_LIGHT_ESTIMATION_MODE_AMBIENT_INTENSITY;
                break;
            }
        }
        ArConfig_setLightEstimationMode(_session, config, arLightingMode);

        // Set plane finding mode
        ArPlaneFindingMode arPlaneFindingMode;
        switch (planeFindingMode) {
            case PlaneFindingMode::Disabled: {
                arPlaneFindingMode = AR_PLANE_FINDING_MODE_DISABLED;
                break;
            }
            case PlaneFindingMode::Horizontal: {
                arPlaneFindingMode = AR_PLANE_FINDING_MODE_HORIZONTAL;
                break;
            }
        }
        ArConfig_setPlaneFindingMode(_session, config, arPlaneFindingMode);

        // Set update mode
        ArUpdateMode arUpdateMode;
        switch (updateMode) {
            case UpdateMode::Blocking: {
                arUpdateMode = AR_UPDATE_MODE_BLOCKING;
                break;
            }
            case UpdateMode::LatestCameraImage: {
                arUpdateMode = AR_UPDATE_MODE_LATEST_CAMERA_IMAGE;
                break;
            }
        }
        ArConfig_setUpdateMode(_session, config, arUpdateMode);
        return new ConfigNative(config);
    }

    Pose *SessionNative::createPose() {
        ArPose *pose;
        ArPose_create(_session, nullptr, &pose);
        return new PoseNative(pose, _session);
    }

    AnchorList *SessionNative::createAnchorList() {
        ArAnchorList *list;
        ArAnchorList_create(_session, &list);
        return new AnchorListNative(list, _session);
    }

    TrackableList *SessionNative::createTrackableList() {
        ArTrackableList *list;
        ArTrackableList_create(_session, &list);
        return new TrackableListNative(list, _session);
    }

    HitResultList *SessionNative::createHitResultList() {
        ArHitResultList *list;
        ArHitResultList_create(_session, &list);
        return new HitResultListNative(list, _session);
    }

    LightEstimate *SessionNative::createLightEstimate() {
        ArLightEstimate *estimate;
        ArLightEstimate_create(_session, &estimate);
        return new LightEstimateNative(estimate, _session);
    }

    HitResult *SessionNative::createHitResult() {
        ArHitResult *result;
        ArHitResult_create(_session, &result);
        return new HitResultNative(result, _session);
    }

    Frame *SessionNative::createFrame() {
        ArFrame *frame;
        ArFrame_create(_session, &frame);
        return new FrameNative(frame, _session);
    }

}