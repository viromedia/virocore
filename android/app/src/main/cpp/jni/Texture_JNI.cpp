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
#include "VROHDRLoader.h"
#include "Image_JNI.h"
#include "VROPlatformUtil.h"
#include "Texture_JNI.h"
#include "VROCompress.h"
#include "VROLog.h"
#include "VROModelIOUtil.h"
#include "VROPlatformUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Texture_##method_name
#endif

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
            std::string strStereoMode = VROPlatformGetString(stereoMode, env);
            return VROTextureUtil::getStereoModeForString(strStereoMode);
        }
        else {
            return VROStereoMode::None;
        }
    }

    void setWrapMode(JNIEnv *env, jclass cls, jobject jTex, const char *jMatFieldName,
                     VROWrapMode mode) {
        std::string enumClassPathName = "com/viro/core/Texture$WrapMode";
        std::string enumValueStr;
        switch(mode) {
            case VROWrapMode::Clamp: enumValueStr = "CLAMP"; break;
            case VROWrapMode::Repeat: enumValueStr = "REPEAT"; break;
            case VROWrapMode::Mirror: enumValueStr = "MIRROR"; break;
            default:
                enumValueStr = "CLAMP";
        }

        VROPlatformSetEnumValue(env, cls, jTex, jMatFieldName, enumClassPathName, enumValueStr);
    }

    void setFilterMode(JNIEnv *env, jclass cls, jobject jTex, const char *jMatFieldName,
                       VROFilterMode mode) {
        std::string enumClassPathName = "com/viro/core/Texture$FilterMode";
        std::string enumValueStr;
        switch(mode) {
            case VROFilterMode::Nearest: enumValueStr = "NEAREST"; break;
            case VROFilterMode::Linear: enumValueStr = "LINEAR"; break;
            default:
                enumValueStr = "LINEAR";
        }

        VROPlatformSetEnumValue(env, cls, jTex, jMatFieldName, enumClassPathName, enumValueStr);
    }

    jobject createJTexture(std::shared_ptr<VROTexture> texture) {
        JNIEnv *env = VROPlatformGetJNIEnv();
        if (env == nullptr) {
            perror("Required JNIEnv to create a jTexture is null!");
            return NULL;
        }

        // Create a persistent native reference that would represent the jTexture object.
        PersistentRef<VROTexture> *persistentRef = new PersistentRef<VROTexture>(texture);
        jlong matRef = reinterpret_cast<intptr_t>(persistentRef);
        jclass cls = env->FindClass("com/viro/core/Texture");
        if (cls == nullptr) {
            perror("Required Texture.java to create a jTexture is not defined!");
            return NULL;
        }

        // Create our Texture.java object with the native reference.
        jmethodID jmethod = env->GetMethodID(cls, "<init>", "(J)V");
        jobject jTexture = env->NewObject(cls, jmethod, matRef);

        // Set visual properties of this Texture.java object and return it
        VROPlatformSetInt(env, cls, jTexture, "mWidth", texture->getWidth());
        VROPlatformSetInt(env, cls, jTexture, "mHeight", texture->getWidth());
        setWrapMode(env, cls, jTexture, "mWrapS", texture->getWrapS());
        setWrapMode(env, cls, jTexture, "mWrapT", texture->getWrapT());
        setFilterMode(env, cls, jTexture, "mMinificationFilter", texture->getMinificationFilter());
        setFilterMode(env, cls, jTexture, "mMagnificationFilter", texture->getMagnificationFilter());
        setFilterMode(env, cls, jTexture, "mMipFilter", texture->getMipFilter());

        env->DeleteLocalRef(cls);
        return jTexture;
    }
}

