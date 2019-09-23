//
//  VRORenderTargetOpenGL.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VRORenderTargetOpenGL_h
#define VRORenderTargetOpenGL_h

#include "VRORenderTarget.h"
#include "VROOpenGL.h"

class VRODriver;
class VRODriverOpenGL;

class VRORenderTargetOpenGL : public VRORenderTarget {
    
public:
    
    /*
     Create a new render-target of the given type. The number of images is only required
     for array types.
     */
    VRORenderTargetOpenGL(VRORenderTargetType type, int numAttachments, int numImages,
                          bool enableMipmaps, bool needsDepthStencil,
                          std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VRORenderTargetOpenGL();
    
#pragma mark - VRORenderTarget Implementation
    
    void bind();
    void bindRead();
    virtual void invalidate();
    virtual void blitColor(std::shared_ptr<VRORenderTarget> destination, bool flipY,
                           std::shared_ptr<VRODriver> driver);
    virtual void blitStencil(std::shared_ptr<VRORenderTarget> destination, bool flipY,
                             std::shared_ptr<VRODriver> driver);
    
    virtual bool setViewport(VROViewport viewport);
    virtual bool hydrate();
    int getWidth() const;
    int getHeight() const;
    
#pragma mark - Render Target Setup
    
    virtual bool hasTextureAttached(int attachment);
    virtual void clearTextures();
    virtual bool attachNewTextures();
    virtual void attachTexture(std::shared_ptr<VROTexture> texture, int attachment);
    virtual void setTextureImageIndex(int index, int attachment);
    virtual void setTextureCubeFace(int face, int mipLevel, int attachmentIndex);
    virtual void setMipLevel(int mipLevel, int attachmentIndex);
    virtual const std::shared_ptr<VROTexture> getTexture(int attachment) const;
    virtual void deleteFramebuffers();
    virtual bool restoreFramebuffers();
    
#pragma mark - Render Target Rendering
    
    void clearStencil();
    void clearDepth();
    void clearColor();
    void clearDepthAndColor();
    void enablePortalStencilWriting(VROFace face);
    void enablePortalStencilRemoval(VROFace face);
    void disablePortalStencilWriting(VROFace face);
    void setPortalStencilPassFunction(VROFace face, VROStencilFunc func, int ref);
    
protected:
    
    /*
     The OpenGL ES names for the framebuffer and depth/stencil buffer(s) used to render to
     this render-target. 0 for those that are not used.
     */
    GLuint _framebuffer, _depthStencilbuffer;
    
    /*
     The viewport of this target.
     */
    VROViewport _viewport;
    
private:
    
#pragma mark - Private
    
    /*
     The colorbuffers are used for pure offscreen rendering and for MSAA texture rendering.
     The textures are used for render-to-texture targets. 0 for those not used.
     */
    GLuint _colorbuffer;
    std::vector<std::shared_ptr<VROTexture>> _textures;
    
    /*
     If this is an array type, indicates the number of images in the texture.
     */
    int _numImages;
    
    /*
     If true, the color textures will have mipmap storage allocated so we can write to
     specific miplevels. To set the active miplevel use either setMipLevel or
     setTextureCubeFace.
     */
    bool _mipmapsEnabled;
    
    /*
     If true then this render target requires a depth/stencil renderbuffer. This can
     be safely set to false (in the constructor) to save memory for render targets
     that just perform color blitting.
     */
    bool _needsDepthStencil;
    
    /*
     The setting for passing the stencil test operation. These are determined by the
     active portal settings.
     */
    int _stencilRef;
    VROStencilFunc _stencilFunc;
    
    /*
     The storage type used for the depth/stencil renderbuffer (e.g. GL_DEPTH24_STENCIL8).
     */
    GLenum _depthStencilRenderbufferStorage;
    
    /*
     Get the underlying OpenGL target and texture name for the currently attached
     texture.
     */
    GLint getTextureName(int attachment) const;
    
    /*
     Get the attachment type used by the texture (e.g. GL_COLOR_ATTACHMENT0, etc.).
     */
    GLenum getTextureAttachmentType(int attachment) const;
    
    /*
     The driver that created this render target.
     */
    std::weak_ptr<VRODriverOpenGL> _driver;
    
    /*
     Create color and depth render buffers.
     */
    void createColorDepthRenderbuffers();
    
    /*
     Create a color render-to-texture target with a depth render buffer.
     */
    bool createColorTextureTarget();
    
    /*
     Create a depth render-to-texture target with a color render buffer.
     */
    bool createDepthTextureTarget();
    
    /*
     Blit the given attachment to the given destination target.
     */
    void blitAttachment(GLenum attachment, GLbitfield mask, GLenum filter,
                        std::shared_ptr<VRORenderTarget> destination,
                        bool flipY, std::shared_ptr<VRODriver> driver);
    
    
};

#endif /* VRORenderTargetOpenGL_h */
