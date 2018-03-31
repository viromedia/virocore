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
#include "VRORendererTestHarness.h"
#include "VROPlatformUtil.h"
#include "VRORendererTest.h"

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
    config.enableHDR = true;
    config.enablePBR = true;
    _renderer = std::make_shared<VRORenderer>(config, std::dynamic_pointer_cast<VROInputControllerBase>(_inputController));
    
    update();
    buildTestScene();
    emscripten_set_main_loop(VROMainLoop, 0, 0);
}

void VROViewScene::buildTestScene() {
    _harness = std::make_shared<VRORendererTestHarness>(_renderer, _renderer->getFrameSynchronizer(), _driver);
    std::shared_ptr<VRORendererTest> test = _harness->loadTest(VRORendererTestType::FBX);
    
    _renderer->setSceneController(test->getSceneController(), _driver);
    if (test->getPointOfView()) {
        _renderer->setPointOfView(test->getPointOfView());
    }
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
