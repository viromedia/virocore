//
//  VROSceneRendererGVR.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROSceneRendererGVR.h"

#include <android/log.h>
#include <assert.h>
#include <stdlib.h>
#include <cmath>
#include <random>
#include <VROTime.h>

#include "VRODriverOpenGLAndroidGVR.h"
#include "VROGVRUtil.h"
#include "VROMatrix4f.h"
#include "VROViewport.h"
#include "VRORenderer.h"
#include "VROSceneController.h"
#include "VRORenderDelegate.h"
#include "VROReticle.h"
#include "VROInputControllerDaydream.h"
#include "VROInputControllerCardboard.h"
#include "VROAllocationTracker.h"
static const uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

#pragma mark - Setup

VROSceneRendererGVR::VROSceneRendererGVR(gvr_context* gvr_context,
                                                     std::shared_ptr<gvr::AudioApi> gvrAudio) :
    _gvr(gvr::GvrApi::WrapNonOwned(gvr_context)),
    _scratchViewport(_gvr->CreateBufferViewport()),
    _rendererSuspended(true),
    _vrModeEnabled(true),
    _suspendedNotificationTime(VROTimeCurrentSeconds()) {

    // Create corresponding controllers - cardboard, or daydream if supported.
    std::shared_ptr<VROInputControllerBase> controller;
    _viewerType = _gvr->GetViewerType();

    if (_viewerType == GVR_VIEWER_TYPE_DAYDREAM) {
        controller = std::make_shared<VROInputControllerDaydream>(gvr_context);
    } else if (_viewerType == GVR_VIEWER_TYPE_CARDBOARD){
        controller = std::make_shared<VROInputControllerCardboard>();
    } else {
        perror("Unrecognized Viewer type! Falling back to Cardboard Controller as default.");
        controller = std::make_shared<VROInputControllerCardboard>();
    }

    // Create renderer and attach the controller to it.
    _renderer = std::make_shared<VRORenderer>(controller);
    _driver = std::make_shared<VRODriverOpenGLAndroidGVR>(gvrAudio);
}

VROSceneRendererGVR::~VROSceneRendererGVR() {

}

#pragma mark - Rendering

void VROSceneRendererGVR::initGL() {
    _gvr->InitializeGl();

    // Because we are using 2X MSAA, we can render to half as many pixels and
    // achieve similar quality.
    _renderSize = halfPixelCount(_gvr->GetMaximumEffectiveRenderTargetSize());

    std::vector<gvr::BufferSpec> specs;
    specs.push_back(_gvr->CreateBufferSpec());

    specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
    specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_24);
    specs[0].SetSize(_renderSize);
    specs[0].SetSamples(2);
    _swapchain.reset(new gvr::SwapChain(_gvr->CreateSwapChain(specs)));

    _viewportList.reset(new gvr::BufferViewportList(
            _gvr->CreateEmptyBufferViewportList()));

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
}

