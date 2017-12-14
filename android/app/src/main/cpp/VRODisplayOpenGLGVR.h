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
            // Do *not* directly call gvr_frame_bind or gvr_frame_unbind here; instead,
            // get the underlying FBO ID for buffer 0 in the frame and bind that.
            // We don't call the gvr functions directly because frame bind and unbind
            // appear to do much more than just bind the underlying FBO. In particular,
            // if we call gvr_frame_unbind in the midst of a render cycle, we are likely
            // to get driver-level (BufferObjectDisableReorderCheck) crashes on S3 devices.
            // And if we call gvr_frame_bind without calling gvr_frame_unbind, we get
            // recurring log spam about performance degradation from GVR.
            _framebuffer = gvr_frame_get_framebuffer_object(_frame, 0);
            VRODisplayOpenGL::bind();
        }
        else {
            // 360 mode, we don't use the gvr frame but we have a valid framebuffer object
            VRODisplayOpenGL::bind();
        }
    }

    void setFrame(gvr::Frame &frame) {
        _frame = frame.cobj();
    }

    void clearFrame() {
        _frame = nullptr;
    }

private:
    gvr_frame *_frame;

};


#endif //ANDROID_VRODISPLAYOPENGLGVR_H
