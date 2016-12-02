//
//  Image_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>

#include "Image_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ImageJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateImage)(JNIEnv *env, jobject obj, jstring resource) {
    const char *cStrResource = env->GetStringUTFChars(resource, NULL);
    std::string strResource(cStrResource);
    std::shared_ptr<VROImageAndroid> imagePtr = std::make_shared<VROImageAndroid>(strResource);
    env->ReleaseStringUTFChars(resource, cStrResource);
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