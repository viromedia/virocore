//
//  VROSceneRendererOVR.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
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
    void setSuspended(bool suspendRenderer);
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
    void onSurfaceChanged(jobject surface, jint width, jint height);
    void onSurfaceDestroyed();

private:

    ovrAppThread *_appThread;
    jobject _surface;
    jobject _view;

};

#endif //ANDROID_VROSCENERENDEREROVR_H
