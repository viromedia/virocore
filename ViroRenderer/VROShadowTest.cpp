//
//  VROShadowTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROShadowTest.h"
#include "VROTestUtil.h"

VROShadowTest::VROShadowTest() :
    VRORendererTest(VRORendererTestType::Shadow) {
        
}

VROShadowTest::~VROShadowTest() {
    
}

void VROShadowTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                          std::shared_ptr<VRODriver> driver) {
    
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.1, 0.1, 0.1 });
    
    std::shared_ptr<VROLight> spotRed = std::make_shared<VROLight>(VROLightType::Spot);
    spotRed->setColor({ 1.0, 0.2, 0.2 });
    spotRed->setPosition( { 5, 5, -3 });
    spotRed->setDirection( { -.25, -1.0, 0 });
    spotRed->setAttenuationStartDistance(20);
    spotRed->setAttenuationEndDistance(30);
    spotRed->setSpotInnerAngle(15);
    spotRed->setSpotOuterAngle(10);
    spotRed->setShadowNearZ(1);
    spotRed->setShadowFarZ(10);
    spotRed->setCastsShadow(true);
    
    std::shared_ptr<VROLight> spotBlue = std::make_shared<VROLight>(VROLightType::Spot);
    spotBlue->setColor({ 0.2, 0.2, 1.0 });
    spotBlue->setPosition( { -3, 5, -5 });
    spotBlue->setDirection( { 0.25, -1.0, 0 });
    spotBlue->setShadowNearZ(1);
    spotBlue->setShadowFarZ(10);
    
    spotBlue->setAttenuationStartDistance(20);
    spotBlue->setAttenuationEndDistance(30);
    spotBlue->setSpotInnerAngle(30);
    spotBlue->setSpotOuterAngle(15);
    spotBlue->setCastsShadow(true);
    
    rootNode->addLight(ambient);
    rootNode->addLight(spotRed);
    rootNode->addLight(spotBlue);
    
    std::shared_ptr<VROTexture> bobaTexture = VROTestUtil::loadDiffuseTexture("boba");
    bobaTexture->setWrapS(VROWrapMode::Repeat);
    bobaTexture->setWrapT(VROWrapMode::Repeat);
    bobaTexture->setMinificationFilter(VROFilterMode::Linear);
    bobaTexture->setMagnificationFilter(VROFilterMode::Linear);
    bobaTexture->setMipFilter(VROFilterMode::Linear);
    
    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(0.5, 1.0, 0.5);
    box->setName("Box 1");
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getDiffuse().setTexture(bobaTexture);
    material->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    material->getSpecular().setTexture(VROTestUtil::loadSpecularTexture("specular"));
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({ 0, 0, -6 });
    rootNode->addChildNode(boxNode);
    
    /*
     Create a surface behind the box.
     */
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(40, 40);
    surface->setName("Surface");
    surface->getMaterials().front()->setLightingModel(VROLightingModel::Lambert);
    
    std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
    surfaceNode->setGeometry(surface);
    surfaceNode->setRotationEuler({ -M_PI_2, 0, 0 });
    surfaceNode->setPosition({0, -3, -6});
    surfaceNode->setOpacity(0.8);
    rootNode->addChildNode(surfaceNode);
    
    VROTransaction::begin();
    VROTransaction::setAnimationDelay(2);
    VROTransaction::setAnimationDuration(10);
    boxNode->setPositionX(2);
    boxNode->setPositionZ(-2.75);
    boxNode->setPositionY(-2.75);
    boxNode->setRotationEulerX(M_PI_2);
    VROTransaction::setFinishCallback([boxNode] {
        VROTransaction::begin();
        VROTransaction::setAnimationDelay(2);
        VROTransaction::setAnimationDuration(10);
        boxNode->setPositionX(8);
        boxNode->setPositionY(3);
        boxNode->setRotationEulerX(0);
        VROTransaction::commit();
    });
    VROTransaction::commit();
}
