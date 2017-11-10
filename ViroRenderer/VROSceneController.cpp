//
//  VROSceneController.cpp
//  ViroKit
//
//  Created by Raj Advani on 11/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSceneController.h"
#include "VROPortalTraversalListener.h"
#include "VROFrameSynchronizer.h"

void VROSceneController::onSceneWillAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    _portalTraversalListener = std::make_shared<VROPortalTraversalListener>(_scene);
    context->getFrameSynchronizer()->addFrameListener(_portalTraversalListener);
    
    std::shared_ptr<VROSceneControllerDelegate> delegate = _sceneDelegateWeak.lock();
    if (delegate) {
        delegate->onSceneWillAppear(context, driver);
    }
}

void VROSceneController::onSceneDidAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    std::shared_ptr<VROSceneControllerDelegate> delegate = _sceneDelegateWeak.lock();
    if (delegate) {
        delegate->onSceneDidAppear(context, driver);
    }
}

void VROSceneController::onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    if (_portalTraversalListener) {
        context->getFrameSynchronizer()->removeFrameListener(_portalTraversalListener);
        _portalTraversalListener.reset();
    }
    
    std::shared_ptr<VROSceneControllerDelegate> delegate = _sceneDelegateWeak.lock();
    if (delegate) {
        delegate->onSceneWillDisappear(context, driver);
    }
}

void VROSceneController::onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    std::shared_ptr<VROSceneControllerDelegate> delegate = _sceneDelegateWeak.lock();
    if (delegate) {
        delegate->onSceneDidDisappear(context, driver);
    }
}

void VROSceneController::startIncomingTransition(float duration, VROTimingFunctionType timingFunctionType,
                                                 VRORenderContext *context) {
    
    VROVector3f forward = context->getCamera().getForward().scale(kTransitionPushDistance);
    
    // Preserve the current opacity and position of the root node. We start each node off at
    // opacity 0 and will animate toward this preserved opacity
    std::shared_ptr<VRONode> root = _scene->getRootNode();
    float preservedOpacity = root->getOpacity();
    root->setOpacity(0.0f);
    
    // Start each root node back 1 unit. The nodes will animate toward
    // the camera. This also eliminates z-fighting with the outgoing scene
    root->setPosition(root->getPosition() + forward);
    
    std::vector<std::shared_ptr<VROGeometry>> backgrounds = _scene->getBackgrounds();
    for (std::shared_ptr<VROGeometry> &background : backgrounds) {
        background->getMaterials().front()->setTransparency(0.0);
    }
    
    // Construct and commit fade-in/pull-in animation for the scene
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(duration);
    VROTransaction::setTimingFunction(timingFunctionType);
    
    root->setOpacity(preservedOpacity);
    root->setPosition(root->getPosition() - forward);
    
    for (std::shared_ptr<VROGeometry> &background : backgrounds) {
        background->getMaterials().front()->setTransparency(1.0);
    }
    
    // Set callback delegates
    std::weak_ptr<VROSceneController> weakPtr = shared_from_this();
    VROTransaction::setFinishCallback([weakPtr]() {
        std::shared_ptr<VROSceneController> scene = weakPtr.lock();
        if (scene) {
            scene->setActiveTransitionAnimation(false);
        }
    });
    
    setActiveTransitionAnimation(true);
    VROTransaction::commit();
}

void VROSceneController::startOutgoingTransition(float duration, VROTimingFunctionType timingFunctionType,
                                                 VRORenderContext *context) {
    
    VROVector3f forward = context->getCamera().getForward().scale(kTransitionPushDistance);
    
    // Preserve the current opacity of root nodes. When the scene disappears, we'll
    // restore that opacity (so that the next time the scene appears, it will be at said
    // previous opacity).
    std::shared_ptr<VRONode> root = _scene->getRootNode();
    float preservedOpacity = root->getOpacity();
    
    // Construct and commit fade-out/push-back animation for the scene
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(duration);
    VROTransaction::setTimingFunction(timingFunctionType);
    
    root->setOpacity(0.0f);
    root->setPosition(root->getPosition() + forward);
    
    std::vector<std::shared_ptr<VROGeometry>> backgrounds = _scene->getBackgrounds();
    for (std::shared_ptr<VROGeometry> &background : backgrounds) {
        background->getMaterials().front()->setTransparency(0.0);
    }
    
    // At the end of the animation, restore the opacity and position of the
    // nodes (since they are no longer visible)
    std::weak_ptr<VROSceneController> weakPtr = shared_from_this();
    VROTransaction::setFinishCallback([weakPtr, preservedOpacity, forward]() {
        std::shared_ptr<VROSceneController> scene = weakPtr.lock();
        if (scene) {
            std::shared_ptr<VRONode> root = scene->getScene()->getRootNode();
            root->setOpacity(preservedOpacity);
            root->setPosition(root->getPosition() - forward);
            
            scene->setActiveTransitionAnimation(false);
        }
    });
    
    setActiveTransitionAnimation(true);
    VROTransaction::commit();
}
