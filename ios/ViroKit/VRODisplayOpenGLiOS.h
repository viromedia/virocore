//
//  VRODisplayOpenGLiOS.hpp
//  ViroKit
//
//  Created by Raj Advani on 8/22/17.
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
