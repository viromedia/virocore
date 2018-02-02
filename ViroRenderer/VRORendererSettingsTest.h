//
//  VRORendererSettingsTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRORendererSettingsTest_h
#define VRORendererSettingsTest_h

#include "VRORendererTest.h"

class VRORendererSettingsTest;

class VRORendererSettingsEventDelegate : public VROEventDelegate {
public:
    VRORendererSettingsEventDelegate(VRORendererSettingsTest *test) : _test(test) {};
    virtual ~VRORendererSettingsEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    VRORendererSettingsTest *_test;
};

class VRORendererSettingsTest : public VRORendererTest {
public:
    
    VRORendererSettingsTest();
    virtual ~VRORendererSettingsTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    void changeSettings();
    
private:

    int _index;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    
};

#endif /* VRORendererSettingsTest_h */
