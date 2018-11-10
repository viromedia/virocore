//
//  VROGLTFTest.h
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROGLTFTest_h
#define VROGLTFTest_h

#include "VRORendererTest.h"
#include "VROEventDelegate.h"
#include "VROMorpher.h"

class VROGLTFTest;

class VROGLTFEventDelegate : public VROEventDelegate {
public:
    VROGLTFEventDelegate(VROGLTFTest *test) : _test(test) {};
    virtual ~VROGLTFEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    
private:
    VROGLTFTest *_test;
};

class VROGLTFModel {
public:
    std::string name;
    std::string ext;
    VROVector3f position;
    VROVector3f scale;
    int lightMask;
    std::string animation;

    VROGLTFModel(std::string name, std::string ext, VROVector3f position, VROVector3f scale, int lightMask, std::string animation) :
        name(name), ext(ext), position(position), scale(scale), lightMask(lightMask), animation(animation) {}
    ~VROGLTFModel() {}
};

class VROGLTFTest : public VRORendererTest {
public:

    VROGLTFTest();
    virtual ~VROGLTFTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    void rotateModel();
    void animate(std::shared_ptr<VRONode> gltfNode, std::string name);
    
private:
    VROMorpher::ComputeLocation _computeLocation;
    std::shared_ptr<VRODriver> _driver;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRONode> _gltfContainerNode;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    int _gltfIndex;
    float _angle;
    std::vector<VROGLTFModel> _models;
    
};

#endif /* VROGLTFTest_h */
