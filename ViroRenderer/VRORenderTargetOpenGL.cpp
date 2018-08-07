//
//  VRORenderTargetOpenGL.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VRORenderTargetOpenGL.h"
#include "VROAllocationTracker.h"
#include "VROTexture.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VRODriverOpenGL.h"
#include "VROMaterial.h"
#include "VROViewport.h"
#include "VRODefines.h"

#ifdef VRO_PLATFORM_ANDROID
#define GL_COMPARE_REF_TO_TEXTURE                        0x884E
#define GL_DEPTH_COMPONENT24                             0x81A6
#define GL_TEXTURE_COMPARE_MODE                          0x884C
#define GL_TEXTURE_COMPARE_FUNC                          0x884D
#endif

VRORenderTargetOpenGL::VRORenderTargetOpenGL(VRORenderTargetType type, int numAttachments, int numImages,
                                             bool enableMipmaps, std::shared_ptr<VRODriverOpenGL> driver) :
    VRORenderTarget(type, numAttachments),
    _framebuffer(0),
    _depthStencilbuffer(0),
    _colorbuffer(0),
    _numImages(numImages),
    _mipmapsEnabled(enableMipmaps),
    _driver(driver),
    _stencilRef(0xFF),
    _stencilFunc(VROStencilFunc::Always) {
    _clearColor.set(0.0, 0.0, 0.0, 1.0);
        
    // Adreno330 or older does not support offscreen render targets
    if (type != VRORenderTargetType::Display) {
        passert (driver->getGPUType() != VROGPUType::Adreno330OrOlder);
    }
    for (int i = 0; i < numAttachments; i++) {
        _textures.push_back({});
    }
    ALLOCATION_TRACKER_ADD(RenderTargets, 1);
}

VRORenderTargetOpenGL::~VRORenderTargetOpenGL() {
    deleteFramebuffers();
    ALLOCATION_TRACKER_SUB(RenderTargets, 1);
}

void VRORenderTargetOpenGL::bind() {
    GL( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _framebuffer) );

    /*
     Bind the viewport and scissor when the render target changes. The scissor
     ensures we only clear (e.g. glClear) over the designated area; this is
     particularly important in VR mode where we have two 'eyes' each with a
     different viewport over the same framebuffer.
     */
    GL( glViewport(_viewport.getX(), _viewport.getY(), _viewport.getWidth(), _viewport.getHeight()) );
    GL( glScissor(_viewport.getX(), _viewport.getY(), _viewport.getWidth(), _viewport.getHeight()) );
    
    /*
     Prevent logical buffer load by immediately clearing.
     */
    GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) );
    GL( glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 0xFF, 0xFF) );
}

void VRORenderTargetOpenGL::bindRead() {
    GL( glBindFramebuffer(GL_READ_FRAMEBUFFER, _framebuffer) );
}

void VRORenderTargetOpenGL::invalidate() {
    switch (_type) {
        case VRORenderTargetType::ColorTexture:
        case VRORenderTargetType::ColorTextureRG16:
        case VRORenderTargetType::ColorTextureSRGB:
        case VRORenderTargetType::ColorTextureHDR16:
        case VRORenderTargetType::ColorTextureHDR32:
        case VRORenderTargetType::CubeTexture:
        case VRORenderTargetType::CubeTextureHDR16:
        case VRORenderTargetType::CubeTextureHDR32:
            GLenum attachments[2];
            attachments[0] = GL_DEPTH_ATTACHMENT;
            attachments[1] = GL_STENCIL_ATTACHMENT;
#if !VRO_PLATFORM_MACOS
            glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
#endif
            break;
            
        case VRORenderTargetType::DepthTexture:
        case VRORenderTargetType::DepthTextureArray:
            // Nothing to discard
            break;
            
        default:
            // Nothing to discard
            break;
    }
}

void VRORenderTargetOpenGL::blitColor(std::shared_ptr<VRORenderTarget> destination, bool flipY,
                                      std::shared_ptr<VRODriver> driver) {
    blitAttachment(GL_COLOR_ATTACHMENT0, GL_COLOR_BUFFER_BIT, GL_LINEAR, destination, flipY, driver);
}

