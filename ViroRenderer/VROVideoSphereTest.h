//
//  VROVideoSphereTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROVideoSphereTest_h
#define VROVideoSphereTest_h

#include "VRORendererTest.h"

class VROVideoSphereTest : public VRORendererTest {
public:
    
    VROVideoSphereTest();
    virtual ~VROVideoSphereTest();
    
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
    std::shared_ptr<VROVideoTexture> _videoTexture;
    
};

#endif /* VROVideoSphereTest_h */
