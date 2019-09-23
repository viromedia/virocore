//
//  VROBoxTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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

#include "VROBoxTest.h"
#include "VROTestUtil.h"
#include "VROCompress.h"
#include "VROToneMappingRenderPass.h"

VROBoxTest::VROBoxTest() :
    VRORendererTest(VRORendererTestType::Box) {
        
}

VROBoxTest::~VROBoxTest() {
    
}

void VROBoxTest::build(std::shared_ptr<VRORenderer> renderer,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    
    std::shared_ptr<VROLight> spotRed = std::make_shared<VROLight>(VROLightType::Spot);
    spotRed->setColor({ 1.0, 0.0, 0.0 });
    spotRed->setPosition( { -5, 0, 0 });
    spotRed->setDirection( { 1.0, 0, -1.0 });
    spotRed->setAttenuationStartDistance(20);
    spotRed->setAttenuationEndDistance(30);
    spotRed->setSpotInnerAngle(5);
    spotRed->setSpotOuterAngle(15);
    
    std::shared_ptr<VROLight> spotBlue = std::make_shared<VROLight>(VROLightType::Spot);
    spotBlue->setColor({ 0.0, 0.0, 1.0 });
    spotBlue->setPosition( { 5, 0, 0 });
    spotBlue->setDirection( { -1.0, 0, -1.0 });
    spotBlue->setAttenuationStartDistance(20);
    spotBlue->setAttenuationEndDistance(30);
    spotBlue->setSpotInnerAngle(5);
    spotBlue->setSpotOuterAngle(15);
    
    rootNode->addLight(ambient);
    //rootNode->addLight(spotRed);
    //rootNode->addLight(spotBlue);
        
    std::shared_ptr<VROTexture> bobaTexture = VROTestUtil::loadDiffuseTexture("boba.png");
    bobaTexture->setWrapS(VROWrapMode::Repeat);
    bobaTexture->setWrapT(VROWrapMode::Repeat);
    bobaTexture->setMinificationFilter(VROFilterMode::Linear);
    bobaTexture->setMagnificationFilter(VROFilterMode::Linear);
    bobaTexture->setMipFilter(VROFilterMode::Linear);
    
    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 2, 2);
    box->setName("Box 1");
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getDiffuse().setTexture(bobaTexture);
    material->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    //material->setBloomThreshold(0.1);
    material->getSpecular().setTexture(VROTestUtil::loadSpecularTexture("specular"));
    
    std::shared_ptr<VRONode> boxParentNode = std::make_shared<VRONode>();
    boxParentNode->setPosition({0, 0, -5});
    
    VROMatrix4f scalePivot;
    scalePivot.translate(1, 1, 0);
    boxParentNode->setScalePivot(scalePivot);
    
    VROMatrix4f rotationPivot;
    rotationPivot.translate(-1, 1, 0);
    //boxParentNode->setRotationPivot(rotationPivot);
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    
    boxParentNode->addChildNode(boxNode);
    rootNode->addChildNode(boxParentNode);
    
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();    
    std::shared_ptr<VRONode> cameraNode = std::make_shared<VRONode>();
    cameraNode->setCamera(camera);
    rootNode->addChildNode(cameraNode);
    
    _pointOfView = cameraNode;
    
    VROTransaction::begin();
    VROTransaction::setAnimationDelay(2);
    VROTransaction::setAnimationDuration(6);
    
    spotRed->setPosition({5, 0, 0});
    spotRed->setDirection({-1, 0, -1});
    
    spotBlue->setPosition({-5, 0, 0});
    spotBlue->setDirection({1, 0, -1});
    
    boxParentNode->setRotationEulerZ(M_PI_2);
    VROTransaction::commit();
}