void VRORenderTargetOpenGL::blitStencil(std::shared_ptr<VRORenderTarget> destination, bool flipY, std::shared_ptr<VRODriver> driver) {
    blitAttachment(GL_COLOR_ATTACHMENT0, GL_STENCIL_BUFFER_BIT, GL_NEAREST, destination, flipY, driver);
}

void VRORenderTargetOpenGL::blitAttachment(GLenum attachment, GLbitfield mask, GLenum filter,
                                           std::shared_ptr<VRORenderTarget> destination,
                                           bool flipY, std::shared_ptr<VRODriver> driver) {
    passert (_viewport.getWidth() == destination->getWidth());
    passert (_viewport.getHeight() == destination->getHeight());
    
    VRORenderTargetOpenGL *t = (VRORenderTargetOpenGL *) destination.get();
    GL( glBindFramebuffer(GL_READ_FRAMEBUFFER, _framebuffer) );
    GL( glReadBuffer(attachment) );
    GL( glDrawBuffers(1, &attachment) );
    
    if (flipY) {
        GL( glBlitFramebuffer(   _viewport.getX(),  _viewport.getY(), _viewport.getX() + _viewport.getWidth(), _viewport.getY() + _viewport.getHeight(),
                              t->_viewport.getX(), t->_viewport.getY() + t->_viewport.getHeight(), t->_viewport.getX() + t->_viewport.getWidth(), t->_viewport.getY(),
                              mask, filter) );
    }
    else {
        GL( glBlitFramebuffer(   _viewport.getX(),    _viewport.getY(),    _viewport.getX() +    _viewport.getWidth(),    _viewport.getY() +    _viewport.getHeight(),
                              t->_viewport.getX(), t->_viewport.getY(), t->_viewport.getX() + t->_viewport.getWidth(), t->_viewport.getY() + t->_viewport.getHeight(),
                              mask, filter) );
    }
}

bool VRORenderTargetOpenGL::setViewport(VROViewport viewport) {
    float previousWidth = _viewport.getWidth();
    float previousHeight = _viewport.getHeight();
    
    _viewport = viewport;

    // If size changed, recreate the target
    if (previousWidth  != viewport.getWidth() || previousHeight != viewport.getHeight()) {
        deleteFramebuffers();
        return restoreFramebuffers();
    }
    else {
        return true;
    }
}

int VRORenderTargetOpenGL::getWidth() const {
    return _viewport.getWidth();
}

int VRORenderTargetOpenGL::getHeight() const {
    return _viewport.getHeight();
}

#pragma mark - Texture Attachments

bool VRORenderTargetOpenGL::hasTextureAttached(int attachment) {
    return _textures.size() > attachment && _textures[attachment] != nullptr;
}

const std::shared_ptr<VROTexture> VRORenderTargetOpenGL::getTexture(int attachment) const {
    return _textures[attachment];
}

void VRORenderTargetOpenGL::clearTextures() {
    for (int i = 0; i < _textures.size(); i++) {
        GLenum attachment = getTextureAttachmentType(i);
        passert (attachment != 0);
        
        GL( glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
        GL( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0) );
    }
    for (std::shared_ptr<VROTexture> &texture : _textures) {
        texture.reset();
    }
}

void VRORenderTargetOpenGL::attachTexture(std::shared_ptr<VROTexture> texture, int attachmentIndex) {
    _textures[attachmentIndex] = texture;
    GLint name = getTextureName(attachmentIndex);
    GLenum attachment = getTextureAttachmentType(attachmentIndex);
    passert (attachment != 0);
    
    GL( glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
    if (_type == VRORenderTargetType::ColorTexture ||
        _type == VRORenderTargetType::ColorTextureRG16 ||
        _type == VRORenderTargetType::ColorTextureHDR16 ||
        _type == VRORenderTargetType::ColorTextureHDR32 ||
        _type == VRORenderTargetType::DepthTexture) {
        GL( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, name, 0) );
    }
    else if (_type == VRORenderTargetType::CubeTexture ||
             _type == VRORenderTargetType::CubeTextureHDR16 ||
             _type == VRORenderTargetType::CubeTextureHDR32) {
        GL( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X, name, 0) );
    }
    else if (_type == VRORenderTargetType::DepthTextureArray) {
        GL( glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, name, 0, 0) );
    }
    else {
        pabort();
    }
}

