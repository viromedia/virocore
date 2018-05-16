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

VROSceneRendererGVR::VROSceneRendererGVR(VRORendererConfiguration config,
                                         gvr_context* gvr_context,
                                         std::shared_ptr<gvr::AudioApi> gvrAudio) :
    _gvr(gvr::GvrApi::WrapNonOwned(gvr_context)),
    _sceneViewport(_gvr->CreateBufferViewport()),
    _rendererSuspended(true),
    _vrModeEnabled(true),
    _suspendedNotificationTime(VROTimeCurrentSeconds()) {

    _driver = std::make_shared<VRODriverOpenGLAndroidGVR>(gvrAudio);

    // Create corresponding controllers - cardboard, or daydream if supported.
    std::shared_ptr<VROInputControllerBase> controller;
    _viewerType = _gvr->GetViewerType();

    if (_viewerType == GVR_VIEWER_TYPE_DAYDREAM) {
        controller = std::make_shared<VROInputControllerDaydream>(gvr_context, _driver);
    } else if (_viewerType == GVR_VIEWER_TYPE_CARDBOARD){
        controller = std::make_shared<VROInputControllerCardboard>(_driver);
    } else {
        perror("Unrecognized Viewer type! Falling back to Cardboard Controller as default.");
        controller = std::make_shared<VROInputControllerCardboard>(_driver);
    }

    // Create renderer and attach the controller to it.
    _renderer = std::make_shared<VRORenderer>(config, controller);
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

    // Buffer specification for the Scene
    specs.push_back(_gvr->CreateBufferSpec());
    specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
    specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_24_STENCIL_8);
    specs[0].SetSize(_renderSize);
    specs[0].SetSamples(2);

    // Buffer specification for the HUD
    specs.push_back(_gvr->CreateBufferSpec());
    specs[1].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
    specs[1].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_NONE);
    specs[1].SetSize(_renderSize);
    specs[1].SetSamples(1);

    _swapchain.reset(new gvr::SwapChain(_gvr->CreateSwapChain(specs)));
    _viewportList.reset(new gvr::BufferViewportList(_gvr->CreateEmptyBufferViewportList()));

    // Configure the common properties of the HUD viewport. Each frame this viewport gets
    // modified for each eye and copied into the viewport list. The most important property
    // here is setting the source buffer index to 1, so GVR knows to read from buffer 1 to
    // get the contents of this viewport
    _hudViewport = _gvr->CreateBufferViewport();
    _hudViewport.SetSourceBufferIndex(1);
    _hudViewport.SetReprojection(GVR_REPROJECTION_NONE);
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
    VROMatrix4f headView = VROGVRUtil::toMatrix4f(_headView);

    if (!_rendererSuspended) {
        if (_vrModeEnabled) {
            renderStereo(headView);
        }
        else {
            renderMono(headView);
        }
    }
    else {
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

// For stereo rendering we use GVR's swapchain, which provides async reprojection
// (async timewarp, basically), and distorts the images. To do this we render to the
// buffers in the Frame. The Submit() call at the end takes the viewports and performs
// distortion and async reprojection.
void VROSceneRendererGVR::renderStereo(VROMatrix4f &headView) {
    VROMatrix4f headRotation = headView.invert();

    // Update the viewports to the latest (these change if the user changed the viewer)
    _viewportList->SetToRecommendedBufferViewports();

    // Acquire a frame from the swap chain
    gvr::Frame frame = _swapchain->AcquireFrame();
    std::dynamic_pointer_cast<VRODisplayOpenGLGVR>(_driver->getDisplay())->setFrame(frame);

    // Get the eye, view, and projection matrices
    VROMatrix4f eyeFromHeadMatrices[GVR_NUM_EYES];
    VROFieldOfView fovs[GVR_NUM_EYES];
    VROViewport viewports[GVR_NUM_EYES];
    VROMatrix4f projectionMatrices[GVR_NUM_EYES];

    for (int i = 0; i < GVR_NUM_EYES; i++) {
        gvr::Eye eye = (gvr::Eye) i;
        eyeFromHeadMatrices[i] = VROGVRUtil::toMatrix4f(_gvr->GetEyeFromHeadMatrix(eye));

        _viewportList->GetBufferViewport(eye, &_sceneViewport);
        extractViewParameters(_sceneViewport, &viewports[i], &fovs[i]);
        projectionMatrices[i] = fovs[i].toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());

        // Setup the HUD viewport for this eye. The HUD viewport has the same parameters as the
        // scene viewport (its common properties however, configured in initGL, are different)
        _hudViewport.SetTargetEye(eye);
        _hudViewport.SetTransform(_sceneViewport.GetTransform());
        _hudViewport.SetSourceFov(_sceneViewport.GetSourceFov());
        _hudViewport.SetSourceUv(_sceneViewport.GetSourceUv());
        _viewportList->SetBufferViewport(2 + i, _hudViewport);
    }

    // Prepare the frame and render the left eye
    _renderer->prepareFrame(_frame, viewports[0], fovs[0], headRotation, projectionMatrices[0], _driver);
    clearViewport(viewports[0], false);
    _renderer->renderEye(VROEyeType::Left,
                         eyeFromHeadMatrices[GVR_LEFT_EYE].multiply(_renderer->getLookAtMatrix()),
                         projectionMatrices[GVR_LEFT_EYE],
                         viewports[GVR_LEFT_EYE], _driver);

    // Render the right eye
    clearViewport(viewports[1], false);
    _renderer->renderEye(VROEyeType::Right,
                         eyeFromHeadMatrices[GVR_RIGHT_EYE].multiply(_renderer->getLookAtMatrix()),
                         projectionMatrices[GVR_RIGHT_EYE],
                         viewports[GVR_RIGHT_EYE], _driver);
    frame.Unbind();

    // Bind the HUD buffer and render to both eyes
    frame.BindBuffer(1);
    clearViewport(viewports[0], true);
    _renderer->renderHUD(VROEyeType::Left,
                         eyeFromHeadMatrices[GVR_LEFT_EYE],
                         projectionMatrices[GVR_LEFT_EYE],
                         _driver);

    clearViewport(viewports[1], true);
    _renderer->renderHUD(VROEyeType::Right,
                         eyeFromHeadMatrices[GVR_RIGHT_EYE],
                         projectionMatrices[GVR_RIGHT_EYE],
                         _driver);
    _renderer->endFrame(_driver);

    frame.Unbind(); // Binds the root OpenGL framebuffer, so we can submit the Frame
    frame.Submit(*_viewportList, _headView); // Submits all layers (buffer 0, buffer 1, etc.) to the framebuffer
}

