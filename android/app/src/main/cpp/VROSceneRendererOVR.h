//
//  VROSceneRendererOVR.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/5/17.
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

#ifndef ANDROID_VROSCENERENDEREROVR_H
#define ANDROID_VROSCENERENDEREROVR_H

#include "VROSceneRenderer.h"
#include <memory>

struct ovrAppThread;
struct ovrFramebuffer;
class VRORendererConfiguration;

/*
 Externally invoked by VRODisplayOpenGLOVR to bind OVR framebuffers.
 */
static void ovrFramebuffer_SetCurrent(ovrFramebuffer *frameBuffer);

class VROSceneRendererOVR : public VROSceneRenderer {

public:

    VROSceneRendererOVR(VRORendererConfiguration config,
                        std::shared_ptr<gvr::AudioApi> gvrAudio,
                        jobject view, jobject activity, JNIEnv *env);
    virtual ~VROSceneRendererOVR();

    /*
     Inherited from VROSceneRenderer.
     */
    void initGL() {} // Not required
    void onDrawFrame() {} // Not required
    void onTouchEvent(int action, float x, float y);
    void onKeyEvent(int keyCode, int action);
    void setVRModeEnabled(bool enabled) {} // Not supported
    void recenterTracking();

    /*
     Activity lifecycle.
     */
    void onStart();
    void onPause();
    void onResume();
    void onStop();
    void onDestroy();

    /*
     Surface lifecycle.
     */
    void onSurfaceCreated(jobject surface);
    void onSurfaceChanged(jobject surface, VRO_INT width, VRO_INT height);
    void onSurfaceDestroyed();

private:

    ovrAppThread *_appThread;
    jobject _surface;
    jobject _view;

};

#endif //ANDROID_VROSCENERENDEREROVR_H
