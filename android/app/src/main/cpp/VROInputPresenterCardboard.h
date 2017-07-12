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
#include <VROPlatformUtil.h>

#include "VROFrameListener.h"
#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROEventDelegate.h"

class VROInputPresenterCardboard : public VROInputPresenter {
public:
    VROInputPresenterCardboard() {
        setReticle(std::make_shared<VROReticle>(nullptr));
        getReticle()->setPointerMode(false);
    }
    ~VROInputPresenterCardboard() {}

    void onClick(int source, ClickState clickState, std::vector<float> position) {
        if (source==ViroCardBoard::InputSource::ViewerButton && clickState==ClickState::ClickUp){
            getReticle()->trigger();
        }

        VROInputPresenter::onClick(source, clickState, position);
    }

    void onGazeHit(int source, const VROHitTestResult &hit) {
        VROInputPresenter::onReticleGazeHit(hit);
     }
};
#endif
