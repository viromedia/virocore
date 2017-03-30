//
//  VROInputPresenterCardboard.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROInputPresenterOVR_H
#define VROInputPresenterOVR_H

#include <memory>
#include <string>
#include <vector>
#include <VROReticle.h>

#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROEventDelegate.h"

class VROInputPresenterOVR : public VROInputPresenter {
public:
    VROInputPresenterOVR() {
        setReticle(std::make_shared<VROReticle>(nullptr));
        getReticle()->setPointerMode(false);
    }
    ~VROInputPresenterOVR() {}

    void onClick(int source, ClickState clickState){
        VROInputPresenter::onClick(source, clickState);
        if (source==ViroOculus::InputSource::TouchPad && clickState==ClickState::ClickUp){
            getReticle()->trigger();
        }
    }

    void onGazeHit(int source, const VROHitTestResult &hit) {
        VROInputPresenter::onReticleGazeHit(hit);
     }
};
#endif
