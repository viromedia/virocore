//
//  Frame_JNI.cpp
//  Viro
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <jni.h>
#include "ARCore_JNI.h"
#include "VROPlatformUtil.h"
#include "VROMatrix4f.h"

namespace arcore {

    struct Config { static constexpr auto Name() { return "com/google/ar/core/Config"; } };
    struct LightingModeEnum { static constexpr auto Name() { return "com/google/ar/core/Config$LightingMode"; } };
    struct PlaneFindingModeEnum { static constexpr auto Name() { return "com/google/ar/core/Config$PlaneFindingMode"; } };
    struct UpdateModeEnum { static constexpr auto Name() { return "com/google/ar/core/Config$UpdateMode"; } };

    struct Pose { static constexpr auto Name() { return "com/google/ar/core/Pose"; } };
    struct Anchor { static constexpr auto Name() { return "com/google/ar/core/Anchor"; } };
    struct AnchorTrackingStateEnum { static constexpr auto Name() { return "com/google/ar/core/Anchor$TrackingState"; } };
    struct Plane { static constexpr auto Name() { return "com/google/ar/core/Plane"; } };
    struct PlaneTrackingStateEnum { static constexpr auto Name() { return "com/google/ar/core/Plane$TrackingState"; } };
    struct PlaneTypeEnum { static constexpr auto Name() { return "com/google/ar/core/Plane$Type"; } };
    struct LightEstimate { static constexpr auto Name() { return "com/google/ar/core/LightEstimate"; } };
    struct Frame { static constexpr auto Name() { return "com/google/ar/core/Frame"; } };
    struct FrameTrackingStateEnum { static constexpr auto Name() { return "com/google/ar/core/Frame$TrackingState"; } };
    struct HitResult { static constexpr auto Name() { return "com/google/ar/core/HitResult"; } };
    struct PointCloudHitResult { static constexpr auto Name() { return "com/google/ar/core/PointCloudHitResult"; } };
    struct Session { static constexpr auto Name() { return "com/google/ar/core/Session"; } };

    struct Object { static constexpr auto Name() { return "java/lang/Object"; } };
    struct Collection { static constexpr auto Name() { return "java/util/Collection"; } };
    struct List { static constexpr auto Name() { return "java/util/List"; } };

    namespace config {

        jni::Object<Config> getConfig(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                                      UpdateMode updateMode) {

            // TODO Implement creating a new Config via JNI. This requires setting the corresponding
            //      enums
        }

    }

    namespace collection {
        std::vector<jni::UniqueObject<Anchor>> toAnchorArray(jni::Object<Collection> collection) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto CollectionClass = *jni::Class<Collection>::Find(env).NewGlobalRef(env).release();
            auto method = CollectionClass.GetMethod<jni::Array<jni::Object<Object>>()>(env, "toArray");
            jni::Array<jni::Object<Object>> objects = collection.Call(env, method);

            int length = objects.Length(env);
            std::vector<jni::UniqueObject<Anchor>> anchorVector;

            for (int i = 0; i < length; i++) {
                jni::Object<Anchor> anchor = (jni::Object<Anchor>) objects.Get(env, i);
                anchorVector.push_back(anchor.NewGlobalRef(env));
            }

            return anchorVector;
        }

        std::vector<jni::UniqueObject<Plane>> toPlaneArray(jni::Object<Collection> collection) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto CollectionClass = *jni::Class<Collection>::Find(env).NewGlobalRef(env).release();
            auto method = CollectionClass.GetMethod<jni::Array<jni::Object<Object>>()>(env, "toArray");
            jni::Array<jni::Object<Object>> objects = collection.Call(env, method);

            int length = objects.Length(env);
            std::vector<jni::UniqueObject<Plane>> planeVector;

            for (int i = 0; i < length; i++) {
                jni::Object<Plane> plane = (jni::Object<Plane>) objects.Get(env, i);
                planeVector.push_back(plane.NewGlobalRef(env));
            }

