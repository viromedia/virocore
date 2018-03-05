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
#include "emscripten.h"
#include "emscripten/html5.h"

static VROViewScene *sInstance = nullptr;

void VROMainLoop() {
    sInstance->drawFrame();
}

VROViewScene::VROViewScene() {
    sInstance = this;
    pinfo("Constructed the VROViewScene");
    
    EmscriptenWebGLContextAttributes attribs;
    emscripten_webgl_init_context_attributes(&attribs);
    attribs.alpha = false;
    attribs.enableExtensionsByDefault = false;
    
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("display", &attribs);
    emscripten_webgl_make_context_current(context);
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
    //ContextRestored();
    emscripten_set_main_loop(VROMainLoop, 0, 0);
}

VROViewScene::~VROViewScene() {
    
}

void VROViewScene::drawFrame() {
    glEnable(GL_DEPTH_TEST);
    _driver->setCullMode(VROCullMode::Back);
    
    VROViewport viewport(0, 0, _width, _height);
    if (viewport.getWidth() == 0 || viewport.getHeight() == 0) {
        return;
    }
    
    VROFieldOfView fov = _renderer->computeUserFieldOfView(viewport.getWidth(), viewport.getHeight());
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    
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
    emscripten_get_element_css_size("display", &w, &h);
    
    pinfo("width %f, height %f", w, h);
    
    _width  = (int)w;
    _height = (int)h;
    
    EM_ASM_({
        const display = document.querySelector("#display");
        display.width = $0;
        display.height = $1;
        display.tabIndex = 0;
    }, _width, _height);
    //Resized();
}

void VROViewScene::onResize() {
    
}

void VROViewScene::onBlur() {
    
}

void VROViewScene::onFocus() {
    
}
