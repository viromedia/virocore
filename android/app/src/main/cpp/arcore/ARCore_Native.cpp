//
//  ARCore_Native_JNI.cpp
//  Viro
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "ARCore_Native.h"
#include "VROPlatformUtil.h"
#include "VROMatrix4f.h"
#include <stdio.h>
#include <sstream>
#include <string>
#include "VROStringUtil.h"
#include "VROLog.h"

namespace arcore {

    namespace config {

        ArConfig *create(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                         UpdateMode updateMode, const ArSession *session) {

            ArConfig *config;
            ArConfig_create(session, &config);

            // Set light estimation mode
            ArLightEstimationMode arLightingMode;
            switch(lightingMode) {
                case LightingMode::Disabled: {
                    arLightingMode = AR_LIGHT_ESTIMATION_MODE_DISABLED;
                    break;
                }
                case LightingMode::AmbientIntensity: {
                    arLightingMode = AR_LIGHT_ESTIMATION_MODE_AMBIENT_INTENSITY;
                    break;
                }
            }
            ArConfig_setLightEstimationMode(session, config, arLightingMode);

            // Set plane finding mode
            ArPlaneFindingMode arPlaneFindingMode;
            switch(planeFindingMode) {
                case PlaneFindingMode::Disabled: {
                    arPlaneFindingMode = AR_PLANE_FINDING_MODE_DISABLED;
                    break;
                }
                case PlaneFindingMode::Horizontal: {
                    arPlaneFindingMode = AR_PLANE_FINDING_MODE_HORIZONTAL;
                    break;
                }
            }
            ArConfig_setPlaneFindingMode(session, config, arPlaneFindingMode);

            // Set update mode
            ArUpdateMode arUpdateMode;
            switch(updateMode) {
                case UpdateMode::Blocking: {
                    arUpdateMode = AR_UPDATE_MODE_BLOCKING;
                    break;
                }
                case UpdateMode::LatestCameraImage: {
                    arUpdateMode = AR_UPDATE_MODE_LATEST_CAMERA_IMAGE;
                    break;
                }
            }
            ArConfig_setUpdateMode(session, config, arUpdateMode);

            return config;
        }

        void destroy(ArConfig *config) {
            ArConfig_destroy(config);
        }

    }

    namespace pose {

        ArPose *create(const ArSession *session) {
            ArPose *pose;
            ArPose_create(session, nullptr, &pose);
            return pose;
        }

        void destroy(ArPose *pose) {
            ArPose_destroy(pose);
        }

        VROMatrix4f toMatrix(const ArPose *pose, const ArSession *session) {
            float matrix[16];
            ArPose_getMatrix(session, pose, matrix);
            return { matrix };
        }

    }

    namespace anchorlist {

        ArAnchorList *create(const ArSession *session) {
            ArAnchorList *list;
            ArAnchorList_create(session, &list);
            return list;
        }

        void destroy(ArAnchorList *anchorList) {
            ArAnchorList_destroy(anchorList);
        }

        ArAnchor *acquireItem(const ArAnchorList *anchorList, int index, const ArSession *session) {
            ArAnchor *anchor;
            ArAnchorList_acquireItem(session, anchorList, index, &anchor);
            return anchor;
        }

        int size(const ArAnchorList *anchorList, const ArSession *session) {
            int size;
            ArAnchorList_getSize(session, anchorList, &size);
            return size;
        }

    }

    namespace anchor {

        uint64_t getHashCode(const ArAnchor *anchor) {
            return reinterpret_cast<uint64_t>(anchor);
        }

        std::string getId(const ArAnchor *anchor) {
            uint64_t nativeHandle = reinterpret_cast<uint64_t>(anchor);
            std::stringstream ss;
            ss << nativeHandle;
            return ss.str();
        }

        void getPose(const ArAnchor *anchor, const ArSession *session, ArPose *outPose) {
            ArAnchor_getPose(session, anchor, outPose);
        }

