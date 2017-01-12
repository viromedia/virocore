//
//  VROControllerPresenterCardboard.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROControllerPresenterCardboardiOS_H
#define VROControllerPresenterCardboardiOS_H

#include <memory>
#include <string>
#include <vector>
#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROEventDelegate.h"

class VROInputPresenterCardboardiOS : public VROInputPresenter {
public:
    VROInputPresenterCardboardiOS(std::shared_ptr<VRORenderContext> context):VROInputPresenter(context) {
    }
    virtual ~VROInputPresenterCardboardiOS() {}

    void onButtonEvent(EventSource type, EventAction event){
        if (type==EventSource::PRIMARY_CLICK && event==EventAction::CLICK_UP){
            getReticle()->trigger();
        }
    }

    void onGazeHit(float distance, VROVector3f hitLocation){
        VROInputPresenter::onReticleGazeHit(distance, hitLocation);
     }
};
#endif
