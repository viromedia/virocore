//
//  ARPlane_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "ARPlane_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ARPlaneJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateARPlane) (JNIEnv *env,
                                        jclass clazz,
                                        jfloat minWidth,
                                        jfloat minHeight) {
    std::shared_ptr<VROARPlane> arPlane = std::make_shared<VROARPlane>(minWidth, minHeight);
    return ARPlane::jptr(arPlane);
}

JNI_METHOD(void, nativeDestroyARPlane) (JNIEnv *env,
                                        jobject object,
                                        jlong nativeARPlane) {
    delete reinterpret_cast<PersistentRef<VROARPlane> *>(nativeARPlane);
}

JNI_METHOD(jlong, nativeCreateARPlaneDelegate) (JNIEnv *env,
                                               jobject object,
                                               jlong nativeNodeRef) {
    std::shared_ptr<ARPlaneDelegate> delegate = std::make_shared<ARPlaneDelegate>(object, env);
    std::shared_ptr<VROARPlane> arPlane = ARPlane::native(nativeNodeRef);
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
    std::shared_ptr<VROARPlane> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

JNI_METHOD(void, nativeSetMinHeight) (JNIEnv *env,
                                     jobject object,
                                     jlong nativeARPlane,
                                     jfloat minHeight) {
    std::shared_ptr<VROARPlane> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setMinHeight(minHeight);
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
        jobject anchorObj = ARPlaneDelegate::createJavaARAnchorFromPlane(planeAnchor);

        // Yes, the function in the bridge is onAnchorFound.
        VROPlatformCallJavaFunction(localObj, "onAnchorFound", "(Lcom/viro/renderer/ARAnchor)V",
                                    anchorObj);
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
        jobject anchorObj = ARPlaneDelegate::createJavaARAnchorFromPlane(planeAnchor);

        // Yes, the function in the bridge is onAnchorUpdated.
        VROPlatformCallJavaFunction(localObj, "onAnchorUpdated", "(Lcom/viro/renderer/ARAnchor)V",
                                    anchorObj);
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
    });
}

/**
 * Creates an ARAnchor from the given VROARPlaneAnchor.
 */
jobject ARPlaneDelegate::createJavaARAnchorFromPlane(std::shared_ptr<VROARPlaneAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jclass cls = env->FindClass("com/viro/renderer/ARAnchor");

    /*
     ARAnchor's constructor has the following args:

     float[] position, float[] rotation, float[] scale,
     String alignment, float[] extent, float[] center
     */
    jmethodID constructor = env->GetMethodID(cls, "<init>", "([F[F[FLjava/lang/String;[F[F)V");

    VROMatrix4f transform = anchor->getTransform();
    VROVector3f rotationRads = transform.extractRotation(transform.extractScale()).toEuler();

    jfloatArray positionArray = ARPlaneDelegate::createFloatArrayFromVector3f(transform.extractTranslation());
    jfloatArray rotationArray = ARPlaneDelegate::createFloatArrayFromVector3f(
            {toDegrees(rotationRads.x), toDegrees(rotationRads.y), toDegrees(rotationRads.z)});
    jfloatArray scaleArray = ARPlaneDelegate::createFloatArrayFromVector3f(transform.extractScale());
    jstring alignment = ARPlaneDelegate::createStringFromAlignment(anchor->getAlignment());
    jfloatArray extentArray = ARPlaneDelegate::createFloatArrayFromVector3f(anchor->getExtent());
    jfloatArray centerArray = ARPlaneDelegate::createFloatArrayFromVector3f(anchor->getCenter());

    jobject javaAnchor = env->NewObject(cls, constructor, positionArray, rotationArray, scaleArray,
                                        alignment, extentArray, centerArray);

    return javaAnchor;
}

jfloatArray ARPlaneDelegate::createFloatArrayFromVector3f(VROVector3f vector) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jfloatArray returnArray = env->NewFloatArray(3);
    jfloat tempArr[3];
    tempArr[0] = vector.x; tempArr[1] = vector.y; tempArr[2] = vector.z;
    env->SetFloatArrayRegion(returnArray, 0, 3, tempArr);
    return returnArray;
}

jstring ARPlaneDelegate::createStringFromAlignment(VROARPlaneAlignment alignment) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    const char *strArr;
    if (alignment == VROARPlaneAlignment::Horizontal) {
        strArr = "Horizontal";
    }
    return env->NewStringUTF(strArr);
}

