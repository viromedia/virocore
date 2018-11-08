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
#include "VROViewport.h"
#include "VROLog.h"

class VROTexture;
class VRODriver;
enum class VROFace;

/*
 Renderbuffers should be used when we do *not* need to sample the output of a
 render target. Textures should be used when we do need to read back the output.
 
 The sRGB formats will make the framebuffer convert from linear RGB to sRGB
 (gamma 2.0) on all writes. This typically is only necessary on the final render pass,
 since we want most color operations to occur in linear space.
 
 Note: on iOS the sRGB render targets do not appear to be properly handling the
       encoding
 */
enum class VRORenderTargetType {
    Display,            // The actual backbuffer
    Renderbuffer,       // Uses depth and color renderbuffers
    ColorTextureRG16,   // Uses a color texture and converts to RG space on write
    ColorTexture,       // Uses a color texture and a depth renderbuffer
    ColorTextureSRGB,   // Uses a color texture and converts to sRGB space on write
    ColorTextureHDR16,  // Uses a Float16 color texture and a depth renderbuffer
    ColorTextureHDR32,  // Uses a Float32 color texture and a depth renderbuffer
    DepthTexture,       // Uses a depth texture and no color buffer
    DepthTextureArray,  // Uses a depth texture array no color buffer
    CubeTexture,        // Uses a color texture and a depth renderbuffer
    CubeTextureHDR16,   // Uses a Float16 color texture and a depth renderbuffer
    CubeTextureHDR32,   // Uses a Float32 color texture and a depth renderbuffer
};

/*
 Possible stencil functions. Stenciling is owned by the render target.
 */
enum class VROStencilFunc {
    Never,
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
    Equal,
    NotEqual,
    Always
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
    VRORenderTarget(VRORenderTargetType type, int numAttachments) :
        _type(type),
        _numAttachments(numAttachments) {
            
        if (numAttachments > 1 &&
            (type == VRORenderTargetType::DepthTexture || type == VRORenderTargetType::DepthTextureArray)) {
            pabort("Only one attachment is supported for depth render targets!");
        }
    }
    virtual ~VRORenderTarget() {}
    
    /*
     Set the viewport of this render target. The viewport will be automatically
     bound when this render target is bound.
     
     If the viewport size is changed, this may invalidate the current texture.
     This must be invoked before using the render target.
     
     Return true if the viewport changed. If the viepwort changes, the old
     graphics resources used by this render target will be deleted, and this
     render target will need to be re-hydrated.
     */
    virtual bool setViewport(VROViewport viewport) = 0;
    
    /*
     Allocates the underlying graphics resources for this render target. Returns
     false if the resources could not be allocated. No-op if this render target
     is already hydrated.
     */
    virtual bool hydrate() = 0;
    
    /*
     Get the dimensions of this render target.
     */
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    /*
     Get the color format (type) of this render target.
     */
    VRORenderTargetType getType() const { return _type; }
    
    /*
     Set the clear color to use for this render target.
     */
    void setClearColor(VROVector4f color) {
        _clearColor = color;
    }
    
    /*
     Bind this render-target. This will bind the target only for drawing.
     */
    virtual void bind() = 0;

    /*
     Bind this render-target for reading.
     */
    virtual void bindRead() = 0;
    
    /*
     Invalidate the buffers in this render-target.
     
     This tells the underlying implementation it does *not* need to transfer these
     buffers from tile memory back to shared memory, avoiding an expensive logical
     buffer store.
     
     Must be invoked before the next render target is bound.
     */
    virtual void invalidate() = 0;
    
    /*
     Blit the color attachment of this framebuffer over to the given
     destination buffer's color attachment. This should be implemented as a
     driver-level fast operation.
     
     The destination render target must already have been bound.
     */
    virtual void blitColor(std::shared_ptr<VRORenderTarget> destination, bool flipY,
                           std::shared_ptr<VRODriver> driver) = 0;
    
