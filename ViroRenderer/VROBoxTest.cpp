//
//  VROBoxTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROBoxTest.h"
#include "VROTestUtil.h"
#include "VROCompress.h"
#include "VROToneMappingRenderPass.h"

VROBoxTest::VROBoxTest() :
    VRORendererTest(VRORendererTestType::Box) {
        
}

VROBoxTest::~VROBoxTest() {
    
}

void VROBoxTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer, std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROSceneController>();
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
    spotRed->setSpotInnerAngle(2.5);
    spotRed->setSpotOuterAngle(5.0);
    
    std::shared_ptr<VROLight> spotBlue = std::make_shared<VROLight>(VROLightType::Spot);
    spotBlue->setColor({ 0.0, 0.0, 1.0 });
    spotBlue->setPosition( { 5, 0, 0 });
    spotBlue->setDirection( { -1.0, 0, -1.0 });
    spotBlue->setAttenuationStartDistance(20);
    spotBlue->setAttenuationEndDistance(30);
    spotBlue->setSpotInnerAngle(2.5);
    spotBlue->setSpotOuterAngle(5.0);
    
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
    material->getSpecular().setTexture(VROTestUtil::loadSpecularTexture("specular"));
    
    /*
     This geometry modifier pushes the first box to the right slightly, by changing _geometry.position.
     */
    std::vector<std::string> modifierCode =  { "uniform float testA;",
        "uniform float testB;",
        "_geometry.position.x = _geometry.position.x + testA;"
    };
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                                      modifierCode);
    
    modifier->setUniformBinder("testA", [](VROUniform *uniform, GLuint location,
                                           const VROGeometry *geometry, const VROMaterial *material) {
        uniform->setFloat(1.0);
    });
    //material->addShaderModifier(modifier);
    
    /*
     This surface modifier doubles the V tex-coord, making boba.png cover only the top half of the box.
     It then shades the entire surface slightly blue.
     */
    std::vector<std::string> surfaceModifierCode =  {
        "uniform lowp vec3 surface_color;",
        "_surface.diffuse_texcoord.y = _surface.diffuse_texcoord.y * 2.0;"
        "_surface.diffuse_color = vec4(surface_color.xyz, 1.0);"
    };
    std::shared_ptr<VROShaderModifier> surfaceModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                                             surfaceModifierCode);
    
    surfaceModifier->setUniformBinder("surface_color", [](VROUniform *uniform, GLuint location,
                                                          const VROGeometry *geometry, const VROMaterial *material) {
        uniform->setVec3({0.6, 0.6, 1.0});
    });
    //material->addShaderModifier(surfaceModifier);
    
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
    //boxParentNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
    
    /*
     Create a second box node behind the first.
     */
    std::shared_ptr<VROBox> box2 = VROBox::createBox(2, 4, 2);
    box2->setName("Box 2");
    
    std::vector<std::shared_ptr<VROMaterial>> boxMaterials;
    for (int i = 0; i < 6; i ++) {
        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
        material->setLightingModel(VROLightingModel::Lambert);
        
        if (i == 0) {
            material->getDiffuse().setTexture(VROTestUtil::loadDiffuseTexture("boba"));
        }
        else if (i == 1) {
            material->getDiffuse().setColor({ 1.0, 0.0, 0.0, 1.0 });
        }
        else if (i == 2) {
            material->getDiffuse().setColor({ 0.0, 1.0, 0.0, 1.0 });
        }
        else if (i == 3) {
            material->getDiffuse().setColor({ 0.0, 0.0, 1.0, 1.0 });
        }
        else if (i == 4) {
            material->getDiffuse().setColor({ 1.0, 0.0, 1.0, 1.0 });
        }
        else if (i == 5) {
            material->getDiffuse().setColor({ 1.0, 1.0, 0.0, 1.0 });
        }
        
        boxMaterials.push_back(material);
    }
    box2->setMaterials(boxMaterials);
    
    std::shared_ptr<VRONode> boxNode2 = std::make_shared<VRONode>();
    boxNode2->setGeometry(box2);
    boxNode2->setPosition({0, 0, -9});
    
    rootNode->addChildNode(boxNode2);
    
    /*
     Create a third box node behind the second.
     */
    std::shared_ptr<VROBox> box3 = VROBox::createBox(2, 4, 2);
    box3->setName("Box 3");
    
    std::shared_ptr<VROMaterial> material3 = box3->getMaterials()[0];
    material3->setLightingModel(VROLightingModel::Lambert);
    material3->getDiffuse().setTexture(VROTestUtil::loadDiffuseTexture("boba"));
    material3->getSpecular().setTexture(VROTestUtil::loadSpecularTexture("specular"));
    
    std::shared_ptr<VRONode> boxNode3 = std::make_shared<VRONode>();
    boxNode3->setGeometry(box3);
    boxNode3->setPosition({0, 0, -13});
    
    rootNode->addChildNode(boxNode3);
    
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
    
    _eventDelegate = std::make_shared<VROBoxEventDelegate>(scene);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnFuse, true);
    boxParentNode->setEventDelegate(_eventDelegate);
}

void VROBoxEventDelegate::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState,
                                  std::vector<float> position) {
    std::shared_ptr<VROScene> scene = _scene.lock();
    if (scene && clickState == ClickState::Clicked) {
        VROToneMappingMethod method = scene->getToneMappingMethod();
        if (method == VROToneMappingMethod::Hable) {
            scene->setToneMappingMethod(VROToneMappingMethod::HableLuminanceOnly);
            pinfo("HABLE (Luminance Only) tone-mapping");
        }
        else if (method == VROToneMappingMethod::HableLuminanceOnly) {
            scene->setToneMappingMethod(VROToneMappingMethod::Disabled);
            pinfo("DISABLED tone-mapping");
        }
        else if (method == VROToneMappingMethod::Disabled) {
            scene->setToneMappingMethod(VROToneMappingMethod::Hable);
            pinfo("HABLE tone-mapping");
        }
        //scene->setToneMappingExposure(1.5);
        //scene->setToneMappingWhitePoint(5.0);
    }
}

