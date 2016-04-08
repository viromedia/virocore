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
#import "VROMagnetSensor.h"
#import "VRORenderContextMetal.h"
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

static const float kDefaultSceneTransitionDuration = 1.0;

#pragma mark - Initialization

VRORenderer::VRORenderer(std::shared_ptr<VRODevice> device, VRORenderContext *renderContext) {
    _magnetSensor = new VROMagnetSensor();
    _headTracker = new VROHeadTracker();
    _camera = std::make_shared<VROCameraMutable>();
    _device = device;
    _renderContext = (VRORenderContextMetal *)renderContext; //TODO fix move semenatics here
    
    _monocularEye = new VROEye(VROEyeType::Monocular);
    _leftEye = new VROEye(VROEyeType::Left);
    _rightEye = new VROEye(VROEyeType::Right);
    
    _vrModeEnabled = true;
    _rendererInitialized = false;
    
    _projectionChanged = true;
    
    _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
    _magnetSensor->start();    
    initBlankTexture(*_renderContext);
    
    _HUD = [[VROScreenUIView alloc] init];
}

VRORenderer::~VRORenderer() {
    [_renderDelegate shutdownRenderer];
    
    delete (_magnetSensor);
    delete (_headTracker);
    delete (_monocularEye);
    delete (_leftEye);
    delete (_rightEye);
    delete (_renderContext);
}

#pragma mark - Settings

void VRORenderer::onOrientationChange(UIInterfaceOrientation orientation) {
    _headTracker->updateDeviceOrientation(orientation);
}

float VRORenderer::getVirtualEyeToScreenDistance() const {
    return _device->getScreenToLensDistance();
}

void VRORenderer::setRenderDelegate(id <VRORenderDelegate> renderDelegate) {
    _renderDelegate = renderDelegate;
}

#pragma mark - Camera

void VRORenderer::setPosition(VROVector3f position) {
    _camera->setPosition(position);
}

void VRORenderer::setBaseRotation(VROQuaternion quaternion) {
    _camera->setBaseRotation(quaternion);
}

float VRORenderer::getWorldPerScreen(float distance) const {
    /*
     Arbitrarily chose left eye's left FOV. tan(fov) = perp/distance, where
     perp is in the direction perpendicular to the camera's up vector and
     forward vector, and distance is in the direction of the camera's forward
     vector.
     */
    float radians = _leftEye->getFOV().getLeft();
    float perp = distance * tan(radians);
    
    /*
     The perspective divide is perp divided by half the viewport.
     */
    return perp / (_leftEye->getViewport().getWidth() / 2.0f);
}

#pragma mark - Rendering

void VRORenderer::drawFrame() {
    if (!_headTracker->isReady()) {
        return;
    }
    
    @autoreleasepool {
        prepareFrame(*_renderContext);
        calculateFrameParameters();
        
        VROTransaction::beginImplicitAnimation();
        VROTransaction::update();
        
        if (!_rendererInitialized) {
            [_renderDelegate setupRendererWithContext:_renderContext];
            _rendererInitialized = YES;
        }
        
        if (_vrModeEnabled) {
            renderVRDistortion(*_renderContext);
        }
        else {
            renderMonocular(*_renderContext);
        }
        
        endFrame(*_renderContext);
        VROTransaction::commitAll();
        
        _renderContext->incFrame();
    }
    
    ALLOCATION_TRACKER_PRINT();
}

#pragma mark - View Computation

void VRORenderer::calculateFrameParameters() {
    VROVector3f cameraForward(0, 0, -1.0);
    
    VROMatrix4f headRotation = _headTracker->getHeadRotation();
    
    VROCamera camera;
    camera.setHeadRotation({headRotation.invert()});
    camera.setBaseRotation(_camera->getBaseRotation().getMatrix());
    camera.setPosition(_camera->getPosition());
    
    _renderContext->setCamera(camera);
    
    float halfLensDistance = _device->getInterLensDistance() * 0.5f;
    if (_vrModeEnabled) {
        /*
         The full eye transform is as follows:
         
         1. Set the camera at the origin, looking in the Z negative direction.
         2. Rotate by the camera by the head rotation picked up by the sensors.
         3. Translate the camera by the interlens distance in each direction to get the two eyes.
         */
        VROMatrix4f cameraMatrix = camera.computeLookAtMatrix();
        
        _leftEye->setEyeView(matrix_from_translation( halfLensDistance, 0, 0).multiply(cameraMatrix));
        _rightEye->setEyeView(matrix_from_translation(-halfLensDistance, 0, 0).multiply(cameraMatrix));
        
        /*
         In VR mode, the monocular eye holds the non-stereoscopic matrix (which is used for objects that
         should appear distant, like skyboxes.
         */
        _monocularEye->setEyeView(cameraMatrix);
    }
    else {
        _monocularEye->setEyeView(headRotation);
    }
    
    if (_projectionChanged) {
        const VROScreen &screen = _device->getScreen();
        _monocularEye->setViewport(0, 0, screen.getWidth(), screen.getHeight());
        
        if (!_vrModeEnabled) {
            updateMonocularEye();
        }
        else {
            updateLeftRightEyes();
            onEyesUpdated(_leftEye, _rightEye);
        }
        
        _projectionChanged = NO;
    }
}

