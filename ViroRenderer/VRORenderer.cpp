//
//  VRORenderer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/5/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VRORenderer.h"
#import "VROTime.h"
#import "VRODevice.h"
#import "VRODistortion.h"
#import "VRODistortionRenderer.h"
#import "VROEye.h"
#import "VROFieldOfView.h"
#import "VROViewport.h"
#import "VROScreen.h"
#import "VROHeadTracker.h"
#import "VRODriverContextMetal.h"
#import "VROTransaction.h"
#import "VROImageUtil.h"
#import "VROProjector.h"
#import "VROAllocationTracker.h"
#import "VROScene.h"
#import "VROSceneController.h"
#import "VROLog.h"
#import "VROCameraMutable.h"
#import "VROScreenUIView.h"
#import "VROView.h"
#import "VRORenderDelegate.h"

static const float kDefaultSceneTransitionDuration = 1.0;

#pragma mark - Initialization

VRORenderer::VRORenderer() :
    _rendererInitialized(false),
    _context(std::make_shared<VRORenderContext>()),
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

void VRORenderer::prepareFrame(int frame, VROMatrix4f headRotation, VRODriverContext &driverContext) {
    if (!_rendererInitialized) {
        [_delegate setupRendererWithDriverContext:&driverContext];
        _rendererInitialized = YES;
    }
    
    _sceneTransitionActive = processSceneTransition();
    
    _context->setFrame(frame);
    _context->notifyFrameStart();
    
    VROCamera camera;
    camera.setHeadRotation({headRotation.invert()});
    camera.setBaseRotation(_camera->getBaseRotation().getMatrix());
    camera.setPosition(_camera->getPosition());
    
    _context->setCamera(camera);
    
    /*
     The full eye transform is as follows:
     
     1. Set the camera at the origin, looking in the Z negative direction.
     2. Rotate by the camera by the head rotation picked up by the sensors.
     3. Translate the camera by the interlens distance in each direction to get the two eyes.
     */
    VROMatrix4f cameraMatrix = camera.computeLookAtMatrix();
        
    /*
     The monocular view matrix is used for objects that should appear distant, like skyboxes.
     */
    _context->setMonocularViewMatrix(cameraMatrix);

    [_HUD updateWithContext:&driverContext];
}

void VRORenderer::renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projectionMatrix,
                            const VRODriverContext &driverContext) {
    [_delegate willRenderEye:eye context:_context.get()];

    VROMatrix4f cameraMatrix = _context->getCamera().computeLookAtMatrix();
    VROMatrix4f eyeView = eyeFromHeadMatrix.multiply(cameraMatrix);
    
    _context->setHUDViewMatrix(eyeFromHeadMatrix.multiply(eyeView.invert()));
    _context->setViewMatrix(eyeView);
    _context->setProjectionMatrix(projectionMatrix);
    _context->setEyeType(eye);
    
    renderEye(eye, driverContext);
    [_HUD renderEye:eye withRenderContext:_context.get() driverContext:&driverContext];
    
    [_delegate didRenderEye:eye context:_context.get()];
}

void VRORenderer::endFrame(const VRODriverContext &driverContext) {
    if (!_sceneTransitionActive && _outgoingSceneController) {
        [_sceneController endIncomingTransition:_context.get()];
        [_outgoingSceneController endOutgoingTransition:_context.get()];
        
        [_sceneController sceneDidAppear:_context.get()];
        [_outgoingSceneController sceneDidDisappear:_context.get()];
        
        _outgoingSceneController = nullptr;
    }
    
    _context->notifyFrameEnd();
}

void VRORenderer::renderEye(VROEyeType eyeType, const VRODriverContext &driverContext) {
    if (_sceneController) {
        if (_outgoingSceneController) {
            [_outgoingSceneController sceneWillRender:_context.get()];
            [_sceneController sceneWillRender:_context.get()];
            
            _outgoingSceneController.scene->renderBackground(*_context, driverContext);
            _sceneController.scene->renderBackground(*_context, driverContext);
            
            _outgoingSceneController.scene->render(*_context, driverContext);
            _sceneController.scene->render(*_context, driverContext);
        }
        else {
            [_sceneController sceneWillRender:_context.get()];
            _sceneController.scene->renderBackground(*_context, driverContext);
            _sceneController.scene->render(*_context, driverContext);
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

void VRORenderer::setSceneController(VROSceneController *sceneController) {
    VROSceneController *outgoingSceneController = _sceneController;
    
    [sceneController sceneWillAppear:_context.get()];
    if (outgoingSceneController) {
        [outgoingSceneController sceneWillDisappear:_context.get()];
    }
    
    _sceneController = sceneController;
    
    [sceneController sceneDidAppear:_context.get()];
    if (outgoingSceneController) {
        [outgoingSceneController sceneDidDisappear:_context.get()];
    }
}

void VRORenderer::setSceneController(VROSceneController *sceneController, bool animated) {
    if (!animated || !_sceneController) {
        _sceneController = sceneController;
        return;
    }
    
    setSceneController(sceneController, kDefaultSceneTransitionDuration, VROTimingFunctionType::EaseIn);
}

void VRORenderer::setSceneController(VROSceneController *sceneController, float seconds, VROTimingFunctionType timingFunctionType) {
    _outgoingSceneController = _sceneController;
    _sceneController = sceneController;
    
    _sceneTransitionStartTime = VROTimeCurrentSeconds();
    _sceneTransitionDuration = seconds;
    _sceneTransitionTimingFunction = VROTimingFunction::forType(timingFunctionType);
    
    [_sceneController sceneWillAppear:_context.get()];
    [_outgoingSceneController sceneWillDisappear:_context.get()];
    
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