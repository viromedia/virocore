//
//  VROSceneRendererGVR.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROSceneRendererGVR.h"
#include "VROLog.h"
#include <assert.h>
#include <stdlib.h>
#include <cmath>
#include <random>
#include "VROTime.h"
#include "VROEye.h"
#include "VROGVRUtil.h"
#include "VROViewport.h"
#include "VRORenderer.h"
#include "VROReticle.h"
#include "VRODisplayOpenGLiOSGVR.h"
#include "VROAllocationTracker.h"

static const uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

#pragma mark - Setup

VROSceneRendererGVR::VROSceneRendererGVR(int width, int height, UIInterfaceOrientation orientation,
                                         float contentScaleFactor,
                                         std::shared_ptr<VRORenderer> renderer,
                                         std::shared_ptr<VRODriver> driver) :
    _frame(0),
    _contentScaleFactor(contentScaleFactor),
    _renderer(renderer),
    _driver(driver),
    _sizeChanged(false),
    _vrModeEnabled(true) {
        setSurfaceSizeInternal(width, height, orientation);
}

VROSceneRendererGVR::~VROSceneRendererGVR() {
    _viewportList.release();
    _swapchain.release();
    _gvr.release();
}

#pragma mark - Rendering

void VROSceneRendererGVR::initGL() {
    _gvr = gvr::GvrApi::Create();
    _gvr->InitializeGl();
    createSwapchain();
    
    _viewportList.reset(new gvr::BufferViewportList(_gvr->CreateEmptyBufferViewportList()));
    _sceneViewport = _gvr->CreateBufferViewport();
    
    // Configure the common properties of the HUD viewport. Each frame this viewport gets
    // modified for each eye and copied into the viewport list. The most important property
    // here is setting the source buffer index to 1, so GVR knows to read from buffer 1 to
    // get the contents of this viewport
    _hudViewport = _gvr->CreateBufferViewport();
    _hudViewport.SetSourceBufferIndex(1);
    _hudViewport.SetReprojection(GVR_REPROJECTION_NONE);
}

void VROSceneRendererGVR::createSwapchain() {
    std::vector<gvr::BufferSpec> specs;
    
    // Buffer specification for the Scene. Do not multisample; we'll do that when rendering
    // from GVR into the GLKView
    specs.push_back(_gvr->CreateBufferSpec());
    specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
    specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_24_STENCIL_8);
    specs[0].SetSize(_surfaceSize);
    specs[0].SetSamples(1);
    
    // Buffer specification for the HUD
    specs.push_back(_gvr->CreateBufferSpec());
    specs[1].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
    specs[1].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_NONE);
    specs[1].SetSize(_surfaceSize);
    specs[1].SetSamples(1);
    
    _swapchain.reset(new gvr::SwapChain(_gvr->CreateSwapChain(specs)));
}

void VROSceneRendererGVR::onDrawFrame() {
    if (_sizeChanged) {
        if (_vrModeEnabled) {
            _swapchain->ResizeBuffer(0, _surfaceSize);
            _swapchain->ResizeBuffer(1, _surfaceSize);
        }
        _sizeChanged = false;
    }

    // Obtain the latest, predicted head pose
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos += kPredictionTimeWithoutVsyncNanos;

    _headView = _gvr->GetHeadSpaceFromStartSpaceRotation(target_time);
    VROMatrix4f headView = VROGVRUtil::toMatrix4f(_headView);
    if (_vrModeEnabled) {
        renderStereo(headView);
    } else {
        renderMono(headView);
    }

    ++_frame;
    ALLOCATION_TRACKER_PRINT();
}

