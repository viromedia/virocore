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
        VRORenderTargetOpenGL(VRORenderTargetType::Display, driver),
        _framebuffer(framebuffer) {}
    virtual ~VRODisplayOpenGL() {}
    
#pragma mark - Unsupported by Displays
    
    void setSize(int width, int height) { pabort(); }
    void clearTexture() { pabort(); }
    void attachNewTexture() { pabort(); }
    void attachTexture(std::shared_ptr<VROTexture> texture) { pabort(); }
    const std::shared_ptr<VROTexture> getTexture() const { pabort(); return nullptr; }
    void discardFramebuffers() { pabort(); }
    void restoreFramebuffers() { pabort(); }
    
private:
    
    GLint _framebuffer;
    
};

#endif /* VRODisplayOpenGL_h */