void VRORenderTargetOpenGL::setTextureImageIndex(int index, int attachmentIndex) {
    GLint name = getTextureName(attachmentIndex);
    GLenum attachment = getTextureAttachmentType(attachmentIndex);
    passert (attachment != 0);
    passert (_type == VRORenderTargetType::DepthTextureArray);
    
    GL( glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
    GL( glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, name, 0, index) );
}

void VRORenderTargetOpenGL::setTextureCubeFace(int face, int mipLevel, int attachmentIndex) {
    GLint name = getTextureName(attachmentIndex);
    GLenum attachment = getTextureAttachmentType(attachmentIndex);
    passert (attachment != 0);
    passert (_type == VRORenderTargetType::CubeTexture ||
             _type == VRORenderTargetType::CubeTextureHDR16 ||
             _type == VRORenderTargetType::CubeTextureHDR32);
    
    GL( glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
    GL( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                               name, mipLevel) );
    if (_mipmapsEnabled) {
        setMipLevel(mipLevel, attachmentIndex);
    }
}

void VRORenderTargetOpenGL::setMipLevel(int mipLevel, int attachmentIndex) {
    passert (_mipmapsEnabled);
    
    unsigned int mipWidth  = _viewport.getWidth()  * std::pow(0.5, mipLevel);
    unsigned int mipHeight = _viewport.getHeight() * std::pow(0.5, mipLevel);
    GL (glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilbuffer) );
    GL (glRenderbufferStorage(GL_RENDERBUFFER, _depthStencilRenderbufferStorage, mipWidth, mipHeight) );
    GL (glViewport(0, 0, mipWidth, mipHeight) );
}

