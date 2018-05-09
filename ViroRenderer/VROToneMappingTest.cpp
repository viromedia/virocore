//
//  VROToneMappingTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROToneMappingTest.h"
#include "VROTestUtil.h"
#include "VROCompress.h"
#include "VROText.h"
#include "VROTypeface.h"
#include "VROToneMappingRenderPass.h"

VROToneMappingTest::VROToneMappingTest() :
    VRORendererTest(VRORendererTestType::Box) {
        
}

VROToneMappingTest::~VROToneMappingTest() {
    
}

void VROToneMappingTest::build(std::shared_ptr<VRORenderer> renderer,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
    /*
     Load the background texture.
     */
    //rootNode->setBackgroundSphere(VROTestUtil::loadHDRTexture("wooden"));
    //rootNode->setBackgroundSphere(VROTestUtil::loadDiffuseTexture("interior_viro.jpg", VROMipmapMode::None));
    
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
    spotRed->setIntensity(1500);
    
    std::shared_ptr<VROLight> spotBlue = std::make_shared<VROLight>(VROLightType::Spot);
    spotBlue->setColor({ 0.0, 0.0, 1.0 });
    spotBlue->setPosition( { 5, 0, 0 });
    spotBlue->setDirection( { -1.0, 0, -1.0 });
    spotBlue->setAttenuationStartDistance(20);
    spotBlue->setAttenuationEndDistance(30);
    spotBlue->setSpotInnerAngle(5);
    spotBlue->setSpotOuterAngle(15);
    spotBlue->setIntensity(1500);
    
    rootNode->addLight(ambient);
    rootNode->addLight(spotRed);
    rootNode->addLight(spotBlue);
        
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
    material->setBloomThreshold(0.1);
    material->getSpecular().setTexture(VROTestUtil::loadSpecularTexture("specular"));
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    
    std::shared_ptr<VRONode> boxParentNode = std::make_shared<VRONode>();
    boxParentNode->setPosition({0, 0, -5});
    boxParentNode->addChildNode(boxNode);
    rootNode->addChildNode(boxParentNode);
    
    VROTransaction::begin();
    VROTransaction::setAnimationDelay(2);
    VROTransaction::setAnimationDuration(6);
    
    spotRed->setPosition({5, 0, 0});
    spotRed->setDirection({-1, 0, -1});
    
    spotBlue->setPosition({-5, 0, 0});
    spotBlue->setDirection({1, 0, -1});
    
    boxParentNode->setRotationEulerZ(M_PI_2);
    VROTransaction::commit();
    
    int width = 10;
    int height = 4;
    
    VROLineBreakMode linebreakMode = VROLineBreakMode::Justify;
    VROTextClipMode clipMode = VROTextClipMode::ClipToBounds;
    std::shared_ptr<VROText> text = VROText::createText(L"Hable (Luminance Only) tone-mapping", "Helvetica", 24,
                                                        VROFontStyle::Normal, VROFontWeight::Regular, {1.0, 1.0, 1.0, 1.0}, width, height,
                                                        VROTextHorizontalAlignment::Center, VROTextVerticalAlignment::Center,
                                                        linebreakMode, clipMode, 0, driver);
    std::shared_ptr<VRONode> textNode = std::make_shared<VRONode>();
    textNode->setGeometry(text);
    textNode->setPosition({0, -1, -2});
    rootNode->addChildNode(textNode);
    
    _eventDelegate = std::make_shared<VROToneMappingEventDelegate>(scene, text);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnFuse, true);
    boxParentNode->setEventDelegate(_eventDelegate);
}

void VROToneMappingEventDelegate::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState,
                                          std::vector<float> position) {
    std::shared_ptr<VROScene> scene = _scene.lock();
    if (scene && clickState == ClickState::Clicked) {
        VROToneMappingMethod method = scene->getToneMappingMethod();
        if (method == VROToneMappingMethod::Hable) {
            scene->setToneMappingMethod(VROToneMappingMethod::HableLuminanceOnly);
            pinfo("HABLE (Luminance Only) tone-mapping");
            _text->setText(L"Hable (Luminance Only) tone-mapping");
        }
        else if (method == VROToneMappingMethod::HableLuminanceOnly) {
            scene->setToneMappingMethod(VROToneMappingMethod::Disabled);
            pinfo("DISABLED tone-mapping");
            _text->setText(L"Disabled tone-mapping");
        }
        else if (method == VROToneMappingMethod::Disabled) {
            scene->setToneMappingMethod(VROToneMappingMethod::Hable);
            pinfo("HABLE tone-mapping");
            _text->setText(L"Hable tone-mapping");
        }
        //scene->setToneMappingExposure(1.5);
        //scene->setToneMappingWhitePoint(5.0);
    }
}

