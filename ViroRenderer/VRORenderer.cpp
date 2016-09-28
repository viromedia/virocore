//
//  VRORenderer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/5/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VRORenderer.h"
#import "VROTime.h"
#import "VROEye.h"
#import "VROFieldOfView.h"
#import "VROViewport.h"
#import "VROTransaction.h"
#import "VROAllocationTracker.h"
#import "VROScene.h"
#import "VROSceneController.h"
#import "VROLog.h"
#import "VROCameraMutable.h"
#import "VROScreenUIView.h"
#import "VRORenderDelegate.h"
#import "VROTransaction.h"
#import "VROFrameSynchronizerInternal.h"

static const float kDefaultSceneTransitionDuration = 1.0;

#pragma mark - Initialization

VRORenderer::VRORenderer() :
    _rendererInitialized(false),
    _frameSynchronizer(std::make_shared<VROFrameSynchronizerInternal>()),
    _context(std::make_shared<VRORenderContext>(_frameSynchronizer)),
    _HUD([[VROScreenUIView alloc] init]),
    _camera(std::make_shared<VROCameraMutable>()),
    _sceneTransitionActive(false) {
    
    initBlankTexture(*_context);
}

VRORenderer::~VRORenderer() {
    [_delegate shutdownRenderer];
}

void VRORenderer::setDelegate(id <VRORenderDelegate> delegate) {
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

float VRORenderer::getWorldPerScreen(float distance, const VROFieldOfView &fov,
                                     const VROViewport &viewport) const {
    /*
     Arbitrarily chose eye's left FOV. tan(fov) = perp/distance, where
     perp is in the direction perpendicular to the camera's up vector and
     forward vector, and distance is in the direction of the camera's forward
     vector.
     */
    float radians = fov.getLeft();
    float perp = distance * tan(radians);
    
    /*
     The perspective divide is perp divided by half the viewport.
     */
    return perp / (viewport.getWidth() / 2.0f);
}

#pragma mark - Stereo renderer methods

void VRORenderer::updateRenderViewSize(CGSize size) {
   [_delegate renderViewDidChangeSize:CGSizeMake(size.width / 2, size.height) context:_context.get()];
}

void VRORenderer::prepareFrame(int frame, VROMatrix4f headRotation, VRODriver &driver) {
    if (!_rendererInitialized) {
        [_delegate setupRendererWithDriver:&driver];
        _rendererInitialized = YES;
    }
    
    VROTransaction::beginImplicitAnimation();
    VROTransaction::update();
    
    _sceneTransitionActive = processSceneTransition();
    
    _context->setFrame(frame);
    notifyFrameStart();
    
    VROCamera camera;
    camera.setHeadRotation(headRotation);
    camera.setBaseRotation(_camera->getBaseRotation().getMatrix());
    
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

    [_HUD updateWithDriver:&driver];
    
    if (_sceneController) {
        if (_outgoingSceneController) {
            _outgoingSceneController.scene->updateSortKeys(*_context.get());
            _sceneController.scene->updateSortKeys(*_context.get());
        }
        else {
            _sceneController.scene->updateSortKeys(*_context.get());
        }
    }
}

void VRORenderer::renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projectionMatrix,
                            VRODriver &driver) {
    [_delegate willRenderEye:eye context:_context.get()];

    VROMatrix4f cameraMatrix = _context->getCamera().computeLookAtMatrix();
    VROMatrix4f eyeView = eyeFromHeadMatrix.multiply(cameraMatrix);
    
    _context->setHUDViewMatrix(eyeFromHeadMatrix.multiply(eyeView.invert()));
    _context->setViewMatrix(eyeView);
    _context->setProjectionMatrix(projectionMatrix);
    _context->setEyeType(eye);
    _context->setZNear(kZNear);
    _context->setZFar(kZFar);
    
    renderEye(eye, driver);
    [_HUD renderEye:eye withRenderContext:_context.get() driver:&driver];
    
    [_delegate didRenderEye:eye context:_context.get()];
}

