//
//  VROPBRDirectTest.h
//  ViroKit
//
//  Created by Raj Advani on 1/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROPBRDirectTest_h
#define VROPBRDirectTest_h

#include "VRORendererTest.h"

class VROPBRDirectTest : public VRORendererTest {
public:
    
    VROPBRDirectTest();
    virtual ~VROPBRDirectTest();
    
    void build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer, std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
private:
    
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    float _angle;
    
};

#endif /* VROPBRDirectTest_h */
