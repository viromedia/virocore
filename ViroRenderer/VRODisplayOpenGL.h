//
//  VRODisplayOpenGL.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRODisplayOpenGL_h
#define VRODisplayOpenGL_h

#include "VRORenderTargetOpenGL.h"
#include "VROOpenGL.h"

class VRODriver;

class VRODisplayOpenGL : public VRORenderTargetOpenGL {
public:
    
    VRODisplayOpenGL(GLint framebuffer, std::shared_ptr<VRODriverOpenGL> driver) :
        VRORenderTargetOpenGL(VRORenderTargetType::Display, 1, 1, driver) {
        _framebuffer = framebuffer;
    }
    virtual ~VRODisplayOpenGL() {}
    virtual void unbind() {};
    void setViewport(VROViewport viewport) { _viewport = viewport; }
    
#pragma mark - Unsupported by Displays
    
    void clearTextures() { pabort(); }
    void attachNewTextures() { pabort(); }
    void setTextureImageIndex(int index, int attachment) { pabort(); }
    void attachTexture(std::shared_ptr<VROTexture> texture, int attachment) { pabort(); }
    const std::shared_ptr<VROTexture> getTexture(int attachment) const { pabort(); return nullptr; }
    void discardFramebuffers() { pabort(); }
    void restoreFramebuffers() { pabort(); }
    
};

#endif /* VRODisplayOpenGL_h */
