//
//  VROTextureSubstrateOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTextureSubstrateOpenGL.h"
#include "VROTexture.h"
#include "VROData.h"
#include "VRODriverOpenGL.h"
#include "VROLog.h"

VROTextureSubstrateOpenGL::VROTextureSubstrateOpenGL(VROTextureType type,
                                                     VROTextureFormat format,
                                                     VROTextureInternalFormat internalFormat, bool sRGB,
                                                     VROMipmapMode mipmapMode,
                                                     std::vector<std::shared_ptr<VROData>> &data,
                                                     int width, int height,
                                                     const std::vector<uint32_t> &mipSizes,
                                                     VROWrapMode wrapS, VROWrapMode wrapT,
                                                     VROFilterMode minFilter, VROFilterMode magFilter, VROFilterMode mipFilter,
                                                     std::shared_ptr<VRODriverOpenGL> driver) :
    _owned(true),
    _driver(driver) {
    
    bool linearRenderingEnabled = driver->isLinearRenderingEnabled();
    driver->setActiveTextureUnit(GL_TEXTURE0);
    loadTexture(type, format, internalFormat, linearRenderingEnabled && sRGB, mipmapMode, data, width, height, mipSizes,
                wrapS, wrapT, minFilter, magFilter, mipFilter);
    ALLOCATION_TRACKER_ADD(TextureSubstrates, 1);
}

VROTextureSubstrateOpenGL::~VROTextureSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(TextureSubstrates, 1);

    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (_owned && driver) {
        driver->deleteTexture(_texture);
    }
}

void VROTextureSubstrateOpenGL::updateWrapMode(VROWrapMode wrapModeS, VROWrapMode wrapModeT) {
    GL( glActiveTexture(GL_TEXTURE0) );
    GL( glBindTexture(_target, _texture) );
    GL( glTexParameteri(_target, GL_TEXTURE_WRAP_S, convertWrapMode(wrapModeS)) );
    GL( glTexParameteri(_target, GL_TEXTURE_WRAP_T, convertWrapMode(wrapModeT)) );
    GL( glBindTexture(_target, 0) );
}

void VROTextureSubstrateOpenGL::loadTexture(VROTextureType type,
                                            VROTextureFormat format,
                                            VROTextureInternalFormat internalFormat, bool sRGB,
                                            VROMipmapMode mipmapMode,
                                            std::vector<std::shared_ptr<VROData>> &data,
                                            int width, int height,
                                            const std::vector<uint32_t> &mipSizes,
                                            VROWrapMode wrapS, VROWrapMode wrapT,
                                            VROFilterMode minFilter, VROFilterMode magFilter, VROFilterMode mipFilter) {
 
    _target = GL_TEXTURE_2D;
    
    GL( glGenTextures(1, &_texture) );
    
    if (type == VROTextureType::Texture2D) {
        GL( glBindTexture(GL_TEXTURE_2D, _texture) );
        
        GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, convertMinFilter(mipmapMode, minFilter, mipFilter)) );
        GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, convertMagFilter(magFilter)) );
        GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, convertWrapMode(wrapS)) );
        GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, convertWrapMode(wrapT)) );
        
        loadFace(GL_TEXTURE_2D, format, internalFormat, sRGB,
                 mipmapMode, data.front(), width, height, mipSizes);
    }
    else if (type == VROTextureType::TextureCube) {
        passert_msg (mipmapMode == VROMipmapMode::None,
                     "Cube textures should not be mipmapped!");
        passert_msg (data.size() == 6,
                     "Cube textures can only be created from exactly six images");
        
        _target = GL_TEXTURE_CUBE_MAP;
        
        GL( glBindTexture(GL_TEXTURE_CUBE_MAP, _texture) );
        GL( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, convertMinFilter(mipmapMode, minFilter, mipFilter)) );
        GL( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, convertMagFilter(magFilter)) );
        GL( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
        GL( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
        GL( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE) );
        
        for (int slice = 0; slice < 6; ++slice) {
            loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice, format, internalFormat, sRGB,
                     mipmapMode, data[slice], width, height, mipSizes);
        }
    }
    else {
        pabort("Invalid texture data received, could not convert to OpenGL");
    }
}