void VRORenderer::updateMonocularEye() {
    const VROScreen &screen = _device->getScreen();
    const float monocularBottomFov = 22.5f;
    const float monocularLeftFov = GLKMathRadiansToDegrees(
                                                           atanf(
                                                                 tanf(GLKMathDegreesToRadians(monocularBottomFov))
                                                                 * screen.getWidthInMeters()
                                                                 / screen.getHeightInMeters()));
    _monocularEye->setFOV(monocularLeftFov, monocularLeftFov, monocularBottomFov, monocularBottomFov);
}

void VRORenderer::updateLeftRightEyes() {
    const VROScreen &screen = _device->getScreen();
    
    const VRODistortion &distortion = _device->getDistortion();
    float eyeToScreenDistance = getVirtualEyeToScreenDistance();
    
    float outerDistance = (screen.getWidthInMeters() - _device->getInterLensDistance() ) / 2.0f;
    float innerDistance = _device->getInterLensDistance() / 2.0f;
    float bottomDistance = _device->getVerticalDistanceToLensCenter() - screen.getBorderSizeInMeters();
    float topDistance = screen.getHeightInMeters() + screen.getBorderSizeInMeters() - _device->getVerticalDistanceToLensCenter();
    
    float outerAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(outerDistance / eyeToScreenDistance)));
    float innerAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(innerDistance / eyeToScreenDistance)));
    float bottomAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(bottomDistance / eyeToScreenDistance)));
    float topAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(topDistance / eyeToScreenDistance)));
    
    _leftEye->setFOV(MIN(outerAngle,  _device->getMaximumLeftEyeFOV().getLeft()),
                     MIN(innerAngle,  _device->getMaximumLeftEyeFOV().getRight()),
                     MIN(bottomAngle, _device->getMaximumLeftEyeFOV().getBottom()),
                     MIN(topAngle,    _device->getMaximumLeftEyeFOV().getTop()));
    
    const VROFieldOfView &leftEyeFov = _leftEye->getFOV();
    _rightEye->setFOV(leftEyeFov.getRight(),
                      leftEyeFov.getLeft(),
                      leftEyeFov.getBottom(),
                      leftEyeFov.getTop());
}

#pragma mark - Stereo renderer methods

void VRORenderer::updateRenderViewSize(CGSize size) {
    if (_vrModeEnabled) {
        [_renderDelegate renderViewDidChangeSize:CGSizeMake(size.width / 2, size.height) context:_renderContext];
    }
    else {
        [_renderDelegate renderViewDidChangeSize:CGSizeMake(size.width, size.height) context:_renderContext];
    }
}

