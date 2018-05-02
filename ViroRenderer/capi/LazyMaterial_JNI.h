//
//  LazyMaterial_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//


#ifndef ANDROID_LAZYMATERIAL_JNI_H
#define ANDROID_LAZYMATERIAL_JNI_H

#include <memory>
#include "VROLazyMaterial.h"
#include "VROPlatformUtil.h"
#include "Material_JNI.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class VROLazyMaterialJNI : public VROLazyMaterial {

public:
    VROLazyMaterialJNI(VRO_OBJECT obj) :
        _jobj(VRO_OBJECT_NULL) {
        VRO_ENV env = VROPlatformGetJNIEnv();
        _jobj = VRO_NEW_WEAK_GLOBAL_REF(obj);
    }

    virtual ~VROLazyMaterialJNI() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_jobj);
    }

    std::shared_ptr<VROMaterial> get() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(_jobj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return nullptr;
        }

        VRO_REF(VROMaterial) jptr = VROPlatformCallHostLongFunction(localObj, "get", "()J");
        VRO_DELETE_LOCAL_REF(localObj);
        return VRO_REF_GET(VROMaterial, jptr);
    }

private:
    // The corresponding LazyMaterialJni
    VRO_OBJECT _jobj;
};

#endif //ANDROID_LAZYMATERIAL_JNI_H
