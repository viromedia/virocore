//
//  VROParticleTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROParticleTest_h
#define VROParticleTest_h

#include "VRORendererTest.h"

class VROParticleTest;

class VROParticleEventDelegate : public VROEventDelegate {
public:
  VROParticleEventDelegate(VROParticleTest *test) : _test(test) {};
  virtual ~VROParticleEventDelegate() {};
  void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
  void onPinch(int source, std::shared_ptr<VRONode> node, float scaleFactor, PinchState pinchState);
  
  void onRotate(int source, std::shared_ptr<VRONode> node, float rotationRadians, RotateState rotateState);
  
private:
  VROParticleTest *_test;
};

class VROParticleTest : public VRORendererTest {
public:
    
    VROParticleTest();
    virtual ~VROParticleTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
  
    void toggleParticleEmitterPause();
private:

    std::shared_ptr<VROParticleEmitter> _emitter;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROParticleEventDelegate> _eventDelegate;
    
};

#endif /* VROParticleTest_h */
