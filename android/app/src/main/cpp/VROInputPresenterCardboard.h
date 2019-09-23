//
//  VROInputPresenterCardboard.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
