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

class VROSceneRendererOVR : public VROSceneRenderer {

public:

    VROSceneRendererOVR(std::shared_ptr<gvr::AudioApi> gvrAudio,
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
    void onSurfaceChanged(jobject surface);
    void onSurfaceDestroyed();

private:

    ovrAppThread *_appThread;
    jobject _surface;
    jobject _view;

};

#endif //ANDROID_VROSCENERENDEREROVR_H
