//
//  VRORecorderEglSurfaceDisplay.h
//  ViroKit
//
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

#ifndef VRO_RECORDER_EGL_DISPLAY_h
#define VRO_RECORDER_EGL_DISPLAY_h

#include "VRODisplayOpenGL.h"
#include "VROAVRecorderAndroid.h"
class VRODriver;

/*
 Display Render target for EGL surfaces that are backed by a VROAVRecorderAndroid component.
 */
class VRORecorderEglSurfaceDisplay : public VRODisplayOpenGL {
public:
    VRORecorderEglSurfaceDisplay(std::shared_ptr<VRODriverOpenGL> driver,
                                 std::shared_ptr<VROAVRecorderAndroid> androidRecorder) :
            VRODisplayOpenGL(0, driver) {
        _w_recorder = androidRecorder;
        _viewport = VROViewport(0, 0, driver->getDisplay()->getWidth(), driver->getDisplay()->getHeight());
    }

    virtual ~VRORecorderEglSurfaceDisplay() {}

    void bind() {
        std::shared_ptr<VROAVRecorderAndroid> recorder = _w_recorder.lock();
        if (!recorder) {
            return;
        }

        recorder->bindToEglSurface();
        glViewport(_viewport.getX(), _viewport.getY(), _viewport.getWidth(), _viewport.getHeight());
        glScissor(_viewport.getX(), _viewport.getY(), _viewport.getWidth(), _viewport.getHeight());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void invalidate() {
        std::shared_ptr<VROAVRecorderAndroid> recorder = _w_recorder.lock();
        if (!recorder) {
            return;
        }

        recorder->eglSwap();
        recorder->unbindFromEGLSurface();
    }

private:
    std::weak_ptr<VROAVRecorderAndroid> _w_recorder;
};

#endif /* VRO_RECORDER_EGL_DISPLAY_h */
