//
//  Image_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
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

#include <memory>
#include "VROPlatformUtil.h"
#include "Image_JNI.h"
#include "Texture_JNI.h"

#if VRO_PLATFORM_ANDROID
#include "VROImageAndroid.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_Image_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Image_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROImage), nativeCreateImage)(VRO_ARGS
                                                 VRO_STRING resource,
                                                 VRO_STRING format) {
    VRO_METHOD_PREAMBLE;
    std::string strResource = VRO_STRING_STL(resource);

    VROTextureInternalFormat internalFormat = Texture::getFormat(env, format);
    std::shared_ptr<VROImage> imagePtr;
#if VRO_PLATFORM_ANDROID
    imagePtr = std::make_shared<VROImageAndroid>(strResource, internalFormat);
#else
    //TODO wasm
#endif

    return VRO_REF_NEW(VROImage, imagePtr);
}

VRO_METHOD(VRO_REF(VROImage), nativeCreateImageFromBitmap)(VRO_ARGS
                                                           VRO_OBJECT jbitmap,
                                                           VRO_STRING format) {
    VRO_METHOD_PREAMBLE;
    VROPlatformSetEnv(env);
    VROTextureInternalFormat internalFormat = Texture::getFormat(env, format);

    std::shared_ptr<VROImage> imagePtr;
#if VRO_PLATFORM_ANDROID
    imagePtr = std::make_shared<VROImageAndroid>(jbitmap, internalFormat);
#else
    //TODO wasm
#endif

    return VRO_REF_NEW(VROImage, imagePtr);
}

VRO_METHOD(VRO_INT, nativeGetWidth)(VRO_ARGS
                                    VRO_REF(VROImage) nativeRef) {
    return VRO_REF_GET(VROImage, nativeRef).get()->getWidth();
}

VRO_METHOD(VRO_INT, nativeGetHeight)(VRO_ARGS
                                     VRO_REF(VROImage) nativeRef) {
    return VRO_REF_GET(VROImage, nativeRef).get()->getHeight();
}

VRO_METHOD(void, nativeDestroyImage)(VRO_ARGS
                                     VRO_REF(VROImage) nativeRef) {
    VRO_REF_DELETE(VROImage, nativeRef);
}

} // extern "C"