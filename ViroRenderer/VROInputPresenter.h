//
//  VROInputPresenter.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROInputPresenter_H
#define VROInputPresenter_H

#include <memory>
#include "VROEventDelegate.h"
#include "VRORenderContext.h"
#include "VROReticle.h"
#include "VRONode.h"
#include "VROMath.h"

static const float kReticleSizeMultiple = 3;

/**
 * VROInputPresenter contains all UI view implementations to be displayed for a given
 * VROInputController.
 */
class VROInputPresenter : public VROEventDelegate {
public:
    VROInputPresenter(std::shared_ptr<VRORenderContext> context) {
        _reticle = std::make_shared<VROReticle>();
        _context = context;
        _rootNode = std::make_shared<VRONode>();
        _delegate = nullptr;
    }

    ~VROInputPresenter() {}

    std::shared_ptr<VRONode> getRootNode(){
        return _rootNode;
    }

    /**
     * Event delegate for triggering calls back to Controller_JNI.
     */
    void setEventDelegate(std::shared_ptr<VROEventDelegate> delegate){
        _delegate = delegate;
    }

    virtual void onButtonEvent(EventSource type, EventAction event){
        if (_delegate != nullptr){
            _delegate->onButtonEvent(type, event);
        }
    }

    virtual void onTouchPadEvent(EventSource type, EventAction event, float x, float y){
        if (_delegate != nullptr){
            _delegate->onTouchPadEvent(type, event, x, y);
        }
    }

    virtual void onRotate(VROVector3f rotation) {
        if (_delegate != nullptr){
            _delegate->onRotate((VROVector3f()));
        }
    }

    virtual void onPosition(VROVector3f location) {
        if (_delegate != nullptr){
            _delegate->onPosition((VROVector3f()));
        }
    }

    virtual void onGaze(bool isGazing){
        if (_delegate != nullptr){
            _delegate->onGaze(isGazing);
        }
    }

    virtual void onGazeHit(float distance, VROVector3f hitLocation){
        if (_delegate != nullptr){
            _delegate->onGazeHit(distance, hitLocation);
        }
    }

    std::shared_ptr<VROReticle> getReticle() {
            return _reticle;
    }
protected:
    std::shared_ptr<VRONode> _rootNode;
    std::shared_ptr<VROEventDelegate> _delegate;

    void onReticleGazeHit(float distance, VROVector3f hitLocation){
        float depth = -distance;

        if (_reticle->getPointerMode()){
            _reticle->setPosition(hitLocation);
        } else {
            // Lock the Reticle's position to the center of the screen
            // for non-pointer mode (usually Cardboard).
            _reticle->setPosition(VROVector3f(0, 0, depth));
        }

        float worldPerScreen = _context->getCamera().getWorldPerScreen(depth);
        float radius = fabs(worldPerScreen) * kReticleSizeMultiple;
        _reticle->setRadius(radius);
    }

private:
    std::shared_ptr<VROReticle> _reticle;
    std::shared_ptr<VRORenderContext> _context;
};
#endif
