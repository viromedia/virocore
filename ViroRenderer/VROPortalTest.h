//
//  VROPortalTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROPortalTest_h
#define VROPortalTest_h

#include "VRORendererTest.h"

class VROPortalTest : public VRORendererTest {
public:
    
    VROPortalTest();
    virtual ~VROPortalTest();
    
    void build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
private:

    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROPortalTraversalListener> _portalTraversalListener;
    
    std::shared_ptr<VROPortalFrame> loadPortalEntrance();
    std::shared_ptr<VROPortalFrame> loadFBXPortalEntrance(std::string fbxPath, float scale);
    
};

#endif /* VROPortalTest_h */