bool VRORenderTargetOpenGL::attachNewTextures() {
    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (!driver) {
        pinfo("Failed to attach new render target textures: driver was released");
        return false;
    }
    
    if (_viewport.getWidth() <= 0 || _viewport.getHeight() <= 0) {
        pinfo("Failed to attach new render target textures: viewport was not set (width or height was 0)");
        return false;
    }
    
    if (_type == VRORenderTargetType::ColorTexture ||
        _type == VRORenderTargetType::ColorTextureRG16 ||
        _type == VRORenderTargetType::ColorTextureSRGB ||
        _type == VRORenderTargetType::ColorTextureHDR16 ||
        _type == VRORenderTargetType::ColorTextureHDR32) {
        
        GLenum target = GL_TEXTURE_2D;
        GLint internalFormat = GL_RGBA;
        GLint format = GL_RGBA;
        GLenum texType = GL_UNSIGNED_BYTE;

        if (_type == VRORenderTargetType::ColorTextureRG16) {
            internalFormat = GL_RG16F;
            format = GL_RG;
            texType = GL_FLOAT;
        }
        else if (_type == VRORenderTargetType::ColorTextureSRGB) {
            internalFormat = GL_SRGB8_ALPHA8;
        }
        else if (_type == VRORenderTargetType::ColorTextureHDR16) {
            internalFormat = GL_RGBA16F;
            texType = GL_FLOAT;
        }
        else if (_type == VRORenderTargetType::ColorTextureHDR32) {
            internalFormat = GL_RGBA32F;
            texType = GL_FLOAT;
        }
        
        GL (glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
        GLuint texNames[_numAttachments];
        GL (glGenTextures(_numAttachments, texNames) );
        
        for (int i = 0 ; i < _numAttachments; i++) {
            GL (glBindTexture(GL_TEXTURE_2D, texNames[i]) );
            GLenum attachment = getTextureAttachmentType(i);

            GL (glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
            GL (glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _mipmapsEnabled ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) );
            GL (glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
            GL (glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
            GL (glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _viewport.getWidth(), _viewport.getHeight(), 0, format, texType, nullptr) );
            if (_mipmapsEnabled) {
                // Allocates memory for the mipmaps
                GL (glGenerateMipmap(GL_TEXTURE_2D) );
            }
            GL (glBindTexture(GL_TEXTURE_2D, 0) );
            GL (glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texNames[i], 0) );
            
            std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(target, texNames[i], driver));
            _textures[i] = std::make_shared<VROTexture>(VROTextureType::Texture2D, VROTextureInternalFormat::RGBA8,
                                                        std::move(substrate));
        }
        
        /*
         Create a depth or depth/stencil renderbuffer, allocate storage for it, and attach it to
         the framebuffer's depth attachment point.
         */
        if (_depthStencilbuffer == 0) {
            _depthStencilRenderbufferStorage = GL_DEPTH24_STENCIL8;
            GL (glGenRenderbuffers(1, &_depthStencilbuffer) );
            GL (glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilbuffer) );
            GL (glRenderbufferStorage(GL_RENDERBUFFER, _depthStencilRenderbufferStorage, _viewport.getWidth(), _viewport.getHeight()) );
            
            GL (glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthStencilbuffer) );
            GL (glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilbuffer) );
        }
        
        /*
         Tell the system we're drawing to all attached color buffers.
         */
        GLuint attachments[_numAttachments];
        for (int i = 0; i < _numAttachments; i++) {
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        GL (glDrawBuffers(_numAttachments, attachments) );
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            pinfo("Failed to make complete resolve framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
                pinfo("   Incomplete attachment");
            }
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
                pinfo("   Incomplete dimensions");
            }
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
                pinfo("   Missing attachment");
            }
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_UNSUPPORTED) {
                pinfo("   Unsupported");
            }
            return false;
        }
    }
    else if (_type == VRORenderTargetType::CubeTexture ||
             _type == VRORenderTargetType::CubeTextureHDR16 ||
             _type == VRORenderTargetType::CubeTextureHDR32) {
        
        GLenum target = GL_TEXTURE_CUBE_MAP;
        GLint internalFormat = GL_RGBA;
        GLint format = GL_RGBA;
        GLenum texType = GL_UNSIGNED_BYTE;
        
        if (_type == VRORenderTargetType::CubeTextureHDR16) {
            internalFormat = GL_RGBA16F;
            texType = GL_FLOAT;
        }
        else if (_type == VRORenderTargetType::CubeTextureHDR32) {
            internalFormat = GL_RGBA32F;
            texType = GL_FLOAT;
        }
        
        GL (glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
        GLuint texNames[_numAttachments];
        GL (glGenTextures(_numAttachments, texNames) );
        
        for (int i = 0 ; i < _numAttachments; i++) {
            GL (glBindTexture(target, texNames[i]) );
            
            GL (glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
            GL (glTexParameterf(target, GL_TEXTURE_MIN_FILTER, _mipmapsEnabled ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) );
            GL (glTexParameterf(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
            GL (glTexParameterf(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
            GL (glTexParameterf(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE) );
            
            for (int s = 0; s < 6; s++) {
                GL( glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + s, 0, internalFormat, _viewport.getWidth(), _viewport.getHeight(),
                                 0, format, texType, nullptr) );
            }
            if (_mipmapsEnabled) {
                // Allocates memory for the mipmaps
                GL (glGenerateMipmap(target) );
            }
            
            GL (glBindTexture(target, 0) );            
            std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(target, texNames[i], driver));
            _textures[i] = std::make_shared<VROTexture>(VROTextureType::TextureCube, VROTextureInternalFormat::RGBA8,
                                                        std::move(substrate));
        }
        
        /*
         Create a depth or depth/stencil renderbuffer, allocate storage for it, and attach it to
         the framebuffer's depth attachment point.
         */
        if (_depthStencilbuffer == 0) {
            _depthStencilRenderbufferStorage = GL_DEPTH_COMPONENT24;
            GL (glGenRenderbuffers(1, &_depthStencilbuffer) );
            GL (glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilbuffer) );
            GL (glRenderbufferStorage(GL_RENDERBUFFER, _depthStencilRenderbufferStorage, _viewport.getWidth(), _viewport.getHeight()) );
            GL (glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthStencilbuffer) );
        }
        
        /*
         Tell the system we're drawing to all attached color buffers.
         */
        GLuint attachments[_numAttachments];
        for (int i = 0; i < _numAttachments; i++) {
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        GL (glDrawBuffers(_numAttachments, attachments) );
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            pinfo("Failed to make complete resolve framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
                pinfo("   Incomplete attachment");
            }
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
                pinfo("   Incomplete dimensions");
            }
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
                pinfo("   Missing attachment");
            }
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_UNSUPPORTED) {
                pinfo("   Unsupported");
            }
            return false;
        }
    }
    else if (_type == VRORenderTargetType::DepthTexture) {
        GLenum target = GL_TEXTURE_2D;
        GL (glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
        
        GLuint texName;
        GL (glGenTextures(1, &texName) );
        GL (glBindTexture(GL_TEXTURE_2D, texName) );
        
        // Setting a depth texture up with linear filtering means OpenGL
        // will use PCF on texture(sampler2DShadow) calls
        GL (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
        GL (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
        GL (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
        GL (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
        GL (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE) );
        GL (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL) );
        GL (glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _viewport.getWidth(), _viewport.getHeight(), 0,
                         GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0) );
        GL (glBindTexture(GL_TEXTURE_2D, 0) );
        GL (glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texName, 0) );
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            pinfo("Failed to make complete resolve depth framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            return false;
        }
        
        std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(target, texName, driver));
        _textures[0] = std::make_shared<VROTexture>(VROTextureType::Texture2D, VROTextureInternalFormat::RGBA8,
                                                    std::move(substrate));
    }
    else if (_type == VRORenderTargetType::DepthTextureArray) {
        GLenum target = GL_TEXTURE_2D_ARRAY;
        GL (glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
        
        GLuint texName;
        GL (glGenTextures(1, &texName) );
        GL (glBindTexture(GL_TEXTURE_2D_ARRAY, texName) );
        
        // Setting a depth texture up with linear filtering means OpenGL
        // will use PCF on texture(sampler2DShadow) calls
        GL (glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
        GL (glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
        GL (glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
        GL (glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
        GL (glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0) );
        GL (glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 1) );
        GL (glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE) );
        GL (glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL) );
        GL (glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, _viewport.getWidth(), _viewport.getHeight(),
                         _numImages, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0) );
        GL (glBindTexture(GL_TEXTURE_2D_ARRAY, 0) );
        GL (glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texName, 0, 0) );
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            pinfo("Failed to make complete resolve depth framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            return false;
        }
        
        std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(target, texName, driver));
        _textures[0] = std::make_shared<VROTexture>(VROTextureType::Texture2D, VROTextureInternalFormat::RGBA8,
                                                    std::move(substrate));
    }
    else {
        pinfo("FBO does not have a texture type, cannot create texture");
        return false;
    }
    
    return true;
}

