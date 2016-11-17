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
class VROSceneRendererCardboard;
class VROTexture;
class VROVideoTexture;

class VROSample : public VRORenderDelegate, public VROFrameListener, public std::enable_shared_from_this<VROSample> {

public:

    VROSample();
    virtual ~VROSample();

    std::shared_ptr<VROSceneController> loadBoxScene(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                                     VRODriver &driver);

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

private:

    std::shared_ptr<VROVideoTexture> _videoA;
    std::shared_ptr<VROVideoTexture> _videoB;

    std::shared_ptr<VROMaterial> _material;
    std::shared_ptr<VROTexture> getNiagaraTexture();

    VRODriver *_driver;

};

#endif //ANDROID_VROSAMPLERENDERER_H
