//
//  VRODiffuseIrradianceTest.h
//  ViroKit
//
//  Created by Raj Advani on 1/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VRODiffuseIrradianceTest_h
#define VRODiffuseIrradianceTest_h

#include "VRORendererTest.h"
#include "VROEventDelegate.h"

class VRODiffuseIrradianceTest;

class VROIBLEventDelegate : public VROEventDelegate {
public:
    VROIBLEventDelegate(std::shared_ptr<VROScene> scene, VRODiffuseIrradianceTest *test) : _scene(scene), _test(test) {};
    virtual ~VROIBLEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    std::weak_ptr<VROScene> _scene;
    VRODiffuseIrradianceTest *_test;
};

class VRODiffuseIrradianceTest : public VRORendererTest {
public:
    
    VRODiffuseIrradianceTest();
    virtual ~VRODiffuseIrradianceTest();
    
    void build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer, std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    void nextEnvironment();
    
private:
    
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    int _textureIndex;
    float _angle;
    
};

#endif /* VRODiffuseIrradianceTest_h */
