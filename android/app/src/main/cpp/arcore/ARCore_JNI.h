//
//  Frame_JNI.h
//  Viro
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ARCORE_JNI_h
#define ARCORE_JNI_h

#include <jni/jni.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include "VROViewport.h"

class VROMatrix4f;

namespace arcore {

    struct Config;
    struct Pose;
    struct Anchor;
    struct Plane;
    struct LightEstimate;
    struct Frame;
    struct HitResult;
    struct PlaneHitResult { static constexpr auto Name() { return "com/google/ar/core/PlaneHitResult"; } };
    struct PointCloud;
    struct Session;
    struct Object;
    struct ViroViewARCore;
    struct Collection;
    struct List;
    struct FloatBuffer;

    enum class TrackingState {
        NotTracking,
        Tracking
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

        jni::Object<Config> getConfig(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                                      UpdateMode updateMode);

    }

    namespace collection {
        std::vector<jni::UniqueObject<Anchor>> toAnchorArray(jni::Object<Collection> collection);
        std::vector<jni::UniqueObject<Plane>> toPlaneArray(jni::Object<Collection> collection);
    }

    namespace list {
        jint size(jni::Object<List> list);
        jni::Object<Object> get(jni::Object<List> list, int index);
    }

    namespace floatbuffer {
        std::vector<float> toVector(jni::Object<FloatBuffer> buffer);
    }

    namespace viroview {
        void setConfig(jni::Object<ViroViewARCore> view, jni::Object<Config> config);
    }

    namespace pose {
        VROMatrix4f toMatrix(jni::Object<Pose> pose);
    }

    namespace anchor {
        const char* getId(jni::Object<Anchor> anchor);
        jni::Object<Pose> getPose(jni::Object<Anchor> anchor);
        TrackingState getTrackingState(jni::Object<Anchor> anchor);
    }

    namespace plane {
        jint getHashCode(jni::Object<Plane> plane);
        jni::Object<Pose> getCenterPose(jni::Object<Plane> plane);
        jni::jfloat getExtentX(jni::Object<Plane> plane);
        jni::jfloat getExtentZ(jni::Object<Plane> plane);
        jni::Object<Plane> getSubsumedBy(jni::Object<Plane> plane);
        TrackingState getTrackingState(jni::Object<Plane> plane);
        PlaneType getType(jni::Object<Plane> plane);
    }

    namespace light_estimate {

        jni::jfloat getPixelIntensity(jni::Object<LightEstimate> lightEstimate);
        jni::jboolean isValid(jni::Object<LightEstimate> lightEstimate);

    }

    namespace frame {

        VROMatrix4f getViewMatrix(jni::Object<Frame> frame);
        TrackingState getTrackingState(jni::Object<Frame> frame);
        jni::Object<LightEstimate> getLightEstimate(jni::Object<Frame> frame);
        jni::jboolean isDisplayRotationChanged(jni::Object<Frame> frame);
        jni::Object<List> hitTest(jni::Object<Frame> frame, float x, float y);
        jni::jlong getTimestampNs(jni::Object<Frame> frame);
        jni::Object<Collection> getUpdatedAnchors(jni::Object<Frame> frame);
        jni::Object<Collection> getUpdatedPlanes(jni::Object<Frame> frame);
        std::vector<float> getBackgroundTexcoords(jni::Object<Frame> frame);
        jni::Object<PointCloud> getPointCloud(jni::Object<Frame> frame);
        VROMatrix4f getPointCloudPose(jni::Object<Frame> frame);

    }

    namespace pointcloud {

        jni::Object<FloatBuffer> getPoints(jni::Object<PointCloud> pointCloud);

    }

    namespace hitresult {

        jfloat getDistance(jni::Object<HitResult> hitResult);
        VROMatrix4f getPose(jni::Object<HitResult> hitResult);

    }

    namespace planehitresult {

        jni::Object<Plane> getPlane(jni::Object<PlaneHitResult> hitResult);
        jboolean isHitInExtents(jni::Object<PlaneHitResult> hitResult);
        jboolean isHitInPolygon(jni::Object<PlaneHitResult> hitResult);
    }

    namespace session {

        VROMatrix4f getProjectionMatrix(jni::Object<Session> session, float near, float far);
        void setCameraTextureName(jni::Object<Session> session, jni::jint textureName);
        void pause(jni::Object<Session> session);
        void resume(jni::Object<Session> session, jni::Object<Config> config);
        jni::Object<Frame> update(jni::Object<Session> session);

    }
}

#endif /* ARCORE_JNI_h */
