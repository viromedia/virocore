//
//  VROBodyTrackerTest.cpp
//  ViroSample
//
//  Created by Vik Advani
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

#include "VROBodyTrackerTest.h"
#include "VROTestUtil.h"
#include "VROSphere.h"
#include "VROBone.h"
#include "VRORenderer.h"
#include "VROInputControllerAR.h"
#include "VROBillboardConstraint.h"
#include "VROMatrix4f.h"
#include "VROBodyIKController.h"
#include "VROTypeface.h"
#include "VROSkinner.h"
#include "VROPencil.h"
#include "VROSkeleton.h"

#if VRO_PLATFORM_IOS
#include "VROBodyTrackeriOS.h"
#include "VROBodyTrackerYolo.h"
#include "VROBodyPlayeriOS.h"
#include "VRODriverOpenGLiOS.h"
#endif

VROBodyTrackerTest::VROBodyTrackerTest() :
VRORendererTest(VRORendererTestType::BodyTracker) {
}

VROBodyTrackerTest::~VROBodyTrackerTest() {
}

void VROBodyTrackerTest::build(std::shared_ptr<VRORenderer> renderer,
                               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                               std::shared_ptr<VRODriver> driver) {
    _driver = driver;
    _renderer = renderer;
    _sceneController = std::make_shared<VROARSceneController>();
    _sceneController->setDelegate(shared_from_this());
    
    // Set up the scene.
    _arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    _arScene->initDeclarativeSession();
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setIntensity(300);
    _arScene->getRootNode()->addLight(ambient);
    
    std::shared_ptr<VROLight> directional = std::make_shared<VROLight>(VROLightType::Directional);
    directional->setIntensity(600);
    directional->setDirection({ 0, 0, 1 });
    _arScene->getRootNode()->addLight(directional);
    
    //_arScene->displayPointCloud(true);

    // Set up the 3D Model to be animated
    VROVector3f pos = VROVector3f( 0, -1.5, -50);
    VROVector3f scale = VROVector3f(0.05, 0.05, 0.05);
    VROVector3f rot = VROVector3f(0, 0, 0);
    _modelNodeNinja1 = VROTestUtil::loadFBXModel("ninja/ninja",
                                                  pos,
                                                  scale, rot,
                                                  1, "", driver,
                                                  [this](std::shared_ptr<VRONode> node, bool success){
                                                      onModelLoaded(node);
                                                  });
    _modelNodeNinja1->setTag("Model");
    _modelNodeNinja1->setEventDelegate(shared_from_this());

    _modelNodeNinja2 = VROTestUtil::loadFBXModel("cute_monster/cute_monster",
                                                 pos,
                                                 scale, rot,
                                                 1, "", driver,
                                                 [this](std::shared_ptr<VRONode> node, bool success){
                                                     onModelLoaded(node);
                                                 });

    _modelNodeNinja2->setTag("Model");
    _modelNodeNinja2->setEventDelegate(shared_from_this());

    _gltfNodeContainer = std::make_shared<VRONode>();
    _gltfNodeContainer->setScale(VROVector3f(1.05,1.05,1.05));
    _arScene->getRootNode()->addChildNode(_gltfNodeContainer);

    // Create our bodyMLController and set register it as a VROBodyTrackerDelegate to VROBodyTracker
    createNewBodyController();

#if VRO_PLATFORM_IOS
    // Create body playback controller for recording.
    _bodyPlaybackController = std::make_shared<VROBodyIKController>(renderer, driver, _arScene->getRootNode());
    _bodyPlaybackController->setDelegate(shared_from_this());
    std::shared_ptr<VROBodyPlayeriOS> bodyPlayeriOS = std::make_shared<VROBodyPlayeriOS>();
    bodyPlayeriOS->setDelegate(_bodyPlaybackController);
    bodyPlayeriOS->initPlayer(frameSynchronizer);
    _bodyPlayer = bodyPlayeriOS;
#endif

    // Visually display the current tracked state fo the VROBodyController
    _trackingStateText = VROText::createText(L"< Body Tracking State >", "Helvetica", 21,
                                                        VROFontStyle::Normal, VROFontWeight::Regular, {1.0, 1.0, 1.0, 1.0}, 0, 5.2, 0.2,
                                                        VROTextHorizontalAlignment::Center, VROTextVerticalAlignment::Center,
                                                        VROLineBreakMode::WordWrap, VROTextClipMode::None, 0, driver);
    std::shared_ptr<VRONode> textNode = std::make_shared<VRONode>();
    textNode->setGeometry(_trackingStateText);
    textNode->setPosition(VROVector3f(0,0.165,0));
    textNode->setScale(VROVector3f(0.04, 0.04, 0.04));
    textNode->setRenderingOrder(11);
    _trackingStateText->setColor(VROVector4f(1.0, 0, 0, 1.0));
    _trackingStateText->getMaterials()[0]->setWritesToDepthBuffer(false);
    _trackingStateText->getMaterials()[0]->setReadsFromDepthBuffer(false);
    _gltfNodeContainer->addChildNode(textNode);

    // Create a recalibration box that the user can click to re-calibrate the model.
    std::shared_ptr<VRONode> debugNode
            = createTriggerBox(VROVector3f(0, 0, 3), VROVector4f(0, 1, 0, 1), "Recalibrate");
    _sceneController->getScene()->getRootNode()->addChildNode(debugNode);


    std::shared_ptr<VRONode> rebindNode
            = createTriggerBox(VROVector3f(-2, 0, 3), VROVector4f(1, 0, 0, 1), "Rebind");
    _sceneController->getScene()->getRootNode()->addChildNode(rebindNode);

    std::shared_ptr<VRONode> autoCalibrateNode
            = createTriggerBox(VROVector3f(2, 0, 3), VROVector4f(0, 0, 1, 1), "AutoCalibrate");
    _sceneController->getScene()->getRootNode()->addChildNode(autoCalibrateNode);

    std::shared_ptr<VRONode> recordNode
    = createTriggerBox(VROVector3f(0, -4, 3), VROVector4f(1, 1, 0, 1), "Record");
    _sceneController->getScene()->getRootNode()->addChildNode(recordNode);
    frameSynchronizer->addFrameListener(shared_from_this());
}

