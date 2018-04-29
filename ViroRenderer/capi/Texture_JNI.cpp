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

    VROTextureFormat getInputFormat(VRO_ENV env, VRO_STRING jformat) {
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

    VROTextureInternalFormat getFormat(VRO_ENV env, VRO_STRING jformat) {
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

    VROWrapMode getWrapMode(VRO_ENV env, VRO_STRING jwrapMode) {
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

    VROFilterMode getFilterMode(VRO_ENV env, VRO_STRING jfilterMode) {
        std::string sfilterMode = VROPlatformGetString(jfilterMode, env);
        VROFilterMode ret = VROFilterMode::Linear;
        if (sfilterMode == "Nearest") {
            ret = VROFilterMode::Nearest;
        }
        return ret;
    }

    VROStereoMode getStereoMode(VRO_ENV env, VRO_STRING stereoMode) {
        if (stereoMode != NULL) {
            std::string strStereoMode = VROPlatformGetString(stereoMode, env);
            return VROTextureUtil::getStereoModeForString(strStereoMode);
        }
        else {
            return VROStereoMode::None;
        }
    }

    void setWrapMode(VRO_ENV env, VRO_OBJECT jTex, const char *jMatFieldName,
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

        VROPlatformSetEnumValue(env, jTex, jMatFieldName, enumClassPathName, enumValueStr);
    }

    void setFilterMode(VRO_ENV env, VRO_OBJECT jTex, const char *jMatFieldName,
                       VROFilterMode mode) {
        std::string enumClassPathName = "com/viro/core/Texture$FilterMode";
        std::string enumValueStr;
        switch(mode) {
            case VROFilterMode::Nearest: enumValueStr = "NEAREST"; break;
            case VROFilterMode::Linear: enumValueStr = "LINEAR"; break;
            default:
                enumValueStr = "LINEAR";
        }

        VROPlatformSetEnumValue(env, jTex, jMatFieldName, enumClassPathName, enumValueStr);
    }

    VRO_OBJECT createJTexture(std::shared_ptr<VROTexture> texture) {
        VRO_ENV env = VROPlatformGetJNIEnv();
        if (env == nullptr) {
            perror("Required JNIEnv to create a jTexture is null!");
            return NULL;
        }

        // Create a persistent native reference that would represent the jTexture object.
        PersistentRef<VROTexture> *persistentRef = new PersistentRef<VROTexture>(texture);
        jlong matRef = reinterpret_cast<intptr_t>(persistentRef);

        // Create our Texture.java object with the native reference.
        VRO_OBJECT jTexture = VROPlatformConstructHostObject("com/viro/core/Texture", "(J)V", matRef);

        // Set visual properties of this Texture.java object and return it
        VROPlatformSetInt(env, jTexture, "mWidth", texture->getWidth());
        VROPlatformSetInt(env, jTexture, "mHeight", texture->getWidth());
        setWrapMode(env, jTexture, "mWrapS", texture->getWrapS());
        setWrapMode(env, jTexture, "mWrapT", texture->getWrapT());
        setFilterMode(env, jTexture, "mMinificationFilter", texture->getMinificationFilter());
        setFilterMode(env, jTexture, "mMagnificationFilter", texture->getMagnificationFilter());
        setFilterMode(env, jTexture, "mMipFilter", texture->getMipFilter());

        return jTexture;
    }
}

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateRadianceHDRTexture)(VRO_ARGS_STATIC
                                                    VRO_STRING uri_j) {
    std::string uri = VROPlatformGetString(uri_j, env);
    bool isTemp, success;
    std::string path = VROModelIOUtil::retrieveResource(uri, VROResourceType::URL, &isTemp, &success);
    std::shared_ptr<VROTexture> texture = VROHDRLoader::loadRadianceHDRTexture(path);
    if (isTemp) {
        VROPlatformDeleteFile(path);
    }

    VRO_REF textureRef = -1;
    if (texture == nullptr){
        return textureRef;
    }

    textureRef = Texture::jptr(texture);
    return textureRef;
}

VRO_METHOD(VRO_REF, nativeCreateCubeTexture)(VRO_ARGS
                                             VRO_REF px, VRO_REF nx,
                                             VRO_REF py, VRO_REF ny,
                                             VRO_REF pz, VRO_REF nz) {
    std::vector<std::shared_ptr<VROImage>> cubeImages = {Image::native(px),
                                                         Image::native(nx),
                                                         Image::native(py),
                                                         Image::native(ny),
                                                         Image::native(pz),
                                                         Image::native(nz)};
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(true, cubeImages);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(VRO_REF, nativeCreateImageTexture)(VRO_ARGS
                                              VRO_REF image,
                                              jboolean sRGB, jboolean mipmap, VRO_STRING stereoMode) {
    VROStereoMode mode = Texture::getStereoMode(env, stereoMode);
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          Image::native(image),
                                                                          mode);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(VRO_REF, nativeCreateCubeTextureBitmap)(VRO_ARGS
                                                   VRO_OBJECT px, VRO_OBJECT nx,
                                                   VRO_OBJECT py, VRO_OBJECT ny,
                                                   VRO_OBJECT pz, VRO_OBJECT nz,
                                                   VRO_STRING format_s) {

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

VRO_METHOD(VRO_REF, nativeCreateImageTextureBitmap)(VRO_ARGS
                                                    VRO_OBJECT bitmap,
                                                    VRO_STRING format_s, jboolean sRGB,
                                                    jboolean mipmap, VRO_STRING stereoMode) {

    VROStereoMode mode = Texture::getStereoMode(env, stereoMode);
    VROTextureInternalFormat format = Texture::getFormat(env, format_s);
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          std::make_shared<VROImageAndroid>(bitmap, format),
                                                                          mode);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(VRO_REF, nativeCreateImageTextureData)(VRO_ARGS
                                                  VRO_OBJECT jbuffer, VRO_INT width, VRO_INT height,
                                                  VRO_STRING inputFormat_s, VRO_STRING storageFormat_s,
                                                  jboolean sRGB, jboolean mipmap,
                                                  VRO_STRING stereoMode_s) {
    void *buffer = VRO_BUFFER_GET_ADDRESS(jbuffer);
    VRO_LONG capacity = VRO_BUFFER_GET_CAPACITY(jbuffer);

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

VRO_METHOD(VRO_REF, nativeCreateImageTextureVHD)(VRO_ARGS
                                                 VRO_OBJECT jbuffer, VRO_STRING stereoMode_s) {
    void *buffer = VRO_BUFFER_GET_ADDRESS(jbuffer);
    VRO_LONG capacity = VRO_BUFFER_GET_CAPACITY(jbuffer);

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

VRO_METHOD(VRO_INT, nativeGetTextureWidth)(VRO_ARGS
                                           VRO_REF texture_j) {
    std::shared_ptr<VROTexture> texture = Texture::native(texture_j);
    return texture->getWidth();
}

VRO_METHOD(VRO_INT, nativeGetTextureHeight)(VRO_ARGS
                                            VRO_REF texture_j) {
    std::shared_ptr<VROTexture> texture = Texture::native(texture_j);
    return texture->getHeight();
}

VRO_METHOD(void, nativeSetWrapS)(VRO_ARGS
                                 VRO_REF nativeRef, VRO_STRING wrapS) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setWrapS(Texture::getWrapMode(env, wrapS));
}

VRO_METHOD(void, nativeSetWrapT)(VRO_ARGS
                                 VRO_REF nativeRef, VRO_STRING wrapT) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setWrapT(Texture::getWrapMode(env, wrapT));
}

VRO_METHOD(void, nativeSetMinificationFilter)(VRO_ARGS
                                              VRO_REF nativeRef, VRO_STRING minFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMinificationFilter(Texture::getFilterMode(env, minFilter));
}

VRO_METHOD(void, nativeSetMagnificationFilter)(VRO_ARGS
                                               VRO_REF nativeRef, VRO_STRING magFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMagnificationFilter(Texture::getFilterMode(env, magFilter));
}

VRO_METHOD(void, nativeSetMipFilter)(VRO_ARGS
                                     VRO_REF nativeRef, VRO_STRING mipFilter) {
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMipFilter(Texture::getFilterMode(env, mipFilter));
}

VRO_METHOD(void, nativeDestroyTexture)(VRO_ARGS
                                       VRO_REF nativeRef) {
    delete reinterpret_cast<PersistentRef<VROTexture> *>(nativeRef);
}

} // extern "C"