//
//  LazyMaterial_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
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

#if VRO_PLATFORM_ANDROID
        VRO_REF(VROMaterial) jptr = VROPlatformCallHostLongFunction(localObj, "get", "()J");
        VRO_DELETE_LOCAL_REF(localObj);
        return VRO_REF_GET(VROMaterial, jptr);
#else
        // TODO wasm
        // Create a new function, VROPlatformCallHostRefFunction
        return nullptr;
#endif

    }

private:
    // The corresponding LazyMaterialJni
    VRO_OBJECT _jobj;
};

#endif //ANDROID_LAZYMATERIAL_JNI_H
