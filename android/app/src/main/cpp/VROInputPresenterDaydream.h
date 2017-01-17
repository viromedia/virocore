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

    void onClick(int source, ClickState clickState){
        if (source ==ViroDayDream::InputSource::TouchPad && clickState == ClickState::ClickUp){
            getReticle()->trigger();
        }
    }

    void onTouch(int source, TouchState touchState, float x, float y){
        /**
          * TODO VIRO-704: Implement UI Components for daydream controller.
          */
    }

    void onMove(int source, VROVector3f rotation, VROVector3f position){
        /**
         * TODO VIRO-704: Implement UI Components for daydream controller.
         */
    }

    void onGazeHit(int source, float distance, VROVector3f hitLocation){
        VROInputPresenter::onReticleGazeHit(distance, hitLocation);
    }
};
#endif
