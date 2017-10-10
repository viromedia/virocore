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
      Java_com_viro_renderer_jni_Camera_##method_name

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

JNI_METHOD(void, nativeAddToNode)(JNIEnv *env,
                                  jobject obj,
                                  jlong nativeCamera, jlong nativeNode) {

    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    std::weak_ptr<VRONode> node_w = Node::native(nativeNode);

    VROPlatformDispatchAsyncRenderer([node_w, camera_w] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && camera) {
            node->setCamera(camera);
        }
    });
}

JNI_METHOD(void, nativeRemoveFromNode)(JNIEnv *env,
                                       jobject obj,
                                       jlong nativeCamera, jlong nativeNode) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    std::weak_ptr<VRONode> node_w = Node::native(nativeNode);

    VROPlatformDispatchAsyncRenderer([node_w, camera_w] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && (node->getCamera() == camera || !camera)) {
            node->setCamera(nullptr);
        }
    });
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

JNI_METHOD(void, nativeSetRotationType)(JNIEnv *env,
                                              jobject obj,
                                              jlong nativeCamera,
                                              jstring rotationType) {
    // Get the string
    const char *cStrRotationType = env->GetStringUTFChars(rotationType, NULL);
    std::string strRotationType(cStrRotationType);
    VROCameraRotationType type;

    if (VROStringUtil::strcmpinsensitive(strRotationType, "orbit")) {
        type = VROCameraRotationType::Orbit;
    } else {
        // default rotation type is standard.
        type = VROCameraRotationType::Standard;
    }
    env->ReleaseStringUTFChars(rotationType, cStrRotationType);

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