//
//  VROInputPresenterDaydream.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROInputPresenterDaydream_H
#define VROInputPresenterDaydream_H

#include <memory>
#include <string>
#include <vector>
#include <VROReticle.h>
#include <VROBox.h>
#include "VRONode.h"
#include "VRORenderContext.h"
#include "VROInputPresenter.h"

class VROInputPresenterDaydream : public VROInputPresenter {
public:
    VROInputPresenterDaydream(std::shared_ptr<VRORenderContext> context):VROInputPresenter(context) {
        getReticle()->setPointerMode(true);
    }

    ~VROInputPresenterDaydream() {}
    void updateController();

    void onButtonEvent(EventSource type, EventAction event){
        if (type==EventSource::PRIMARY_CLICK && event==EventAction::CLICK_UP){
            getReticle()->trigger();
        }
    }

    void onTouchPadEvent(EventSource type, EventAction event, float x, float y){
        /**
          * TODO VIRO-704: Implement UI Components for daydream controller.
          */
    }

    void onRotate(VROVector3f rotation){
        /**
         * TODO VIRO-704: Implement UI Components for daydream controller.
         */
    }

    void onGazeHit(float distance, VROVector3f hitLocation){
        VROInputPresenter::onReticleGazeHit(distance, hitLocation);
    }
};
#endif
