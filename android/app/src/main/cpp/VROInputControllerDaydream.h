//
//  VROInputControllerDaydream.h
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

#ifndef VROInputControllerDaydream_H
#define VROInputControllerDaydream_H

#include <memory>
#include <string>
#include <vector>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_controller.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include "VROFrameSynchronizer.h"
#include "VROFrameListener.h"
#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROEventDelegate.h"
#include "VROInputPresenterDaydream.h"

class VROInputControllerDaydream : public VROInputControllerBase {

public:
    VROInputControllerDaydream(gvr_context *gvr_context, std::shared_ptr<VRODriver> driver);
    virtual ~VROInputControllerDaydream();

    virtual VROVector3f getDragForwardOffset();

    void onProcess(const VROCamera &camera);
    void onPause();
    void onResume();

    virtual std::string getHeadset() {
        return "daydream";
    }

    virtual std::string getController() {
        return "daydream";
    }

protected:
    std::shared_ptr<VROInputPresenter> createPresenter(std::shared_ptr<VRODriver> driver) {
        _daydreamPresenter = std::make_shared<VROInputPresenterDaydream>(driver);
        return _daydreamPresenter;
    }

private:
    // Daydream-specific event update functions
    bool isControllerReady();
    void updateOrientation(const VROCamera &camera);
    void updateButtons();
    void updateTouchPad();
    void notifyButtonEventForType(gvr::ControllerButton button, ViroDayDream::InputSource source);
    void updateSwipeGesture(VROVector3f start, VROVector3f end);
    void updateScrollGesture(VROVector3f start, VROVector3f end);
    VROVector3f getDaydreamForwardVector(const VROQuaternion rotation);
    VROVector3f getDaydreamControllerPosition(const VROQuaternion rotation,
                                              const VROVector3f forward,
                                              const VROVector3f cameraPos);

    std::unique_ptr<gvr::ControllerApi> _gvr_controller;
    gvr::ControllerState _controller_state;
    std::shared_ptr<VROInputPresenterDaydream> _daydreamPresenter;
    bool _hasInitalized;
    VROVector3f _touchDownLocationStart;
    gvr_context *_gvrContext;
};
#endif