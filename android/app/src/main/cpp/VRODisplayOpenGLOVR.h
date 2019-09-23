//
//  Created by Raj Advani on 8/23/17.
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

#ifndef ANDROID_VRODISPLAYOPENGLOVR_H
#define ANDROID_VRODISPLAYOPENGLOVR_H

#include <memory>
#include "VROOpenGL.h"
#include "VRODisplayOpenGL.h"
#include <VrApi.h>
#include <VrApi_Types.h>
#include "VROSceneRendererOVR.h"

class VRODriverOpenGL;

/*
 In OVR the VRODisplay (the primary framebuffer) is managed by an
 ovrFrameBuffer object.
 */
class VRODisplayOpenGLOVR : public VRODisplayOpenGL {
public:

    VRODisplayOpenGLOVR(std::shared_ptr<VRODriverOpenGL> driver) :
            VRODisplayOpenGL(0, driver) {}
    virtual ~VRODisplayOpenGLOVR() {}

    void bind() {
        ovrFramebuffer_SetCurrent(_frameBuffer);

        /*
         Bind the viewport and scissor when the render target changes. The scissor
         ensures we only clear (e.g. glClear) over the designated area; this is
         particularly important in VR mode where we have two 'eyes' each with a
         different viewport over the same framebuffer.
         */
        glViewport(_viewport.getX(), _viewport.getY(), _viewport.getWidth(), _viewport.getHeight());
        glScissor(_viewport.getX(), _viewport.getY(), _viewport.getWidth(), _viewport.getHeight());

        /*
         Prevent logical buffer load by immediately clearing.
         */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void setFrameBuffer(ovrFramebuffer *framebuffer) {
        _frameBuffer = framebuffer;
    }

private:
    ovrFramebuffer *_frameBuffer;

};

#endif //ANDROID_VRODISPLAYOPENGLOVR_H
