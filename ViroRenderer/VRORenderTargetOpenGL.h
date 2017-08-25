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
     Create a new render-target of the given type. The number of images is only required
     for array types.
     */
    VRORenderTargetOpenGL(VRORenderTargetType type, int numAttachments, int numImages,
                          std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VRORenderTargetOpenGL();
    
#pragma mark - VRORenderTarget Implementation
    
    void bind();
    void blitColor(std::shared_ptr<VRORenderTarget> destination);
    
    virtual void setViewport(VROViewport viewport);
    int getWidth() const;
    int getHeight() const;
    
#pragma mark - Render Target Setup
    
    virtual bool hasTextureAttached(int attachment);
    virtual void clearTextures();
    virtual void attachNewTextures();
    virtual void attachTexture(std::shared_ptr<VROTexture> texture, int attachment);
    virtual void setTextureImageIndex(int index, int attachment);
    virtual const std::shared_ptr<VROTexture> getTexture(int attachment) const;
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
    void createColorTextureTarget();
    
    /*
     Create a depth render-to-texture target with a color render buffer.
     */
    void createDepthTextureTarget();
    
};

#endif /* VRORenderTargetOpenGL_h */
