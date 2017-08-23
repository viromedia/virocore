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
#include <VROFrameSynchronizer.h>
#include "VRORenderDelegate.h"
#include "VROFrameListener.h"

class VROSceneController;
class VROTexture;
class VROVideoTextureAndroid;
class VROVideoTextureAVP;
class VROSoundEffect;
class VROAudioPlayer;

class VROSample : public VRORenderDelegate, public std::enable_shared_from_this<VROSample> {

public:

    VROSample();
    virtual ~VROSample();

    std::shared_ptr<VROSceneController> loadBoxScene(std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VROSceneController> loadHDRScene(std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VROSceneController> loadShadowScene(std::shared_ptr<VRODriver> driver);

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

    void setupRendererWithDriver(std::shared_ptr<VRODriver> driver);

private:

    std::shared_ptr<VROTexture> getNiagaraTexture();
    void animateTake(std::shared_ptr<VRONode> node);

    std::shared_ptr<VRODriver> _driver;
    float _objAngle;
};

#endif //ANDROID_VROSAMPLERENDERER_H
