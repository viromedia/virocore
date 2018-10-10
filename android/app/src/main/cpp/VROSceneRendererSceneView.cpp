//
//  VROSceneRendererSceneView.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/2017.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSceneRendererSceneView.h"

#include <android/log.h>
#include <assert.h>
#include <stdlib.h>
#include <cmath>
#include <random>
#include <VROTime.h>
#include <VROProjector.h>
#include <VROARHitTestResult.h>
#include <VROInputControllerAR.h>

#include "VRODriverOpenGLAndroid.h"
#include "VROGVRUtil.h"
#include "VRONodeCamera.h"
#include "VROMatrix4f.h"
#include "VROViewport.h"
#include "VRORenderer.h"
#include "VROSurface.h"
#include "VRONode.h"
#include "VROInputControllerCardboard.h"
#include "VROAllocationTracker.h"
#include "VROInputControllerARAndroid.h"

static VROVector3f const kZeroVector = VROVector3f();

#pragma mark - Setup

VROSceneRendererSceneView::VROSceneRendererSceneView(VRORendererConfiguration config,
                                                     std::shared_ptr<gvr::AudioApi> gvrAudio,
                                                     jobject viroViewJNI) :
        _rendererSuspended(true),
        _suspendedNotificationTime(VROTimeCurrentSeconds()) {

    _driver = std::make_shared<VRODriverOpenGLAndroid>(gvrAudio);

    // instantiate the input controller w/ viewport size (0,0) and update it later.
    std::shared_ptr<VROInputControllerAR> controller = std::make_shared<VROInputControllerARAndroid>(0, 0, _driver);
    _renderer = std::make_shared<VRORenderer>(config, controller);
}

VROSceneRendererSceneView::~VROSceneRendererSceneView() {

}

#pragma mark - Rendering

void VROSceneRendererSceneView::initGL() {

}

void VROSceneRendererSceneView::onDrawFrame() {
    if (!_rendererSuspended) {
        renderFrame();
    }
    else {
        renderSuspended();
    }

    ++_frame;
    ALLOCATION_TRACKER_PRINT();
}

void VROSceneRendererSceneView::renderFrame() {
    glEnable(GL_DEPTH_TEST);
    _driver->setCullMode(VROCullMode::Back);

    VROViewport viewport(0, 0, _surfaceSize.width, _surfaceSize.height);
    VROFieldOfView fov = _renderer->computeUserFieldOfView(viewport.getWidth(), viewport.getHeight());
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());

    _renderer->prepareFrame(_frame, viewport, fov, VROMatrix4f::identity(), projection, _driver);
    _renderer->renderEye(VROEyeType::Monocular, _renderer->getLookAtMatrix(), projection, viewport, _driver);
    _renderer->renderHUD(VROEyeType::Monocular, VROMatrix4f::identity(), projection, _driver);
    _renderer->endFrame(_driver);
}

void VROSceneRendererSceneView::renderSuspended() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Notify the user about bad keys 5 times a second (every 200ms/.2s)
    double newTime = VROTimeCurrentSeconds();
    if (newTime - _suspendedNotificationTime > .2) {
        perr("Renderer suspended! Do you have a valid key?");
        _suspendedNotificationTime = newTime;
    }
}

/*
 Update render sizes as the surface changes.
 */
void VROSceneRendererSceneView::onSurfaceChanged(jobject surface, VRO_INT width, VRO_INT height) {
    VROThreadRestricted::setThread(VROThreadName::Renderer);

    _surfaceSize.width = width;
    _surfaceSize.height = height;

    std::shared_ptr<VROInputControllerARAndroid> inputControllerAR =
            std::dynamic_pointer_cast<VROInputControllerARAndroid>(_renderer->getInputController());
    if (inputControllerAR) {
        inputControllerAR->setViewportSize(width, height);
    }
}

void VROSceneRendererSceneView::onTouchEvent(int action, float x, float y) {
    std::shared_ptr<VROInputControllerBase> baseController = _renderer->getInputController();
    std::shared_ptr<VROInputControllerARAndroid> arTouchController
            = std::dynamic_pointer_cast<VROInputControllerARAndroid>(baseController);
    arTouchController->onTouchEvent(action, x, y);
}

void VROSceneRendererSceneView::onPinchEvent(int pinchState, float scaleFactor,
                                          float viewportX, float viewportY) {
    std::shared_ptr<VROInputControllerBase> baseController = _renderer->getInputController();
    std::shared_ptr<VROInputControllerARAndroid> arTouchController
            = std::dynamic_pointer_cast<VROInputControllerARAndroid>(baseController);
    arTouchController->onPinchEvent(pinchState, scaleFactor, viewportX, viewportY);
}

void VROSceneRendererSceneView::onRotateEvent(int rotateState, float rotateRadians, float viewportX,
                                           float viewportY) {
    std::shared_ptr<VROInputControllerBase> baseController = _renderer->getInputController();
    std::shared_ptr<VROInputControllerARAndroid> arTouchController
            = std::dynamic_pointer_cast<VROInputControllerARAndroid>(baseController);
    arTouchController->onRotateEvent(rotateState, rotateRadians, viewportX, viewportY);
}

void VROSceneRendererSceneView::onPause() {
    std::shared_ptr<VROSceneRendererSceneView> shared = shared_from_this();
    VROPlatformDispatchAsyncRenderer([shared] {
        shared->_renderer->getInputController()->onPause();
        shared->_driver->pause();
    });
}

void VROSceneRendererSceneView::onResume() {
    std::shared_ptr<VROSceneRendererSceneView> shared = shared_from_this();

    VROPlatformDispatchAsyncRenderer([shared] {
        shared->_renderer->getInputController()->onResume();
        shared->_driver->resume();
    });
}

void VROSceneRendererSceneView::setVRModeEnabled(bool enabled) {

}

void VROSceneRendererSceneView::setSuspended(bool suspendRenderer) {
    _rendererSuspended = suspendRenderer;
}


