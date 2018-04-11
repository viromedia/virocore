//
//  VROBoxTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROBoxTest_h
#define VROBoxTest_h

#include "VRORendererTest.h"

class VROBoxEventDelegate : public VROEventDelegate {
public:
    VROBoxEventDelegate(std::shared_ptr<VROScene> scene) : _scene(scene) {};
    virtual ~VROBoxEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    std::weak_ptr<VROScene> _scene;
};

class VROBoxTest : public VRORendererTest {
public:
    
    VROBoxTest();
    virtual ~VROBoxTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
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
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    
    std::shared_ptr<VRONode> buildTransparentFrontBox();
};

#endif /* VROBoxTest_h */
