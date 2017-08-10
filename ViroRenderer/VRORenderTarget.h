//
//  VRORenderTarget.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRORenderTarget_h
#define VRORenderTarget_h

#include <memory>
#include "VROVector4f.h"

class VROTexture;

/*
 Renderbuffers should be used when we do *not* need to sample the output of a
 render target. Textures should be used when we do need to read back the output.
 */
enum class VRORenderTargetType {
    Display,       // The actual backbuffer
    Renderbuffer,  // Uses depth and color renderbuffers
    ColorTexture,  // Uses a color texture and a depth renderbuffer
    DepthTexture   // Uses a depth texture and a color renderbuffer
};

enum class VROFace {
    Front,
    Back,
    FrontAndBack
};

/*
 Represents a render target, with any number of color, stencil, or depth attachments.
 In OpenGL, this is represented by an FBO.
 */
class VRORenderTarget  {
public:
    
#pragma mark - Initializtion
    
    /*
     Create a new render-target of the given type.
     */
    VRORenderTarget(VRORenderTargetType type) :
        _type(type) {
    }
    virtual ~VRORenderTarget() {}
    
    /*
     Initialize or resize this render target to the given width and height. This
     may invalidate the current texture. This must be invoked before using the
     render target.
     */
    virtual void setSize(int width, int height) = 0;
    
    /*
     Get the dimensions of this render target.
     */
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    
    /*
     Set the clear color to use for this render target.
     */
    void setClearColor(VROVector4f color) {
        _clearColor = color;
    }
    
    /*
     Bind this render-target.
     */
    virtual void bind() = 0;
    
    /*
     Discard all existing framebuffers.
     */
    virtual void discardFramebuffers() = 0;
    
    /*
     Restores the frame-buffer if the context was lost.
     */
    virtual void restoreFramebuffers() = 0;
    
#pragma mark - Render to Texture Setup
    
    /*
     Check if this render target has a texture attached.
     */
    virtual bool hasTextureAttached() = 0;
    
    /*
     Clear the texture in this render target. The texture will not be deleted.
     */
    virtual void clearTexture() = 0;
    
    /*
     Create and attach a new texture to this render-target. The texture will be
     sized to fit the render target. The old texture is not deleted, but this
     render-target's reference to it is released.
     */
    virtual void attachNewTexture() = 0;
    
    /*
     Attach the given texture to this render-target. The width and height of the texture
     must match that of the render-target.
     */
    virtual void attachTexture(std::shared_ptr<VROTexture> texture) = 0;
    
    /*
     Get the texture to which this render-target is rendering. Returns nullptr if
     this is not a render-to-texture target.
     */
    virtual const std::shared_ptr<VROTexture> getTexture() const = 0;
    
#pragma mark - Rendering Operations
    
    /*
     Clear the stencil buffer with the given clear value.
     */
    virtual void clearStencil(int bits) = 0;
    
    /*
     Clear the depth buffer.
     */
    virtual void clearDepth() = 0;
    
    /*
     Clear the color buffer.
     */
    virtual void clearColor() = 0;
    
    /*
     Clear both depth and color at the same time.
     */
    virtual void clearDepthAndColor() = 0;
    
    /*
     Enable the color buffer for writing.
     */
    virtual void enableColorBuffer() = 0;
    
    /*
     Disable the color buffer from writing.
     */
    virtual void disableColorBuffer() = 0;
    
    /*
     Enable portal stencil functions. When writing, we INCR the stencil
     buffer. When removing, we DECR the buffer. Finally when reading, we
     make the stencil buffer read-only.
     */
    virtual void enablePortalStencilWriting(VROFace face) = 0;
    virtual void enablePortalStencilRemoval(VROFace face) = 0;
    virtual void disablePortalStencilWriting(VROFace face) = 0;
    
    /*
     Set the reference bits for the stencil test. If passIfLess is
     false, we pass the stencil test if ref equals the value in the
     stencil buffer. If passIsLess is true, we pass the stencil test
     if ref <= value in stencil buffer.
     */
    virtual void setStencilPassBits(VROFace face, int bits, bool passIfLess) = 0;
    
protected:
    
    /*
     The type of render target (to texture, or offscreen).
     */
    VRORenderTargetType _type;
    
    /*
     The clear color of this render target.
     */
    VROVector4f _clearColor;
    
};

#endif /* VRORenderTarget_h */
