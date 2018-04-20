//
//  Polygon_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "Polygon_JNI.h"
#include "VROMaterial.h"
#include "ViroContext_JNI.h"
#include "Material_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Polygon_##method_name
#endif

extern "C" {

VRO_METHOD(jlong, nativeCreateSurface)(VRO_ARGS
                                       jobjectArray jpoints,
                                       jfloat u0, jfloat v0,
                                       jfloat u1, jfloat v1) {
    std::vector<VROVector3f> initialValues;
    int numberOfValues = env->GetArrayLength(jpoints);
    for (int i = 0; i < numberOfValues; i++) {
        jfloatArray vec3Value = (jfloatArray) env->GetObjectArrayElement(jpoints, i);
        jfloat *vec3ValueArray = env->GetFloatArrayElements(vec3Value, 0);
        VROVector3f vec3 = VROVector3f(vec3ValueArray[0], vec3ValueArray[1]);
        initialValues.push_back(vec3);
        env->ReleaseFloatArrayElements(vec3Value, vec3ValueArray, JNI_ABORT);
    }

    std::shared_ptr<VROPolygon> surface  = VROPolygon::createPolygon(initialValues, u0, v0, u1, v1);
    return Polygon::jptr(surface);
}

VRO_METHOD(void, nativeDestroySurface)(VRO_ARGS
                                       jlong nativeSurface) {
    delete reinterpret_cast<PersistentRef<VROPolygon> *>(nativeSurface);
}

}  // extern "C"