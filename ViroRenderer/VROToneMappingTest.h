//
//  VROToneMappingTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROToneMappingTest_h
#define VROToneMappingTest_h

#include "VRORendererTest.h"

class VROToneMappingEventDelegate : public VROEventDelegate {
public:
    VROToneMappingEventDelegate(std::shared_ptr<VROScene> scene, std::shared_ptr<VROText> text) : _scene(scene), _text(text), _state(0) {};
    virtual ~VROToneMappingEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    std::weak_ptr<VROScene> _scene;
    std::shared_ptr<VROText> _text;
    int _state;
};

class VROToneMappingTest : public VRORendererTest {
public:
    
    VROToneMappingTest();
    virtual ~VROToneMappingTest();
    
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

};

#endif /* VROToneMappingTest_h */
