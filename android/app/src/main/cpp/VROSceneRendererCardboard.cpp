//
//  VROSceneRendererCardboard.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROSceneRendererCardboard.h"

#include <android/log.h>
#include <assert.h>
#include <stdlib.h>
#include <cmath>
#include <random>

#include "VROMatrix4f.h"
#include "VROViewport.h"

// TODO Remove
#include "VROSampleRenderer.h"

static const uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

static gvr::Rectf modulateRect(const gvr::Rectf& rect, float width,
                               float height) {
    gvr::Rectf result = {rect.left * width, rect.right * width,
                         rect.bottom * height, rect.top * height};
    return result;
}

static gvr::Recti calculatePixelSpaceRect(const gvr::Sizei& texture_size,
                                          const gvr::Rectf& texture_rect) {
    float width = static_cast<float>(texture_size.width);
    float height = static_cast<float>(texture_size.height);
    gvr::Rectf rect = modulateRect(texture_rect, width, height);
    gvr::Recti result = {
            static_cast<int>(rect.left), static_cast<int>(rect.right),
            static_cast<int>(rect.bottom), static_cast<int>(rect.top)};
    return result;
}

static gvr::Sizei halfPixelCount(const gvr::Sizei& in) {
    // Scale each dimension by sqrt(2)/2 ~= 7/10ths.
    gvr::Sizei out;
    out.width = (7 * in.width) / 10;
    out.height = (7 * in.height) / 10;
    return out;
}

static VROMatrix4f toMatrix4f(const gvr::Mat4f &glm) {
    float m[16] = {
            glm.m[0][0], glm.m[1][0], glm.m[2][0], glm.m[3][0],
            glm.m[0][1], glm.m[1][1], glm.m[2][1], glm.m[3][1],
            glm.m[0][2], glm.m[1][2], glm.m[2][2], glm.m[3][2],
            glm.m[0][3], glm.m[1][3], glm.m[2][3], glm.m[3][3],
    };

    return VROMatrix4f(m);
}

void VROSceneRendererCardboard::extractViewParameters(gvr::BufferViewport &viewport,
                                                      VROViewport *outViewport, VROFieldOfView *outFov) {

    const gvr::Recti rect = calculatePixelSpaceRect(_renderSize, viewport.GetSourceUv());
    *outViewport = VROViewport(rect.left, rect.bottom,
                              rect.right - rect.left,
                              rect.top   - rect.bottom);
    const gvr::Rectf fov = _scratchViewport.GetSourceFov();
    *outFov = VROFieldOfView(fov.left, fov.right,
                             fov.bottom, fov.top);
}

VROSceneRendererCardboard::VROSceneRendererCardboard(gvr_context* gvr_context,
                                                     std::unique_ptr<gvr::AudioApi> gvr_audio_api) :
    _frame(0),
    _gvr(gvr::GvrApi::WrapNonOwned(gvr_context)),
    _gvrAudio(std::move(gvr_audio_api)),
    _scratchViewport(_gvr->CreateBufferViewport()) {

    _renderer = std::make_shared<VRORenderer>();
    _driver = std::make_shared<VRODriverOpenGLAndroid>();

  // TODO Remove sample renderer, place somewhere else
    _renderDelegate = std::make_shared<VROSampleRenderer>(_renderer, this);
    _renderer->setDelegate(_renderDelegate);
}

VROSceneRendererCardboard::~VROSceneRendererCardboard() {
}

void VROSceneRendererCardboard::setSceneController(std::shared_ptr<VROSceneControllerInternal> sceneController, VRODriver &driver) {
    _renderer->setSceneController(sceneController, driver);
}

void VROSceneRendererCardboard::initGL() {
  _gvr->InitializeGl();

  // Because we are using 2X MSAA, we can render to half as many pixels and
  // achieve similar quality.
  _renderSize = halfPixelCount(_gvr->GetMaximumEffectiveRenderTargetSize());

  std::vector<gvr::BufferSpec> specs;
  specs.push_back(_gvr->CreateBufferSpec());

  specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
  specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_16);
  specs[0].SetSize(_renderSize);
  specs[0].SetSamples(2);
  _swapchain.reset(new gvr::SwapChain(_gvr->CreateSwapChain(specs)));

  _viewportList.reset(new gvr::BufferViewportList(
          _gvr->CreateEmptyBufferViewportList()));

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_TRUE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void VROSceneRendererCardboard::onDrawFrame() {
    // Because we are using 2X MSAA, we can render to half as many pixels and
    // achieve similar quality. If the size changed, resize the framebuffer
    gvr::Sizei recommended_size = halfPixelCount(_gvr->GetMaximumEffectiveRenderTargetSize());
    if (_renderSize.width != recommended_size.width || _renderSize.height != recommended_size.height) {
        _swapchain->ResizeBuffer(0, recommended_size);
        _renderSize = recommended_size;
    }

    // Update the viewports to the latest (these change if the user changed the viewer)
    _viewportList->SetToRecommendedBufferViewports();

    // Acquire a frame from the swap chain
    gvr::Frame frame = _swapchain->AcquireFrame();

    // Obtain the latest, predicted head pose
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos += kPredictionTimeWithoutVsyncNanos;

    _headView = _gvr->GetHeadSpaceFromStartSpaceRotation(target_time);
    VROMatrix4f headRotation = toMatrix4f(_headView).invert();

    frame.BindBuffer(0);

    // Extract the left viewport parameters
    _viewportList->GetBufferViewport(GVR_LEFT_EYE, &_scratchViewport);
    VROViewport leftViewport;
    VROFieldOfView leftFov;
    extractViewParameters(_scratchViewport, &leftViewport, &leftFov);

    // Prepare the frame and render the left eye
    prepareFrame(leftViewport, leftFov, headRotation);
    renderEye(VROEyeType::Left, toMatrix4f(_gvr->GetEyeFromHeadMatrix(GVR_LEFT_EYE)),
              leftViewport, leftFov);

    // Extract the right viewport parameters
    _viewportList->GetBufferViewport(GVR_RIGHT_EYE, &_scratchViewport);
    VROViewport rightViewport;
    VROFieldOfView rightFov;
    extractViewParameters(_scratchViewport, &rightViewport, &rightFov);

    // Render the right eye and end the frame
    renderEye(VROEyeType::Right, toMatrix4f(_gvr->GetEyeFromHeadMatrix(GVR_RIGHT_EYE)),
              rightViewport, rightFov);
    _renderer->endFrame(*_driver.get());

    frame.Unbind();
    frame.Submit(*_viewportList, _headView);
}

void VROSceneRendererCardboard::onTriggerEvent() {

}

void VROSceneRendererCardboard::onPause() {
  _gvr->PauseTracking();
  _gvrAudio->Pause();
}

void VROSceneRendererCardboard::onResume() {
  _gvr->RefreshViewerProfile();
  _gvr->ResumeTracking();
  _gvrAudio->Resume();
}

void VROSceneRendererCardboard::prepareFrame(VROViewport leftViewport, VROFieldOfView fov, VROMatrix4f headRotation) {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // Must enable writes to clear depth buffer

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    _renderer->prepareFrame(_frame, leftViewport, fov, headRotation, *_driver.get());
}

void VROSceneRendererCardboard::renderEye(VROEyeType eyeType,
                                          VROMatrix4f eyeFromHeadMatrix,
                                          VROViewport viewport,
                                          VROFieldOfView fov) {

    VROMatrix4f projectionMatrix = fov.toPerspectiveMatrix(kZNear, kZFar);

    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    _renderer->renderEye(eyeType, eyeFromHeadMatrix, projectionMatrix, *_driver.get());
}





