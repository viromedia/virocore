//
//  VROBodyMeshingTest.h
//  ViroSample
//
//  Created by vik.advani on 9/7/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
//
//  VROBodyMeshingTest.h
//  ViroSample
//

#ifndef VROBodyMeshingTest_h
#define VROBodyMeshingTest_h

#include "VRORendererTest.h"
#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VROARBodyMeshingPointsiOS.h"
#include "VROARSessioniOS.h"



class VROBodyMeshingTest : public VRORendererTest, public VROSceneController::VROSceneControllerDelegate, public VROARBodyMeshingPointsiOS::VROBodyMeshingDelegate,
public std::enable_shared_from_this<VROBodyMeshingTest> {
public:
    
    VROBodyMeshingTest();
    virtual ~VROBodyMeshingTest();
    
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
        std::shared_ptr<VROARDeclarativeSession> arDeclarativeSession = _arScene->getDeclarativeSession();
        std::weak_ptr<VROARSession> arSessionWeak = arDeclarativeSession->getARSession();
        std::shared_ptr<VROARSessioniOS> arSessioniOS = std::dynamic_pointer_cast<VROARSessioniOS>(arSessionWeak.lock());
        arSessioniOS->setBodyMeshing(_bodyMeshingPoints);
    }
    virtual void onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {

    }
    virtual void onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    }
    
    virtual void onBodyMeshJointsAvail(NSDictionary *joints);
private:
    
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROARBodyMeshingPointsiOS> _bodyMeshingPoints;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROARScene> _arScene;
    std::vector<std::shared_ptr<VRONode>> _bodyPointsSpheres;
    std::shared_ptr<VRORenderer> _renderer;

};




#endif /* VROARObjectTrackingTest_h  */
