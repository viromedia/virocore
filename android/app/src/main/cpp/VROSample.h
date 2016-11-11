//
//  VROSample.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/9/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROSAMPLERENDERER_H
#define ANDROID_VROSAMPLERENDERER_H

#include <memory>
#include "VRORenderDelegate.h"

class VROSceneController;
class VROSceneRendererCardboard;
class VROTexture;

class VROSample : public VRORenderDelegate {

public:

    VROSample();
    virtual ~VROSample();

    std::shared_ptr<VROSceneController> loadBoxScene();

private:

    std::shared_ptr<VROTexture> getNiagaraTexture();

};

#endif //ANDROID_VROSAMPLERENDERER_H
