//
// Created by Raj Advani on 10/11/17.
//

#include "Geometry_JNI.h"
#include "Material_JNI.h"
#include "VROPlatformUtil.h"
#include "VROGeometry.h"
#include <jni.h>

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Geometry_##method_name

extern "C" {

JNI_METHOD(void, nativeSetMaterials)(JNIEnv *env,
                                     jobject obj,
                                     jlong nativeGeoRef,
                                     jlongArray longArrayRef) {
    jlong *longArray = env->GetLongArrayElements(longArrayRef, 0);
    jsize len = env->GetArrayLength(longArrayRef);

    std::vector<std::shared_ptr<VROMaterial>> tempMaterials;
    for (int i = 0; i < len; i++) {
        // Always copy materials from the material manager, as they may be
        // modified by animations, etc. and we don't want these changes to
        // propagate to the reference material held by the material manager
        tempMaterials.push_back(std::make_shared<VROMaterial>(Material::native(longArray[i])));
    }

    std::weak_ptr<VROGeometry> geo_w = Geometry::native(nativeGeoRef);
    VROPlatformDispatchAsyncRenderer([geo_w, tempMaterials] {
        std::shared_ptr<VROGeometry> geo = geo_w.lock();

        std::vector<std::shared_ptr<VROMaterial>> nonConstMaterials = tempMaterials;
        // If there was no materials given, just create an empty one and set that.
        if (tempMaterials.size() == 0) {
            nonConstMaterials.push_back(std::make_shared<VROMaterial>());
        }
        if (geo) {
            geo->setMaterials(nonConstMaterials);
        }
    });

    env->ReleaseLongArrayElements(longArrayRef, longArray, 0);
}
}