void VROBodyTrackerTest::createNewBodyTracker() {
#if VRO_PLATFORM_IOS
    VROViewAR *view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(_driver)->getView();

    std::shared_ptr<VROBodyTracker> tracker = std::make_shared<VROBodyTrackeriOS>();
    tracker->initBodyTracking(view.cameraPosition, _driver);
    tracker->startBodyTracking();
    tracker->setDelegate(_bodyMLController);
    _bodyTracker = tracker;

    std::shared_ptr<VROARSession> arSession = _arScene->getARSession();
    arSession->setVisionModel(_bodyTracker);
    return;
#endif

    pabort("Unable to create Body Tracker in Android!");
}

void VROBodyTrackerTest::createNewBodyController() {
    // Create our bodyMLController and set register it as a VROBodyTrackerDelegate to VROBodyTracker
    _bodyMLController = std::make_shared<VROBodyIKController>(_renderer, _driver, _arScene->getRootNode());
    _bodyMLController->setDelegate(shared_from_this());
    _bodyMLController->setDisplayDebugCubes(true);

#if VRO_PLATFORM_IOS
    if (_bodyTracker != nullptr) {
        _bodyTracker->setDelegate(_bodyMLController);
    }
    //bodyMLController->setDisplayDebugCubes(false);
    _bodyMLController->enableDebugMLViewIOS();
#endif
}

std::shared_ptr<VRONode> VROBodyTrackerTest::createTriggerBox(VROVector3f pos,
                                                              VROVector4f color,
                                                              std::string tag) {
    std::shared_ptr<VROBox> box = VROBox::createBox(1, 1, 1);
    std::shared_ptr<VROMaterial> mat = std::make_shared<VROMaterial>();
    mat->setReadsFromDepthBuffer(false);
    mat->setWritesToDepthBuffer(false);
    mat->getDiffuse().setColor(color);
    mat->setLightingModel(VROLightingModel::Lambert);
    
    std::vector<std::shared_ptr<VROMaterial>> mats;
    mats.push_back(mat);
    box->setMaterials(mats);
    
    std::shared_ptr<VRONode> debugNode = std::make_shared<VRONode>();
    debugNode->setGeometry(box);
    debugNode->setRenderingOrder(10);
    debugNode->setTag(tag);
    debugNode->setEventDelegate(shared_from_this());
    debugNode->setPosition(pos);
    setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    
    return debugNode;
}

void VROBodyTrackerTest::onModelLoaded(std::shared_ptr<VRONode> node) {
    std::vector<std::shared_ptr<VROSkinner>> skinners;
    node->getSkinner(skinners, true);
    if (skinners.size() < 0) {
        return;
    }
    _skinner = skinners[0];
}

void VROBodyTrackerTest::onBodyTrackStateUpdate(VROBodyTrackedState state){
    switch (state) {
        case NotAvailable:
            _trackingStateText->setText(L"< State: LOST >");
            break;
        case FullEffectors:
            _trackingStateText->setText(L"< State: Full >");
            break;
        case LimitedEffectors:
            _trackingStateText->setText(L"< State: Limited >");
            break;
        default:
            break;
    }
}

void VROBodyTrackerTest::onJointUpdate(const std::map<VROBodyJointType, VROJointPos> &mlJointsFiltered,
                                       const std::map<VROBodyJointType, VROVector3f> &mlJointsDampened,
                                       const std::map<VROBodyJointType, VROMatrix4f> &modelJoints) {

}

void VROBodyTrackerTest::onFrameWillRender(const VRORenderContext &context) {
    //context.getPencil()->setBrushThickness(0.001f);
    //renderDebugSkeletal(context.getPencil(), 0);
}

void VROBodyTrackerTest::onFrameDidRender(const VRORenderContext &context) {
}

void VROBodyTrackerTest::renderDebugSkeletal(std::shared_ptr<VROPencil> pencil, int jointIndex) {
    if (_skinner == nullptr) {
        return;
    }

    std::shared_ptr<VROSkeleton> skeleton = _skinner->getSkeleton();
    // First get all the child joints for this jointIndex
    std::vector<int> childBoneIndexes;
    for (int i = 0; i < skeleton->getNumBones(); i++) {
        const std::shared_ptr<VROBone> &bone = skeleton->getBone(i);
        if (bone->getParentIndex() == jointIndex && jointIndex != i){
            childBoneIndexes.push_back(i);
        }
    }

    if (childBoneIndexes.size() <=0) {
        return;
    }

    // Now draw a line from the child joint to it's parent.
    for (int childJointIndex : childBoneIndexes) {
        // Grab the animated bone in world space.
        VROMatrix4f animatedChild = _skinner->getSkeleton()->getCurrentBoneWorldTransform(childJointIndex);
        VROMatrix4f animatedParent = _skinner->getSkeleton()->getCurrentBoneWorldTransform(jointIndex);

        VROVector3f from = animatedChild.extractTranslation();
        VROVector3f to = animatedParent.extractTranslation();
        pencil->draw(from, to);

        // Now move down and treat the child as a parent
        renderDebugSkeletal(pencil, childJointIndex);
    }
}

