//
//  VRODisplayOpenGLGVR.h
//  ViroKit
//
//  Created by Raj Advani on 11/29/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRODisplayOpenGLGVR_h
#define VRODisplayOpenGLGVR_h

#include <GLKit/GLKit.h>
#include <memory>
#include "VROOpenGL.h"
#include "VRODisplayOpenGLiOS.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VRODriverOpenGL;

/*
 In GVR the VRODisplay (the primary framebuffer) is managed by a
 gvr::Frame object.
 */
class VRODisplayOpenGLiOSGVR : public VRODisplayOpenGLiOS {
public:
    
    VRODisplayOpenGLiOSGVR(GLKView *viewGL, std::shared_ptr<VRODriverOpenGL> driver) :
        VRODisplayOpenGLiOS(viewGL, driver),
        _frame(nullptr) {}
    virtual ~VRODisplayOpenGLiOSGVR() {}
    
    void bind() {
        if (_frame != nullptr) {
            // Unbind first, ensures the previous framebuffer is invalidated (preventing
            // logical buffer stores). Also prevents GVR log spam.
            gvr_frame_unbind(_frame);
            gvr_frame_bind_buffer(_frame, 0);
        }
        else {
            // 360 mode, we don't use the gvr frame but we have a GLKView
            VRODisplayOpenGLiOS::bind();
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
    
    void setFrame(gvr::Frame &frame) {
        _frame = frame.cobj();
    }
    
private:
    gvr_frame *_frame;
    
};

#endif /* VRODisplayOpenGLGVR_h */
