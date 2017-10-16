//
//  ARPlane_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "ARPlane_JNI.h"
#include "ARUtils_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ARPlane_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateARPlane) (JNIEnv *env,
                                        jclass clazz,
                                        jfloat minWidth,
                                        jfloat minHeight) {
    std::shared_ptr<VROARPlaneNode> arPlane = std::make_shared<VROARPlaneNode>(minWidth, minHeight);
    return ARPlane::jptr(arPlane);
}

JNI_METHOD(void, nativeDestroyARPlane) (JNIEnv *env,
                                        jobject object,
                                        jlong nativeARPlane) {
    delete reinterpret_cast<PersistentRef<VROARPlaneNode> *>(nativeARPlane);
}

JNI_METHOD(jlong, nativeCreateARPlaneDelegate) (JNIEnv *env,
                                               jobject object,
                                               jlong nativeNodeRef) {
    std::shared_ptr<ARPlaneDelegate> delegate = std::make_shared<ARPlaneDelegate>(object, env);
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeNodeRef);
    arPlane->setARNodeDelegate(delegate);
    return ARPlaneDelegate::jptr(delegate);
}

JNI_METHOD(void, nativeDestroyARPlaneDelegate) (JNIEnv *env,
                                                jobject object,
                                                jlong delegateRef) {
    delete reinterpret_cast<PersistentRef<ARPlaneDelegate> *>(delegateRef);
}


JNI_METHOD(void, nativeSetMinWidth) (JNIEnv *env,
                                     jobject object,
                                     jlong nativeARPlane,
                                     jfloat minWidth) {
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

JNI_METHOD(void, nativeSetMinHeight) (JNIEnv *env,
                                     jobject object,
                                     jlong nativeARPlane,
                                     jfloat minHeight) {
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setMinHeight(minHeight);
}


JNI_METHOD(void, nativeSetAnchorId) (JNIEnv *env,
                               jobject object,
                               jlong nativeARPlane,
                               jstring id) {
    const char *cStrId = env->GetStringUTFChars(id, NULL);
    std::string idStr(cStrId);

    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setId(idStr);

    env->ReleaseStringUTFChars(id, cStrId);
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
        VROPlatformCallJavaFunction(localObj, "onAnchorFound", "(Lcom/viro/renderer/ARAnchor;)V",
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
        VROPlatformCallJavaFunction(localObj, "onAnchorUpdated", "(Lcom/viro/renderer/ARAnchor;)V",
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

