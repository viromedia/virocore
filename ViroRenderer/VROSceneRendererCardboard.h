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
#import "GCSCardboardView.h"

@class VROSceneController;
enum class VROTimingFunctionType;

/*
 Leverages the VRORenderer and a VRODriver to render a Viro scene
 to cardboard.
 */
class VROSceneRendererCardboard {
    
public:
    
    /*
     Invoked once with the active GL context.
     */
    virtual void initRenderer(GCSHeadTransform *headTransform) = 0;
    
    /*
     Invoked at the start of each frame, then once per eye, and at the end
     of the frame, respectively.
     */
    virtual void prepareFrame(GCSHeadTransform *headTransform) = 0;
    virtual void renderEye(GCSEye eye, GCSHeadTransform *headTransform) = 0;
    virtual void endFrame()= 0;
    
    /*
     Set the active scene.
     */
    virtual void setSceneController(VROSceneController *sceneController) = 0;
    virtual void setSceneController(VROSceneController *sceneController, bool animated) = 0;
    virtual void setSceneController(VROSceneController *sceneController, float seconds,
                                    VROTimingFunctionType timingFunctionType) = 0;
    
};

#endif /* VROSceneRendererCardboard_h */
