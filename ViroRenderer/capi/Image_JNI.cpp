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
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateImage)(VRO_ARGS
                                       VRO_STRING resource, VRO_STRING format) {
    std::string strResource = VROPlatformGetString(resource, env);

    VROTextureInternalFormat internalFormat = Texture::getFormat(env, format);
    std::shared_ptr<VROImage> imagePtr;
#if VRO_PLATFORM_ANDROID
    imagePtr = std::make_shared<VROImageAndroid>(strResource, internalFormat);
#else
    //TODO wasm
#endif

    return Image::jptr(imagePtr);
}

VRO_METHOD(VRO_REF, nativeCreateImageFromBitmap)(VRO_ARGS
                                                 VRO_OBJECT jbitmap, VRO_STRING format) {
    VROPlatformSetEnv(env);
    VROTextureInternalFormat internalFormat = Texture::getFormat(env, format);

    std::shared_ptr<VROImage> imagePtr;
#if VRO_PLATFORM_ANDROID
    imagePtr = std::make_shared<VROImageAndroid>(jbitmap, internalFormat);
#else
    //TODO wasm
#endif

    return Image::jptr(imagePtr);
}

VRO_METHOD(VRO_INT, nativeGetWidth)(VRO_ARGS
                                    VRO_REF nativeRef) {
    return Image::native(nativeRef).get()->getWidth();
}

VRO_METHOD(VRO_INT, nativeGetHeight)(VRO_ARGS
                                     VRO_REF nativeRef) {
    return Image::native(nativeRef).get()->getHeight();
}

VRO_METHOD(void, nativeDestroyImage)(VRO_ARGS
                                     VRO_REF nativeRef) {
    delete reinterpret_cast<PersistentRef<VROImage> *>(nativeRef);
}

} // extern "C"