        TrackingState getTrackingState(const ArAnchor *anchor, const ArSession *session) {
            ArTrackingState trackingState;
            ArAnchor_getTrackingState(session, anchor, &trackingState);

            if (trackingState == AR_TRACKING_STATE_TRACKING) {
                return TrackingState::Tracking;
            }
            else {
                return TrackingState::NotTracking;
            }
        }

        void detach(ArAnchor *anchor, ArSession *session) {
            ArAnchor_detach(session, anchor);
        }

        void release(ArAnchor *anchor) {
            ArAnchor_release(anchor);
        }

    }

    namespace trackablelist {

        ArTrackableList *create(const ArSession *session) {
            ArTrackableList *list;
            ArTrackableList_create(session, &list);
            return list;
        }

        void destroy(ArTrackableList *trackableList) {
            ArTrackableList_destroy(trackableList);
        }

        ArTrackable *acquireItem(const ArTrackableList *trackableList, int index, const ArSession *session) {
            ArTrackable *trackable;
            ArTrackableList_acquireItem(session, trackableList, index, &trackable);
            return trackable;
        }

        int size(const ArTrackableList *trackableList, const ArSession *session) {
            int size;
            ArTrackableList_getSize(session, trackableList, &size);
            return size;
        }

    }

    namespace trackable {

        ArAnchor *acquireAnchor(ArTrackable *trackable, ArPose *pose, ArSession *session) {
            ArAnchor *anchor;
            ArTrackable_acquireNewAnchor(session, trackable, pose, &anchor);
            return anchor;
        }

        TrackingState getTrackingState(const ArTrackable *trackable, const ArSession *session) {
            ArTrackingState trackingState;
            ArTrackable_getTrackingState(session, trackable, &trackingState);

            if (trackingState == AR_TRACKING_STATE_TRACKING) {
                return TrackingState::Tracking;
            }
            else {
                return TrackingState::NotTracking;
            }
        }

        TrackableType getType(const ArTrackable *trackable, const ArSession *session) {
            ArTrackableType type;
            ArTrackable_getType(session, trackable, &type);

            if (type == AR_TRACKABLE_PLANE) {
                return TrackableType::Plane;
            }
            else {
                return TrackableType::Point;
            }
        }


        void release(ArTrackable *trackable) {
            ArTrackable_release(trackable);
        }

    }

    namespace plane {

        uint64_t getHashCode(const ArPlane *plane) {
            return reinterpret_cast<uint64_t>(plane);
        }

        void getCenterPose(const ArPlane *plane, const ArSession *session, ArPose *outPose) {
            return ArPlane_getCenterPose(session, plane, outPose);
        }

        float getExtentX(const ArPlane *plane, const ArSession *session) {
            float extent;
            ArPlane_getExtentX(session, plane, &extent);
            return extent;
        }

        float getExtentZ(const ArPlane *plane, const ArSession *session) {
            float extent;
            ArPlane_getExtentZ(session, plane, &extent);
            return extent;
        }

        ArPlane *acquireSubsumedBy(const ArPlane *plane, const ArSession *session) {
            ArPlane *subsumedBy;
            ArPlane_acquireSubsumedBy(session, plane, &subsumedBy);
            return subsumedBy;
        }

        PlaneType getType(const ArPlane *plane, const ArSession *session) {
            ArPlaneType type;
            ArPlane_getType(session, plane, &type);

            if (type == AR_PLANE_HORIZONTAL_DOWNWARD_FACING) {
                return PlaneType::HorizontalDownward;
            }
            else if (type == AR_PLANE_HORIZONTAL_UPWARD_FACING) {
                return PlaneType::HorizontalUpward;
            }
            else {
                return PlaneType::NonHorizontal;
            }
        }

        bool isPoseInExtents(const ArPlane *plane, const ArPose *pose, const ArSession *session) {
            int result;
            ArPlane_isPoseInExtents(session, plane, pose, &result);
            return (bool) result;
        }

