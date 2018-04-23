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

VRO_METHOD(VRO_REF, nativeCreateSurface)(VRO_ARGS
                                         jobjectArray jpoints,
                                         VRO_FLOAT u0, VRO_FLOAT v0,
                                         VRO_FLOAT u1, VRO_FLOAT v1) {
    std::vector<VROVector3f> initialValues;
    int numberOfValues = env->GetArrayLength(jpoints);
    for (int i = 0; i < numberOfValues; i++) {
        VRO_FLOAT_ARRAY vec3Value = (VRO_FLOAT_ARRAY) env->GetObjectArrayElement(jpoints, i);
        VRO_FLOAT *vec3ValueArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(vec3Value);
        VROVector3f vec3 = VROVector3f(vec3ValueArray[0], vec3ValueArray[1]);
        initialValues.push_back(vec3);
        VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(vec3Value, vec3ValueArray);
    }

    std::shared_ptr<VROPolygon> surface  = VROPolygon::createPolygon(initialValues, u0, v0, u1, v1);
    return Polygon::jptr(surface);
}

VRO_METHOD(void, nativeDestroySurface)(VRO_ARGS
                                       VRO_REF nativeSurface) {
    delete reinterpret_cast<PersistentRef<VROPolygon> *>(nativeSurface);
}

}  // extern "C"