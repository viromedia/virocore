//
//  ARPlane_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include <VROARDeclarativePlane.h>
#include <VROStringUtil.h>
#include "ARDeclarativePlane_JNI.h"
#include "ARUtils_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ARDeclarativePlane_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateARPlane)(JNIEnv *env,
                                       jclass clazz,
                                       jfloat minWidth,
                                       jfloat minHeight,
                                       jstring jAlignment) {

    std::string strAlignment = VROPlatformGetString(jAlignment, env);
    VROARPlaneAlignment alignment = VROARPlaneAlignment::Horizontal;

    if (VROStringUtil::strcmpinsensitive(strAlignment, "Horizontal")) {
        alignment = VROARPlaneAlignment::Horizontal;
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "HorizontalUpward")) {
        alignment = VROARPlaneAlignment::HorizontalUpward;
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "HorizontalDownward")) {
        alignment = VROARPlaneAlignment::HorizontalDownward;
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "Vertical")) {
        alignment = VROARPlaneAlignment::Vertical;
    }

    std::shared_ptr<VROARDeclarativePlane> arPlane = std::make_shared<VROARDeclarativePlane>(
            minWidth, minHeight, alignment);
    return ARDeclarativePlane::jptr(arPlane);
}

JNI_METHOD(void, nativeSetMinWidth)(JNIEnv *env,
                                    jobject object,
                                    jlong nativeARPlane,
                                    jfloat minWidth) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = ARDeclarativePlane::native(nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

JNI_METHOD(void, nativeSetMinHeight)(JNIEnv *env,
                                     jobject object,
                                     jlong nativeARPlane,
                                     jfloat minHeight) {
    std::shared_ptr<VROARDeclarativePlane> arPlane = ARDeclarativePlane::native(nativeARPlane);
    arPlane->setMinHeight(minHeight);
}

JNI_METHOD(void, nativeSetAlignment)(JNIEnv *env,
                                     jobject object,
                                     jlong nativeARPlane,
                                     jstring jAlignment) {
    std::string strAlignment = VROPlatformGetString(jAlignment, env);
    std::shared_ptr<VROARDeclarativePlane> arPlane = ARDeclarativePlane::native(nativeARPlane);
    if (VROStringUtil::strcmpinsensitive(strAlignment, "Horizontal")) {
        arPlane->setAlignment(VROARPlaneAlignment::Horizontal);
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "HorizontalUpward")) {
        arPlane->setAlignment(VROARPlaneAlignment::HorizontalUpward);
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "HorizontalDownward")) {
        arPlane->setAlignment(VROARPlaneAlignment::HorizontalDownward);
    } else if (VROStringUtil::strcmpinsensitive(strAlignment, "Vertical")) {
        arPlane->setAlignment(VROARPlaneAlignment::Vertical);
    }
}

} // extern "C"