void VRORenderer::drawFrame(bool monocular) {
    id <MTLRenderCommandEncoder> renderEncoder = _renderContext->getRenderTarget()->getRenderEncoder();
    
    const float zNear = 0.1;
    const float zFar  = 100;
    
    BOOL sceneTransitionActive = processSceneTransition();
    _renderContext->notifyFrameStart();
    
    [renderEncoder setViewport:_leftEye->getViewport().toMetalViewport()];
    [renderEncoder setScissorRect:_leftEye->getViewport().toMetalScissor()];
    
    _renderContext->setMonocularViewMatrix(_monocularEye->getEyeView());
    _renderContext->setHUDViewMatrix(matrix_from_translation(_device->getInterLensDistance() * 0.5, 0, 0).multiply(_leftEye->getEyeView().invert()));
    _renderContext->setViewMatrix(_leftEye->getEyeView());
    _renderContext->setProjectionMatrix(_leftEye->perspective(zNear, zFar));
    _renderContext->setEyeType(VROEyeType::Left);
    
    renderEye(VROEyeType::Left);
    [_HUD updateWithContext:_renderContext];
    [_HUD renderEye:_leftEye withContext:_renderContext];
    
    if (monocular) {
        _renderContext->notifyFrameEnd();
        return;
    }
    
    [renderEncoder setViewport:_rightEye->getViewport().toMetalViewport()];
    [renderEncoder setScissorRect:_rightEye->getViewport().toMetalScissor()];
    
    _renderContext->setHUDViewMatrix(matrix_from_translation(-_device->getInterLensDistance() * 0.5, 0, 0).multiply(_rightEye->getEyeView().invert()));
    _renderContext->setViewMatrix(_rightEye->getEyeView());
    _renderContext->setProjectionMatrix(_rightEye->perspective(zNear, zFar));
    _renderContext->setEyeType(VROEyeType::Right);
    
    renderEye(VROEyeType::Right);
    [_HUD renderEye:_rightEye withContext:_renderContext];
    
    _renderContext->notifyFrameEnd();
    
    if (!sceneTransitionActive && _outgoingSceneController) {
        [_sceneController endIncomingTransition:_renderContext];
        [_outgoingSceneController endOutgoingTransition:_renderContext];
        
        [_sceneController sceneDidAppear:_renderContext];
        [_outgoingSceneController sceneDidDisappear:_renderContext];
        
        _outgoingSceneController = nullptr;
    }
}

void VRORenderer::renderEye(VROEyeType eyeType) {
    [_renderDelegate willRenderEye:eyeType context:_renderContext];
    if (_sceneController) {
        if (_outgoingSceneController) {
            [_outgoingSceneController sceneWillRender:_renderContext];
            [_sceneController sceneWillRender:_renderContext];
            
            _outgoingSceneController.scene->renderBackground(*_renderContext);
            _sceneController.scene->renderBackground(*_renderContext);
            
            _outgoingSceneController.scene->render(*_renderContext);
            _sceneController.scene->render(*_renderContext);
        }
        else {
            [_sceneController sceneWillRender:_renderContext];
            _sceneController.scene->renderBackground(*_renderContext);
            _sceneController.scene->render(*_renderContext);
        }
    }
    [_renderDelegate didRenderEye:eyeType context:_renderContext];
}

VRORenderContext *VRORenderer::getRenderContext() {
    return _renderContext;
}

#pragma mark - Reticle

void VRORenderer::handleTap() {
    [_HUD.reticle trigger];
    [_renderDelegate reticleTapped:_renderContext->getCamera().getForward()
                           context:_renderContext];
    if (_sceneController) {
        [_sceneController reticleTapped:_renderContext->getCamera().getForward()
                                context:_renderContext];
    }
}

#pragma mark - Scene Loading

void VRORenderer::setSceneController(VROSceneController *sceneController) {
    VROSceneController *outgoingSceneController = _sceneController;
    
    [sceneController sceneWillAppear:_renderContext];
    if (outgoingSceneController) {
        [outgoingSceneController sceneWillDisappear:_renderContext];
    }
    
    _sceneController = sceneController;
    
    [sceneController sceneDidAppear:_renderContext];
    if (outgoingSceneController) {
        [outgoingSceneController sceneDidDisappear:_renderContext];
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
    
    [_sceneController sceneWillAppear:_renderContext];
    [_outgoingSceneController sceneWillDisappear:_renderContext];
    
    [_sceneController startIncomingTransition:_renderContext duration:seconds];
    [_outgoingSceneController startOutgoingTransition:_renderContext duration:seconds];
}

bool VRORenderer::processSceneTransition() {
    if (!_sceneController || !_outgoingSceneController) {
        return NO;
    }
    
    float percent = (VROTimeCurrentSeconds() - _sceneTransitionStartTime) / _sceneTransitionDuration;
    float t = _sceneTransitionTimingFunction->getT(percent);
    
    BOOL sceneTransitionActive = percent < 0.9999;
    if (sceneTransitionActive) {
        [_sceneController animateIncomingTransition:_renderContext percentComplete:t];
        [_outgoingSceneController animateOutgoingTransition:_renderContext percentComplete:t];
    }
    else {
        [_sceneController animateIncomingTransition:_renderContext percentComplete:1.0];
        [_outgoingSceneController animateOutgoingTransition:_renderContext percentComplete:1.0];
    }
    
    return sceneTransitionActive;
}