        bool isPoseInPolygon(const ArPlane *plane, const ArPose *pose, const ArSession *session) {
            int result;
            ArPlane_isPoseInPolygon(session, plane, pose, &result);
            return (bool) result;
        }
    }

    namespace light_estimate {

        ArLightEstimate *create(const ArSession *session) {
            ArLightEstimate *estimate;
            ArLightEstimate_create(session,  &estimate);
            return estimate;
        }

        void destroy(ArLightEstimate *lightEstimate) {
            ArLightEstimate_destroy(lightEstimate);
        }

        float getPixelIntensity(const ArLightEstimate *lightEstimate, const ArSession *session) {
            float intensity;
            ArLightEstimate_getPixelIntensity(session, lightEstimate, &intensity);
            return intensity;
        }

        bool isValid(const ArLightEstimate *lightEstimate, const ArSession *session) {
            ArLightEstimateState state;
            ArLightEstimate_getState(session, lightEstimate, &state);
            if (state == AR_LIGHT_ESTIMATE_STATE_NOT_VALID) {
                return false;
            }
            else {
                return true;
            }
        }

    }

    namespace frame {

        ArFrame *create(const ArSession *session) {
            ArFrame *frame;
            ArFrame_create(session, &frame);
            return frame;
        }

        void destroy(ArFrame *frame) {
            ArFrame_destroy(frame);
        }

        VROMatrix4f getViewMatrix(const ArFrame *frame, const ArSession *session) {
            ArCamera *camera;
            ArFrame_acquireCamera(session, frame, &camera);
            float matrix[16];
            ArCamera_getViewMatrix(session, camera, matrix);
            ArCamera_release(camera);

            return { matrix };
        }

        VROMatrix4f getProjectionMatrix(const ArFrame *frame, float near, float far, const ArSession *session) {
            ArCamera *camera;
            ArFrame_acquireCamera(session, frame, &camera);
            float matrix[16];
            ArCamera_getProjectionMatrix(session, camera, near, far, matrix);
            ArCamera_release(camera);

            return { matrix };
        }

        TrackingState getTrackingState(const ArFrame *frame, const ArSession *session) {
            ArCamera *camera;
            ArFrame_acquireCamera(session, frame, &camera);

            ArTrackingState trackingState;
            ArCamera_getTrackingState(session, camera, &trackingState);
            ArCamera_release(camera);

            if (trackingState == AR_TRACKING_STATE_PAUSED || trackingState == AR_TRACKING_STATE_STOPPED) {
                return TrackingState::NotTracking;
            }
            else {
                return TrackingState::Tracking;
            }
        }

        void getLightEstimate(const ArFrame *frame, const ArSession *session, ArLightEstimate *outLightEstimate) {
            ArFrame_getLightEstimate(session, frame, outLightEstimate);
        }

        int64_t getTimestampNs(const ArFrame *frame, const ArSession *session) {
            int64_t timestamp;
            ArFrame_getTimestamp(session, frame, &timestamp);
            return timestamp;
        }

        void getUpdatedAnchors(const ArFrame *frame, const ArSession *session, ArAnchorList *outList) {
            ArFrame_getUpdatedAnchors(session, frame, outList);
        }

        void getUpdatedPlanes(const ArFrame *frame, const ArSession *session, ArTrackableList *outList) {
            ArFrame_getUpdatedTrackables(session, frame, AR_TRACKABLE_PLANE, outList);
        }

        bool hasDisplayGeometryChanged(const ArFrame *frame, const ArSession *session) {
            int changed;
            ArFrame_getDisplayGeometryChanged(session, frame, &changed);
            return (bool) changed;
        }

        void hitTest(const ArFrame *frame, float x, float y, const ArSession *session, ArHitResultList *outList) {
            ArFrame_hitTest(session, frame, x, y, outList);
        }

