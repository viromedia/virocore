//
//  VRORenderer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/5/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VRORenderer.h"
#include "VROTime.h"
#include "VROEye.h"
#include "VROTransaction.h"
#include "VROAllocationTracker.h"
#include "VROScene.h"
#include "VROLog.h"
#include "VROCameraMutable.h"
#include "VROTransaction.h"
#include "VROReticle.h"
#include "VRORenderDelegateInternal.h"
#include "VROFrameSynchronizerInternal.h"
#include "VROImageUtil.h"
#include "VRORenderContext.h"
#include "VROCamera.h"
#include "VROReticleSizeListener.h"

static const float kDefaultSceneTransitionDuration = 1.0;

#pragma mark - Initialization

VRORenderer::VRORenderer() :
    _rendererInitialized(false),
    _frameSynchronizer(std::make_shared<VROFrameSynchronizerInternal>()),
    _context(std::make_shared<VRORenderContext>(_frameSynchronizer)),
    _reticle(std::make_shared<VROReticle>()),
    _camera(std::make_shared<VROCameraMutable>()),
    _sceneTransitionActive(false){

    _eventManager = std::make_shared<VROEventManager>(_context);

    /*
     * Binds the size listener to the reticle and register it within
     * the eventController in order to respond to gaze events (we
     * change the size of the reticle based on the gazed location).
     */
    std::shared_ptr<VROReticleSizeListener> _reticleSizeListener
            = std::make_shared<VROReticleSizeListener>(_reticle, _context);
    _eventManager->registerEventDelegate(_reticleSizeListener);
    initBlankTexture(*_context);
}

VRORenderer::~VRORenderer() {
    std::shared_ptr<VRORenderDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->shutdownRenderer();
    }
}

void VRORenderer::setDelegate(std::shared_ptr<VRORenderDelegateInternal> delegate) {
    _delegate = delegate;
}

#pragma mark - Camera

void VRORenderer::setPosition(VROVector3f position) {
    _camera->setPosition(position);
}

void VRORenderer::setBaseRotation(VROQuaternion quaternion) {
    _camera->setBaseRotation(quaternion);
}

void VRORenderer::setCameraRotationType(VROCameraRotationType type) {
    _camera->setRotationType(type);
}

void VRORenderer::setOrbitFocalPoint(VROVector3f focalPt) {
    _camera->setOrbitFocalPoint(focalPt);
}

#pragma mark - Stereo renderer methods

void VRORenderer::updateRenderViewSize(float width, float height) {
    std::shared_ptr<VRORenderDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->renderViewDidChangeSize(width, height, _context.get());
    }
}

void VRORenderer::prepareFrame(int frame, VROViewport viewport, VROFieldOfView fov,
                               VROMatrix4f headRotation, VRODriver &driver) {
    
    if (!_rendererInitialized) {
        std::shared_ptr<VRORenderDelegateInternal> delegate = _delegate.lock();
        if (delegate) {
            delegate->setupRendererWithDriver(&driver);
        }
        _rendererInitialized = true;
    }
    
    VROTransaction::beginImplicitAnimation();
    VROTransaction::update();
    
    _sceneTransitionActive = processSceneTransition();
    
    _context->setFrame(frame);
    notifyFrameStart();

    VROCamera camera;
    camera.setHeadRotation(headRotation);
    camera.setBaseRotation(_camera->getBaseRotation().getMatrix());
    camera.setViewport(viewport);
    camera.setFOV(fov);
    
    if (_camera->getRotationType() == VROCameraRotationType::Standard) {
        camera.setPosition(_camera->getPosition());
    }
    else { // Orbit
        VROVector3f pos = _camera->getPosition();
        VROVector3f focal = _camera->getOrbitFocalPoint();
        
        VROVector3f v = focal.subtract(pos);
        VROVector3f ray = v.normalize();
        
        // Set the orbit position by pushing out the camera at an angle
        // defined by the current head rotation
        VROVector3f orbitedRay = headRotation.multiply(v.normalize());
        camera.setPosition(focal - orbitedRay.scale(v.magnitude()));
        
        // Set the orbit rotation. This is the current head rotation plus
        // the rotation required to get from kBaseForward to the forward
        // vector defined by the camera's position and focal point
        VROQuaternion rotation = VROQuaternion::rotationFromTo(ray, kBaseForward);
        camera.setHeadRotation(rotation.getMatrix().invert().multiply(headRotation));
    }
    
    _context->setCamera(camera);
    
    /*
     This matrix is used for rendering objects that follow the camera, such
     as skyboxes. To get them to follow the camera, we do not include the
     camera's translation component in the view matrix.
     */
    VROMatrix4f enclosureMatrix = VROMathComputeLookAtMatrix({ 0, 0, 0 }, camera.getForward(), camera.getUp());
    _context->setEnclosureViewMatrix(enclosureMatrix);
    
    if (_sceneController) {
        if (_outgoingSceneController) {
            _outgoingSceneController->getScene()->updateSortKeys(*_context.get(), driver);
            _sceneController->getScene()->updateSortKeys(*_context.get(), driver);
        }
        else {
            _sceneController->getScene()->updateSortKeys(*_context.get(), driver);
        }
    }

    /**
     * Proccess events at the end of prepareFrame.
     * TODO VIRO-696: Remove this in favor of an input controller during DayDream integration.
     */
    _eventManager->onHeadGearGaze();
}

