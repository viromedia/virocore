//
//  Image_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VROPlatformUtil.h>

#include "Image_JNI.h"
#include "Texture_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_Image_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateImage)(JNIEnv *env, jobject obj, jstring resource, jstring format) {
    std::string strResource = VROPlatformGetString(resource);

    VROTextureInternalFormat internalFormat = Texture::getFormat(env, format);
    std::shared_ptr<VROImageAndroid> imagePtr = std::make_shared<VROImageAndroid>(strResource, internalFormat);

    return Image::jptr(imagePtr);
}

JNI_METHOD(jlong, nativeCreateImageFromBitmap)(JNIEnv *env, jobject obj, jobject jbitmap, jstring format) {
    VROPlatformSetEnv(env);
    VROTextureInternalFormat internalFormat = Texture::getFormat(env, format);
    std::shared_ptr<VROImageAndroid> imagePtr = std::make_shared<VROImageAndroid>(jbitmap, internalFormat);
    return Image::jptr(imagePtr);
}

JNI_METHOD(jlong, nativeGetWidth)(JNIEnv *env, jobject obj, jlong nativeRef) {
    return Image::native(nativeRef).get()->getWidth();
}

JNI_METHOD(jlong, nativeGetHeight)(JNIEnv *env, jobject obj, jlong nativeRef) {
    return Image::native(nativeRef).get()->getHeight();
}

JNI_METHOD(void, nativeDestroyImage)(JNIEnv *env, jobject obj, jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROImageAndroid> *>(nativeRef);
}

} // extern "C"