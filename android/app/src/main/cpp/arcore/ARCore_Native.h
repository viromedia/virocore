//
//  ARCore_Native.h
//  Viro
//
//  Created by Raj Advani on 2/20/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ARCORE_NATIVE_h
#define ARCORE_NATIVE_h

#include "arcore_c_api.h"
#include <string>
#include <vector>

class VROMatrix4f;

namespace arcore {

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
        VROMatrix4f toMatrix(const ArPose *pose, const ArSession *session);

    }

    namespace anchorlist {

        ArAnchorList *create(const ArSession *session);
        void destroy(ArAnchorList *anchorList);
        ArAnchor *acquireItem(const ArAnchorList *anchorList, int index, const ArSession *session);
        int size(const ArAnchorList *anchorList, const ArSession *session);

    }

    namespace anchor {

        uint64_t getHashCode(const ArAnchor *anchor);
        std::string getId(const ArAnchor *anchor);
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
        VROMatrix4f getViewMatrix(const ArFrame *frame, const ArSession *session);
        VROMatrix4f getProjectionMatrix(const ArFrame *frame, float near, float far, const ArSession *session);
        TrackingState getTrackingState(const ArFrame *frame, const ArSession *session);
        void getLightEstimate(const ArFrame *frame, const ArSession *session, ArLightEstimate *outLightEstimate);
        bool hasDisplayGeometryChanged(const ArFrame *frame, const ArSession *session);
        void hitTest(const ArFrame *frame, float x, float y, const ArSession *session, ArHitResultList *outList);
        int64_t getTimestampNs(const ArFrame *frame, const ArSession *session);
        void getUpdatedAnchors(const ArFrame *frame, const ArSession *session, ArAnchorList *outList);
        void getUpdatedPlanes(const ArFrame *frame, const ArSession *session, ArTrackableList *outList);
        std::vector<float> getBackgroundTexcoords(const ArFrame *frame, const ArSession *session);
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

        ArSession *create(void *applicationContext);
        void destroy(ArSession *session);
        void configure(ArSession *session, const ArConfig *config);
        bool checkSupported(ArSession *session, const ArConfig *config);
        void setDisplayGeometry(ArSession *session, int rotation, int width, int height);
        void setCameraTextureName(ArSession *session, int32_t textureId);
        void pause(ArSession *session);
        void resume(ArSession *session);
        void update(ArSession *session, ArFrame *frame);

    }
}

#endif /* ARCORE_Native_h */
