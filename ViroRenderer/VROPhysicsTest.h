//
//  VROPhysicsTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROPhysicsTest_h
#define VROPhysicsTest_h

#include "VRORendererTest.h"

class VROPhysicsTest;

class VROPhysicsEventDelegate : public VROEventDelegate {
public:
    VROPhysicsEventDelegate(VROPhysicsTest *test) : _test(test) {};
    virtual ~VROPhysicsEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    VROPhysicsTest *_test;
};

class VROPhysicsTest : public VRORendererTest {
public:
    
    VROPhysicsTest();
    virtual ~VROPhysicsTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    std::shared_ptr<VRONode> createPhysicsBox(VROVector3f position, std::string tag);
    
private:

    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    std::shared_ptr<VRONode> _rootNode;
    
};

#endif /* VROPhysicsTest_h */
