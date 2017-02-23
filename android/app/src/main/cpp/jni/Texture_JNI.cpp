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
#include "VROLog.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_TextureJni_##method_name

namespace Texture {
    VROTextureInternalFormat getFormat(JNIEnv *env, jstring jformat) {
        const char *format = env->GetStringUTFChars(jformat, 0);
        std::string sformat(format);

        VROTextureInternalFormat ret = VROTextureInternalFormat::RGBA8;
        if (sformat == "RGBA4") {
            ret = VROTextureInternalFormat::RGBA4;
        }
        else if (sformat == "RGB565") {
            ret = VROTextureInternalFormat::RGB565;
        }
        env->ReleaseStringUTFChars(jformat, format);
        return ret;
    }
}

extern "C" {

JNI_METHOD(jlong, nativeCreateCubeTexture)(JNIEnv *env, jobject obj,
                                           jlong px, jlong nx,
                                           jlong py, jlong ny,
                                           jlong pz, jlong nz,
                                           jstring format) {
    std::vector<std::shared_ptr<VROImage>> cubeImages = {Image::native(px),
                                                         Image::native(nx),
                                                         Image::native(py),
                                                         Image::native(ny),
                                                         Image::native(pz),
                                                         Image::native(nz)};
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(Texture::getFormat(env, format),
                                                                          cubeImages);
    return Texture::jptr(texturePtr);
}

JNI_METHOD(jlong, nativeCreateImageTexture)(JNIEnv *env, jobject obj, jlong image,
                                            jstring format, jboolean mipmap) {
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(Texture::getFormat(env, format),
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          Image::native(image));
    return Texture::jptr(texturePtr);
}

JNI_METHOD(void, nativeDestroyTexture)(JNIEnv *env, jobject obj, jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROTexture> *>(nativeRef);
}

} // extern "C"