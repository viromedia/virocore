//
//  VROPBRTexturedTest.h
//  ViroKit
//
//  Created by Raj Advani on 1/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROPBRTexturedTest_h
#define VROPBRTexturedTest_h

#include "VRORendererTest.h"

class VROPBRTexturedTest : public VRORendererTest {
public:
    
    VROPBRTexturedTest();
    virtual ~VROPBRTexturedTest();
    
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

#endif /* VROPBRTexturedTest_h */
