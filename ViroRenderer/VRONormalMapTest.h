//
//  VRONormalMapTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRONormalMapTest_h
#define VRONormalMapTest_h

#include "VRORendererTest.h"

class VRONormalMapTest : public VRORendererTest {
public:
    
    VRONormalMapTest();
    virtual ~VRONormalMapTest();
    
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
    float _objAngle;
    
};

#endif /* VRONormalMapTest_h */
