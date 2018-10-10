//
//  VRODisplayOpenGLiOS.hpp
//  ViroKit
//
//  Created by Raj Advani on 8/22/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRODisplayOpenGLiOS_hpp
#define VRODisplayOpenGLiOS_hpp

#include "VRODisplayOpenGL.h"
#include <GLKit/GLKit.h>

class VRODriverOpenGL;

class VRODisplayOpenGLiOS : public VRODisplayOpenGL {
public:
    
    VRODisplayOpenGLiOS(GLKView *view, std::shared_ptr<VRODriverOpenGL> driver) :
        VRODisplayOpenGL(0, driver),
        _view(view) { }
    virtual ~VRODisplayOpenGLiOS() {}
    
    void bind() {
        [_view bindDrawable];
        
        /*
         The viewport is already set by [_view bindDrawable] so we do not need to set
         that here. We still set the scissor, so ensure we only clear (e.g. glClear)
         over the designated area; this is particularly important in VR mode where we have
         two 'eyes' each with different viewport over the same framebuffer.
         */
        glScissor(_viewport.getX(), _viewport.getY(), _viewport.getWidth(), _viewport.getHeight());
        
        /*
         Prevent logical buffer loads.
         */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    
private:
    __weak GLKView *_view;
    
};

#endif /* VRODisplayOpenGLiOS_hpp */
