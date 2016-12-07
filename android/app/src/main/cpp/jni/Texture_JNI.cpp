//
//  VideoTexture_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>

#include "Image_JNI.h"
#include "Texture_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_TextureJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateCubeTexture)(JNIEnv *env, jobject obj,
                                       jlong px, jlong nx,
                                       jlong py, jlong ny,
                                       jlong pz, jlong nz) {
    std::vector<std::shared_ptr<VROImage>> cubeImages = {Image::native(px),
                                                         Image::native(nx),
                                                         Image::native(py),
                                                         Image::native(ny),
                                                         Image::native(pz),
                                                         Image::native(nz)};
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(cubeImages);
    return Texture::jptr(texturePtr);
}

JNI_METHOD(jlong, nativeCreateImageTexture)(JNIEnv *env, jobject obj, jlong image) {
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(Image::native(image));
    return Texture::jptr(texturePtr);
}

JNI_METHOD(void, nativeDestroyTexture)(JNIEnv *env, jobject obj, jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROTexture> *>(nativeRef);
}

} // extern "C"