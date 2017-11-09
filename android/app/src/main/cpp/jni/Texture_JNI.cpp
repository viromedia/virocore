//
//  VideoTexture_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VROTextureUtil.h>
#include "VROData.h"
#include <VROPlatformUtil.h>

#include "Image_JNI.h"
#include "VROPlatformUtil.h"
#include "Texture_JNI.h"
#include "VROCompress.h"
#include "VROLog.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Texture_##method_name

namespace Texture {

    VROTextureFormat getInputFormat(JNIEnv *env, jstring jformat) {
        std::string sformat = VROPlatformGetString(jformat, env);

        VROTextureFormat ret = VROTextureFormat::RGBA8;
        if (sformat == "RGB565") {
            ret = VROTextureFormat::RGB565;
        }
        else if (sformat == "RGB9_E5") {
            ret = VROTextureFormat::RGB9_E5;
        }

        return ret;
    }

    VROTextureInternalFormat getFormat(JNIEnv *env, jstring jformat) {
        std::string sformat = VROPlatformGetString(jformat, env);
        VROTextureInternalFormat ret = VROTextureInternalFormat::RGBA8;
        if (sformat == "RGBA4") {
            ret = VROTextureInternalFormat::RGBA4;
        }
        else if (sformat == "RGB565") {
            ret = VROTextureInternalFormat::RGB565;
        }
        else if (sformat == "RGB9_E5") {
            ret = VROTextureInternalFormat::RGB9_E5;
        }
        return ret;
    }

    VROWrapMode getWrapMode(JNIEnv *env, jstring jwrapMode) {
        std::string swrapMode = VROPlatformGetString(jwrapMode, env);
        VROWrapMode ret = VROWrapMode::Clamp;
        if (swrapMode == "Repeat") {
            ret = VROWrapMode::Repeat;
        }
        else if (swrapMode == "Mirror") {
            ret = VROWrapMode::Mirror;
        }
        return ret;
    }

    VROFilterMode getFilterMode(JNIEnv *env, jstring jfilterMode) {
        std::string sfilterMode = VROPlatformGetString(jfilterMode, env);
        VROFilterMode ret = VROFilterMode::Linear;
        if (sfilterMode == "Nearest") {
            ret = VROFilterMode::Nearest;
        }
        return ret;
    }

    VROStereoMode getStereoMode(JNIEnv *env, jstring stereoMode) {
        if (stereoMode != NULL) {
            const char *cStrStereoMode = env->GetStringUTFChars(stereoMode, NULL);
            std::string strStereoMode(cStrStereoMode);
            env->ReleaseStringUTFChars(stereoMode, cStrStereoMode);
            return VROTextureUtil::getStereoModeForString(strStereoMode);
        }
        else {
            return VROStereoMode::None;
        }
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
    VROStereoMode mode = Texture::getStereoMode(env, stereoMode);
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(Texture::getFormat(env, format), sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          Image::native(image),
                                                                          mode);
    return Texture::jptr(texturePtr);
}

JNI_METHOD(jlong, nativeCreateCubeTextureBitmap)(JNIEnv *env, jobject obj,
                                                 jobject px, jobject nx,
                                                 jobject py, jobject ny,
                                                 jobject pz, jobject nz,
                                                 jstring format_s) {

    VROTextureInternalFormat format = Texture::getFormat(env, format_s);
    std::vector<std::shared_ptr<VROImage>> cubeImages = {std::make_shared<VROImageAndroid>(px, format),
                                                         std::make_shared<VROImageAndroid>(nx, format),
                                                         std::make_shared<VROImageAndroid>(py, format),
                                                         std::make_shared<VROImageAndroid>(ny, format),
                                                         std::make_shared<VROImageAndroid>(pz, format),
                                                         std::make_shared<VROImageAndroid>(nz, format)};
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(format, true, cubeImages);
    return Texture::jptr(texturePtr);
}

JNI_METHOD(jlong, nativeCreateImageTextureBitmap)(JNIEnv *env, jobject obj, jobject bitmap,
                                                 jstring format_s, jboolean sRGB, jboolean mipmap, jstring stereoMode) {

    VROStereoMode mode = Texture::getStereoMode(env, stereoMode);
    VROTextureInternalFormat format = Texture::getFormat(env, format_s);
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(format, sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          std::make_shared<VROImageAndroid>(bitmap, format),
                                                                          mode);
    return Texture::jptr(texturePtr);
}

JNI_METHOD(jlong, nativeCreateImageTextureData)(JNIEnv *env, jobject obj, jobject jbuffer, jint width, jint height,
                                                jstring inputFormat_s, jstring storageFormat_s, jboolean sRGB, jboolean mipmap,
                                                jstring stereoMode_s) {
    void *buffer = env->GetDirectBufferAddress(jbuffer);
    jlong capacity = env->GetDirectBufferCapacity(jbuffer);

    std::shared_ptr<VROData> data = std::make_shared<VROData>(buffer, capacity);
    VROTextureFormat inputFormat = Texture::getInputFormat(env, inputFormat_s);
    VROTextureInternalFormat storageFormat = Texture::getFormat(env, storageFormat_s);
    VROStereoMode stereoMode = Texture::getStereoMode(env, stereoMode_s);

    std::vector<uint32_t> mipSizes;
    std::vector<std::shared_ptr<VROData>> dataVec = { data };
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(VROTextureType::Texture2D, inputFormat, storageFormat, sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          dataVec, width, height, mipSizes, stereoMode);
    return Texture::jptr(texturePtr);
}

JNI_METHOD(jlong, nativeCreateImageTextureVHD)(JNIEnv *env, jobject obj, jobject jbuffer, jstring stereoMode_s) {
    void *buffer = env->GetDirectBufferAddress(jbuffer);
    jlong capacity = env->GetDirectBufferCapacity(jbuffer);

    std::string data_gzip((char *)buffer, capacity);
    std::string data_texture = VROCompress::decompress(data_gzip);

    VROTextureFormat format;
    int texWidth;
    int texHeight;
    std::vector<uint32_t> mipSizes;
    std::shared_ptr<VROData> texData = VROTextureUtil::readVHDHeader(data_texture,
                                                                     &format, &texWidth, &texHeight, &mipSizes);
    std::vector<std::shared_ptr<VROData>> dataVec = { texData };
    std::shared_ptr<VROTexture> texture = std::make_shared<VROTexture>(VROTextureType::Texture2D,
                                                                       format,
                                                                       VROTextureInternalFormat::RGB9_E5,
                                                                       true,
                                                                       VROMipmapMode::None,
                                                                       dataVec, texWidth, texHeight,
                                                                       mipSizes);
    return Texture::jptr(texture);
}

JNI_METHOD(jint, nativeGetTextureWidth)(JNIEnv *env, jobject obj, jlong texture_j) {
    std::shared_ptr<VROTexture> texture = Texture::native(texture_j);
    return texture->getWidth();
}

JNI_METHOD(jint, nativeGetTextureHeight)(JNIEnv *env, jobject obj, jlong texture_j) {
    std::shared_ptr<VROTexture> texture = Texture::native(texture_j);
    return texture->getHeight();
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