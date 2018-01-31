//
//  VROPhotometricLightTest.h
//  ViroKit
//
//  Created by Raj Advani on 1/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROPhotometricLightTest_h
#define VROPhotometricLightTest_h

#include "VRORendererTest.h"
#include "VROEventDelegate.h"

class VROPhotometricLightTest;

class VROPhotometricLightEventDelegate : public VROEventDelegate {
public:
    VROPhotometricLightEventDelegate(VROPhotometricLightTest *test) : _test(test) {};
    virtual ~VROPhotometricLightEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    VROPhotometricLightTest *_test;
};

class VROPhotometricLightTest : public VRORendererTest {
public:
    
    VROPhotometricLightTest();
    virtual ~VROPhotometricLightTest();
    
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
    int _textureIndex;
    float _angle;
    
};

#endif /* VROPhotometricLightTest_h */
