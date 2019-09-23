//
//  VRODisplayOpenGLiOSGVR.h
//  ViroKit
//
//  Created by Raj Advani on 11/29/17.
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
            VRODisplayOpenGLiOS::bind();
        }
    }
    
    void setFrame(gvr::Frame &frame) {
        _frame = frame.cobj();
    }
    
private:
    gvr_frame *_frame;
    
};

#endif /* VRODisplayOpenGLGVR_h */