void VRORenderer::endFrame(VRODriver &driver) {
    if (!_sceneTransitionActive && _outgoingSceneController) {
        [_sceneController endIncomingTransition:_context.get()];
        [_outgoingSceneController endOutgoingTransition:_context.get()];
        
        [_sceneController sceneDidAppear:_context.get() driver:&driver];
        [_outgoingSceneController sceneDidDisappear:_context.get() driver:&driver];
        
        _outgoingSceneController = nullptr;
    }
    
    notifyFrameEnd();
    VROTransaction::commitAll();
}

void VRORenderer::renderEye(VROEyeType eyeType, VRODriver &driver) {
    if (_sceneController) {
        if (_outgoingSceneController) {
            [_outgoingSceneController sceneWillRender:_context.get()];
            [_sceneController sceneWillRender:_context.get()];
            
            _outgoingSceneController.scene->renderBackground(*_context.get(), driver);
            _sceneController.scene->renderBackground(*_context.get(), driver);
            
            _outgoingSceneController.scene->render(*_context.get(), driver);
            _sceneController.scene->render(*_context.get(), driver);
        }
        else {
            [_sceneController sceneWillRender:_context.get()];
            _sceneController.scene->renderBackground(*_context.get(), driver);
            _sceneController.scene->render(*_context.get(), driver);
        }
    }
}

#pragma mark - Reticle

void VRORenderer::handleTap() {
    [_HUD.reticle trigger];
    
    [_delegate reticleTapped:_context->getCamera().getForward()
                           context:_context.get()];
    if (_sceneController) {
        [_sceneController reticleTapped:_context->getCamera().getForward()
                                context:_context.get()];
    }
}

#pragma mark - Scene Loading

void VRORenderer::setSceneController(VROSceneController *sceneController, VRODriver &driver) {
    VROSceneController *outgoingSceneController = _sceneController;
    
    [sceneController sceneWillAppear:_context.get() driver:&driver];
    if (outgoingSceneController) {
        [outgoingSceneController sceneWillDisappear:_context.get() driver:&driver];
    }
    
    _sceneController = sceneController;
    
    [sceneController sceneDidAppear:_context.get() driver:&driver];
    if (outgoingSceneController) {
        [outgoingSceneController sceneDidDisappear:_context.get() driver:&driver];
    }
}

void VRORenderer::setSceneController(VROSceneController *sceneController, bool animated, VRODriver &driver) {
    if (!animated || !_sceneController) {
        _sceneController = sceneController;
        return;
    }
    
    setSceneController(sceneController, kDefaultSceneTransitionDuration, VROTimingFunctionType::EaseIn, driver);
}

void VRORenderer::setSceneController(VROSceneController *sceneController, float seconds,
                                     VROTimingFunctionType timingFunctionType, VRODriver &driver) {
    passert (_sceneController != nil);
    
    _outgoingSceneController = _sceneController;
    _sceneController = sceneController;
    
    _sceneTransitionStartTime = VROTimeCurrentSeconds();
    _sceneTransitionDuration = seconds;
    _sceneTransitionTimingFunction = VROTimingFunction::forType(timingFunctionType);
    
    [_sceneController sceneWillAppear:_context.get() driver:&driver];
    [_outgoingSceneController sceneWillDisappear:_context.get() driver:&driver];
    
    [_sceneController startIncomingTransition:_context.get() duration:seconds];
    [_outgoingSceneController startOutgoingTransition:_context.get() duration:seconds];
}

bool VRORenderer::processSceneTransition() {
    if (!_sceneController || !_outgoingSceneController) {
        return NO;
    }
    
    float percent = (VROTimeCurrentSeconds() - _sceneTransitionStartTime) / _sceneTransitionDuration;
    float t = _sceneTransitionTimingFunction->getT(percent);
    
    BOOL sceneTransitionActive = percent < 0.9999;
    if (sceneTransitionActive) {
        [_sceneController animateIncomingTransition:_context.get() percentComplete:t];
        [_outgoingSceneController animateOutgoingTransition:_context.get() percentComplete:t];
    }
    else {
        [_sceneController animateIncomingTransition:_context.get() percentComplete:1.0];
        [_outgoingSceneController animateOutgoingTransition:_context.get() percentComplete:1.0];
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