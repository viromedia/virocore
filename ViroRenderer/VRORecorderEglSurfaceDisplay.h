//
//  VRORecorderEglSurfaceDisplay.h
//  ViroKit
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRO_RECORDER_EGL_DISPLAY_h
#define VRO_RECORDER_EGL_DISPLAY_h

#include "VRODisplayOpenGL.h"
#include "VROAVRecorderAndroid.h"
class VRODriver;

/*
 Display Render target for egl surfaces that are backed by a VROVRecorderAndroid component.
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

    void unbind() {
        std::shared_ptr<VROAVRecorderAndroid> recorder = _w_recorder.lock();
        if (!recorder) {
            return;
        }

        recorder->eglSwap();
        recorder->unBindFromEGLSurface();
    }

private:
    std::weak_ptr<VROAVRecorderAndroid> _w_recorder;
};

#endif /* VRO_RECORDER_EGL_DISPLAY_h */
