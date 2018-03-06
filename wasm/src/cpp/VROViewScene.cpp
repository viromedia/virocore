//
//  VROViewScene.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROViewScene.h"
#include "VROLog.h"
#include "VRORenderer.h"
#include "VRORendererConfiguration.h"
#include "VRODriverOpenGLWasm.h"
#include "VROInputControllerWasm.h"
#include "VROEye.h"

#include "VROMaterial.h"
#include "VRONode.h"
#include "VROBox.h"

static VROViewScene *sInstance = nullptr;

void VROMainLoop() {
    sInstance->drawFrame();
}

VROViewScene::VROViewScene() {
    sInstance = this;
    VROThreadRestricted::setThread(VROThreadName::Renderer);
    
    pinfo("Constructed the VROViewScene");

    EmscriptenWebGLContextAttributes attribs;
    emscripten_webgl_init_context_attributes(&attribs);
    attribs.majorVersion = 2.0;
    attribs.minorVersion = 0.0;
    attribs.explicitSwapControl = 0;
    attribs.depth = 1;
    attribs.stencil = 1;
    attribs.antialias = 0;
    
    _context = emscripten_webgl_create_context("viroCanvas", &attribs);
    emscripten_webgl_make_context_current(_context);
    //emscripten_set_resize_callback(0 /* Window */, this, false, &VROViewScene::onResize);
    //emscripten_set_blur_callback("#window", NULL, false, onBlur);
    //emscripten_set_focus_callback("#window", NULL, false, onFocus);
    
    _driver = std::make_shared<VRODriverOpenGLWasm>();
    _frame = 0;
    _suspendedNotificationTime = 0;//VROTimeCurrentSeconds();
    _inputController = std::make_shared<VROInputControllerWasm>();
    
    VRORendererConfiguration config;
    config.enableShadows = false;
    config.enableBloom = false;
    config.enableHDR = false;
    config.enablePBR = false;
    _renderer = std::make_shared<VRORenderer>(config, std::dynamic_pointer_cast<VROInputControllerBase>(_inputController));
    
    update();
    buildTestScene();
    emscripten_set_main_loop(VROMainLoop, 0, 0);
}

void VROViewScene::buildTestScene() {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    
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
    rootNode->addLight(spotRed);
    rootNode->addLight(spotBlue);
    
    /*
    std::shared_ptr<VROTexture> bobaTexture = VROTestUtil::loadDiffuseTexture("boba.png");
    bobaTexture->setWrapS(VROWrapMode::Repeat);
    bobaTexture->setWrapT(VROWrapMode::Repeat);
    bobaTexture->setMinificationFilter(VROFilterMode::Linear);
    bobaTexture->setMagnificationFilter(VROFilterMode::Linear);
    bobaTexture->setMipFilter(VROFilterMode::Linear);
    */
    std::shared_ptr<VROBox> box = VROBox::createBox(3, 3, 3);
    box->setName("Box 1");
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    //material->getDiffuse().setTexture(bobaTexture);
    material->getDiffuse().setColor({0.8, 0.8, 0.8, 1.0});
    //material->getSpecular().setTexture(VROTestUtil::loadSpecularTexture("specular"));
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -5});
    rootNode->addChildNode(boxNode);
    
    _renderer->setSceneController(sceneController, _driver);
    
    VROTransaction::begin();
    VROTransaction::setAnimationDelay(2);
    VROTransaction::setAnimationDuration(6);
    
    spotRed->setPosition({5, 0, 0});
    spotRed->setDirection({-1, 0, -1});
    spotBlue->setPosition({-5, 0, 0});
    spotBlue->setDirection({1, 0, -1});
    boxNode->setRotationEulerZ(M_PI_2);
    
    VROTransaction::commit();
}

VROViewScene::~VROViewScene() {
    // destroy the context
}

void VROViewScene::drawFrame() {
    emscripten_webgl_make_context_current(_context);
    
    VROViewport viewport(0, 0, _width, _height);
    if (viewport.getWidth() == 0 || viewport.getHeight() == 0) {
        return;
    }
    
    VROFieldOfView fov = _renderer->computeUserFieldOfView(viewport.getWidth(), viewport.getHeight());
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    
    _renderer->setClearColor({0.0, 1.0, 0.0, 1.0}, _driver);
    
    _renderer->prepareFrame(_frame, viewport, fov, VROMatrix4f::identity(), projection, _driver);
    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    _renderer->renderEye(VROEyeType::Monocular, _renderer->getLookAtMatrix(), projection, viewport, _driver);
    _renderer->renderHUD(VROEyeType::Monocular, VROMatrix4f::identity(), projection, _driver);
    _renderer->endFrame(_driver);
    
    _frame++;
}

void VROViewScene::update() {
    double w = 0.0;
    double h = 0.0;
    emscripten_get_element_css_size("viroCanvas", &w, &h);
    _width  = (int)w;
    _height = (int)h;
}

void VROViewScene::onResize() {
    
}

void VROViewScene::onBlur() {
    
}

void VROViewScene::onFocus() {
    
}
