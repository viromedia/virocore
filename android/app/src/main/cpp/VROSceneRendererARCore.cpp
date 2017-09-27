//
//  VROSceneRendererARCore.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 9/27/178.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSceneRendererARCore.h"

#include <android/log.h>
#include <assert.h>
#include <stdlib.h>
#include <cmath>
#include <random>
#include <VROTime.h>

#include "VRODriverOpenGLAndroid.h"
#include "VROGVRUtil.h"
#include "VROMatrix4f.h"
#include "VROViewport.h"
#include "VRORenderer.h"
#include "VROSceneController.h"
#include "VRORenderDelegate.h"
#include "VROInputControllerDaydream.h"
#include "VROInputControllerCardboard.h"
#include "VROAllocationTracker.h"

#pragma mark - Setup

VROSceneRendererARCore::VROSceneRendererARCore(std::shared_ptr<gvr::AudioApi> gvrAudio) :
    _rendererSuspended(true),
    _suspendedNotificationTime(VROTimeCurrentSeconds()) {

    // TODO We need to install an ARCore controller
    std::shared_ptr<VROInputControllerBase> controller = std::make_shared<VROInputControllerCardboard>();

    // Create renderer and attach the controller to it
    _renderer = std::make_shared<VRORenderer>(controller);
    _driver = std::make_shared<VRODriverOpenGLAndroid>(gvrAudio);
}

VROSceneRendererARCore::~VROSceneRendererARCore() {

}

#pragma mark - Rendering

void VROSceneRendererARCore::initGL() {
    glEnable(GL_DEPTH_TEST);
}

void VROSceneRendererARCore::onDrawFrame() {
    if (!_rendererSuspended) {
        renderMono();
    } else {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        double newTime = VROTimeCurrentSeconds();
        // notify the user about bad keys 5 times a second (every 200ms/.2s)
        if (newTime - _suspendedNotificationTime > .2) {
            perr("Renderer suspended! Do you have a valid key?");
            _suspendedNotificationTime = newTime;
        }
    }

    ++_frame;
    ALLOCATION_TRACKER_PRINT();
}

void VROSceneRendererARCore::renderMono() {
    VROMatrix4f headRotation; // Identity

    // TODO We don't need this method, use surface size directly (computes to same thing)
    const gvr::Recti rect = calculatePixelSpaceRect(_surfaceSize, {0, 1, 0, 1});
    VROViewport viewport(rect.left, rect.bottom,
                         rect.right - rect.left,
                         rect.top   - rect.bottom);

    VROFieldOfView fov = VRORenderer::computeMonoFOV(viewport.getWidth(), viewport.getHeight());
    prepareFrame(viewport, fov, headRotation);

    VROMatrix4f projectionMatrix = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    VROMatrix4f eyeFromHeadMatrix; // Identity

    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    _renderer->renderEye(VROEyeType::Monocular, eyeFromHeadMatrix, projectionMatrix, viewport,  _driver);
    _renderer->endFrame(_driver);
}

/**
 * Update render sizes as the surface changes.
 */
void VROSceneRendererARCore::onSurfaceChanged(jobject surface, jint width, jint height) {
    VROThreadRestricted::setThread(VROThreadName::Renderer, pthread_self());

    _surfaceSize.width = width;
    _surfaceSize.height = height;
}

void VROSceneRendererARCore::onTouchEvent(int action, float x, float y) {
    // TODO Change this for AR
    std::shared_ptr<VROInputControllerBase> baseController = _renderer->getInputController();
    std::shared_ptr<VROInputControllerCardboard> cardboardController
            = std::dynamic_pointer_cast<VROInputControllerCardboard>(baseController);
    cardboardController->updateScreenTouch(action);
}

void VROSceneRendererARCore::onPause() {
    std::shared_ptr<VROSceneRendererARCore> shared = shared_from_this();

    VROPlatformDispatchAsyncRenderer([shared] {
        shared->_renderer->getInputController()->onPause();
        shared->_driver->onPause();
    });
}

void VROSceneRendererARCore::onResume() {
    std::shared_ptr<VROSceneRendererARCore> shared = shared_from_this();

    VROPlatformDispatchAsyncRenderer([shared] {
        shared->_renderer->getInputController()->onResume();
        shared->_driver->onResume();
    });
}

void VROSceneRendererARCore::prepareFrame(VROViewport leftViewport, VROFieldOfView fov, VROMatrix4f headRotation) {
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // Must enable writes to clear depth buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    _renderer->prepareFrame(_frame, leftViewport, fov, headRotation, projection, _driver);
}

void VROSceneRendererARCore::renderEye(VROEyeType eyeType,
                                    VROMatrix4f eyeFromHeadMatrix,
                                    VROViewport viewport,
                                    VROFieldOfView fov) {

    VROMatrix4f projectionMatrix = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());

    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    glScissor(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    _renderer->renderEye(eyeType, eyeFromHeadMatrix, projectionMatrix, viewport, _driver);
}

#pragma mark - Utility Methods

gvr::Rectf VROSceneRendererARCore::modulateRect(const gvr::Rectf &rect, float width,
                                             float height) {
    gvr::Rectf result = {rect.left * width, rect.right * width,
                         rect.bottom * height, rect.top * height};
    return result;
}

gvr::Recti VROSceneRendererARCore::calculatePixelSpaceRect(const gvr::Sizei &texture_size,
                                                        const gvr::Rectf &texture_rect) {
    float width = static_cast<float>(texture_size.width);
    float height = static_cast<float>(texture_size.height);
    gvr::Rectf rect = modulateRect(texture_rect, width, height);
    gvr::Recti result = {
            static_cast<int>(rect.left), static_cast<int>(rect.right),
            static_cast<int>(rect.bottom), static_cast<int>(rect.top)};
    return result;
}

void VROSceneRendererARCore::setVRModeEnabled(bool enabled) {

}

void VROSceneRendererARCore::setSuspended(bool suspendRenderer) {
    _rendererSuspended = suspendRenderer;
}