    /*
     Blit the stencil attachment of this framebuffer over to the given destination
     buffer's stencil attachment. This should be implemented as a driver-level fast
     operation.
     
     The destination render target must already have been bound.
     */
    virtual void blitStencil(std::shared_ptr<VRORenderTarget> destination, bool flipY,
                                  std::shared_ptr<VRODriver> driver) = 0;
    
    /*
     Delete all existing framebuffers.
     */
    virtual void deleteFramebuffers() = 0;
    
    /*
     Restores the frame-buffer if the context was lost. Returns false on failure.
     */
    virtual bool restoreFramebuffers() = 0;
    
#pragma mark - Render to Texture Setup
    
    /*
     Check if this render target has a texture attached at the given
     attachment point.
     */
    virtual bool hasTextureAttached(int attachment) = 0;
    
    /*
     Clear the textures in this render target, across all attachments.
     */
    virtual void clearTextures() = 0;
    
    /*
     Create and attach a new texture to this render-target. The texture will be
     sized to fit the render target. The old texture is not deleted, but this
     render-target's reference to it is released.
     
     Note if this render target type is a texture array, the image at index 0
     will be bound.
     
     New textures will be created for all attachment points.
     Returns true if successful, and false if an error was encountered.
     */
    virtual bool attachNewTextures() = 0;
    
    /*
     Set the index of the image to write to via this render target. This is only
     valid if this render target is of array type.
     */
    virtual void setTextureImageIndex(int index, int attachment) = 0;
    
    /*
     Set the face of the image to write to via this render target. This is only
     valid if this render target is of one of the cube types. The miplevel
     to write can also be set here.
     */
    virtual void setTextureCubeFace(int face, int mipLevel, int attachmentIndex) = 0;
    
    /*
     Set the miplevel to use for writing. This will resize the renderbuffers
     to match the given miplevel.
     */
    virtual void setMipLevel(int mipLevel, int attachmentIndex) = 0;

    /*
     Attach the given texture to this render-target. The width and height of
     the texture must match that of the render-target.
     
     Note if this render target type is a texture array, then the image at
     index 0 will be initially bound.
     
     The attachment parameter indicates the attachment point in which to
     set the texture.
     */
    virtual void attachTexture(std::shared_ptr<VROTexture> texture, int attachment) = 0;
    
    /*
     Get the texture to which this render-target is rendering. Returns nullptr if
     this is not a render-to-texture target.
     */
    virtual const std::shared_ptr<VROTexture> getTexture(int attachment) const = 0;
    
#pragma mark - Rendering Operations
    
    /*
     Clear the stencil buffer.
     */
    virtual void clearStencil() = 0;
    
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
     Enable portal stencil functions. When writing, we INCR the stencil
     buffer. When removing, we DECR the buffer. Finally when reading, we
     make the stencil buffer read-only. Portal stencil functions only
     operate on the lower 4 stencil bits (stencil mask 0x0F).
     */
    virtual void enablePortalStencilWriting(VROFace face) = 0;
    virtual void enablePortalStencilRemoval(VROFace face) = 0;
    virtual void disablePortalStencilWriting(VROFace face) = 0;
    
    /*
     Set the pass function and reference bits for the portal stencil test. The
     func parameter controls the stencil function that is used. The test is passed
     if func, applied to the ref and the value in the stencil buffer, equates to
     true. For example, if func is 'Less' and ref < stencil value, then the test
     passes.
     
     Note: only the lower four bits of ref and the lower four bits in the stencil buffer
           are used for this comparison. The upper four bits are reserved for other
           stencil operations, like per-pixel tone mapping. In other words, this call is
           equivalent to using glStencilFunc with a mask of 0x0F. (Additionally, any bits
           in the upper four region for the passed-in ref are flipped to 1).
     */
    virtual void setPortalStencilPassFunction(VROFace face, VROStencilFunc func, int ref) = 0;
    
protected:
    
    /*
     The type of render target (to texture, or offscreen).
     */
    VRORenderTargetType _type;
    
    /*
     The number of attachments in this render target. Each attachment is
     off the same type.
     */
    int _numAttachments;
    
    /*
     The clear color of this render target.
     */
    VROVector4f _clearColor;
    
};

#endif /* VRORenderTarget_h */
