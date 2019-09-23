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

#ifndef VROIKTest_h
#define VROIKTest_h

#include "VRORendererTest.h"
#include "VROEventDelegate.h"

class VROPencil;
class VROIKEventDelegate;
class VROIKTest : public VROFrameListener, public VRORendererTest, public std::enable_shared_from_this<VROFrameListener> {
public:
    VROIKTest();
    virtual ~VROIKTest();

    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);

    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }

    void testSingleHorizontalChainRig();
    void testSingleVerticalChainRig();
    void testSingleTJointRig();
    void testSingleTJointRigComplex();
    void test3DSkinner(std::shared_ptr<VRODriver> driver);

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

    void onClick(int source, std::shared_ptr<VRONode> node, VROEventDelegate::ClickState clickState, std::vector<float> position);
    void onDrag(int source, std::shared_ptr<VRONode> node, VROVector3f newPosition);
private:
    bool _is3DModelTest;
    std::vector<std::shared_ptr<VRONode>> _targetBoxes;

    std::shared_ptr<VRODriver> _driver;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROEventDelegate> _eventDelegate;
    std::shared_ptr<VRONode> _pointOfView;

    std::shared_ptr<VRONode> createBlock(bool draggable, std::string tag, VROVector4f color);
    std::shared_ptr<VRONode> _currentRoot;
    std::shared_ptr<VROPolyline> _debugRigSkeletalLine;

    std::shared_ptr<VROIKRig> _rig;
    std::shared_ptr<VROSkinner> _skinner;
    std::map<std::string, int> _endEffectorBones;
    void initSkinner(std::shared_ptr<VRONode> gltfNode);
    void restoreTopBoneTransform();
    void scaleBoneTransform(std::string from, std::string to, float ratio, VROVector3f scaleDirection);

    void refreshSkeletalRig();
    std::shared_ptr<VRONode> createGLTFEffectorBlock(bool isAffector, std::string tag, VROVector4f color);
    void calculateSkeletalLines(std::shared_ptr<VRONode> node,
                                std::vector<std::vector<VROVector3f>> &paths);
    void renderDebugSkeletal(std::shared_ptr<VROPencil> pencil, int jointIndex);
};

class VROIKEventDelegate : public VROEventDelegate {
public:
    VROIKEventDelegate(VROIKTest *test) : _test(test) {};
    virtual ~VROIKEventDelegate() {};
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position){
        _test->onClick(source, node, clickState, position);
    }
    void onDrag(int source, std::shared_ptr<VRONode> node, VROVector3f newPosition){
        _test->onDrag(source, node, newPosition);
    }
private:
    VROIKTest *_test;
};

#endif /* VROIKTest_h */
