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

class VRONode;
class VRORenderer;
class VROSceneController;
class VRORendererTestHarness;

class VROSample : public VRORenderDelegate, public std::enable_shared_from_this<VROSample> {

public:

    VROSample();
    virtual ~VROSample();

    void loadTestHarness(std::shared_ptr<VRORenderer> renderer,
                         std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                         std::shared_ptr<VRODriver> driver);

    std::shared_ptr<VROSceneController> getSceneController();
    std::shared_ptr<VRONode> getPointOfView();

private:

    std::shared_ptr<VRORendererTestHarness> _harness;

};

#endif //ANDROID_VROSAMPLERENDERER_H
