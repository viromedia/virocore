//
// Created by Raj Advani on 10/11/17.
//

#include "Geometry_JNI.h"
#include "Material_JNI.h"
#include "VROPlatformUtil.h"
#include "VROGeometry.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Geometry_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Geometry_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeSetMaterials)(VRO_ARGS
                                     VRO_REF(VROGeometry) geo_j,
                                     VRO_REF_ARRAY(VROMaterial) materials_j) {
    VRO_REF(VROMaterial) *materials_c = VRO_REF_ARRAY_GET_ELEMENTS(materials_j);
    int len = VRO_ARRAY_LENGTH(materials_j);
    std::vector<std::shared_ptr<VROMaterial>> materials;
    for (int i = 0; i < len; i++) {
        materials.push_back(VRO_REF_GET(VROMaterial, materials_c[i]));
    }

    std::weak_ptr<VROGeometry> geo_w = VRO_REF_GET(VROGeometry, geo_j);
    VROPlatformDispatchAsyncRenderer([geo_w, materials] {
        std::shared_ptr<VROGeometry> geo = geo_w.lock();
        if (geo) {
            geo->setMaterials(materials);
        }
    });
    VRO_REF_ARRAY_RELEASE_ELEMENTS(materials_j, materials_c);
}

VRO_METHOD(void, nativeCopyAndSetMaterials)(VRO_ARGS
                                            VRO_REF(VROGeometry) nativeGeoRef,
                                            VRO_REF_ARRAY(VROMaterial) materials_j) {
    VRO_REF(VROMaterial) *materials_c = VRO_REF_ARRAY_GET_ELEMENTS(materials_j);
    int len = VRO_ARRAY_LENGTH(materials_j);

    std::vector<std::shared_ptr<VROMaterial>> tempMaterials;
    for (int i = 0; i < len; i++) {
        // Always copy materials from the material manager, as they may be
        // modified by animations, etc. and we don't want these changes to
        // propagate to the reference material held by the material manager
        tempMaterials.push_back(std::make_shared<VROMaterial>(VRO_REF_GET(VROMaterial, materials_c[i])));
    }

    std::weak_ptr<VROGeometry> geo_w = VRO_REF_GET(VROGeometry, nativeGeoRef);
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

    VRO_REF_ARRAY_RELEASE_ELEMENTS(materials_j, materials_c);
}
}