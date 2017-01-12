//
//  VROInputPresenterCardboard.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROInputPresenterCardboard_H
#define VROInputPresenterCardboard_H

#include <memory>
#include <string>
#include <vector>
#include <VROReticle.h>

#include "VROFrameListener.h"
#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROEventDelegate.h"

class VROInputPresenterCardboard : public VROInputPresenter {
public:
    VROInputPresenterCardboard(std::shared_ptr<VRORenderContext> context):VROInputPresenter(context) {
        getReticle()->setPointerMode(false);
    }
    ~VROInputPresenterCardboard() {}

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
