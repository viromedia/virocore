//
//  ARCore_Native.h
//  Viro
//
//  Created by Raj Advani on 2/20/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ARCORE_NATIVE_h
#define ARCORE_NATIVE_h

#include <stdint.h>
#include <jni.h>
#include "arcore_c_api.h"

/*
struct ArSession;
struct ArConfig;
struct ArPose;
struct ArAnchorList;
struct ArAnchor;
struct ArTrackableList;
struct ArTrackable;
struct ArPlane;
struct ArLightEstimate;
struct ArFrame;
struct ArPointCloud;
struct ArHitResult;
struct ArHitResultList;
 */

namespace arcore {

    enum class ConfigStatus {
        Success,
        UnsupportedConfiguration,
        SessionNotPaused,
    };

    enum class TrackingState {
        NotTracking,
        Tracking
    };

    enum class TrackableType {
        Plane,
        Point
    };

    enum class PlaneType {
        NonHorizontal,
        HorizontalUpward,
        HorizontalDownward,
    };

    namespace config {

        enum class LightingMode {
            Disabled,
            AmbientIntensity
        };
        enum class PlaneFindingMode {
            Disabled,
            Horizontal
        };
        enum class UpdateMode {
            Blocking,
            LatestCameraImage
        };

        ArConfig *create(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                         UpdateMode updateMode, const ArSession *session);
        void destroy(ArConfig *config);

    }

    namespace pose {

        ArPose *create(const ArSession *session);
        void destroy(ArPose *pose);
        void toMatrix(const ArPose *pose, const ArSession *session, float *outMatrix);

    }

    namespace anchorlist {

        ArAnchorList *create(const ArSession *session);
        void destroy(ArAnchorList *anchorList);
        ArAnchor *acquireItem(const ArAnchorList *anchorList, int index, const ArSession *session);
        int size(const ArAnchorList *anchorList, const ArSession *session);

    }

    namespace anchor {

        uint64_t getHashCode(const ArAnchor *anchor);
        uint64_t getId(const ArAnchor *anchor);
        void getPose(const ArAnchor *anchor, const ArSession *session, ArPose *outPose);
        TrackingState getTrackingState(const ArAnchor *anchor, const ArSession *session);
        void detach(ArAnchor *anchor, ArSession *session);
        void release(ArAnchor *anchor);

    }

    namespace trackablelist {

        ArTrackableList *create(const ArSession *session);
        void destroy(ArTrackableList *trackableList);
        ArTrackable *acquireItem(const ArTrackableList *trackableList, int index, const ArSession *session);
        int size(const ArTrackableList *trackableList, const ArSession *session);

    }

    namespace trackable {

        ArAnchor *acquireAnchor(ArTrackable *trackable, ArPose *pose, ArSession *session);
        TrackingState getTrackingState(const ArTrackable *trackable, const ArSession *session);
        void release(ArTrackable *trackable);
        TrackableType getType(const ArTrackable *trackable, const ArSession *session);
        ArPlane *asPlane(ArTrackable *trackable);

    }

    namespace plane {

        uint64_t getHashCode(const ArPlane *plane);
        void getCenterPose(const ArPlane *plane, const ArSession *session, ArPose *outPose);
        float getExtentX(const ArPlane *plane, const ArSession *session);
        float getExtentZ(const ArPlane *plane, const ArSession *session);
        ArPlane *acquireSubsumedBy(const ArPlane *plane, const ArSession *session);
        PlaneType getType(const ArPlane *plane, const ArSession *session);
        bool isPoseInExtents(const ArPlane *plane, const ArPose *pose, const ArSession *session);
        bool isPoseInPolygon(const ArPlane *plane, const ArPose *pose, const ArSession *session);
        ArTrackable *asTrackable(ArPlane *plane);
    }

    namespace light_estimate {

        ArLightEstimate *create(const ArSession *session);
        void destroy(ArLightEstimate *lightEstimate);
        float getPixelIntensity(const ArLightEstimate *lightEstimate, const ArSession *session);
        bool isValid(const ArLightEstimate *lightEstimate, const ArSession *session);

    }

    namespace frame {

        ArFrame *create(const ArSession *session);
        void destroy(ArFrame *frame);
        void getViewMatrix(const ArFrame *frame, const ArSession *session, float *outMatrix);
        void getProjectionMatrix(const ArFrame *frame, float near, float far, const ArSession *session, float *outMatrix);
        TrackingState getTrackingState(const ArFrame *frame, const ArSession *session);
        void getLightEstimate(const ArFrame *frame, const ArSession *session, ArLightEstimate *outLightEstimate);
        bool hasDisplayGeometryChanged(const ArFrame *frame, const ArSession *session);
        void hitTest(const ArFrame *frame, float x, float y, const ArSession *session, ArHitResultList *outList);
        int64_t getTimestampNs(const ArFrame *frame, const ArSession *session);
        void getUpdatedAnchors(const ArFrame *frame, const ArSession *session, ArAnchorList *outList);
        void getUpdatedPlanes(const ArFrame *frame, const ArSession *session, ArTrackableList *outList);
        void getBackgroundTexcoords(const ArFrame *frame, const ArSession *session, float *outTexcoords);
        ArPointCloud *acquirePointCloud(const ArFrame *frame, const ArSession *session);

    }

    namespace pointcloud {

        const float *getPoints(const ArPointCloud *cloud, const ArSession *session);
        int getNumPoints(const ArPointCloud *cloud, const ArSession *session);
        void release(ArPointCloud *cloud);

    }

    namespace hitresultlist {

        ArHitResultList *create(const ArSession *session);
        void destroy(ArHitResultList *hitResultList);
        void getItem(const ArHitResultList *hitResultList, int index, const ArSession *session, ArHitResult *outResult);
        int size(const ArHitResultList *hitResultList, const ArSession *session);

    }

    namespace hitresult {

        ArHitResult *create(const ArSession *session);
        void destroy(ArHitResult *hitResult);
        float getDistance(const ArHitResult *hitResult, const ArSession *session);
        void getPose(const ArHitResult *hitResult, const ArSession *session, ArPose *outPose);
        ArTrackable *acquireTrackable(const ArHitResult *hitResult, const ArSession *session);
        ArAnchor *acquireAnchor(ArHitResult *hitResult, ArSession *session);

    }

    namespace session {

        ArSession *create(void *applicationContext, JNIEnv *env);
        void destroy(ArSession *session);
        ConfigStatus configure(ArSession *session, const ArConfig *config);
        bool checkSupported(ArSession *session, const ArConfig *config);
        void setDisplayGeometry(ArSession *session, int rotation, int width, int height);
        void setCameraTextureName(ArSession *session, int32_t textureId);
        void pause(ArSession *session);
        void resume(ArSession *session);
        void update(ArSession *session, ArFrame *frame);

    }
}

#endif /* ARCORE_Native_h */