// For stereo rendering we use GVR's swapchain, which provides async reprojection
// (async timewarp, basically), and distorts the images. To do this we render to the
// buffers in the Frame. The Submit() call at the end takes the viewports and performs
// distortion and async reprojection.
void VROSceneRendererGVR::renderStereo(VROMatrix4f &headView) {
    gvr::Sizei renderSize = _surfaceSize;
    VROMatrix4f headRotation = headView.invert();

    // Update the viewports to the latest (these change if the user changed the viewer)
    _viewportList->SetToRecommendedBufferViewports();

    // Acquire a frame from the swap chain
    gvr::Frame frame = _swapchain->AcquireFrame();
    std::dynamic_pointer_cast<VRODisplayOpenGLiOSGVR>(_driver->getDisplay())->setFrame(frame);

    // Get the eye, view, and projection matrices
    VROMatrix4f eyeFromHeadMatrices[GVR_NUM_EYES];
    VROFieldOfView fovs[GVR_NUM_EYES];
    VROViewport viewports[GVR_NUM_EYES];
    VROMatrix4f projectionMatrices[GVR_NUM_EYES];

    for (int i = 0; i < GVR_NUM_EYES; i++) {
        gvr::Eye eye = (gvr::Eye) i;
        eyeFromHeadMatrices[i] = VROGVRUtil::toMatrix4f(_gvr->GetEyeFromHeadMatrix(eye));
        
        _viewportList->GetBufferViewport(eye, &_sceneViewport);
        extractViewParameters(_sceneViewport, renderSize, &viewports[i], &fovs[i]);
        projectionMatrices[i] = fovs[i].toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());

        // Setup the HUD viewport for this eye. The HUD viewport has the same parameters as the
        // scene viewport (its common properties however, configured in initGL, are different)
        _hudViewport.SetTransform(_sceneViewport.GetTransform());
        _hudViewport.SetTargetEye(eye);
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
    headView = _orientationMatrix * headView;
    VROMatrix4f headRotation = headView.invert();

    const gvr::Recti rect = calculatePixelSpaceRect(_surfaceSize, {0, 1, 0, 1});
    VROViewport viewport(rect.left, rect.bottom,
                         rect.right - rect.left,
                         rect.top   - rect.bottom);

    VROFieldOfView fov = _renderer->computeUserFieldOfView(viewport.getWidth(), viewport.getHeight());
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    VROMatrix4f eyeFromHeadMatrix; // Identity

    clearViewport(viewport, false);
    _renderer->prepareFrame(_frame, viewport, fov, headRotation, projection, _driver);
    _renderer->renderEye(VROEyeType::Monocular, _renderer->getLookAtMatrix(), projection, viewport, _driver);
    _renderer->renderHUD(VROEyeType::Monocular, eyeFromHeadMatrix, projection, _driver);
    _renderer->endFrame(_driver);
}

#pragma mark - Lifecycle and Settings

void VROSceneRendererGVR::refreshViewerProfile() {
    if (!_gvr) {
        return;
    }
    _gvr->RefreshViewerProfile();
}

void VROSceneRendererGVR::recenterTracking() {
    if (!_gvr) {
        return;
    }
    _gvr->RecenterTracking();
}

void VROSceneRendererGVR::setVRModeEnabled(bool enabled) {
    _vrModeEnabled = enabled;
}

void VROSceneRendererGVR::pause() {
    if (!_gvr) {
        return;
    }
    _gvr->PauseTracking();
}

void VROSceneRendererGVR::resume() {
    if (!_gvr) {
        return;
    }
    _gvr->ResumeTracking();
}

void VROSceneRendererGVR::setSurfaceSize(int width, int height, UIInterfaceOrientation orientation) {
    int previousWidth = _surfaceSize.width;
    int previousHeight = _surfaceSize.height;
    
    setSurfaceSizeInternal(width, height, orientation);
    if (width != previousWidth || height != previousHeight) {
        _sizeChanged = true;
    }
}

void VROSceneRendererGVR::setSurfaceSizeInternal(int width, int height, UIInterfaceOrientation orientation) {
    _surfaceSize.width = width;
    _surfaceSize.height = height;
    _orientationMatrix = VROMatrix4f::identity();
    
    if (!_vrModeEnabled) {
        if (orientation == UIInterfaceOrientationPortrait) {
            _orientationMatrix.rotateZ(-M_PI_2);
        }
        else if (orientation == UIInterfaceOrientationPortraitUpsideDown) {
            _orientationMatrix.rotateZ(M_PI_2);
        }
        else if (orientation == UIInterfaceOrientationLandscapeLeft) {
            _orientationMatrix.rotateZ(M_PI);
        }
    }
}

#pragma mark - Utility Methods

void VROSceneRendererGVR::clearViewport(VROViewport viewport, bool transparent) {
    // Scissor must be enabled to ensure we only clear the viewport's area
    glEnable(GL_SCISSOR_TEST);
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

gvr::Rectf VROSceneRendererGVR::modulateRect(const gvr::Rectf &rect, float width, float height) {
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

void VROSceneRendererGVR::extractViewParameters(gvr::BufferViewport &viewport, gvr::Sizei renderSize,
                                                VROViewport *outViewport, VROFieldOfView *outFov) {
    const gvr::Recti rect = calculatePixelSpaceRect(renderSize, viewport.GetSourceUv());
    *outViewport = VROViewport(rect.left, rect.bottom,
                               rect.right - rect.left,
                               rect.top   - rect.bottom);
    outViewport->setContentScaleFactor(_contentScaleFactor);
    const gvr::Rectf fov = viewport.GetSourceFov();
    *outFov = VROFieldOfView(fov.left, fov.right, fov.bottom, fov.top);
}


