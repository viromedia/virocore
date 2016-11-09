//
//  VROSceneController.h
//  ViroKit
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROSCENECONTROLLER_H
#define ANDROID_VROSCENECONTROLLER_H

#include "VROSceneControllerInternal.h"
#include <memory>

class VROScene;
class VROVector3f;

class VROSceneController : public VROSceneControllerInternal {

public:

    VROSceneController(std::shared_ptr<VROReticle> reticle,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer) :
            VROSceneControllerInternal(reticle, frameSynchronizer) {}
    virtual ~VROSceneController() {}

    std::shared_ptr<VROScene> getScene() {
        return VROSceneControllerInternal::getScene();
    }
    void setHoverEnabled(bool enabled, bool boundsOnly) {
        VROSceneControllerInternal::setHoverEnabled(enabled, boundsOnly);
    }

    /*
     Scene appeared delegate methods.
     */
    virtual void onSceneWillAppear(VRORenderContext &context, VRODriver &driver) {
        VROSceneControllerInternal::onSceneWillAppear(context, driver);
    }
    virtual void onSceneDidAppear(VRORenderContext &context, VRODriver &driver) {
        VROSceneControllerInternal::onSceneDidAppear(context, driver);
    }
    virtual void onSceneWillDisappear(VRORenderContext &context, VRODriver &driver) {
        VROSceneControllerInternal::onSceneWillDisappear(context, driver);
    }
    virtual void onSceneDidDisappear(VRORenderContext &context, VRODriver &driver) {
        VROSceneControllerInternal::onSceneDidDisappear(context, driver);
    }

    /*
     Scene animation delegate methods.
     */
    virtual void startIncomingTransition(VRORenderContext *context, float duration) {}
    virtual void startOutgoingTransition(VRORenderContext *context, float duration) {}
    virtual void endIncomingTransition(VRORenderContext *context) {}
    virtual void endOutgoingTransition(VRORenderContext *context) {}

    virtual void animateIncomingTransition(VRORenderContext *context, float t) {}
    virtual void animateOutgoingTransition(VRORenderContext *context, float t) {}

    /*
     Per-frame rendering delegate methods.
     */
    virtual void sceneWillRender(const VRORenderContext *context) {}

    /*
     Hover delegate methods.
     */
    virtual bool isHoverable(std::shared_ptr<VRONode> node) { return false; }
    virtual void hoverOnNode(std::shared_ptr<VRONode> node) {}
    virtual void hoverOffNode(std::shared_ptr<VRONode> node) {}

    /*
     Reticle delegate methods.
     */
    virtual void reticleTapped(VROVector3f ray, const VRORenderContext *context) {}

private:

    VROSceneController *_sceneController;

};

#endif //ANDROID_VROSCENECONTROLLER_H
