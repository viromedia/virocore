//
//  Geometry_JNI.h
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
