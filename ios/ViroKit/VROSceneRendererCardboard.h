//
//  VROSceneRendererCardboard.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROSceneRendererCardboard_h
#define VROSceneRendererCardboard_h

#include <stdio.h>
#include <memory>
#import "GVRCardboardView.h"
#include "VROViewport.h"
#include "VROFieldOfView.h"

class VROSceneController;
enum class VROTimingFunctionType;

/*
 Leverages the VRORenderer and a VRODriver to render a Viro scene
 to cardboard.
 */
class VROSceneRendererCardboard {
    
public:
    
    virtual ~VROSceneRendererCardboard() {}
    
    /*
     Invoked once with the active GL context.
     */
    virtual void initRenderer(GVRHeadTransform *headTransform) = 0;
    
    /*
     Invoked at the start of each frame, then once per eye, and at the end
     of the frame, respectively.
     */
    virtual void prepareFrame(VROViewport viewport, VROFieldOfView fov, GVRHeadTransform *headTransform) = 0;
    virtual void renderEye(GVREye eye, GVRHeadTransform *headTransform) = 0;
    virtual void endFrame()= 0;
    
    /*
     Set the active scene.
     */
    virtual void setSceneController(std::shared_ptr<VROSceneController> sceneController) = 0;
    virtual void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                                    VROTimingFunctionType timingFunctionType) = 0;
    
    /*
     Suspend the renderer, displaying only a black screen.
     */
    virtual void setSuspended(bool suspended) = 0;
    
};

#endif /* VROSceneRendererCardboard_h */
