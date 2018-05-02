//
//  Image_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
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
    delete reinterpret_cast<PersistentRef<VROImage> *>(nativeRef);
}

} // extern "C"