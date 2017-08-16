//
//  VRORenderTargetOpenGL.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRORenderTargetOpenGL_h
#define VRORenderTargetOpenGL_h

#include "VRORenderTarget.h"
#include "VROOpenGL.h"

class VRODriverOpenGL;

class VRORenderTargetOpenGL : public VRORenderTarget {
    
public:
    
    /*
     Create a new render-target of the given type, with the given dimensions.
     */
    VRORenderTargetOpenGL(VRORenderTargetType type, std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VRORenderTargetOpenGL();
    
#pragma mark - VRORenderTarget Implementation
    
    void bind();
    
    virtual void setViewport(VROViewport viewport);
    int getWidth() const;
    int getHeight() const;
    
#pragma mark - Render Target Setup
    
    virtual bool hasTextureAttached();
    virtual void clearTexture();
    virtual void attachNewTexture();
    virtual void attachTexture(std::shared_ptr<VROTexture> texture);
    virtual const std::shared_ptr<VROTexture> getTexture() const;
    virtual void discardFramebuffers();
    virtual void restoreFramebuffers();
    
#pragma mark - Render Target Rendering
    
    void clearStencil(int bits);
    void clearDepth();
    void clearColor();
    void clearDepthAndColor();
    void enablePortalStencilWriting(VROFace face);
    void enablePortalStencilRemoval(VROFace face);
    void disablePortalStencilWriting(VROFace face);
    void setStencilPassBits(VROFace face, int bits, bool passIfLess);
    
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
     The colorbuffer is used for pure offscreen rendering and for MSAA texture rendering.
     The texture is used for render-to-texture targets. 0 for those not used.
     */
    GLuint _colorbuffer;
    std::shared_ptr<VROTexture> _texture;
    
    /*
     Get the underlying OpenGL target and texture name for the currently attached
     texture.
     */
    GLint getTextureName() const;
    
    /*
     Get the attachment type used by the texture (e.g. GL_COLOR_ATTACHMENT0, etc.).
     */
    GLenum getTextureAttachmentType() const;
    
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
    void createColorTextureTarget();
    
    /*
     Create a depth render-to-texture target with a color render buffer.
     */
    void createDepthTextureTarget();
    
};

#endif /* VRORenderTargetOpenGL_h */
