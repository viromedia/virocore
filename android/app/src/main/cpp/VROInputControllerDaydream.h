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

    void onProcess();
    void onPause();
    void onResume();

protected:
    std::shared_ptr<VROInputPresenter> createPresenter(std::shared_ptr<VRORenderContext> context){
        return std::make_shared<VROInputPresenterDaydream>(context);
    }

private:
    // Daydream-specific event update functions
    bool isControllerReady();
    void updateOrientation();
    void updateButtons();
    void updateTouchPad();
    void notifyButtonEventForType(gvr::ControllerButton button, VROEventDelegate::EventSource type);

    std::unique_ptr<gvr::ControllerApi> _gvr_controller;
    gvr::ControllerState _controller_state;
    bool _hasInitalized;
};
#endif