GLint VRORenderTargetOpenGL::getTextureName(int attachment) const {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (!driver) {
        return 0;
    }
    if (!_textures[attachment]) {
        return 0;
    }
    
    VROTextureSubstrate *substrate = _textures[attachment]->getSubstrate(0, driver, true);
    VROTextureSubstrateOpenGL *oglSubstrate = dynamic_cast<VROTextureSubstrateOpenGL *>(substrate);
    passert (oglSubstrate != nullptr);
    
    std::pair<GLenum, GLuint> target_name = oglSubstrate->getTexture();
    return target_name.second;
}

GLenum VRORenderTargetOpenGL::getTextureAttachmentType(int attachment) const {
    switch (_type) {
        case VRORenderTargetType::ColorTexture:
        case VRORenderTargetType::ColorTextureRG16:
        case VRORenderTargetType::ColorTextureSRGB:
        case VRORenderTargetType::ColorTextureHDR16:
        case VRORenderTargetType::ColorTextureHDR32:
        case VRORenderTargetType::CubeTexture:
        case VRORenderTargetType::CubeTextureHDR16:
        case VRORenderTargetType::CubeTextureHDR32:
            return GL_COLOR_ATTACHMENT0 + attachment;
        case VRORenderTargetType::DepthTexture:
        case VRORenderTargetType::DepthTextureArray:
            return GL_DEPTH_ATTACHMENT;
        default:
            return 0;
    }
}

#pragma mark - Lifecycle

bool VRORenderTargetOpenGL::restoreFramebuffers() {
    switch (_type) {
        case VRORenderTargetType::Renderbuffer:
            createColorDepthRenderbuffers();
            return true;
        case VRORenderTargetType::ColorTexture:
        case VRORenderTargetType::ColorTextureRG16:
        case VRORenderTargetType::ColorTextureSRGB:
        case VRORenderTargetType::ColorTextureHDR16:
        case VRORenderTargetType::ColorTextureHDR32:
        case VRORenderTargetType::CubeTexture:
        case VRORenderTargetType::CubeTextureHDR16:
        case VRORenderTargetType::CubeTextureHDR32:
            return createColorTextureTarget();
            
        case VRORenderTargetType::DepthTexture:
        case VRORenderTargetType::DepthTextureArray:
            return createDepthTextureTarget();
            
        default:
            pinfo("Invalid render target type, cannot restore framebuffers");
            return false;
    }
    return true;
}

