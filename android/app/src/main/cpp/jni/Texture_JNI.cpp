//
//  VideoTexture_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VROTextureUtil.h>

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

    VROWrapMode getWrapMode(JNIEnv *env, jstring jwrapMode) {
        const char *wrapMode = env->GetStringUTFChars(jwrapMode, 0);
        std::string swrapMode(wrapMode);

        VROWrapMode ret = VROWrapMode::Clamp;
        if (swrapMode == "Repeat") {
            ret = VROWrapMode::Repeat;
        }
        else if (swrapMode == "Mirror") {
            ret = VROWrapMode::Mirror;
        }
        env->ReleaseStringUTFChars(jwrapMode, wrapMode);
        return ret;
    }

    VROFilterMode getFilterMode(JNIEnv *env, jstring jfilterMode) {
        const char *filterMode = env->GetStringUTFChars(jfilterMode, 0);
        std::string sfilterMode(filterMode);

        VROFilterMode ret = VROFilterMode::Linear;
        if (sfilterMode == "Nearest") {
            ret = VROFilterMode::Nearest;
        }
        env->ReleaseStringUTFChars(jfilterMode, filterMode);
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
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(Texture::getFormat(env, format), true,
                                                                          cubeImages);
    return Texture::jptr(texturePtr);
}

JNI_METHOD(jlong, nativeCreateImageTexture)(JNIEnv *env, jobject obj, jlong image,
                                            jstring format, jboolean sRGB, jboolean mipmap, jstring stereoMode) {

    VROStereoMode mode = VROStereoMode::None;
    if (stereoMode != NULL) {
        const char *cStrStereoMode = env->GetStringUTFChars(stereoMode, NULL);
        std::string strStereoMode(cStrStereoMode);
        env->ReleaseStringUTFChars(stereoMode, cStrStereoMode);
        mode = VROTextureUtil::getStereoModeForString(strStereoMode);
    }

    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(Texture::getFormat(env, format), sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          Image::native(image),
                                                                          mode);
    return Texture::jptr(texturePtr);
}

JNI_METHOD(void, nativeSetWrapS)(JNIEnv *env, jobject obj, jlong nativeRef, jstring wrapS) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setWrapS(Texture::getWrapMode(env, wrapS));
}

JNI_METHOD(void, nativeSetWrapT)(JNIEnv *env, jobject obj, jlong nativeRef, jstring wrapT) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setWrapT(Texture::getWrapMode(env, wrapT));
}

JNI_METHOD(void, nativeSetMinificationFilter)(JNIEnv *env, jobject obj, jlong nativeRef, jstring minFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMinificationFilter(Texture::getFilterMode(env, minFilter));
}

JNI_METHOD(void, nativeSetMagnificationFilter)(JNIEnv *env, jobject obj, jlong nativeRef, jstring magFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMagnificationFilter(Texture::getFilterMode(env, magFilter));
}

JNI_METHOD(void, nativeSetMipFilter)(JNIEnv *env, jobject obj, jlong nativeRef, jstring mipFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMipFilter(Texture::getFilterMode(env, mipFilter));
}

JNI_METHOD(void, nativeDestroyTexture)(JNIEnv *env, jobject obj, jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROTexture> *>(nativeRef);
}

} // extern "C"