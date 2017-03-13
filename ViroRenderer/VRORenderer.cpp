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
#include "VRONodeCamera.h"
#include "VROTransaction.h"
#include "VROReticle.h"
#include "VRORenderDelegateInternal.h"
#include "VROFrameSynchronizerInternal.h"
#include "VROImageUtil.h"
#include "VRORenderContext.h"
#include "VROCamera.h"

#pragma mark - Initialization

VRORenderer::VRORenderer(std::shared_ptr<VROInputControllerBase> inputController) :
    _rendererInitialized(false),
    _frameSynchronizer(std::make_shared<VROFrameSynchronizerInternal>()),
    _context(std::make_shared<VRORenderContext>(_frameSynchronizer)),
    _inputController(inputController){
    initBlankTexture(*_context);
    _inputController->setContext(_context);
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

void VRORenderer::setPointOfView(std::shared_ptr<VRONode> node) {
    _pointOfView = node;
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

    _context->setFrame(frame);
    notifyFrameStart();

    VROCamera camera;
    camera.setHeadRotation(headRotation);
    camera.setViewport(viewport);
    camera.setFOV(fov);
    
    // Make a default camera if no point of view is set
    if (!_pointOfView) {
        camera.setPosition({0, 0, 0 });
        camera.setBaseRotation({});
    }
    else {
        // If no node camera is set, just use the point of view node's position and
        // rotation, with standard rotation type
        if (!_pointOfView->getCamera()) {
            camera.setPosition(_pointOfView->getPosition());
            camera.setBaseRotation(_pointOfView->getRotation().getMatrix());
        }
        
        // Otherwise our camera is fully specified
        else {
            const std::shared_ptr<VRONodeCamera> &nodeCamera = _pointOfView->getCamera();
            camera.setBaseRotation(_pointOfView->getRotation().getMatrix().multiply(nodeCamera->getBaseRotation().getMatrix()));
            
            if (nodeCamera->getRotationType() == VROCameraRotationType::Standard) {
                camera.setPosition(_pointOfView->getPosition() + nodeCamera->getPosition());
            }
            else { // Orbit
                VROVector3f pos = _pointOfView->getPosition() + nodeCamera->getPosition();
                VROVector3f focal = _pointOfView->getPosition() + nodeCamera->getOrbitFocalPoint();
                
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
        }
    }

    camera.computeLookAtMatrix();
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

        _inputController->onProcess(camera);
    }

    driver.onFrame(*_context.get());
}

void VRORenderer::renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projectionMatrix,
                            VRODriver &driver) {
    std::shared_ptr<VRORenderDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->willRenderEye(eye, _context.get());
    }

    VROMatrix4f cameraMatrix = _context->getCamera().getLookAtMatrix();
    VROMatrix4f eyeView = eyeFromHeadMatrix.multiply(cameraMatrix);

    _context->setHUDViewMatrix(eyeFromHeadMatrix.multiply(eyeView.invert()));
    _context->setViewMatrix(eyeView);
    _context->setProjectionMatrix(projectionMatrix);
    _context->setEyeType(eye);
    _context->setZNear(kZNear);
    _context->setZFar(kZFar);

    renderEye(eye, driver);

    /**
     * Render the reticle with a HudViewMatrix instead.
     */
    std::shared_ptr<VROReticle> reticle = _inputController->getPresenter()->getReticle();
    if (reticle != nullptr){
        reticle->renderEye(eye, _context.get(), &driver);
    }

    if (delegate) {
        delegate->didRenderEye(eye, _context.get());
    }
}

void VRORenderer::endFrame(VRODriver &driver) {
    if (_outgoingSceneController && !_outgoingSceneController->hasActiveTransitionAnimation()) {
        _sceneController->onSceneDidAppear(_context.get(), driver);
        _outgoingSceneController->onSceneDidDisappear(_context.get(), driver);
        _outgoingSceneController = nullptr;
    }

    notifyFrameEnd();
    VROTransaction::commitAll();
}

void VRORenderer::renderEye(VROEyeType eyeType, VRODriver &driver) {
    if (_sceneController) {
        if (_outgoingSceneController && _outgoingSceneController->hasActiveTransitionAnimation()) {
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
  
    _inputController->attachScene(sceneController->getScene());
    sceneController->onSceneWillAppear(_context.get(), driver);
    if (outgoingSceneController) {
        outgoingSceneController->onSceneWillDisappear(_context.get(), driver);
    }

    _sceneController = sceneController;

    sceneController->onSceneDidAppear(_context.get(), driver);
    if (outgoingSceneController) {
        outgoingSceneController->onSceneDidDisappear(_context.get(), driver);
    }
}

void VRORenderer::setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                                     VROTimingFunctionType timingFunctionType, VRODriver &driver) {
    passert (sceneController != nullptr);

    _outgoingSceneController = _sceneController;
    _sceneController = sceneController;
    _inputController->attachScene(_sceneController->getScene());

    _sceneController->onSceneWillAppear(_context.get(), driver);
    if (_outgoingSceneController) {
        _outgoingSceneController->onSceneWillDisappear(_context.get(), driver);
    }

    _sceneController->startIncomingTransition(seconds, timingFunctionType);
    if (_outgoingSceneController) {
        _outgoingSceneController->startOutgoingTransition(seconds, timingFunctionType);
    }
}

#pragma mark - Frame Listeners

void VRORenderer::notifyFrameStart() {
    ((VROFrameSynchronizerInternal *)_frameSynchronizer.get())->
            notifyFrameStart(*_context.get());
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
