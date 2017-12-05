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
         Bind the viewport and scissor when the render target changes. The scissor
         ensures we only clear (e.g. glClear) over the designated area; this is
         particularly important in VR mode where we have two 'eyes' each with a
         different viewport over the same framebuffer.
         */
        glViewport(_viewport.getX(), _viewport.getY(), _viewport.getWidth(), _viewport.getHeight());
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
