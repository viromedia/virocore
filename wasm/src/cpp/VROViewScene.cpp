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
#include "VROPlatformUtil.h"
#include "VROText.h"
#include "VROOBJLoader.h"
#include "VROParticleEmitter.h"
#include "VROTestUtil.h"

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
    attribs.antialias = 1;
    
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
    _renderer->setSceneController(sceneController, _driver);

    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 1.0, 1.0, 1.0 });
    
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
    
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
    
    std::shared_ptr<VRONode> objNode = std::make_shared<VRONode>();
    VROOBJLoader::loadOBJFromResource("test/male02.obj", VROResourceType::URL, objNode,
                                      [](std::shared_ptr<VRONode> node, bool success) {
                                          if (!success) {
                                              pinfo("Failed to load OBJ");
                                              return;
                                          }
                                          node->setPosition({0, -5, -8});
                                          node->setScale({0.05f, 0.05f, 0.05f});
                                          pinfo("Finished loading OBJ file");
                                      });
    rootNode->addChildNode(objNode);
    
    VROTransaction::begin();
    VROTransaction::setAnimationDelay(2);
    VROTransaction::setAnimationDuration(12);
    
    objNode->setRotationEulerY(359.0f * M_PI / 180.0f);
    
    VROTransaction::commit();
    
    // Text test
    VROLineBreakMode linebreakMode = VROLineBreakMode::WordWrap;
    VROTextClipMode clipMode = VROTextClipMode::None;
    int width = 5;
    int height = 8;
    
    std::wstring string = L"In older times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face.\n\nClose by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything.";
    
    std::shared_ptr<VROText> text = VROText::createText(string, "Helvetica", 28, VROFontStyle::Normal, VROFontWeight::Regular,
                                                        {0.0, 0.0, 1.0, 1.0}, width, height,
                                                        VROTextHorizontalAlignment::Left, VROTextVerticalAlignment::Top,
                                                        linebreakMode, clipMode, 0, _driver);
    
    text->setName("Text");
    pinfo("Realized size %f, %f", text->getRealizedWidth(), text->getRealizedHeight());
    
    std::shared_ptr<VRONode> textNode = std::make_shared<VRONode>();
    textNode->setGeometry(text);
    textNode->setPosition({0, 0, -4});
    
    rootNode->addChildNode(textNode);
    
    std::shared_ptr<VRONode> particleNode = std::make_shared<VRONode>();
    particleNode->setPosition({0, -10, -15});
    particleNode->setTag("Particles");
    
    std::shared_ptr<VROTexture> imgTexture = std::make_shared<VROTexture>(true, VROMipmapMode::Runtime,
                                                                           VROPlatformLoadImageFromFile("cloud.png", format));
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(1,1);
    std::shared_ptr<VROParticleEmitter> particleEmitter = std::make_shared<VROParticleEmitter>(_driver, surface);
    
    // Vec of intervals to interpolate this modifier along.
    std::vector<VROParticleModifier::VROModifierInterval> intervals;
    VROParticleModifier::VROModifierInterval interval1;
    interval1.startFactor = 0;
    interval1.endFactor = 1000;
    interval1.targetedValue = VROVector3f(0,0,0);
    intervals.push_back(interval1);
    
    // Modifier's starting configuration. Provide different numbers to randomize.
    VROVector3f sizeMinStart = VROVector3f(2,2,2);
    VROVector3f sizeMaxStart = VROVector3f(2,2,2);
    
    std::shared_ptr<VROParticleModifier> mod = std::make_shared<VROParticleModifier>(sizeMinStart,
                                                                                     sizeMaxStart,
                                                                                     VROParticleModifier::VROModifierFactor::Time,
                                                                                     intervals);
    // Finally set this modifier.
    particleEmitter->setScaleModifier(mod);
    particleNode->setParticleEmitter(particleEmitter);
    rootNode->addChildNode(particleNode);
    particleEmitter->setRun(true);
    
    //std::shared_ptr<VRONode> fbxNode = VROTestUtil::loadFBXModel("test/cylinder_pbr", { 0, -1.5, -3 }, { 0.4, 0.4, 0.4 }, 1, "02_spin");
    //std::shared_ptr<VRONode> fbxNode = VROTestUtil::loadFBXModel("test/dragon", { 0, -1.5, -6 }, { 0.2, 0.2, 0.2 }, 1, "01");
    //std::shared_ptr<VRONode> fbxNode = VROTestUtil::loadFBXModel("test/pumpkin", { 0, -1.5, -3 }, { 1, 1, 1 }, 1, "02");
    //std::shared_ptr<VRONode> fbxNode = VROTestUtil::loadFBXModel("test/portal_archway", { 0, 0, -3 }, { 1, 1, 1 }, 1, "02");


    //rootNode->addChildNode(fbxNode);
    
    /*
    std::shared_ptr<VROTexture> bobaTexture = std::make_shared<VROTexture>(format, true, VROMipmapMode::Runtime,
                                                                           VROPlatformLoadImageFromFile("boba.png", format));
    bobaTexture->setWrapS(VROWrapMode::Repeat);
    bobaTexture->setWrapT(VROWrapMode::Repeat);
    bobaTexture->setMinificationFilter(VROFilterMode::Linear);
    bobaTexture->setMagnificationFilter(VROFilterMode::Linear);
    bobaTexture->setMipFilter(VROFilterMode::Linear);
    
    std::shared_ptr<VROBox> box = VROBox::createBox(3, 3, 3);
    box->setName("Box 1");
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getDiffuse().setTexture(bobaTexture);
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
     */
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
    
    _renderer->setClearColor({0.8, 0.8, 0.8, 1.0}, _driver);
    
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
