//
// Created by Raj Advani on 10/11/17.
//

#ifndef ANDROID_GEOMETRY_JNI_H
#define ANDROID_GEOMETRY_JNI_H

#include <memory>
#include <VROPlatformUtil.h>
#include "VRONode.h"
#include "VROGeometry.h"
#include "VRODefines.h"
#include VRO_C_INCLUDE
#include "Material_JNI.h"

class Geometry {
public:
    static VRO_OBJECT createJGeometry(std::shared_ptr<VROGeometry> &geom) {
        VRO_ENV env = VROPlatformGetJNIEnv();
        if (env == nullptr) {
            perror("Required JNIEnv to create a jGeometry is null!");
            return VRO_OBJECT_NULL;
        }

        // Create a persistent native reference that would represent the jGeom object.
        VRO_REF(VROGeometry) geomRef = VRO_REF_NEW(VROGeometry, geom);
        VRO_OBJECT jGeom = VROPlatformConstructHostObject("com/viro/core/Geometry", "(J)V", geomRef);
        VRO_OBJECT arrayList = VROPlatformConstructHostObject("java/util/ArrayList", "()V");

        const std::vector<std::shared_ptr<VROMaterial>> mats = geom->getMaterials();
        for (int i = 0; i < mats.size(); i ++) {
            VRO_OBJECT jMat = Material::createJMaterial(mats[i]);
            VROPlatformCallHostBoolFunction(arrayList, "add", "(Ljava/lang/Object;)Z", jMat);
            VRO_DELETE_LOCAL_REF(jMat);
        }

        VROPlatformSetObject(env, jGeom, "mMaterials", "Ljava/util/List;", arrayList);
        VRO_DELETE_LOCAL_REF(arrayList);
        return jGeom;
    }
};

#endif //ANDROID_GEOMETRY_JNI_H
