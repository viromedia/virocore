//
//  VROHDRTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROHDRTest_h
#define VROHDRTest_h

#include "VRORendererTest.h"

class VROHDRTest;

class VROHDREventDelegate : public VROEventDelegate {
public:
    VROHDREventDelegate(VROHDRTest *test) : _test(test) {};
    virtual ~VROHDREventDelegate() {};
    void onClick(int source, ClickState clickState, std::vector<float> position);
    
private:
    VROHDRTest *_test;
};


class VROHDRTest : public VRORendererTest {
public:
    
    VROHDRTest();
    virtual ~VROHDRTest();
    
    void build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    void changeScene();
    
private:

    int _activeScene;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    
    std::shared_ptr<VRONode> buildBoxScene();
    std::shared_ptr<VRONode> buildWoodenDoorScene();
    std::shared_ptr<VRONode> buildPlayaScene();
    std::shared_ptr<VRONode> buildIndoorScene();
    
};

#endif /* VROHDRTest_h */
