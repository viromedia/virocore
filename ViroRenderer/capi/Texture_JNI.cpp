//
//  VideoTexture_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

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
#include "VROImageAndroid.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Texture_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Texture_##method_name
#endif

namespace Texture {

    VROTextureFormat getInputFormat(VRO_ENV env, VRO_STRING jformat) {
        std::string sformat = VRO_STRING_STL(jformat);

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
        std::string sformat = VRO_STRING_STL(jformat);
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
        std::string swrapMode = VRO_STRING_STL(jwrapMode);
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
        std::string sfilterMode = VRO_STRING_STL(jfilterMode);
        VROFilterMode ret = VROFilterMode::Linear;
        if (sfilterMode == "Nearest") {
            ret = VROFilterMode::Nearest;
        }
        return ret;
    }

    VROStereoMode getStereoMode(VRO_ENV env, VRO_STRING stereoMode) {
        if (!VRO_IS_STRING_EMPTY(stereoMode)) {
            std::string strStereoMode = VRO_STRING_STL(stereoMode);
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
            return VRO_OBJECT_NULL;
        }

        // Create a persistent native reference that would represent the jTexture object.
        PersistentRef<VROTexture> *persistentRef = new PersistentRef<VROTexture>(texture);
        VRO_REF(VROTexture) matRef = reinterpret_cast<VRO_REF(VROTexture)>(persistentRef);

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

VRO_METHOD(VRO_REF(VROTexture), nativeCreateRadianceHDRTexture)(VRO_ARGS_STATIC
                                                                VRO_STRING uri_j) {
    std::string uri = VRO_STRING_STL(uri_j);
    bool isTemp, success;
    std::string path = VROModelIOUtil::retrieveResource(uri, VROResourceType::URL, &isTemp, &success);
    std::shared_ptr<VROTexture> texture = VROHDRLoader::loadRadianceHDRTexture(path);
    if (isTemp) {
        VROPlatformDeleteFile(path);
    }

    VRO_REF(VROTexture) textureRef = -1;
    if (texture == nullptr){
        return textureRef;
    }

    textureRef = Texture::jptr(texture);
    return textureRef;
}

VRO_METHOD(VRO_REF(VROTexture), nativeCreateCubeTexture)(VRO_ARGS
                                                         VRO_REF(VROImage) px, VRO_REF(VROImage) nx,
                                                         VRO_REF(VROImage) py, VRO_REF(VROImage) ny,
                                                         VRO_REF(VROImage) pz, VRO_REF(VROImage) nz) {
    std::vector<std::shared_ptr<VROImage>> cubeImages = {Image::native(px),
                                                         Image::native(nx),
                                                         Image::native(py),
                                                         Image::native(ny),
                                                         Image::native(pz),
                                                         Image::native(nz)};
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(true, cubeImages);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(VRO_REF(VROTexture), nativeCreateImageTexture)(VRO_ARGS
                                                          VRO_REF(VROImage) image,
                                                          VRO_BOOL sRGB, VRO_BOOL mipmap, VRO_STRING stereoMode) {
    VRO_METHOD_PREAMBLE;
    VROStereoMode mode = Texture::getStereoMode(env, stereoMode);
    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          Image::native(image),
                                                                          mode);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(VRO_REF(VROTexture), nativeCreateCubeTextureBitmap)(VRO_ARGS
                                                               VRO_OBJECT px, VRO_OBJECT nx,
                                                               VRO_OBJECT py, VRO_OBJECT ny,
                                                               VRO_OBJECT pz, VRO_OBJECT nz,
                                                               VRO_STRING format_s) {
    VRO_METHOD_PREAMBLE;
    VROTextureInternalFormat format = Texture::getFormat(env, format_s);
    std::vector<std::shared_ptr<VROImage>> cubeImages;

#if VRO_PLATFORM_ANDROID
    cubeImages = {std::make_shared<VROImageAndroid>(px, format),
                  std::make_shared<VROImageAndroid>(nx, format),
                  std::make_shared<VROImageAndroid>(py, format),
                  std::make_shared<VROImageAndroid>(ny, format),
                  std::make_shared<VROImageAndroid>(pz, format),
                  std::make_shared<VROImageAndroid>(nz, format)};
#else
    //TODO Wasm
#endif

    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(true, cubeImages);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(VRO_REF(VROTexture), nativeCreateImageTextureBitmap)(VRO_ARGS
                                                                VRO_OBJECT bitmap,
                                                                VRO_STRING format_s, VRO_BOOL sRGB,
                                                                VRO_BOOL mipmap, VRO_STRING stereoMode) {
    VRO_METHOD_PREAMBLE;
    VROStereoMode mode = Texture::getStereoMode(env, stereoMode);
    VROTextureInternalFormat format = Texture::getFormat(env, format_s);

    std::shared_ptr<VROImage> image;
#if VRO_PLATFORM_ANDROID
    image = std::make_shared<VROImageAndroid>(bitmap, format);
#else
    //TODO Wasm
#endif

    std::shared_ptr<VROTexture> texturePtr = std::make_shared<VROTexture>(sRGB,
                                                                          mipmap ? VROMipmapMode::Runtime : VROMipmapMode::None,
                                                                          image, mode);
    return Texture::jptr(texturePtr);
}

VRO_METHOD(VRO_REF(VROTexture), nativeCreateImageTextureData)(VRO_ARGS
                                                              VRO_OBJECT jbuffer, VRO_INT width, VRO_INT height,
                                                              VRO_STRING inputFormat_s, VRO_STRING storageFormat_s,
                                                              VRO_BOOL sRGB, VRO_BOOL mipmap,
                                                              VRO_STRING stereoMode_s) {
    VRO_METHOD_PREAMBLE;
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

VRO_METHOD(VRO_REF(VROTexture), nativeCreateImageTextureVHD)(VRO_ARGS
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
                                           VRO_REF(VROTexture) texture_j) {
    std::shared_ptr<VROTexture> texture = Texture::native(texture_j);
    return texture->getWidth();
}

VRO_METHOD(VRO_INT, nativeGetTextureHeight)(VRO_ARGS
                                            VRO_REF(VROTexture) texture_j) {
    std::shared_ptr<VROTexture> texture = Texture::native(texture_j);
    return texture->getHeight();
}

VRO_METHOD(void, nativeSetWrapS)(VRO_ARGS
                                 VRO_REF(VROTexture) nativeRef,
                                 VRO_STRING wrapS) {
    VRO_METHOD_PREAMBLE;
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setWrapS(Texture::getWrapMode(env, wrapS));
}

VRO_METHOD(void, nativeSetWrapT)(VRO_ARGS
                                 VRO_REF(VROTexture) nativeRef,
                                 VRO_STRING wrapT) {
    VRO_METHOD_PREAMBLE;
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setWrapT(Texture::getWrapMode(env, wrapT));
}

VRO_METHOD(void, nativeSetMinificationFilter)(VRO_ARGS
                                              VRO_REF(VROTexture) nativeRef,
                                              VRO_STRING minFilter) {
    VRO_METHOD_PREAMBLE;
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMinificationFilter(Texture::getFilterMode(env, minFilter));
}

VRO_METHOD(void, nativeSetMagnificationFilter)(VRO_ARGS
                                               VRO_REF(VROTexture) nativeRef,
                                               VRO_STRING magFilter) {
    VRO_METHOD_PREAMBLE;
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMagnificationFilter(Texture::getFilterMode(env, magFilter));
}

VRO_METHOD(void, nativeSetMipFilter)(VRO_ARGS
                                     VRO_REF(VROTexture) nativeRef,
                                     VRO_STRING mipFilter) {
    VRO_METHOD_PREAMBLE;
    std::shared_ptr<VROTexture> texture = Texture::native(nativeRef);
    texture->setMipFilter(Texture::getFilterMode(env, mipFilter));
}

VRO_METHOD(void, nativeDestroyTexture)(VRO_ARGS
                                       VRO_REF(VROTexture) nativeRef) {
    delete reinterpret_cast<PersistentRef<VROTexture> *>(nativeRef);
}

} // extern "C"