//
//  VROSceneControlleriOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROSceneControlleriOS_h
#define VROSceneControlleriOS_h

#include "VROSceneControllerInternal.h"
#include "VROSceneController.h"

class VROSceneControlleriOS : public VROSceneControllerInternal {
    
public:
    
    VROSceneControlleriOS(std::shared_ptr<VROHoverDistanceListener> reticleSizeListener,
                          std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                          VROSceneController *sceneController) :
        VROSceneControllerInternal(reticleSizeListener, frameSynchronizer),
        _sceneController(sceneController) {}
    virtual ~VROSceneControlleriOS() {}
    
    /*
     Scene appeared delegate methods.
     */
    void onSceneWillAppear(VRORenderContext &context, VRODriver &driver) {
        VROSceneControllerInternal::onSceneWillAppear(context, driver);
        [_sceneController sceneWillAppear:&context driver:&driver];
    }
    void onSceneDidAppear(VRORenderContext &context, VRODriver &driver) {
        VROSceneControllerInternal::onSceneDidAppear(context, driver);
        [_sceneController sceneDidAppear:&context driver:&driver];
    }
    void onSceneWillDisappear(VRORenderContext &context, VRODriver &driver) {
        VROSceneControllerInternal::onSceneWillDisappear(context, driver);
        [_sceneController sceneWillDisappear:&context driver:&driver];
    }
    void onSceneDidDisappear(VRORenderContext &context, VRODriver &driver) {
        VROSceneControllerInternal::onSceneDidDisappear(context, driver);
        [_sceneController sceneDidDisappear:&context driver:&driver];
    }
    
    /*
     Scene animation delegate methods.
     */
    void startIncomingTransition(VRORenderContext *context, float duration) {
        [_sceneController startIncomingTransition:context duration:duration];
    }
    void startOutgoingTransition(VRORenderContext *context, float duration) {
        [_sceneController startOutgoingTransition:context duration:duration];
    }
    void endIncomingTransition(VRORenderContext *context) {
        [_sceneController endIncomingTransition:context];
    }
    void endOutgoingTransition(VRORenderContext *context) {
        [_sceneController endOutgoingTransition:context];
    }
    void animateIncomingTransition(VRORenderContext *context, float t) {
        [_sceneController animateIncomingTransition:context percentComplete:t];
    }
    void animateOutgoingTransition(VRORenderContext *context, float t) {
        [_sceneController animateOutgoingTransition:context percentComplete:t];
    }
    
    /*
     Per-frame rendering delegate methods.
     */
    void sceneWillRender(const VRORenderContext *context) {
        [_sceneController sceneWillRender:context];
    }
    
    /*
     Hover delegate methods.
     */
    bool isHoverable(std::shared_ptr<VRONode> node) {
        return [_sceneController isHoverable:node];
    }
    void hoverOnNode(std::shared_ptr<VRONode> node) {
        [_sceneController hoverOnNode:node];
    }
    void hoverOffNode(std::shared_ptr<VRONode> node) {
        [_sceneController hoverOffNode:node];
    }
    
    /*
     Reticle delegate methods.
     */
    void reticleTapped(VROVector3f ray, const VRORenderContext *context) {
        [_sceneController reticleTapped:ray context:context];
    }
    
private:
    
    VROSceneController *_sceneController;
    
};

#endif /* VROSceneControlleriOS_h */