// For mono rendering we simply render direct to our GLSurfaceView, avoiding
// the gvr::Frame and its buffers (so long as gvr::Frame.BindBuffer is not
// called, the GLSurfaceView's primary default framebuffer is bound)
void VROSceneRendererGVR::renderMono(VROMatrix4f &headView) {
    VROMatrix4f headRotation = headView.invert();
    std::dynamic_pointer_cast<VRODisplayOpenGLGVR>(_driver->getDisplay())->clearFrame();

    const gvr::Recti rect = calculatePixelSpaceRect(_renderSize, {0, 1, 0, 1});
    VROViewport viewport(rect.left, rect.bottom,
                         rect.right - rect.left,
                         rect.top   - rect.bottom);

    VROFieldOfView fov = _renderer->computeUserFieldOfView(viewport.getWidth(), viewport.getHeight());
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    VROMatrix4f eyeFromHeadMatrix; // Identity

    clearViewport(viewport, false);
    _renderer->prepareFrame(_frame, viewport, fov, headRotation, projection, _driver);
    _renderer->renderEye(VROEyeType::Monocular, _renderer->getLookAtMatrix(), projection, viewport,  _driver);
    _renderer->renderHUD(VROEyeType::Monocular, eyeFromHeadMatrix, projection, _driver);
    _renderer->endFrame(_driver);
}

void VROSceneRendererGVR::onSurfaceChanged(jobject surface, VRO_INT width, VRO_INT height) {
    VROThreadRestricted::setThread(VROThreadName::Renderer);
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
        shared->_driver->pause();
    });
}

void VROSceneRendererGVR::onResume() {
    std::shared_ptr<VROSceneRendererGVR> shared = shared_from_this();

    VROPlatformDispatchAsyncRenderer([shared] {
        shared->_renderer->getInputController()->onResume();
        shared->_gvr->RefreshViewerProfile();
        shared->_gvr->ResumeTracking();
        shared->_driver->resume();
    });
}

#pragma mark - Utility Methods

void VROSceneRendererGVR::clearViewport(VROViewport viewport, bool transparent) {
    glEnable(GL_SCISSOR_TEST); // Must enable to ensure glClear only clears active 'eye'
    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    glScissor (viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    if (transparent) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

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
    const gvr::Rectf fov = viewport.GetSourceFov();
    *outFov = VROFieldOfView(fov.left, fov.right, fov.bottom, fov.top);
}

void VROSceneRendererGVR::setVRModeEnabled(bool enabled) {
    _vrModeEnabled = enabled;
}

void VROSceneRendererGVR::setSuspended(bool suspendRenderer) {
    _rendererSuspended = suspendRenderer;
}


