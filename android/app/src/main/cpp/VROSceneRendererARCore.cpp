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
#include "VROARCamera.h"
#include "arcore/VROARSessionARCore.h"
#include "arcore/VROARFrameARCore.h"
#include "arcore/VROARCameraARCore.h"

#include "VRODriverOpenGLAndroid.h"
#include "VROGVRUtil.h"
#include "VRONodeCamera.h"
#include "VROMatrix4f.h"
#include "VROViewport.h"
#include "VRORenderer.h"
#include "VROSurface.h"
#include "VRONode.h"
#include "VROARScene.h"
#include "VROInputControllerCardboard.h"
#include "VROAllocationTracker.h"

static VROVector3f const kZeroVector = VROVector3f();

#pragma mark - Setup

VROSceneRendererARCore::VROSceneRendererARCore(std::shared_ptr<gvr::AudioApi> gvrAudio,
                                               jni::Object<arcore::Session> sessionJNI) :
    _rendererSuspended(true),
    _suspendedNotificationTime(VROTimeCurrentSeconds()),
    _hasTrackingInitialized(false) {

    // TODO We need to install an ARCore controller
    std::shared_ptr<VROInputControllerBase> controller = std::make_shared<VROInputControllerCardboard>();

    _renderer = std::make_shared<VRORenderer>(controller);
    _driver = std::make_shared<VRODriverOpenGLAndroid>(gvrAudio);
    _session = std::make_shared<VROARSessionARCore>(sessionJNI, _driver);

    _pointOfView = std::make_shared<VRONode>();
    _pointOfView->setCamera(std::make_shared<VRONodeCamera>());
    _renderer->setPointOfView(_pointOfView);
}

VROSceneRendererARCore::~VROSceneRendererARCore() {

}

#pragma mark - Rendering

void VROSceneRendererARCore::initGL() {

}

void VROSceneRendererARCore::onDrawFrame() {
    if (!_rendererSuspended) {
        renderFrame();
    }
    else {
        renderSuspended();
    }

    ++_frame;
    ALLOCATION_TRACKER_PRINT();
}

void VROSceneRendererARCore::renderFrame() {
    /*
     Setup GL state.
     */
    glEnable(GL_DEPTH_TEST);
    _driver->setCullMode(VROCullMode::Back);

    VROViewport viewport(0, 0, _surfaceSize.width, _surfaceSize.height);

    if (_sceneController) {
        if (!_cameraBackground) {
            initARSession(viewport, _sceneController->getScene());
        }
    }

    if (_session->isReady()) {
        // TODO Only on viewport change
        _session->setViewport(viewport);
        if (!_sceneController->getScene()->getRootNode()->getBackground()) {
            _sceneController->getScene()->getRootNode()->setBackground(_cameraBackground);
        }

        /*
         Retrieve transforms from the AR session.
         */
        std::unique_ptr<VROARFrame> &frame = _session->updateFrame();
        const std::shared_ptr<VROARCamera> camera = frame->getCamera();

        VROFieldOfView fov;
        VROMatrix4f projection = camera->getProjection(viewport, kZNear, _renderer->getFarClippingPlane(), &fov);
        VROMatrix4f rotation = camera->getRotation();
        VROVector3f position = camera->getPosition();

        if (_sceneController && !_hasTrackingInitialized) {
            if (position != kZeroVector) {
                _hasTrackingInitialized = true;

                std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
                passert_msg (arScene != nullptr, "AR View requires an AR Scene!");
                arScene->trackingHasInitialized();
            }
        }

        // TODO Only on orientation change
        //VROMatrix4f backgroundTransform = frame->getViewportToCameraImageTransform();
        //_cameraBackground->setTexcoordTransform(backgroundTransform);

        /*
         Render the 3D scene.
         */
        _pointOfView->getCamera()->setPosition(position);
        _renderer->prepareFrame(_frame, viewport, fov, rotation, projection, _driver);

        glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
        _renderer->renderEye(VROEyeType::Monocular, VROMatrix4f::identity(), projection, viewport, _driver);
        _renderer->endFrame(_driver);

        /*
         Notify scene of the updated ambient light estimates
         */
        std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(_session->getScene());
        scene->updateAmbientLight(frame->getAmbientLightIntensity(), frame->getAmbientLightColorTemperature());
    }
    else {
        /*
         Render black while waiting for the AR session to initialize.
         */
        VROFieldOfView fov = _renderer->computeMonoFOV(viewport.getWidth(), viewport.getHeight());
        VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());

        _renderer->prepareFrame(_frame, viewport, fov, VROMatrix4f::identity(), projection, _driver);
        glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
        _renderer->renderEye(VROEyeType::Monocular, VROMatrix4f::identity(), projection, viewport, _driver);
        _renderer->endFrame(_driver);
    }
}

void VROSceneRendererARCore::renderSuspended() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Notify the user about bad keys 5 times a second (every 200ms/.2s)
    double newTime = VROTimeCurrentSeconds();
    if (newTime - _suspendedNotificationTime > .2) {
        perr("Renderer suspended! Do you have a valid key?");
        _suspendedNotificationTime = newTime;
    }
}

void VROSceneRendererARCore::initARSession(VROViewport viewport, std::shared_ptr<VROScene> scene) {
    // Create the background surface
    _cameraBackground = VROSurface::createSurface(viewport.getX() + viewport.getWidth() / 2.0,
                                                  viewport.getY() + viewport.getHeight() / 2.0,
                                                  viewport.getWidth(), viewport.getHeight(),
                                                  0, 0, 1, 1);
    _cameraBackground->setScreenSpace(true);
    _cameraBackground->setName("Camera");

    // Assign the background texture to the background surface
    std::shared_ptr<VROMaterial> material = _cameraBackground->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Constant);
    material->getDiffuse().setTexture(_session->getCameraBackgroundTexture());
    material->setWritesToDepthBuffer(false);

    _session->setScene(scene);
    _session->setViewport(viewport);
    _session->setAnchorDetection({VROAnchorDetection::PlanesHorizontal});
    _session->run();

    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(scene);
    passert_msg (arScene != nullptr, "AR View requires an AR Scene!");

    // TODO: set the AR Component Manager?
    //arScene->setARComponentManager(_arComponentManager);
    arScene->addNode(_pointOfView);
}

/*
 Update render sizes as the surface changes.
 */
void VROSceneRendererARCore::onSurfaceChanged(jobject surface, jint width, jint height) {
    VROThreadRestricted::setThread(VROThreadName::Renderer, pthread_self());

    _surfaceSize.width = width;
    _surfaceSize.height = height;
}

void VROSceneRendererARCore::onTouchEvent(int action, float x, float y) {
    // TODO Change the touch/controller handling for AR
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

void VROSceneRendererARCore::setVRModeEnabled(bool enabled) {

}

void VROSceneRendererARCore::setSuspended(bool suspendRenderer) {
    _rendererSuspended = suspendRenderer;
}

void VROSceneRendererARCore::setSceneController(std::shared_ptr<VROSceneController> sceneController) {
    _sceneController = sceneController;
    VROSceneRenderer::setSceneController(sceneController);
}

void VROSceneRendererARCore::setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                        VROTimingFunctionType timingFunction) {

    _sceneController = sceneController;
    VROSceneRenderer::setSceneController(sceneController, seconds, timingFunction);
}




