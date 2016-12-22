//
//  VROSceneController.h
//  ViroKit
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROSCENECONTROLLER_H
#define ANDROID_VROSCENECONTROLLER_H
#include <memory>
#include "VROScene.h"

class VROScene;
class VROVector3f;
class VRODriver;
class VRORenderContext;

class VROSceneController {

public:
    // Delegate for callbacks across the bridge
    class VROSceneControllerDelegate {
    public:
        virtual void onSceneWillAppear(VRORenderContext &context, VRODriver &driver) =0;
        virtual void onSceneDidAppear(VRORenderContext &context, VRODriver &driver) =0;
        virtual void onSceneWillDisappear(VRORenderContext &context, VRODriver &driver) =0;
        virtual void onSceneDidDisappear(VRORenderContext &context, VRODriver &driver) =0;
    };

    void setDelegate(std::shared_ptr<VROSceneControllerDelegate> delegate){
        _delegate = delegate;
    }

    VROSceneController() {
        _scene = std::make_shared<VROScene>();
    }
    virtual ~VROSceneController() {}

    std::shared_ptr<VROScene> getScene() {
        return _scene;
    }

    /*
     Scene appeared delegate methods.
     */
    void onSceneWillAppear(VRORenderContext &context, VRODriver &driver) {
        if (_delegate != NULL){
            _delegate->onSceneWillAppear(context, driver);
        }

    }
    void onSceneDidAppear(VRORenderContext &context, VRODriver &driver) {
        if (_delegate != NULL){
            _delegate->onSceneDidAppear(context, driver);
        }

    }
    void onSceneWillDisappear(VRORenderContext &context, VRODriver &driver) {
        if (_delegate != NULL){
            _delegate->onSceneWillDisappear(context, driver);
        }

    }
    void onSceneDidDisappear(VRORenderContext &context, VRODriver &driver) {
        if (_delegate != NULL){
            _delegate->onSceneDidDisappear(context, driver);
        }
    }

    /*
     Scene animation delegate methods.
     */
    void startIncomingTransition(VRORenderContext *context, float duration){}
    void startOutgoingTransition(VRORenderContext *context, float duration){}
    void endIncomingTransition(VRORenderContext *context) {}
    void endOutgoingTransition(VRORenderContext *context) {}
    void animateIncomingTransition(VRORenderContext *context, float t){}
    void animateOutgoingTransition(VRORenderContext *context, float t){}

    /*
     Per-frame rendering delegate methods.
     */
    void sceneWillRender(const VRORenderContext *context) {}

private:
    std::shared_ptr<VROScene> _scene;
    std::shared_ptr<VROSceneControllerDelegate> _delegate;
};

#endif //ANDROID_VROSCENECONTROLLER_H