extern "C" {

VRO_METHOD(jlong, nativeCreateRadianceHDRTexture)(VRO_ARGS_STATIC
                                                  jstring uri_j) {
    std::string uri = VROPlatformGetString(uri_j, env);
    bool isTemp, success;
    std::string path = VROModelIOUtil::retrieveResource(uri, VROResourceType::URL, &isTemp, &success);
    std::shared_ptr<VROTexture> texture = VROHDRLoader::loadRadianceHDRTexture(path);
    if (isTemp) {
        VROPlatformDeleteFile(path);
    }

    jlong textureRef = -1;
    if (texture == nullptr){
        return textureRef;
    }

    textureRef = Texture::jptr(texture);
    return textureRef;
}

VRO_METHOD(jlong, nativeCreateCubeTexture)(VRO_ARGS
                                           jlong px, jlong nx,
                                           jlong py, jlong ny,
                                           jlong pz, jlong nz) {
    std::vector<std::shared_ptr<VROImage>> cubeImages = {Image::native(px),
                                                         Image::native(nx),
                                                         Image::native(py),
                                                         Image::native(ny),
                                                         Image::native(pz),
                                                         Image::native(nz)};
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(true, cubeImages);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(jlong, nativeCreateImageTexture)(VRO_ARGS
                                            jlong image,
                                            jboolean sRGB, jboolean mipmap, jstring stereoMode) {
    VROStereoMode mode = Texture::getStereoMode(env, stereoMode);
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          Image::native(image),
                                                                          mode);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(jlong, nativeCreateCubeTextureBitmap)(VRO_ARGS
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
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(true, cubeImages);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(jlong, nativeCreateImageTextureBitmap)(VRO_ARGS
                                                  jobject bitmap,
                                                  jstring format_s, jboolean sRGB,
                                                  jboolean mipmap, jstring stereoMode) {

    VROStereoMode mode = Texture::getStereoMode(env, stereoMode);
    VROTextureInternalFormat format = Texture::getFormat(env, format_s);
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          std::make_shared<VROImageAndroid>(bitmap, format),
                                                                          mode);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(jlong, nativeCreateImageTextureData)(VRO_ARGS
                                                jobject jbuffer, jint width, jint height,
                                                jstring inputFormat_s, jstring storageFormat_s,
                                                jboolean sRGB, jboolean mipmap,
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

VRO_METHOD(jlong, nativeCreateImageTextureVHD)(VRO_ARGS
                                               jobject jbuffer, jstring stereoMode_s) {
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

VRO_METHOD(jint, nativeGetTextureWidth)(VRO_ARGS
                                        jlong texture_j) {
    std::shared_ptr<VROTexture> texture = Texture::native(texture_j);
    return texture->getWidth();
}

VRO_METHOD(jint, nativeGetTextureHeight)(VRO_ARGS
                                         jlong texture_j) {
    std::shared_ptr<VROTexture> texture = Texture::native(texture_j);
    return texture->getHeight();
}

VRO_METHOD(void, nativeSetWrapS)(VRO_ARGS
                                 jlong nativeRef, jstring wrapS) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setWrapS(Texture::getWrapMode(env, wrapS));
}

VRO_METHOD(void, nativeSetWrapT)(VRO_ARGS
                                 jlong nativeRef, jstring wrapT) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setWrapT(Texture::getWrapMode(env, wrapT));
}

VRO_METHOD(void, nativeSetMinificationFilter)(VRO_ARGS
                                              jlong nativeRef, jstring minFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMinificationFilter(Texture::getFilterMode(env, minFilter));
}

VRO_METHOD(void, nativeSetMagnificationFilter)(VRO_ARGS
                                               jlong nativeRef, jstring magFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMagnificationFilter(Texture::getFilterMode(env, magFilter));
}

VRO_METHOD(void, nativeSetMipFilter)(VRO_ARGS
                                     jlong nativeRef, jstring mipFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMipFilter(Texture::getFilterMode(env, mipFilter));
}

VRO_METHOD(void, nativeDestroyTexture)(VRO_ARGS
                                       jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROTexture> *>(nativeRef);
}

} // extern "C"