            return planeVector;
        }
    }

    namespace list {
        jint size(jni::Object<List> list) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto ListClass = *jni::Class<List>::Find(env).NewGlobalRef(env).release();
            auto method = ListClass.GetMethod<jint()>(env, "size");
            return list.Call(env, method);
        }

        jni::Object<Object> get(jni::Object<List> list, int index) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto ListClass = *jni::Class<List>::Find(env).NewGlobalRef(env).release();
            auto method = ListClass.GetMethod<jni::Object<Object>(jni::jint)>(env, "get");
            return list.Call(env, method, index);
        }
    }

    namespace pose {
        VROMatrix4f toMatrix(jni::Object<Pose> pose) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PoseClass = *jni::Class<Pose>::Find(env).NewGlobalRef(env).release();
            auto method = PoseClass.GetMethod<void(jni::Array<jni::jfloat>, jint)>(env, "toMatrix");

            std::vector<float> vector(16, 0);
            jni::Array<jni::jfloat> array = jni::Make<jni::Array<jni::jfloat>>(env, vector);
            pose.Call(env, method, array, 0);

            jfloat *elements = env.GetFloatArrayElements((jfloatArray)array.Get(), NULL);
            VROMatrix4f matrix(elements);
            env.ReleaseFloatArrayElements((jfloatArray)array.Get(), elements, JNI_ABORT);

            return matrix;
        }
    }

    namespace anchor {
        const char* getId(jni::Object<Anchor> anchor) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto AnchorClass = *jni::Class<Anchor>::Find(env).NewGlobalRef(env).release();
            auto method = AnchorClass.GetMethod<jni::String()>(env, "getId");
            jni::String idJni = anchor.Call(env, method);
            return env.GetStringUTFChars((jstring)idJni.Get(), NULL);
        }

        jni::Object<Pose> getPose(jni::Object<Anchor> anchor) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto AnchorClass = *jni::Class<Anchor>::Find(env).NewGlobalRef(env).release();
            auto method = AnchorClass.GetMethod<jni::Object<Pose>()>(env, "getPose");
            return anchor.Call(env, method);
        }

        TrackingState getTrackingState(jni::Object<Anchor> anchor) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto AnchorClass = *jni::Class<Anchor>::Find(env).NewGlobalRef(env).release();
            auto method = AnchorClass.GetMethod<jni::Object<AnchorTrackingStateEnum>()>(env, "getTrackingState");
            jni::Object<AnchorTrackingStateEnum> trackingState = anchor.Call(env, method);

            static auto TrackingStateEnumClass = *jni::Class<AnchorTrackingStateEnum>::Find(env).NewGlobalRef(env).release();
            auto ordinalMethod = TrackingStateEnumClass.GetMethod<jni::jint()>(env, "ordinal");
            jni::jint ordinal = trackingState.Call(env, ordinalMethod);

            /* Anchor TrackingState enum value/order:
             TRACKING,
             NOT_CURRENTLY_TRACKING,
             STOPPED_TRACKING;
             */
            if (ordinal == 0) {
                return TrackingState::Tracking;
            }
            else {
                return TrackingState::NotTracking;
            }
        }
    }

    namespace plane {

        jint getHashCode(jni::Object<Plane> plane) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PlaneClass = *jni::Class<Plane>::Find(env).NewGlobalRef(env).release();
            auto method = PlaneClass.GetMethod<jint()>(env, "hashCode");
            return plane.Call(env, method);
        }

        jni::Object<Pose> getCenterPose(jni::Object<Plane> plane) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PlaneClass = *jni::Class<Plane>::Find(env).NewGlobalRef(env).release();
            auto method = PlaneClass.GetMethod<jni::Object<Pose>()>(env, "getCenterPose");
            return plane.Call(env, method);
        }

        jni::jfloat getExtentX(jni::Object<Plane> plane) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PlaneClass = *jni::Class<Plane>::Find(env).NewGlobalRef(env).release();
            auto method = PlaneClass.GetMethod<jni::jfloat()>(env, "getExtentX");
            return plane.Call(env, method);
        }

        jni::jfloat getExtentZ(jni::Object<Plane> plane) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PlaneClass = *jni::Class<Plane>::Find(env).NewGlobalRef(env).release();
            auto method = PlaneClass.GetMethod<jni::jfloat()>(env, "getExtentZ");
            return plane.Call(env, method);
        }

        jni::Object<Plane> getSubsumedBy(jni::Object<Plane> plane) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PlaneClass = *jni::Class<Plane>::Find(env).NewGlobalRef(env).release();
            auto method = PlaneClass.GetMethod<jni::Object<Plane>()>(env, "getSubsumedBy");
            return plane.Call(env, method);
        }

        TrackingState getTrackingState(jni::Object<Plane> plane) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PlaneClass = *jni::Class<Plane>::Find(env).NewGlobalRef(env).release();
            auto method = PlaneClass.GetMethod<jni::Object<PlaneTrackingStateEnum>()>(env, "getTrackingState");
            jni::Object<PlaneTrackingStateEnum> trackingState = plane.Call(env, method);

            static auto TrackingStateEnumClass = *jni::Class<PlaneTrackingStateEnum>::Find(env).NewGlobalRef(env).release();
            auto ordinalMethod = TrackingStateEnumClass.GetMethod<jni::jint()>(env, "ordinal");
            jni::jint ordinal = trackingState.Call(env, ordinalMethod);

            /* Plane TrackingState enum value/order:
             TRACKING,
             NOT_CURRENTLY_TRACKING,
             STOPPED_TRACKING;
             */
            if (ordinal == 0) {
                return TrackingState::Tracking;
            }
            else {
                return TrackingState::NotTracking;
            }
        }

        PlaneType getType(jni::Object<Plane> plane) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PlaneClass = *jni::Class<Plane>::Find(env).NewGlobalRef(env).release();
            auto method = PlaneClass.GetMethod<jni::Object<PlaneTypeEnum>()>(env, "getType");
            jni::Object<PlaneTypeEnum> type = plane.Call(env, method);

            static auto PlaneTypeEnumClass = *jni::Class<PlaneTypeEnum>::Find(env).NewGlobalRef(env).release();
            auto ordinalMethod = PlaneTypeEnumClass.GetMethod<jni::jint()>(env, "ordinal");
            jni::jint ordinal = type.Call(env, ordinalMethod);

            /* Plane Type enum value/order:
             HORIZONTAL_UPWARD_FACING,
             HORIZONTAL_DOWNWARD_FACING,
             NON_HORIZONTAL;
             */
            switch(ordinal) {
                case 0:
                    return PlaneType::HorizontalUpward;
                case 1:
                    return PlaneType::HorizontalDownward;
                default:
                    return PlaneType::NonHorizontal;
            }
        }
    }

    namespace light_estimate {

        jni::jfloat getPixelIntensity(jni::Object<LightEstimate> lightEstimate) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto LightEstimateClass = *jni::Class<LightEstimate>::Find(env).NewGlobalRef(env).release();
            auto method = LightEstimateClass.GetMethod<jni::jfloat()>(env, "getPixelIntensity");
            return lightEstimate.Call(env, method);
        }

        jni::jboolean isValid(jni::Object<LightEstimate> lightEstimate) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto LightEstimateClass = *jni::Class<LightEstimate>::Find(env).NewGlobalRef(env).release();
            auto method = LightEstimateClass.GetMethod<jni::jboolean()>(env, "isValid");
            return lightEstimate.Call(env, method);
        }

    }

    namespace frame {

        VROMatrix4f getViewMatrix(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<void(jni::Array<jni::jfloat>, jint)>(env, "getViewMatrix");

            std::vector<float> vector(16, 0);
            jni::Array<jni::jfloat> array = jni::Make<jni::Array<jni::jfloat>>(env, vector);
            frame.Call(env, method, array, 0);

            jfloat *elements = env.GetFloatArrayElements((jfloatArray)array.Get(), NULL);
            VROMatrix4f matrix(elements);
            env.ReleaseFloatArrayElements((jfloatArray)array.Get(), elements, JNI_ABORT);

            return matrix;
        }

        TrackingState getTrackingState(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<jni::Object<FrameTrackingStateEnum>()>(env, "getTrackingState");
            jni::Object<FrameTrackingStateEnum> trackingState = frame.Call(env, method);

            static auto TrackingStateEnumClass = *jni::Class<FrameTrackingStateEnum>::Find(env).NewGlobalRef(env).release();
            auto ordinalMethod = TrackingStateEnumClass.GetMethod<jni::jint()>(env, "ordinal");
            jni::jint ordinal = trackingState.Call(env, ordinalMethod);

            if (ordinal == 1) {
                return TrackingState::NotTracking;
            }
            else {
                return TrackingState::Tracking;
            }
        }

        jni::Object<LightEstimate> getLightEstimate(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<jni::Object<LightEstimate>()>(env, "getLightEstimate");
            return frame.Call(env, method);
        }

        jni::jlong getTimestampNs(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<jni::jlong()>(env, "getTimestampNs");
            return frame.Call(env, method);
        }

        jni::Object<Collection> getUpdatedAnchors(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<jni::Object<Collection>()>(env, "getUpdatedAnchors");
            return frame.Call(env, method);
        }

        jni::Object<Collection> getUpdatedPlanes(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<jni::Object<Collection>()>(env, "getUpdatedPlanes");
            return frame.Call(env, method);
        }

        jni::jboolean isDisplayRotationChanged(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<jni::jboolean()>(env, "isDisplayRotationChanged");
            return frame.Call(env, method);
        }

        jni::Object<List> hitTest(jni::Object<Frame> frame, float x, float y) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<jni::Object<List>(jni::jfloat, jni::jfloat)>(env, "hitTest");
            return frame.Call(env, method, x, y);
        }

        void setOrderNative(jni::JNIEnv &env, jobject byteBuffer) {
            jclass byteOrderClass = env.FindClass("java/nio/ByteOrder");
            jmethodID nativeByteOrderMethod = env.GetStaticMethodID(byteOrderClass, "nativeOrder", "()Ljava/nio/ByteOrder;");
            jobject byteOrder = env.CallStaticObjectMethod(byteOrderClass, nativeByteOrderMethod);

            jclass cls = env.FindClass("java/nio/ByteBuffer");
            jmethodID jmethod = env.GetMethodID(cls, "order", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;");
            env.CallObjectMethod(byteBuffer, jmethod, byteOrder);
        }

        jobject toFloatBuffer(jni::JNIEnv &env, jobject directByteBuffer) {
            jclass cls = env.FindClass("java/nio/ByteBuffer");
            jmethodID jmethod = env.GetMethodID(cls, "asFloatBuffer", "()Ljava/nio/FloatBuffer;");
            return env.CallObjectMethod(directByteBuffer, jmethod);
        }

        std::vector<float> getBackgroundTexcoords(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();

            // BL, TL, BR, TR
            float source[8] = { 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0 };
            jobject sourceBuffer = env.NewDirectByteBuffer(source, sizeof(float) * 8);
            setOrderNative(env, sourceBuffer);
            jobject sourceBufferFloat = toFloatBuffer(env, sourceBuffer);

            float dest[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            jobject destBuffer = env.NewDirectByteBuffer(dest, sizeof(float) * 8);
            setOrderNative(env, destBuffer);
            jobject destBufferFloat = toFloatBuffer(env, destBuffer);

            jclass cls = env.FindClass("com/google/ar/core/Frame");
            jmethodID jmethod = env.GetMethodID(cls, "transformDisplayUvCoords", "(Ljava/nio/FloatBuffer;Ljava/nio/FloatBuffer;)V");

            jobject obj = (jobject) frame.Get();
            env.CallVoidMethod(obj, jmethod, sourceBufferFloat, destBufferFloat);

            std::vector<float> result;
            result.assign(dest, dest + 8);

            env.DeleteLocalRef(sourceBuffer);
            env.DeleteLocalRef(sourceBufferFloat);
            env.DeleteLocalRef(destBuffer);
            env.DeleteLocalRef(destBufferFloat);

            return result;
        }

    }

    namespace hitresult {

        jfloat getDistance(jni::Object<HitResult> hitResult) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto HitResultClass = *jni::Class<HitResult>::Find(env).NewGlobalRef(env).release();
            auto method = HitResultClass.GetMethod<jni::jfloat()>(env, "getDistance");
            return hitResult.Call(env, method);
        }

        VROMatrix4f getPose(jni::Object<HitResult> hitResult) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto HitResultClass = *jni::Class<HitResult>::Find(env).NewGlobalRef(env).release();
            auto method = HitResultClass.GetMethod<jni::Object<Pose>()>(env, "getHitPose");
            jni::Object<Pose> pose = hitResult.Call(env, method);
            return pose::toMatrix(pose);
        }

    }

    namespace planehitresult {

        jni::Object<Plane> getPlane(jni::Object<PlaneHitResult> hitResult) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto PlaneHitResultClass = *jni::Class<PlaneHitResult>::Find(env).NewGlobalRef(env).release();
            auto method = PlaneHitResultClass.GetMethod<jni::Object<Plane>()>(env, "getPlane");
            return hitResult.Call(env, method);
        }

    }

    namespace session {

        VROMatrix4f getProjectionMatrix(jni::Object<Session> session, float near, float far) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto SessionClass = *jni::Class<Session>::Find(env).NewGlobalRef(env).release();
            auto method = SessionClass.GetMethod<void(jni::Array<jni::jfloat>, jint, jfloat, jfloat)>(env, "getProjectionMatrix");

            std::vector<float> vector(16, 0);
            jni::Array<jni::jfloat> array = jni::Make<jni::Array<jni::jfloat>>(env, vector);
            session.Call(env, method, array, 0, near, far);

            jfloat *elements = env.GetFloatArrayElements((jfloatArray)array.Get(), NULL);
            VROMatrix4f matrix(elements);
            env.ReleaseFloatArrayElements((jfloatArray)array.Get(), elements, JNI_ABORT);

            return matrix;
        }

        void setCameraTextureName(jni::Object<Session> session, jni::jint textureName) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto SessionClass = *jni::Class<Session>::Find(env).NewGlobalRef(env).release();
            auto method = SessionClass.GetMethod<void(jni::jint)>(env, "setCameraTextureName");
            session.Call(env, method, textureName);
        }

        void pause(jni::Object<Session> session) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto SessionClass = *jni::Class<Session>::Find(env).NewGlobalRef(env).release();
            auto method = SessionClass.GetMethod<void()>(env, "pause");
            session.Call(env, method);
        }

        jni::Object<Frame> update(jni::Object<Session> session) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto SessionClass = *jni::Class<Session>::Find(env).NewGlobalRef(env).release();
            auto method = SessionClass.GetMethod<jni::Object<Frame>()>(env, "update");
            return session.Call(env, method);
        }

    }
}