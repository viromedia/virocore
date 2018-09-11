//
// Created by Raj Advani on 9/10/18.
//

#include "Submesh_JNI.h"
#include "VROPlatformUtil.h"
#include "VROGeometryUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Submesh_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Geometry_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROGeometryElement), nativeCreateGeometryElement)(VRO_NO_ARGS) {
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>();
    return VRO_REF_NEW(VROGeometryElement, element);
}

VRO_METHOD(void, nativeSetTriangleIndices)(VRO_ARGS
                                           VRO_REF(VROGeometryElement) nativeRef,
                                           VRO_INT_ARRAY indices_j) {
    int *indices_c = VRO_INT_ARRAY_GET_ELEMENTS(indices_j);
    int numIndices = VRO_ARRAY_LENGTH(indices_j);

    int indexSize = sizeof(int);
    std::shared_ptr<VROData> indicesData = std::make_shared<VROData>((void *) indices_c, numIndices * indexSize);

    std::weak_ptr<VROGeometryElement> element_w = VRO_REF_GET(VROGeometryElement, nativeRef);
    VROPlatformDispatchAsyncRenderer([element_w, indicesData, numIndices, indexSize] {
        std::shared_ptr<VROGeometryElement> element = element_w.lock();
        if (element) {
            element->setPrimitiveType(VROGeometryPrimitiveType::Triangle);
            element->setPrimitiveCount(VROGeometryUtilGetPrimitiveCount(numIndices, VROGeometryPrimitiveType::Triangle));
            element->setBytesPerIndex(indexSize);
            element->setData(indicesData);
        }
    });
    VRO_INT_ARRAY_RELEASE_ELEMENTS(indices_j, indices_c);
}

}