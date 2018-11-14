//
//  VROBodyTrackerTest.h
//  ViroSample
//
//  Created by vik.advani on 9/7/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
//
//  VROBodyTrackerTest.h
//  ViroSample
//

#ifndef VROBodyTrackerTest_h
#define VROBodyTrackerTest_h

#include "VRORendererTest.h"
#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VRODefines.h"
#include "VROBodyTracker.h"

#if VRO_PLATFORM_IOS
#include "VROARSessioniOS.h"
#include "VROViewAR.h"
#import <UIKit/UIKit.h>
#endif

class VROBodyTrackerTest : public VRORendererTest, public VROSceneController::VROSceneControllerDelegate,
                           public VROBodyTrackerDelegate,
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
        // Commented out for Viro React release 2.12.0
        //arSessioniOS->setBodyTracker(_bodyTracker);
#endif
    }
    virtual void onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {

    }
    virtual void onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    }
    
    virtual void onBodyJointsFound(const std::map<VROBodyJointType, VROBodyJoint> &joints);
    
private:
    
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROARScene> _arScene;
    std::vector<std::shared_ptr<VRONode>> _bodyPointsSpheres;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROBodyTracker> _bodyTracker;

#if VRO_PLATFORM_IOS
    UIView *_bodyViews[14];
    VROViewAR *_view;
#endif

};

#endif /* VROARObjectTrackingTest_h  */