void VRORenderTargetOpenGL::deleteFramebuffers() {
    if (_framebuffer) {
        GL (glDeleteFramebuffers(1, &_framebuffer) );
        _framebuffer = 0;
    }
    if (_colorbuffer) {
        GL (glDeleteRenderbuffers(1, &_colorbuffer) );
        _colorbuffer = 0;
    }
    if (_depthStencilbuffer) {
        GL (glDeleteRenderbuffers(1, &_depthStencilbuffer) );
        _depthStencilbuffer = 0;
    }
    
    for (std::shared_ptr<VROTexture> &texture : _textures) {
        texture.reset();
    }
}

#pragma mark - Render Target Creation

void VRORenderTargetOpenGL::createColorDepthRenderbuffers() {
    passert_msg(_viewport.getWidth() > 0 && _viewport.getHeight() > 0,
                "Must invoke setViewport before using a render target");
    /*
     Create framebuffer.
     */
    GL (glGenFramebuffers(1, &_framebuffer) );
    GL (glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer) );
    
    /*
     Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer's color
     attachment point.
     */
    GLuint colorRenderbuffer;
    GL (glGenRenderbuffers(1, &colorRenderbuffer) );
    GL (glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer) );
    GL (glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, _viewport.getWidth(), _viewport.getHeight()) );
    GL (glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer) );
    
    /*
     Create a depth or depth/stencil renderbuffer, allocate storage for it, and attach it to the
     framebuffer's depth attachment point.
     */
    _depthStencilRenderbufferStorage = GL_DEPTH24_STENCIL8;
    GL (glGenRenderbuffers(1, &_depthStencilbuffer) );
    GL (glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilbuffer) );
    GL (glRenderbufferStorage(GL_RENDERBUFFER, _depthStencilRenderbufferStorage, _viewport.getWidth(), _viewport.getHeight()) );
    
    GL (glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthStencilbuffer) );
    GL (glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilbuffer) );
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        pinfo("Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
            pinfo("   Incomplete attachment");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
            pinfo("   Incomplete dimensions");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
            pinfo("   Missing attachment");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_UNSUPPORTED) {
            pinfo("   Unsupported");
        }
        pabort("Failed to create offscreen render buffer");
    }
}

bool VRORenderTargetOpenGL::createColorTextureTarget() {
    passert_msg(_viewport.getWidth() > 0 && _viewport.getHeight() > 0,
                "Must invoke setViewport before using a render target");
    /*
     Create framebuffer.
     */
    GL (glGenFramebuffers(1, &_framebuffer) );
    if (!attachNewTextures()) {
        pinfo("Failed to create color texture target: texture creation failed");
        GL (glDeleteFramebuffers(1, &_framebuffer) );
        return false;
    }
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        pinfo("Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
            pinfo("   Incomplete attachment");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
            pinfo("   Incomplete dimensions");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
            pinfo("   Missing attachment");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_UNSUPPORTED) {
            pinfo("   Unsupported");
        }
        pinfo("Failed to create color texture render target");
        GL (glDeleteFramebuffers(1, &_framebuffer) );
        return false;
    }
    return true;
}

bool VRORenderTargetOpenGL::createDepthTextureTarget() {
    passert_msg(_viewport.getWidth() > 0 && _viewport.getHeight() > 0,
                "Must invoke setViewport before using a render target");

    GL (glGenFramebuffers(1, &_framebuffer) );
    if (!attachNewTextures()) {
        pinfo("Failed to create depth texture target [width %d, height %d]: texture creation failed",
              _viewport.getWidth(), _viewport.getHeight());
        GL (glDeleteFramebuffers(1, &_framebuffer) );
        return false;
    }
    
    GLenum none[] = { GL_NONE };
    GL (glDrawBuffers(1, none) );
    GL (glReadBuffer(GL_NONE) );
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        pinfo("Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
            pinfo("   Incomplete attachment");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
            pinfo("   Incomplete dimensions");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
            pinfo("   Missing attachment");
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_UNSUPPORTED) {
            pinfo("   Unsupported");
        }
        pinfo("Failed to create depth texture render target");
        GL (glDeleteFramebuffers(1, &_framebuffer) );
        return false;
    }
    return true;
}

