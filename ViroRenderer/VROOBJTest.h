//
//  VROOBJTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROOBJTest_h
#define VROOBJTest_h

#include "VRORendererTest.h"

class VROOBJTest : public VRORendererTest {
public:
    
    VROOBJTest();
    virtual ~VROOBJTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    std::shared_ptr<VRONode> loadOBJ(std::shared_ptr<VRODriver> driver);
    
private:

    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    float _objAngle;
    
};

#endif /* VROOBJTest_h */
