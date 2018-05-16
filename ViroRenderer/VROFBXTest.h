//
//  VROFBXTest.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROFBXTest_h
#define VROFBXTest_h

#include "VRORendererTest.h"

class VROFBXTest;

class VROFBXEventDelegate : public VROEventDelegate {
public:
    VROFBXEventDelegate(VROFBXTest *test) : _test(test) {};
    virtual ~VROFBXEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    VROFBXTest *_test;
};

class VROFBXModel {
public:
    std::string name;
    VROVector3f position;
    VROVector3f scale;
    int lightMask;
    std::string animation;
    
    VROFBXModel(std::string name, VROVector3f position, VROVector3f scale, int lightMask, std::string animation) :
        name(name), position(position), scale(scale), lightMask(lightMask), animation(animation) {}
    ~VROFBXModel() {}
};

class VROFBXTest : public VRORendererTest {
public:
    
    VROFBXTest();
    virtual ~VROFBXTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    void rotateFBX();
    
private:

    std::shared_ptr<VRODriver> _driver;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRONode> _fbxContainerNode;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    int _fbxIndex;
    float _angle;
    std::vector<VROFBXModel> _models;
    
};

#endif /* VROFBXTest_h */