        std::vector<float> getBackgroundTexcoords(const ArFrame *frame, const ArSession *session) {
            // BL, TL, BR, TR
            const float source[8] = { 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0 };
            float dest[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            ArFrame_transformDisplayUvCoords(session, frame, 8, source, dest);

            std::vector<float> result;
            result.assign(dest, dest + 8);
            return result;
        }

        ArPointCloud *acquirePointCloud(const ArFrame *frame, const ArSession *session) {
            ArPointCloud *cloud;
            ArFrame_acquirePointCloud(session, frame, &cloud);
            return cloud;
        }
    }

    namespace pointcloud {

        const float *getPoints(const ArPointCloud *cloud, const ArSession *session) {
            const float *points;
            ArPointCloud_getData(session, cloud, &points);
            return points;
        }

        int getNumPoints(const ArPointCloud *cloud, const ArSession *session) {
            int numPoints;
            ArPointCloud_getNumberOfPoints(session, cloud, &numPoints);
            return numPoints;
        }

        void release(ArPointCloud *cloud) {
            ArPointCloud_release(cloud);
        }

    }

    namespace hitresultlist {

        ArHitResultList *create(const ArSession *session) {
            ArHitResultList *list;
            ArHitResultList_create(session, &list);
            return list;
        }

        void destroy(ArHitResultList *hitResultList) {
            ArHitResultList_destroy(hitResultList);
        }

        void getItem(const ArHitResultList *hitResultList, int index, const ArSession *session, ArHitResult *outResult) {
            ArHitResultList_getItem(session, hitResultList, index, outResult);
        }

        int size(const ArHitResultList *hitResultList, const ArSession *session) {
            int size;
            ArHitResultList_getSize(session, hitResultList, &size);
            return size;
        }

    }

    namespace hitresult {

        ArHitResult *create(const ArSession *session) {
            ArHitResult *result;
            ArHitResult_create(session, &result);
            return result;
        }

        void destroy(ArHitResult *hitResult) {
            ArHitResult_destroy(hitResult);
        }

        float getDistance(const ArHitResult *hitResult, const ArSession *session) {
            float distance;
            ArHitResult_getDistance(session, hitResult, &distance);
            return distance;
        }

        void getPose(const ArHitResult *hitResult, const ArSession *session, ArPose *outPose) {
            ArHitResult_getHitPose(session, hitResult, outPose);
        }

        ArTrackable *acquireTrackable(const ArHitResult *hitResult, const ArSession *session) {
            ArTrackable *trackable;
            ArHitResult_acquireTrackable(session, hitResult, &trackable);
            return trackable;
        }

        ArAnchor *acquireAnchor(ArHitResult *hitResult, ArSession *session) {
            ArAnchor *anchor;
            ArHitResult_acquireNewAnchor(session, hitResult, &anchor);
            return anchor;
        }

    }

    namespace session {

        ArSession *create(void *applicationContext) {
            JNIEnv *env = VROPlatformGetJNIEnv();
            ArSession *session;
            ArSession_create(env, applicationContext, &session);
            return session;
        }

        void destroy(ArSession *session) {
            ArSession_destroy(session);
        }

        ArStatus configure(ArSession *session, const ArConfig *config) {
            return ArSession_configure(session, config);
        }

        bool checkSupported(ArSession *session, const ArConfig *config) {
            return ArSession_checkSupported(session, config) == AR_SUCCESS;
        }

        void setDisplayGeometry(ArSession *session, int rotation, int width, int height) {
            ArSession_setDisplayGeometry(session, rotation, width, height);
        }

        void setCameraTextureName(ArSession *session, int32_t textureId) {
            ArSession_setCameraTextureName(session, textureId);
        }

        void pause(ArSession *session) {
            ArSession_pause(session);
        }

        void resume(ArSession *session) {
            ArSession_resume(session);
        }

        void update(ArSession *session, ArFrame *frame) {
            ArSession_update(session, frame);
        }

    }
}