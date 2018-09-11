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

VRO_METHOD(VRO_REF(VROGeometry), nativeCreateGeometry)(VRO_NO_ARGS) {
    std::shared_ptr<VROGeometry> geo = std::make_shared<VROGeometry>();
    return VRO_REF_NEW(VROGeometry, geo);
}

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

VRO_METHOD(void, nativeSetGeometryElements)(VRO_ARGS
                                            VRO_REF(VROGeometry) geo_j,
                                            VRO_REF_ARRAY(VROGeometryElement) elements_j) {
    VRO_REF(VROGeometryElement) *elements_c = VRO_REF_ARRAY_GET_ELEMENTS(elements_j);
    int len = VRO_ARRAY_LENGTH(elements_j);

    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    for (int i = 0; i < len; i++) {
        elements.push_back(VRO_REF_GET(VROGeometryElement, elements_c[i]));
    }

    std::weak_ptr<VROGeometry> geo_w = VRO_REF_GET(VROGeometry, geo_j);
    VROPlatformDispatchAsyncRenderer([geo_w, elements] {
        std::shared_ptr<VROGeometry> geo = geo_w.lock();
        if (geo) {
            geo->setElements(elements);
        }
    });
    VRO_REF_ARRAY_RELEASE_ELEMENTS(elements_j, elements_c);
}

VRO_METHOD(void, nativeSetVertices)(VRO_ARGS
                                    VRO_REF(VROGeometry) nativeRef,
                                    VRO_FLOAT_ARRAY vertices_j) {
    float *vertices_c = VRO_FLOAT_ARRAY_GET_ELEMENTS(vertices_j);
    int len = VRO_ARRAY_LENGTH(vertices_j);

    int numVertices = len / 3;
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) vertices_c, len * sizeof(float));

    std::weak_ptr<VROGeometry> geo_w = VRO_REF_GET(VROGeometry, nativeRef);
    VROPlatformDispatchAsyncRenderer([geo_w, vertexData, numVertices] {
        std::shared_ptr<VROGeometry> geo = geo_w.lock();
        if (geo) {
            std::shared_ptr<VROGeometrySource> position = std::make_shared<VROGeometrySource>(vertexData,
                                                                                              VROGeometrySourceSemantic::Vertex,
                                                                                              numVertices,
                                                                                              true, 3,
                                                                                              sizeof(float),
                                                                                              0,
                                                                                              sizeof(float) * 3);
            geo->setGeometrySourceForSemantic(VROGeometrySourceSemantic::Vertex, position);
        }
    });
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(vertices_j, vertices_c);
}

VRO_METHOD(void, nativeSetNormals)(VRO_ARGS
                                   VRO_REF(VROGeometry) nativeRef,
                                   VRO_FLOAT_ARRAY normals_j) {
    float *normals_c = VRO_FLOAT_ARRAY_GET_ELEMENTS(normals_j);
    int len = VRO_ARRAY_LENGTH(normals_j);

    int numNormals = len / 3;
    std::shared_ptr<VROData> normalData = std::make_shared<VROData>((void *) normals_c, len * sizeof(float));

    std::weak_ptr<VROGeometry> geo_w = VRO_REF_GET(VROGeometry, nativeRef);
    VROPlatformDispatchAsyncRenderer([geo_w, normalData, numNormals] {
        std::shared_ptr<VROGeometry> geo = geo_w.lock();
        if (geo) {
            std::shared_ptr<VROGeometrySource> normal = std::make_shared<VROGeometrySource>(normalData,
                                                                                            VROGeometrySourceSemantic::Normal,
                                                                                            numNormals,
                                                                                            true, 3,
                                                                                            sizeof(float),
                                                                                            0,
                                                                                            sizeof(float) * 3);
            geo->setGeometrySourceForSemantic(VROGeometrySourceSemantic::Normal, normal);
        }
    });
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(normals_j, normals_c);
}

VRO_METHOD(void, nativeSetTextureCoordinates)(VRO_ARGS
                                              VRO_REF(VROGeometry) nativeRef,
                                              VRO_FLOAT_ARRAY texCoords_j) {
    float *texcoords_c = VRO_FLOAT_ARRAY_GET_ELEMENTS(texCoords_j);
    int len = VRO_ARRAY_LENGTH(texCoords_j);

    int numTexcoords = len / 2;
    std::shared_ptr<VROData> texcoordData = std::make_shared<VROData>((void *) texcoords_c, len * sizeof(float));

    std::weak_ptr<VROGeometry> geo_w = VRO_REF_GET(VROGeometry, nativeRef);
    VROPlatformDispatchAsyncRenderer([geo_w, texcoordData, numTexcoords] {
        std::shared_ptr<VROGeometry> geo = geo_w.lock();
        if (geo) {
            std::shared_ptr<VROGeometrySource> texcoord = std::make_shared<VROGeometrySource>(texcoordData,
                                                                                              VROGeometrySourceSemantic::Texcoord,
                                                                                              numTexcoords,
                                                                                              true, 2,
                                                                                              sizeof(float),
                                                                                              0,
                                                                                              sizeof(float) * 2);
            geo->setGeometrySourceForSemantic(VROGeometrySourceSemantic::Texcoord, texcoord);
        }
    });
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(texCoords_j, texcoords_c);
}

}