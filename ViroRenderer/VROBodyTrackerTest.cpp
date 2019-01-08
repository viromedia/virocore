//
//  VROBodyTrackerTest.cpp
//  ViroSample
//
//  Created by Vik Advani
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROBodyTrackerTest.h"
#include "VROTestUtil.h"
#include "VROSphere.h"
#include "VROBone.h"
#include "VRORenderer.h"
#include "VROInputControllerAR.h"
#include "VROBillboardConstraint.h"
#include "VROMatrix4f.h"
#include "VROBodyTrackerController.h"
#include "VROTypeface.h"

#if VRO_PLATFORM_IOS
#include "VROBodyTrackeriOS.h"
#include "VRODriverOpenGLiOS.h"
static std::string pointLabels[14] = {
    "top\t\t\t", //0
    "neck\t\t", //1
    "R shoulder\t", //2
    "R elbow\t\t", //3
    "R wrist\t\t", //4
    "L shoulder\t", //5
    "L elbow\t\t", //6
    "L wrist\t\t", //7
    "R hip\t\t", //8
    "R knee\t\t", //9
    "R ankle\t\t", //10
    "L hip\t\t", //11
    "L knee\t\t", //12
    "L ankle\t\t", //13
};

static UIColor *colors[14] = {
    [UIColor redColor],
    [UIColor greenColor],
    [UIColor blueColor],
    [UIColor cyanColor],
    [UIColor yellowColor],
    [UIColor magentaColor],
    [UIColor orangeColor],
    [UIColor purpleColor],
    [UIColor brownColor],
    [UIColor blackColor],
    [UIColor darkGrayColor],
    [UIColor lightGrayColor],
    [UIColor whiteColor],
    [UIColor grayColor]
};

#endif

VROBodyTrackerTest::VROBodyTrackerTest() :
VRORendererTest(VRORendererTestType::BodyTracker) {
}

VROBodyTrackerTest::~VROBodyTrackerTest() {
}

void VROBodyTrackerTest::build(std::shared_ptr<VRORenderer> renderer,
                               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                               std::shared_ptr<VRODriver> driver) {
    _renderer = renderer;
    _sceneController = std::make_shared<VROARSceneController>();
    _sceneController->setDelegate(shared_from_this());

    // Set up the scene.
    _arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    _arScene->initDeclarativeSession();
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setIntensity(1000);
    _arScene->getRootNode()->addLight(ambient);

    // Set up the 3D Model to be animated
    std::shared_ptr<VRONode> rootModelNode;
    VROVector3f pos = VROVector3f( 0, -1.5, -50);
    VROVector3f scale = VROVector3f(0.05, 0.05, 0.05);
    VROVector3f rot = VROVector3f(0,0,0);
    rootModelNode = VROTestUtil::loadFBXModel("ninja/ninja",
                                                  pos,
                                                  scale, rot,
                                                  1, "", driver,
                                                  [this](std::shared_ptr<VRONode> node, bool success){
                                                      onModelLoaded(node);
                                                  });

    _gltfNodeContainer = std::make_shared<VRONode>();
    _gltfNodeContainer->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::Y));
    _gltfNodeContainer->addChildNode(rootModelNode);
    _gltfNodeContainer->setScale(VROVector3f(1.05,1.05,1.05));
    _arScene->getRootNode()->addChildNode(_gltfNodeContainer);

    // Create our bodyMLController and set register it as a VROBodyTrackerDelegate to VROBodyTracker
    _bodyMLController = std::make_shared<VROBodyTrackerController>(renderer, _arScene->getRootNode());
    _bodyMLController->setDelegate(shared_from_this());

    #if VRO_PLATFORM_IOS
    std::shared_ptr<VROBodyTrackeriOS> trackeriOS = std::make_shared<VROBodyTrackeriOS>();
    trackeriOS->initBodyTracking(VROCameraPosition::Back, driver);
    trackeriOS->startBodyTracking();
    trackeriOS->setDelegate(_bodyMLController);
    _bodyTracker = trackeriOS;
    _bodyMLController->enableDebugMLViewIOS(driver);
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
    std::shared_ptr<VROBox> box = VROBox::createBox(1, 1, 1);
    std::shared_ptr<VROMaterial> mat = std::make_shared<VROMaterial>();
    mat->setCullMode(VROCullMode::None);
    mat->setReadsFromDepthBuffer(false);
    mat->setWritesToDepthBuffer(false);
    mat->getDiffuse().setColor(VROVector4f(0.0, 1.0, 0, 1.0));

    std::vector<std::shared_ptr<VROMaterial>> mats;
    mats.push_back(mat);
    box->setMaterials(mats);

    std::shared_ptr<VRONode> debugNode = std::make_shared<VRONode>();
    debugNode->setGeometry(box);
    debugNode->setRenderingOrder(10);
    debugNode->setTag("Recalibrate");
    debugNode->setEventDelegate(shared_from_this());
    debugNode->setPosition(VROVector3f(-3,0,-2));
    setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    _sceneController->getScene()->getRootNode()->addChildNode(debugNode);
}

void VROBodyTrackerTest::onModelLoaded(std::shared_ptr<VRONode> node) {
    _bodyMLController->bindModel(node);
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
    }
}
