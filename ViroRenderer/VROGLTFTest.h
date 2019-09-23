//
//  VROGLTFTest.h
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
