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

struct ovrAppThread;

class VROSceneRendererOVR : public VROSceneRenderer {

public:

    VROSceneRendererOVR(std::shared_ptr<gvr::AudioApi> gvrAudio,
                        jobject activity, JNIEnv *env);
    virtual ~VROSceneRendererOVR();

    /*
     Inherited from VROSceneRenderer.
     */
    void initGL();
    void onDrawFrame();
    void onTriggerEvent();

    /*
     Activity lifecycle.
     */
    void onStart();
    void onPause();
    void onResume();
    void onStop();

    /*
     Surface lifecycle.
     */
    void onSurfaceCreated(jobject surface);
    void onSurfaceChanged(jobject surface);
    void onSurfaceDestroyed();

private:

    ovrAppThread *_appThread;
    jobject _surface;

};

#endif //ANDROID_VROSCENERENDEREROVR_H