void VROSceneRendererGVR::onDrawFrame() {

    // Because we are using 2X MSAA, we can render to half as many pixels and
    // achieve similar quality. If the size changed, resize the framebuffer
    gvr::Sizei recommended_size = _vrModeEnabled ?
            halfPixelCount(_gvr->GetMaximumEffectiveRenderTargetSize()) : _surfaceSize;
    if (_renderSize.width != recommended_size.width || _renderSize.height != recommended_size.height) {
        if (_vrModeEnabled) {
            // For some reason, Samsung phones won't work if this line is run when switching to
            // Mono/360 mode (the Axon/Pixel work, so i wonder if it's a diff in Mali vs Adreno GPUs)
            _swapchain->ResizeBuffer(0, recommended_size);
        }
        _renderSize = recommended_size;
    }

    // Obtain the latest, predicted head pose
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos += kPredictionTimeWithoutVsyncNanos;

    _headView = _gvr->GetHeadSpaceFromStartSpaceRotation(target_time);
    VROMatrix4f headRotation = VROGVRUtil::toMatrix4f(_headView).invert();

    if (!_rendererSuspended) {
        if (_vrModeEnabled) {
            renderStereo(headRotation);
        }
        else {
            renderMono(headRotation);
        }
    } else {
        _viewportList->SetToRecommendedBufferViewports();
        gvr::Frame frame = _swapchain->AcquireFrame();
        std::dynamic_pointer_cast<VRODisplayOpenGLGVR>(_driver->getDisplay())->setFrame(frame);
        frame.BindBuffer(0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        frame.Unbind();
        frame.Submit(*_viewportList, _headView);

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

// For stereo rendering we use GVR's swapchain, which provides async projection
// (async timewarp, basically), and distorts the images. To do this we render
// to each of the buffers. The gvr::Frame performs distortion and async reprojection
// when we invoke Submit().
void VROSceneRendererGVR::renderStereo(VROMatrix4f &headRotation) {
    // Update the viewports to the latest (these change if the user changed the viewer)
    _viewportList->SetToRecommendedBufferViewports();

    // Acquire a frame from the swap chain
    gvr::Frame frame = _swapchain->AcquireFrame();
    std::dynamic_pointer_cast<VRODisplayOpenGLGVR>(_driver->getDisplay())->setFrame(frame);
    frame.BindBuffer(0);

    // Extract the left viewport parameters
    _viewportList->GetBufferViewport(GVR_LEFT_EYE, &_scratchViewport);
    VROViewport leftViewport;
    VROFieldOfView leftFov;
    extractViewParameters(_scratchViewport, &leftViewport, &leftFov);

    // Prepare the frame and render the left eye
    prepareFrame(leftViewport, leftFov, headRotation);
    renderEye(VROEyeType::Left, VROGVRUtil::toMatrix4f(_gvr->GetEyeFromHeadMatrix(GVR_LEFT_EYE)),
              leftViewport, leftFov);

    // Extract the right viewport parameters
    _viewportList->GetBufferViewport(GVR_RIGHT_EYE, &_scratchViewport);
    VROViewport rightViewport;
    VROFieldOfView rightFov;
    extractViewParameters(_scratchViewport, &rightViewport, &rightFov);

    // Render the right eye and end the frame
    renderEye(VROEyeType::Right, VROGVRUtil::toMatrix4f(_gvr->GetEyeFromHeadMatrix(GVR_RIGHT_EYE)),
              rightViewport, rightFov);
    _renderer->endFrame(_driver);

    frame.Unbind();
    frame.Submit(*_viewportList, _headView);
}

// For mono rendering we simply render direct to our GLSurfaceView, avoiding
// the gvr::Frame and its buffers (so long as gvr::Frame.BindBuffer is not
// called, the GLSurfaceView's primary default framebuffer is bound)
void VROSceneRendererGVR::renderMono(VROMatrix4f &headRotation) {
    const gvr::Recti rect = calculatePixelSpaceRect(_renderSize, {0, 1, 0, 1});
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
void VROSceneRendererGVR::onSurfaceChanged(jobject surface, jint width, jint height) {
    VROThreadRestricted::setThread(VROThreadName::Renderer, pthread_self());

    _surfaceSize.width = width;
    _surfaceSize.height = height;
}

void VROSceneRendererGVR::onTouchEvent(int action, float x, float y) {
    if (_viewerType == GVR_VIEWER_TYPE_CARDBOARD) {
        std::shared_ptr<VROInputControllerBase> baseController =  _renderer->getInputController();
        std::shared_ptr<VROInputControllerCardboard> cardboardController
                = std::dynamic_pointer_cast<VROInputControllerCardboard>(baseController);
        cardboardController->updateScreenTouch(action);
    }
}

void VROSceneRendererGVR::onPause() {
    std::shared_ptr<VROSceneRendererGVR> shared = shared_from_this();

    VROPlatformDispatchAsyncRenderer([shared] {
        shared->_renderer->getInputController()->onPause();
        shared->_gvr->PauseTracking();
        shared->_driver->onPause();
    });
}

void VROSceneRendererGVR::onResume() {
    std::shared_ptr<VROSceneRendererGVR> shared = shared_from_this();

    VROPlatformDispatchAsyncRenderer([shared] {
        shared->_renderer->getInputController()->onResume();
        shared->_gvr->RefreshViewerProfile();
        shared->_gvr->ResumeTracking();
        shared->_driver->onResume();
    });
}

void VROSceneRendererGVR::prepareFrame(VROViewport leftViewport, VROFieldOfView fov, VROMatrix4f headRotation) {
    glEnable(GL_SCISSOR_TEST); // Must enable to ensure glClear only clears active 'eye'
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // Must enable writes to clear depth buffer
    _driver->setBlendingMode(VROBlendMode::Alpha);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    _renderer->prepareFrame(_frame, leftViewport, fov, headRotation, projection, _driver);
}

void VROSceneRendererGVR::renderEye(VROEyeType eyeType,
                                    VROMatrix4f eyeFromHeadMatrix,
                                    VROViewport viewport,
                                    VROFieldOfView fov) {

    VROMatrix4f projectionMatrix = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());

    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    glScissor(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    _renderer->renderEye(eyeType, eyeFromHeadMatrix, projectionMatrix, viewport, _driver);
}

#pragma mark - Utility Methods

gvr::Rectf VROSceneRendererGVR::modulateRect(const gvr::Rectf &rect, float width,
                                             float height) {
    gvr::Rectf result = {rect.left * width, rect.right * width,
                         rect.bottom * height, rect.top * height};
    return result;
}

gvr::Recti VROSceneRendererGVR::calculatePixelSpaceRect(const gvr::Sizei &texture_size,
                                                        const gvr::Rectf &texture_rect) {
    float width = static_cast<float>(texture_size.width);
    float height = static_cast<float>(texture_size.height);
    gvr::Rectf rect = modulateRect(texture_rect, width, height);
    gvr::Recti result = {
            static_cast<int>(rect.left), static_cast<int>(rect.right),
            static_cast<int>(rect.bottom), static_cast<int>(rect.top)};
    return result;
}

gvr::Sizei VROSceneRendererGVR::halfPixelCount(const gvr::Sizei& in) {
    // Scale each dimension by sqrt(2)/2 ~= 7/10ths.
    gvr::Sizei out;
    out.width = (7 * in.width) / 10;
    out.height = (7 * in.height) / 10;
    return out;
}

void VROSceneRendererGVR::extractViewParameters(gvr::BufferViewport &viewport,
                                                VROViewport *outViewport, VROFieldOfView *outFov) {

    const gvr::Recti rect = calculatePixelSpaceRect(_renderSize, viewport.GetSourceUv());
    *outViewport = VROViewport(rect.left, rect.bottom,
                               rect.right - rect.left,
                               rect.top   - rect.bottom);
    const gvr::Rectf fov = _scratchViewport.GetSourceFov();
    *outFov = VROFieldOfView(fov.left, fov.right,
                             fov.bottom, fov.top);
}

void VROSceneRendererGVR::setVRModeEnabled(bool enabled) {
    _vrModeEnabled = enabled;
}

void VROSceneRendererGVR::setSuspended(bool suspendRenderer) {
    _rendererSuspended = suspendRenderer;
}


