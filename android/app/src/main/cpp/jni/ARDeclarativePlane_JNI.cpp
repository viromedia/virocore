//
//  ARPlane_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include <VROARDeclarativePlane.h>
#include "ARDeclarativePlane_JNI.h"
#include "ARUtils_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ARDeclarativePlane_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateARPlane) (JNIEnv *env,
                                        jclass clazz,
                                        jfloat minWidth,
                                        jfloat minHeight) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = std::make_shared<VROARDeclarativePlane>(minWidth, minHeight);
    return ARDeclarativePlane::jptr(arPlane);
}

JNI_METHOD(void, nativeSetMinWidth) (JNIEnv *env,
                                     jobject object,
                                     jlong nativeARPlane,
                                     jfloat minWidth) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = ARDeclarativePlane::native(nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

JNI_METHOD(void, nativeSetMinHeight) (JNIEnv *env,
                                     jobject object,
                                     jlong nativeARPlane,
                                     jfloat minHeight) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = ARDeclarativePlane::native(nativeARPlane);
    arPlane->setMinHeight(minHeight);
}

} // extern "C"


