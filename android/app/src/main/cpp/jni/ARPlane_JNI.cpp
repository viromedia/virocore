//
//  ARPlane_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

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

}