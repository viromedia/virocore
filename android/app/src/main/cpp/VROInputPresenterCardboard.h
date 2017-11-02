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
        getReticle()->setPointerFixed(true);
    }
    ~VROInputPresenterCardboard() {}

    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position) {
        if (source==ViroCardBoard::InputSource::ViewerButton && clickState==ClickState::ClickUp){
            getReticle()->trigger();
        }

        VROInputPresenter::onClick(source, node, clickState, position);
    }

    void onGazeHit(int source, std::shared_ptr<VRONode> node, const VROHitTestResult &hit) {
        VROInputPresenter::onReticleGazeHit(hit);
     }
};
#endif
