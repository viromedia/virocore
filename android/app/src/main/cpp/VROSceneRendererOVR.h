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

class VROSceneRendererOVR : public VROSceneRenderer {

public:

    VROSceneRendererOVR();
    virtual ~VROSceneRendererOVR();

    /*
     Inherited from VROSceneRenderer.
     */
    void initGL();
    void onDrawFrame();
    void onTriggerEvent();
    void onPause();
    void onResume();

private:

};

#endif //ANDROID_VROSCENERENDEREROVR_H
