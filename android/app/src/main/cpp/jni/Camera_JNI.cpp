//
//  Camera_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "Camera_JNI.h"
#include <iostream>
#include <jni.h>
#include <memory>
#include <VROPlatformUtil.h>
#include "PersistentRef.h"
#include "VRONodeCamera.h"
#include "VROCamera.h"
#include "VROStringUtil.h"
#include "Node_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Camera_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateCamera)(JNIEnv *env,
                                      jclass clazz) {
    std::shared_ptr<VRONodeCamera> node = std::make_shared<VRONodeCamera>();
    return Camera::jptr(node);
}

JNI_METHOD(void, nativeDestroyCamera)(JNIEnv *env,
                                    jclass clazz,
                                    jlong native_node_ref) {
    delete reinterpret_cast<PersistentRef<VRONodeCamera> *>(native_node_ref);
}

JNI_METHOD(void, nativeSetPosition)(JNIEnv *env,
                                    jobject obj,
                                    jlong nativeCamera, jfloat x, jfloat y, jfloat z) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setPosition(VROVector3f(x, y, z));
        }
    });
}

JNI_METHOD(void, nativeSetRotationEuler)(JNIEnv *env,
                                         jobject obj,
                                         jlong nativeCamera, jfloat x, jfloat y, jfloat z) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            VROQuaternion quaternion = { x, y, z };
            camera->setBaseRotation(quaternion);
        }
    });
}

JNI_METHOD(void, nativeSetRotationQuaternion)(JNIEnv *env,
                                              jobject obj,
                                              jlong nativeCamera, jfloat x, jfloat y, jfloat z, jfloat w) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z, w] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            VROQuaternion quaternion = { x, y, z, w };
            camera->setBaseRotation(quaternion);
        }
    });
}

JNI_METHOD(void, nativeSetRotationType)(JNIEnv *env,
                                              jobject obj,
                                              jlong nativeCamera,
                                              jstring rotationType) {
    VROCameraRotationType type;
    if (VROStringUtil::strcmpinsensitive(VROPlatformGetString(rotationType, env), "orbit")) {
        type = VROCameraRotationType::Orbit;
    } else {
        // default rotation type is standard.
        type = VROCameraRotationType::Standard;
    }

    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, type] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setRotationType(type);
        }
    });
}

JNI_METHOD(void, nativeSetOrbitFocalPoint)(JNIEnv *env,
                                           jobject obj,
                                           jlong nativeCamera, jfloat x, jfloat y, jfloat z) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setOrbitFocalPoint(VROVector3f(x, y, z));
        }
    });
}

} // extern "C"