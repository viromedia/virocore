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
#include "VROLog.h"
#include "VROMatrix4f.h"

namespace arcore {

    struct Config { static constexpr auto Name() { return "com/google/ar/core/Config"; } };
    struct LightingModeEnum { static constexpr auto Name() { return "com/google/ar/core/Config$LightingMode"; } };
    struct PlaneFindingModeEnum { static constexpr auto Name() { return "com/google/ar/core/Config$PlaneFindingMode"; } };
    struct UpdateModeEnum { static constexpr auto Name() { return "com/google/ar/core/Config$UpdateMode"; } };

    struct LightEstimate { static constexpr auto Name() { return "com/google/ar/core/LightEstimate"; } };
    struct Frame { static constexpr auto Name() { return "com/google/ar/core/Frame"; } };
    struct TrackingStateEnum { static constexpr auto Name() { return "com/google/ar/core/Frame$TrackingState"; } };
    struct Session { static constexpr auto Name() { return "com/google/ar/core/Session"; } };

    namespace config {

        jni::Object<Config> getConfig(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                                      UpdateMode updateMode) {

            // TODO Implement creating a new Config via JNI. This requires setting the corresponding
            //      enums
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
            auto method = FrameClass.GetMethod<jni::Object<TrackingStateEnum>()>(env, "getTrackingState");
            jni::Object<TrackingStateEnum> trackingState = frame.Call(env, method);

            static auto TrackingStateEnumClass = *jni::Class<TrackingStateEnum>::Find(env).NewGlobalRef(env).release();
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

        jni::jboolean isDisplayRotationChanged(jni::Object<Frame> frame) {
            jni::JNIEnv &env = *VROPlatformGetJNIEnv();
            static auto FrameClass = *jni::Class<Frame>::Find(env).NewGlobalRef(env).release();
            auto method = FrameClass.GetMethod<jni::jboolean()>(env, "isDisplayRotationChanged");
            return frame.Call(env, method);
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