//
//  ARPlane_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "ARPlane_JNI.h"
#include "ARUtils_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARPlane_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateARPlane)(VRO_ARGS
                                         jfloat minWidth,
                                         jfloat minHeight) {
    std::shared_ptr<VROARPlaneNode> arPlane = std::make_shared<VROARPlaneNode>(minWidth, minHeight);
    return ARPlane::jptr(arPlane);
}

VRO_METHOD(void, nativeDestroyARPlane)(VRO_ARGS
                                       VRO_REF nativeARPlane) {
    delete reinterpret_cast<PersistentRef<VROARPlaneNode> *>(nativeARPlane);
}

VRO_METHOD(VRO_REF, nativeCreateARPlaneDelegate)(VRO_ARGS
                                                 VRO_REF nativeNodeRef) {
    std::shared_ptr<ARPlaneDelegate> delegate = std::make_shared<ARPlaneDelegate>(object, env);
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeNodeRef);
    arPlane->setARNodeDelegate(delegate);
    return ARPlaneDelegate::jptr(delegate);
}

VRO_METHOD(void, nativeDestroyARPlaneDelegate)(VRO_ARGS
                                               VRO_REF delegateRef) {
    delete reinterpret_cast<PersistentRef<ARPlaneDelegate> *>(delegateRef);
}


VRO_METHOD(void, nativeSetMinWidth)(VRO_ARGS
                                    VRO_REF nativeARPlane,
                                    jfloat minWidth) {
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

VRO_METHOD(void, nativeSetMinHeight)(VRO_ARGS
                                     VRO_REF nativeARPlane,
                                     jfloat minHeight) {
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setMinHeight(minHeight);
}

VRO_METHOD(void, nativeSetAnchorId)(VRO_ARGS
                                    VRO_REF nativeARPlane,
                                    jstring id) {
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setId(VROPlatformGetString(id, env));
}

VRO_METHOD(void, nativeSetPauseUpdates)(VRO_ARGS
                                        VRO_REF nativeARPlane,
                                        jboolean pauseUpdates) {
    std::weak_ptr<VROARPlaneNode> arPlane_w = ARPlane::native(nativeARPlane);
    VROPlatformDispatchAsyncRenderer([arPlane_w, pauseUpdates] {
        std::shared_ptr<VROARPlaneNode> arPlane = arPlane_w.lock();
        arPlane->setPauseUpdates(pauseUpdates);
    });
}


} // extern "C"

void ARPlaneDelegate::onARAnchorAttached(std::shared_ptr<VROARAnchor> anchor) {
    std::weak_ptr<VROARPlaneAnchor> planeAnchor_w = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObject_w = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([this, jObject_w, planeAnchor_w] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObject_w);
        std::shared_ptr<VROARPlaneAnchor> planeAnchor = planeAnchor_w.lock();
        if (localObj == NULL || !planeAnchor) {
            return;
        }

        // create the Java ARAnchor POJO
        jobject anchorObj = ARUtilsCreateJavaARAnchorFromAnchor(planeAnchor);

        // Yes, the function in the bridge is onAnchorFound.
        VROPlatformCallJavaFunction(localObj, "onAnchorFound", "(Lcom/viro/core/ARAnchor;)V",
                                    anchorObj);
        env->DeleteWeakGlobalRef(jObject_w);
    });
}

void ARPlaneDelegate::onARAnchorUpdated(std::shared_ptr<VROARAnchor> anchor) {
    std::weak_ptr<VROARPlaneAnchor> planeAnchor_w = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObject_w = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([this, jObject_w, planeAnchor_w] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObject_w);
        std::shared_ptr<VROARPlaneAnchor> planeAnchor = planeAnchor_w.lock();
        if (localObj == NULL || !planeAnchor) {
            return;
        }

        // create the Java ARAnchor POJO
        jobject anchorObj = ARUtilsCreateJavaARAnchorFromAnchor(planeAnchor);

        // Yes, the function in the bridge is onAnchorUpdated.
        VROPlatformCallJavaFunction(localObj, "onAnchorUpdated", "(Lcom/viro/core/ARAnchor;)V",
                                    anchorObj);

        env->DeleteWeakGlobalRef(jObject_w);
    });
}

void ARPlaneDelegate::onARAnchorRemoved() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObject_w = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObject_w] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObject_w);
        if (localObj == NULL) {
            return;
        }

        // Yes, the function in the bridge is onAnchorRemoved.
        VROPlatformCallJavaFunction(localObj, "onAnchorRemoved", "()V");
        env->DeleteWeakGlobalRef(jObject_w);
    });
}

