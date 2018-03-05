//
//  VROInputPresenterWasm.h
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROInputPresenterWasm_H
#define VROInputPresenterWasm_H

#include <memory>
#include <string>
#include <vector>
#include "VRODefines.h"
#include "VROReticle.h"
#include "VROEventDelegate.h"

class VROInputPresenterWasm : public VROInputPresenter {
public:
    VROInputPresenterWasm() {
        setReticle(std::make_shared<VROReticle>(nullptr));
        getReticle()->setPointerFixed(true);
    }
    ~VROInputPresenterWasm() {}

    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position) {
        getReticle()->trigger();
        VROInputPresenter::onClick(source, node, clickState, position);
    }

    void onGazeHit(int source, std::shared_ptr<VRONode> node, const VROHitTestResult &hit) {
        VROInputPresenter::onReticleGazeHit(hit);
     }
};
#endif
