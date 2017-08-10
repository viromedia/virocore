//
//  VROTextureSubstrateOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROTextureSubstrateOpenGL_h
#define VROTextureSubstrateOpenGL_h

#include "VROTextureSubstrate.h"
#include "VROOpenGL.h"
#include <memory>
#include <vector>
#include "VROAllocationTracker.h"

class VROData;
class VRODriver;
class VRODriverOpenGL;
enum class VROTextureType;
enum class VROTextureFormat;
enum class VROTextureInternalFormat;
enum class VROMipmapMode;
enum class VROWrapMode;
enum class VROFilterMode;

class VROTextureSubstrateOpenGL : public VROTextureSubstrate {
    
public:
    
    /*
     Create a new texture substrate with the given underlying OpenGL target
     and name. If owned is true, then the underlying texture will be deleted
     when this substrate is deleted.
     */
    VROTextureSubstrateOpenGL(GLenum target, GLuint name,
                              std::shared_ptr<VRODriverOpenGL> driver,
                              bool owned = true) :
        _target(target),
        _texture(name),
        _owned(owned),
        _driver(driver) {
        
        ALLOCATION_TRACKER_ADD(TextureSubstrates, 1);
    }
    
    /*
     Create a new OpenGL texture from the given source data, which is of the given 
     format. The generated texture will be stored on the GPU in the internalFormat
     (unless the source data is of a compressed type, in which case the data is stored
     as-is on the GPU).
     */
    VROTextureSubstrateOpenGL(VROTextureType type,
                              VROTextureFormat format,
                              VROTextureInternalFormat internalFormat,
                              VROMipmapMode mipmapMode,
                              std::vector<std::shared_ptr<VROData>> &data,
                              int width, int height,
                              const std::vector<uint32_t> &mipSizes,
                              VROWrapMode wrapS, VROWrapMode wrapT,
                              VROFilterMode minFilter, VROFilterMode magFilter, VROFilterMode mipFilter,
                              std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROTextureSubstrateOpenGL();
    
    std::pair<GLenum, GLuint> getTexture() const {
        return std::pair<GLenum, GLuint>(_target, _texture);
    }
    void setTexture(GLuint texture) {
        _texture = texture;
    }
    
private:
    
    GLenum _target;
    GLuint _texture;
    bool _owned;

    /*
     Weak reference to the driver that created this program. The driver's lifecycle
     is tied to the parent EGL context, so we only delete GL objects if the driver
     is alive, to ensure we're deleting them under the correct context.
    */
    std::weak_ptr<VRODriverOpenGL> _driver;
    
    void loadTexture(VROTextureType type,
                     VROTextureFormat format,
                     VROTextureInternalFormat internalFormat,
                     VROMipmapMode mipmapMode,
                     std::vector<std::shared_ptr<VROData>> &data,
                     int width, int height,
                     const std::vector<uint32_t> &mipSizes,
                     VROWrapMode wrapS, VROWrapMode wrapT,
                     VROFilterMode minFilter, VROFilterMode magFilter, VROFilterMode mipFilter);
    void loadFace(GLenum target,
                  VROTextureFormat format,
                  VROTextureInternalFormat internalFormat,
                  VROMipmapMode mipmapMode,
                  std::shared_ptr<VROData> &faceData,
                  int width, int height,
                  const std::vector<uint32_t> &mipSizes);
    
    GLuint getInternalFormat(VROTextureInternalFormat format);
    GLenum convertWrapMode(VROWrapMode wrapMode);
    GLenum convertMagFilter(VROFilterMode magFilter);
    GLenum convertMinFilter(VROMipmapMode mipmapMode, VROFilterMode minFilter, VROFilterMode mipFilter);
    
};

#endif /* VROTextureSubstrateOpenGL_h */
