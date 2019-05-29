//
//  VROBodyMesherTest.h
//  ViroKit
//
//  Created by Raj Advani on 5/23/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROBodyMesherTest_h
#define VROBodyMesherTest_h

#include "VRORendererTest.h"
#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VRODefines.h"
#include "VROBodyMesher.h"
#include "VROARSession.h"

#if VRO_PLATFORM_IOS
#import "VROViewAR.h"
#import "VROBodySurfaceRenderer.h"
#endif

class VROPoseFilter;

class VROBodyMesherTest : public VRORendererTest, public VROSceneController::VROSceneControllerDelegate, public VROBodyMesherDelegate, public std::enable_shared_from_this<VROBodyMesherTest> {
public:
    
    VROBodyMesherTest();
    virtual ~VROBodyMesherTest();
    
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
        std::shared_ptr<VROARSession> arSession = _arScene->getARSession();
        arSession->setVisionModel(_bodyMesher);
    }
    
    virtual void onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
        
    }
    virtual void onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    }
    
    virtual void onBodyMeshUpdated(const std::vector<float> &vertices, std::shared_ptr<VROGeometry> mesh);
private:
    
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROARScene> _arScene;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROBodyMesher> _bodyMesher;
    
    std::shared_ptr<VRONode> _bodyMeshNode;
    std::shared_ptr<VROGeometry> _bodyMesh;
    
#if VRO_PLATFORM_IOS
    std::shared_ptr<VROBodySurfaceRenderer> _surfaceRenderer;
#endif
};

#endif /* VROBodyMesherTest_h */
