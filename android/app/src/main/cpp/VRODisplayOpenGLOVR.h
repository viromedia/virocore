//
// Created by Raj Advani on 8/23/17.
//

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
