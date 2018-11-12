//
//  OBJLoaderDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//


#include <memory>
#include "VROPlatformUtil.h"
#include "OBJLoaderDelegate_JNI.h"
#include "Node_JNI.h"
#include "Geometry_JNI.h"
#include "VROGeometry.h"
#include "VROMaterial.h"
#include "Material_JNI.h"

OBJLoaderDelegate::OBJLoaderDelegate(VRO_OBJECT nodeJavaObject, VRO_ENV env) :
    _javaObject(VRO_NEW_WEAK_GLOBAL_REF(nodeJavaObject)) {
}

OBJLoaderDelegate::~OBJLoaderDelegate() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
}

void OBJLoaderDelegate::objLoaded(std::shared_ptr<VRONode> node, ModelType modelType, VRO_LONG requestId) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);


    VROPlatformDispatchAsyncApplication([weakObj, node, modelType] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
            return;
        }

        // If the request was for an OBJ, create a persistent ref for the Java Geometry and
        // pass that up as well. This enables Java SDK users to set materials on the Geometry
        VRO_REF(VROGeometry) geometryRef = 0;
        if (modelType == ModelType::OBJ && node->getGeometry()) {
            geometryRef = VRO_REF_NEW(VROGeometry, node->getGeometry());
        }

        // Generate a map of unique jMaterials representing this 3D model.
        std::map<std::string, std::shared_ptr<VROMaterial>> mats;
        generateJMaterials(mats, node);

        // Generate an array containing a unique list of jMaterials
        VRO_OBJECT_ARRAY materialArray = VRO_NEW_OBJECT_ARRAY(mats.size(), "com/viro/core/Material");
        if (mats.size() > 0) {
            int i = 0;
            for(std::map<std::string, std::shared_ptr<VROMaterial>>::iterator
                        it = mats.begin(); it != mats.end(); ++it) {
                // Create the Material.java and added it to the array
                VRO_OBJECT jMat = Material::createJMaterial(it->second);
                VRO_ARRAY_SET(materialArray, i, jMat);

                // Clean up our local reference to jMaterial after.
                VRO_DELETE_LOCAL_REF(jMat);
                i++;
            }
        }

        // Call the nodeDidFinishCreation callback.
        VRO_REF(VROGeometry) jGeometryRef = geometryRef;
        VROPlatformCallHostFunction(localObj,
                                    "nodeDidFinishCreation",
                                    "([Lcom/viro/core/Material;IJ)V",
                                    materialArray, (int)modelType, jGeometryRef);

        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void OBJLoaderDelegate::generateJMaterials(std::map<std::string, std::shared_ptr<VROMaterial>> &matOut,
                                           std::shared_ptr<VRONode> node) {

    std::shared_ptr<VROGeometry> geom = node->getGeometry();
    if (geom != nullptr){
        for (std::shared_ptr<VROMaterial> mat : geom->getMaterials()) {

            // Skip, if we have already tracked this material.
            std::string materialId = VROStringUtil::toString(mat->getMaterialId());
            if (matOut.find(materialId) != matOut.end()){
                continue;
            }

            // Grab the material name devs to reference by. If no name is provided,
            // generate one from with a combination of geometryName + material ID.
            if (mat->getName().empty()) {
                std::string id = VROStringUtil::toString(mat->getMaterialId());
                std::string geomName = geom->getName();
                std::string matName = geomName + id;
                mat->setName(matName);
            }

            matOut[materialId] = mat;
        }
    }

    for (std::shared_ptr<VRONode> childNode : node->getChildNodes()) {
        generateJMaterials(matOut, childNode);
    }
}

void OBJLoaderDelegate::objFailed(std::string error) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, error] {
        VRO_ENV env = VROPlatformGetJNIEnv();

        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
            return;
        }

        VRO_STRING jerror = VRO_NEW_STRING(error.c_str());
        VROPlatformCallHostFunction(localObj, "nodeDidFailOBJLoad", "(Ljava/lang/String;)V", jerror);

        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_LOCAL_REF(jerror);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

ModelType VROGetModelType(VRO_INT jModelType) {
    if (jModelType == (int) ModelType::FBX) {
        return ModelType::FBX;
    } else if (jModelType == (int) ModelType::OBJ) {
        return ModelType::OBJ;
    } else if (jModelType == (int) ModelType::GLTF) {
        return ModelType::GLTF;
    } else if (jModelType == (int) ModelType::GLB) {
        return ModelType::GLB;
    }

    return ModelType::Unknown;
}
