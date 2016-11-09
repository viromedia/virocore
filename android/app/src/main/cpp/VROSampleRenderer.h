//
// Created by Raj Advani on 11/9/16.
//

#ifndef ANDROID_VROSAMPLERENDERER_H
#define ANDROID_VROSAMPLERENDERER_H

#include <memory>
#include "VRORenderDelegate.h"

class VROSceneController;
class VROSceneRendererCardboard;
class VRODriver;
class VRORenderer;

class VROSampleRenderer : public VRORenderDelegate {

public:

    VROSampleRenderer(std::shared_ptr<VRORenderer> renderer, VROSceneRendererCardboard *sceneRenderer);
    virtual ~VROSampleRenderer();

    void setupRendererWithDriver(VRODriver *driver);

private:

    VROSceneRendererCardboard *_sceneRenderer;
    VRODriver *_driver;
    std::shared_ptr<VROSceneController> _sceneController;

};

#endif //ANDROID_VROSAMPLERENDERER_H
