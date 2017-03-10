//
//  VROInputControllerDaydream.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

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
    VROInputControllerDaydream(gvr_context *gvr_context);
    virtual ~VROInputControllerDaydream();

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
    std::shared_ptr<VROInputPresenter> createPresenter(std::shared_ptr<VRORenderContext> context){
        _daydreamPresenter = std::make_shared<VROInputPresenterDaydream>(context);
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
    VROVector3f getDaydreamControllerPosition(const VROQuaternion rotation);

    std::unique_ptr<gvr::ControllerApi> _gvr_controller;
    gvr::ControllerState _controller_state;
    std::shared_ptr<VROInputPresenterDaydream> _daydreamPresenter;
    bool _hasInitalized;
    VROVector3f _touchDownLocationStart;
};
#endif