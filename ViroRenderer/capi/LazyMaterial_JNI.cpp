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

#include "LazyMaterial_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_LazyMaterial_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type LazyMaterial_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROLazyMaterialJNI), nativeCreateLazyMaterial)(VRO_NO_ARGS) {
    std::shared_ptr<VROLazyMaterialJNI> materialPtr = std::make_shared<VROLazyMaterialJNI>(obj);
    return VRO_REF_NEW(VROLazyMaterialJNI, materialPtr);
}

VRO_METHOD(void, nativeDestroyLazyMaterial)(VRO_ARGS
                                            VRO_REF(VROLazyMaterialJNI) nativeRef) {
    VRO_REF_DELETE(VROLazyMaterialJNI, nativeRef);
}

}  // extern "C"