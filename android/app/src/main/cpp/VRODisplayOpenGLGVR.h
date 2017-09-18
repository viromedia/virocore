//
// Created by Raj Advani on 8/23/17.
//

#ifndef ANDROID_VRODISPLAYOPENGLGVR_H
#define ANDROID_VRODISPLAYOPENGLGVR_H

#include <memory>
#include "VROOpenGL.h"
#include "VRODisplayOpenGL.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VRODriverOpenGL;

/*
 In GVR the VRODisplay (the primary framebuffer) is managed by a
 gvr::Frame object.
 */
class VRODisplayOpenGLGVR : public VRODisplayOpenGL {
public:

    VRODisplayOpenGLGVR(std::shared_ptr<VRODriverOpenGL> driver) :
        VRODisplayOpenGL(0, driver),
        _frame(nullptr) {}
    virtual ~VRODisplayOpenGLGVR() {}

    void bind() {
        if (_frame != nullptr) {
            gvr_frame_bind_buffer(_frame, 0);
        }
        else {
            // 360 mode, we don't use the gvr frame but we have a valid framebuffer object
            VRODisplayOpenGL::bind();
            return;
        }

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

    void unbind() {
        if (_frame != nullptr) {
            gvr_frame_unbind(_frame);
        }
        else {
            VRODisplayOpenGL::unbind();
        }
    }

    void setFrame(gvr::Frame &frame) {
        _frame = frame.cobj();
    }

private:
    gvr_frame *_frame;

};


#endif //ANDROID_VRODISPLAYOPENGLGVR_H