void VRORenderer::renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projectionMatrix,
                            VRODriver &driver) {
    std::shared_ptr<VRORenderDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->willRenderEye(eye, _context.get());
    }

    VROMatrix4f cameraMatrix = _context->getCamera().computeLookAtMatrix();
    VROMatrix4f eyeView = eyeFromHeadMatrix.multiply(cameraMatrix);
    
    _context->setHUDViewMatrix(eyeFromHeadMatrix.multiply(eyeView.invert()));
    _context->setViewMatrix(eyeView);
    _context->setProjectionMatrix(projectionMatrix);
    _context->setEyeType(eye);
    _context->setZNear(kZNear);
    _context->setZFar(kZFar);
    
    renderEye(eye, driver);

    _reticle->renderEye(eye, _context.get(), &driver);
    
    if (delegate) {
        delegate->didRenderEye(eye, _context.get());
    }
}

void VRORenderer::endFrame(VRODriver &driver) {
    if (!_sceneTransitionActive && _outgoingSceneController) {
        _sceneController->endIncomingTransition(_context.get());
        _outgoingSceneController->endOutgoingTransition(_context.get());
    
        _sceneController->onSceneDidAppear(_context.get(), driver);
        _outgoingSceneController->onSceneDidDisappear(_context.get(), driver);
        
        _outgoingSceneController = nullptr;
    }
    
    notifyFrameEnd();
    VROTransaction::commitAll();
}

void VRORenderer::renderEye(VROEyeType eyeType, VRODriver &driver) {
    if (_sceneController) {
        if (_outgoingSceneController) {
            _outgoingSceneController->sceneWillRender(_context.get());
            _sceneController->sceneWillRender(_context.get());
            
            _outgoingSceneController->getScene()->renderBackground(*_context.get(), driver);
            _sceneController->getScene()->renderBackground(*_context.get(), driver);
            
            _outgoingSceneController->getScene()->render(*_context.get(), driver);
            _sceneController->getScene()->render(*_context.get(), driver);
        }
        else {
            _sceneController->sceneWillRender(_context.get());
            _sceneController->getScene()->renderBackground(*_context.get(), driver);
            _sceneController->getScene()->render(*_context.get(), driver);
        }
    }
}

#pragma mark - Scene Loading

void VRORenderer::setSceneController(std::shared_ptr<VROSceneController> sceneController, VRODriver &driver) {
    std::shared_ptr<VROSceneController> outgoingSceneController = _sceneController;
    sceneController->onSceneWillAppear(_context.get(), driver);
    if (outgoingSceneController) {
        outgoingSceneController->onSceneWillDisappear(_context.get(), driver);
    }
    
    _sceneController = sceneController;
    _eventManager->attachScene(_sceneController->getScene());
    
    sceneController->onSceneDidAppear(_context.get(), driver);
    if (outgoingSceneController) {
        outgoingSceneController->onSceneDidDisappear(_context.get(), driver);
    }
}

void VRORenderer::setSceneController(std::shared_ptr<VROSceneController> sceneController, bool animated, VRODriver &driver) {
    if (!animated || !_sceneController) {
        _sceneController = sceneController;
        return;
    }
    
    setSceneController(sceneController, kDefaultSceneTransitionDuration, VROTimingFunctionType::EaseIn, driver);
}

void VRORenderer::setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                                     VROTimingFunctionType timingFunctionType, VRODriver &driver) {
    passert (_sceneController != nullptr);
    
    _outgoingSceneController = _sceneController;
    _sceneController = sceneController;
    
    _sceneTransitionStartTime = VROTimeCurrentSeconds();
    _sceneTransitionDuration = seconds;
    _sceneTransitionTimingFunction = VROTimingFunction::forType(timingFunctionType);

    _eventManager->attachScene(_sceneController->getScene());
    _sceneController->onSceneWillAppear(_context.get(), driver);
    _outgoingSceneController->onSceneWillDisappear(_context.get(), driver);
    
    _sceneController->startIncomingTransition(_context.get(), seconds);
    _outgoingSceneController->startOutgoingTransition(_context.get(), seconds);
}

bool VRORenderer::processSceneTransition() {
    if (!_sceneController || !_outgoingSceneController) {
        return false;
    }
    
    float percent = (VROTimeCurrentSeconds() - _sceneTransitionStartTime) / _sceneTransitionDuration;
    float t = _sceneTransitionTimingFunction->getT(percent);
    
    bool sceneTransitionActive = percent < 0.9999;
    if (sceneTransitionActive) {
        _sceneController->animateIncomingTransition(_context.get(), t);
        _outgoingSceneController->animateOutgoingTransition(_context.get(), t);
    }
    else {
        _sceneController->animateIncomingTransition(_context.get(), 1.0);
        _outgoingSceneController->animateOutgoingTransition(_context.get(), 1.0);
    }
    
    return sceneTransitionActive;
}

#pragma mark - Frame Listeners

void VRORenderer::notifyFrameStart() {
    ((VROFrameSynchronizerInternal *)_frameSynchronizer.get())->notifyFrameStart(*_context.get());
}

void VRORenderer::notifyFrameEnd() {
    ((VROFrameSynchronizerInternal *)_frameSynchronizer.get())->notifyFrameEnd(*_context.get());

}

#pragma mark - VR Framework Specific

void VRORenderer::requestExitVR() {
    std::shared_ptr<VRORenderDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->userDidRequestExitVR();
    }
}
