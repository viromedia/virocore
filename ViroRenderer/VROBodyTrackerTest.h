//
//  VROBodyTrackerTest.h
//  ViroSample
//
//  Created by vik.advani on 9/7/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROBodyTrackerTest_h
#define VROBodyTrackerTest_h

#include "VRORendererTest.h"
#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VRODefines.h"
#include "VROBodyTracker.h"
#include "VROEventDelegate.h"
#include "VROBodyTrackerController.h"

#if VRO_PLATFORM_IOS
#include "VROARSessioniOS.h"
#import <UIKit/UIKit.h>
#endif

class VROBodyTrackerTest : public VRORendererTest,
                           public VROEventDelegate,
                           public VROSceneController::VROSceneControllerDelegate,
                           public VROBodyTrackerControllerDelegate,
                           public VROFrameListener,
                           public std::enable_shared_from_this<VROBodyTrackerTest> {
public:
    
    VROBodyTrackerTest();
    virtual ~VROBodyTrackerTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);

    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    virtual void onSceneWillAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    }
    virtual void onSceneDidAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
#if VRO_PLATFORM_IOS
        std::shared_ptr<VROARSession> arSession = _arScene->getARSession();
        std::shared_ptr<VROARSessioniOS> arSessioniOS = std::dynamic_pointer_cast<VROARSessioniOS>(arSession);
        arSessioniOS->setVisionModel(_bodyTracker);
#endif
    }
    virtual void onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {

    }
    virtual void onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    }

    virtual void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position) {
        if (VROStringUtil::strcmpinsensitive(node->getTag(), "Recalibrate")) {
            _bodyMLController->startCalibration();
        }
    }

    void onModelLoaded(std::shared_ptr<VRONode> node);
    void onBodyTrackStateUpdate(VROBodyTrackedState state);
    void renderDebugSkeletal(std::shared_ptr<VROPencil> pencil, int jointIndex);
    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

private:
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROARScene> _arScene;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRONode> _gltfNodeContainer;
    std::shared_ptr<VROBodyTracker> _bodyTracker;
    std::shared_ptr<VROBodyTrackerController> _bodyMLController;
    std::shared_ptr<VROText> _trackingStateText;
    std::shared_ptr<VROSkinner> _skinner;
};

#endif /* VROBodyTrackerTest_h  */