#pragma mark - Rendering Operations

void VRORenderTargetOpenGL::clearStencil(int bits)  {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        GL (glStencilMask(0xFF) );
        GL (glClearStencil(bits) );
        GL (glClear(GL_STENCIL_BUFFER_BIT) );
    }
    else {
        pabort();
    }
}

void VRORenderTargetOpenGL::clearDepth() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        driver->setDepthWritingEnabled(true);
        GL (glClear(GL_DEPTH_BUFFER_BIT) );
    }
    else {
        pabort();
    }
}

void VRORenderTargetOpenGL::clearColor() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        driver->setRenderTargetColorWritingMask(VROColorMaskAll);
        GL (glClearColor(_clearColor.x, _clearColor.y, _clearColor.z, _clearColor.w) );
        GL (glClear(GL_COLOR_BUFFER_BIT) );
    }
    else {
        pabort();
    }
}

void VRORenderTargetOpenGL::clearDepthAndColor() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        driver->setDepthWritingEnabled(true);
        driver->setRenderTargetColorWritingMask(VROColorMaskAll);
        GL (glClearColor(_clearColor.x, _clearColor.y, _clearColor.z, _clearColor.w) );
        GL (glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
    }
    else {
        pabort();
    }
}

GLenum toGL(VROFace face) {
    switch (face) {
        case VROFace::Front:
            return GL_FRONT;
        case VROFace::Back:
            return GL_BACK;
        default:
            return GL_FRONT_AND_BACK;
    }
}

GLenum toGL(VROStencilFunc func) {
    switch (func) {
        case VROStencilFunc::Never:
            return GL_NEVER;
        case VROStencilFunc::Less:
            return GL_LESS;
        case VROStencilFunc::LessOrEqual:
            return GL_LEQUAL;
        case VROStencilFunc::Greater:
            return GL_GREATER;
        case VROStencilFunc::GreaterOrEqual:
            return GL_GEQUAL;
        case VROStencilFunc::Equal:
            return GL_EQUAL;
        case VROStencilFunc::NotEqual:
            return GL_NOTEQUAL;
        case VROStencilFunc::Always:
            return GL_ALWAYS;
        default:
            return GL_ALWAYS;
    }
}

void VRORenderTargetOpenGL::enablePortalStencilWriting(VROFace face) {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        driver->setStencilTestEnabled(true);
        GL (glStencilOpSeparate(toGL(face), GL_KEEP, GL_KEEP, GL_INCR) );   // Increment stencil buffer when pass
        GL (glStencilMaskSeparate(toGL(face), 0x0F) );                      // Allow writing to the lower four bits in stencil buffer
    }
}

void VRORenderTargetOpenGL::enablePortalStencilRemoval(VROFace face) {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        driver->setStencilTestEnabled(true);
        GL (glStencilOpSeparate(toGL(face), GL_KEEP, GL_KEEP, GL_DECR) );   // Decrement stencil buffer when pass
        GL (glStencilMaskSeparate(toGL(face), 0x0F) );                      // Allow writing to the lower four bits in stencil buffer
    }
}

void VRORenderTargetOpenGL::disablePortalStencilWriting(VROFace face) {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        driver->setStencilTestEnabled(true);
        GL( glStencilOpSeparate(toGL(face), GL_KEEP, GL_KEEP, GL_KEEP) );   // Do not write to stencil buffer
    }
}

void VRORenderTargetOpenGL::setPortalStencilPassFunction(VROFace face, VROStencilFunc func, int ref) {
    // The bits set here are for the portal stencil ref; e.g. only the bottom four bits. The upper
    // four bits are kept as 1111; they are used for controlling other masking operations.
    _stencilRef = (0xF0 | (0x0F & ref));
    _stencilFunc = func;
    
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        driver->setStencilTestEnabled(true);
        GL( glStencilFuncSeparate(toGL(face), toGL(_stencilFunc), _stencilRef, 0x0F) );
    }
}
