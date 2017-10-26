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

    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position) {
        VROInputPresenter::onClick(source, node, clickState, position);
        if (source==ViroOculus::InputSource::TouchPad && clickState==ClickState::ClickUp){
            getReticle()->trigger();
        }
    }

    void onGazeHit(int source, std::shared_ptr<VRONode> node, const VROHitTestResult &hit) {
        VROInputPresenter::onReticleGazeHit(hit);
     }
};
#endif
