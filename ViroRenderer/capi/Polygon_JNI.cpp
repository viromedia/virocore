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
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Polygon_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROPolygon), nativeCreatePolygon)(VRO_ARGS
                                                     VRO_ARRAY(VRO_FLOAT_ARRAY) points_j,
                                                     VRO_ARRAY(VRO_ARRAY(VRO_FLOAT_ARRAY)) holes_j,
                                                     VRO_FLOAT u0, VRO_FLOAT v0,
                                                     VRO_FLOAT u1, VRO_FLOAT v1) {
    std::vector<VROVector3f> path;
    int pathSize = VRO_ARRAY_LENGTH(points_j);
    for (int i = 0; i < pathSize; i++) {
        VRO_FLOAT_ARRAY point_j = (VRO_FLOAT_ARRAY) VRO_ARRAY_GET(points_j, i);
        VRO_FLOAT *point_c = VRO_FLOAT_ARRAY_GET_ELEMENTS(point_j);
        VROVector3f point = VROVector3f(point_c[0], point_c[1]);
        path.push_back(point);

        VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(point_j, point_c);
    }

    std::vector<std::vector<VROVector3f>> holes;
    if (holes_j != nullptr) {
        int numHoles = VRO_ARRAY_LENGTH(holes_j);
        for (int i = 0; i < numHoles; i++) {
            std::vector<VROVector3f> hole;
            VRO_ARRAY(VRO_FLOAT_ARRAY) hole_j = (VRO_ARRAY(VRO_FLOAT_ARRAY)) VRO_ARRAY_GET(holes_j, i);
            int holeSize = VRO_ARRAY_LENGTH(hole_j);

            for (int i = 0; i < holeSize; i++) {
                VRO_FLOAT_ARRAY point_j = (VRO_FLOAT_ARRAY) VRO_ARRAY_GET(hole_j, i);
                VRO_FLOAT *point_c = VRO_FLOAT_ARRAY_GET_ELEMENTS(point_j);
                VROVector3f point = VROVector3f(point_c[0], point_c[1]);
                hole.push_back(point);

                VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(point_j, point_c);
            }

            holes.push_back(hole);
        }
    }

    std::shared_ptr<VROPolygon> surface  = VROPolygon::createPolygon(path, holes, u0, v0, u1, v1);
    return VRO_REF_NEW(VROPolygon, surface);
}

VRO_METHOD(void, nativeDestroyPolygon)(VRO_ARGS
                                       VRO_REF(VROPolygon) polygon) {
    VRO_REF_DELETE(VROPolygon, polygon);
}

}  // extern "C"