void VROTextureSubstrateOpenGL::loadFace(GLenum target,
                                         VROTextureFormat format,
                                         VROTextureInternalFormat internalFormat,
                                         bool sRGB,
                                         VROMipmapMode mipmapMode,
                                         std::shared_ptr<VROData> &faceData,
                                         int width, int height,
                                         const std::vector<uint32_t> &mipSizes) {
    
    if (format == VROTextureFormat::ETC2_RGBA8_EAC) {
        passert (mipmapMode != VROMipmapMode::Runtime);
        
        if (mipmapMode == VROMipmapMode::Pregenerated) {
            uint32_t offset = 0;
            
            GLenum internalFormat = sRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC : GL_COMPRESSED_RGBA8_ETC2_EAC;
            for (int level = 0; level < mipSizes.size(); level++) {
                uint32_t mipSize = mipSizes[level];
                GL( glCompressedTexImage2D(target, level, internalFormat,
                                           width >> level, height >> level, 0,
                                           mipSize, ((const char *)faceData->getData()) + offset) );
                offset += mipSize;
            }
        }
        else { // VROMipmapMode::None
            // Note the data received may have mipmaps, we just might not be using them
            // If no mipsizes are provided, though, then we just use the full data length
            GLenum internalFormat = sRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC : GL_COMPRESSED_RGBA8_ETC2_EAC;
            if (!mipSizes.empty()) {
                GL( glCompressedTexImage2D(target, 0, internalFormat, width, height, 0,
                                           mipSizes.front(), faceData->getData()) );
            }
            else {
                GL( glCompressedTexImage2D(target, 0, GL_COMPRESSED_RGBA8_ETC2_EAC, width, height, 0,
                                           faceData->getDataLength(), faceData->getData()) );
            }
        }
    }
    else if (format == VROTextureFormat::ASTC_4x4_LDR) {
        passert (mipmapMode == VROMipmapMode::None);
        GLenum internalFormat = sRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR : GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
        GL( glCompressedTexImage2D(target, 0, internalFormat, width, height, 0,
                                   faceData->getDataLength(), faceData->getData()) );
    }
    else if (format == VROTextureFormat::RGBA8 || format == VROTextureFormat::RGB8) {
        // We write format RGB8 into internal format RGBA8, because sRGB8 does not work
        // with automatic mipmap generation (it is not guaranteed color renderable)
        passert (mipmapMode != VROMipmapMode::Pregenerated);
        passert_msg (internalFormat != VROTextureInternalFormat::RGB565,
                     "RGB565 internal format requires RGB565 or RGB8 source data!");
        
        GL( glTexImage2D(target, 0, getInternalFormat(internalFormat, sRGB), width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, faceData->getData()) );
        if (mipmapMode == VROMipmapMode::Runtime) {
            GL( glGenerateMipmap(GL_TEXTURE_2D) );
        }
    }
    else if (format == VROTextureFormat::RGB9_E5) {
        // RGB9_E5 is not color renderable so automatic mipmap generation is not
        // supported
        passert (mipmapMode == VROMipmapMode::None);
        passert_msg (internalFormat == VROTextureInternalFormat::RGB9_E5,
                     "RGB9_E5 internal format requires RGB9_E5 source data!");
        
        GL( glTexImage2D(target, 0, GL_RGB9_E5, width, height, 0,
                         GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, faceData->getData()) );
    }
    else if (format == VROTextureFormat::RGB565) {
        passert_msg (internalFormat == VROTextureInternalFormat::RGB565,
                     "RGB565 source format is only compatible with RGB565 internal format!");

        GL( glTexImage2D(target, 0, getInternalFormat(internalFormat, sRGB), width, height, 0,
                         GL_RGB, GL_UNSIGNED_SHORT_5_6_5, faceData->getData()) );
        if (mipmapMode == VROMipmapMode::Runtime) {
            GL( glGenerateMipmap(GL_TEXTURE_2D) );
        }
    }
    else {
        pabort();
    }
}

GLuint VROTextureSubstrateOpenGL::getInternalFormat(VROTextureInternalFormat format, bool sRGB) {
    switch (format) {
        case VROTextureInternalFormat::RGBA8:
            return sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA;
        case VROTextureInternalFormat::RGBA4:
            return GL_RGBA4;
        case VROTextureInternalFormat::RGB565:
            return GL_RGB565;
        default:
            return GL_RGBA;
    }
}

GLenum VROTextureSubstrateOpenGL::convertWrapMode(VROWrapMode wrapMode) {
    switch (wrapMode) {
        case VROWrapMode::Clamp:
        case VROWrapMode::ClampToBorder:
            return GL_CLAMP_TO_EDGE;
        case VROWrapMode::Mirror:
            return GL_MIRRORED_REPEAT;
        default:
            return GL_REPEAT;
    }
}

GLenum VROTextureSubstrateOpenGL::convertMagFilter(VROFilterMode magFilter) {
  switch (magFilter) {
    case VROFilterMode::Nearest:
    case VROFilterMode::None:
      return GL_NEAREST;
    default:
      return GL_LINEAR;
  }
}

GLenum VROTextureSubstrateOpenGL::convertMinFilter(VROMipmapMode mipmapMode, VROFilterMode minFilter, VROFilterMode mipFilter) {
    if (minFilter == VROFilterMode::Nearest || minFilter == VROFilterMode::None) {
        if (mipmapMode != VROMipmapMode::None) {
            if (mipFilter == VROFilterMode::Linear) {
                return GL_NEAREST_MIPMAP_LINEAR;
            }
            else if (mipFilter == VROFilterMode::Nearest) {
                return GL_NEAREST_MIPMAP_NEAREST;
            }
            else {
                return GL_NEAREST;
            }
        }
        else {
            return GL_NEAREST;
        }
    }
    else if (minFilter == VROFilterMode::Linear) {
        if (mipmapMode != VROMipmapMode::None) {
            if (mipFilter == VROFilterMode::Linear) {
                return GL_LINEAR_MIPMAP_LINEAR;
            }
            else if (mipFilter == VROFilterMode::Nearest) {
                return GL_LINEAR_MIPMAP_NEAREST;
            }
            else {
                return GL_LINEAR;
            }
        }
        else {
            return GL_LINEAR;
        }
    }
    else {
        return GL_NEAREST;
    }
}
