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

#define LOG_TAG "TreasureHuntCPP"
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace {

static const uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

static gvr::Mat4f MatrixMul(const gvr::Mat4f& matrix1,
                            const gvr::Mat4f& matrix2) {
  gvr::Mat4f result;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result.m[i][j] = 0.0f;
      for (int k = 0; k < 4; ++k) {
        result.m[i][j] += matrix1.m[i][k]*matrix2.m[k][j];
      }
    }
  }
  return result;
}

}  // namespace

VROSceneRendererCardboard::VROSceneRendererCardboard(
    gvr_context* gvr_context, std::unique_ptr<gvr::AudioApi> gvr_audio_api)
    : _gvr(gvr::GvrApi::WrapNonOwned(gvr_context)),
      _gvrAudio(std::move(gvr_audio_api)),
      _scratchViewport(_gvr->CreateBufferViewport())
{}

VROSceneRendererCardboard::~VROSceneRendererCardboard() {
}

static gvr::Sizei HalfPixelCount(const gvr::Sizei& in) {
  // Scale each dimension by sqrt(2)/2 ~= 7/10ths.
  gvr::Sizei out;
  out.width = (7 * in.width) / 10;
  out.height = (7 * in.height) / 10;
  return out;
}

void VROSceneRendererCardboard::InitializeGl() {
  _gvr->InitializeGl();

  // Because we are using 2X MSAA, we can render to half as many pixels and
  // achieve similar quality.
  _renderSize = HalfPixelCount(_gvr->GetMaximumEffectiveRenderTargetSize());

  std::vector<gvr::BufferSpec> specs;
  specs.push_back(_gvr->CreateBufferSpec());

  specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
  specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_16);
  specs[0].SetSize(_renderSize);
  specs[0].SetSamples(2);
  _swapchain.reset(new gvr::SwapChain(_gvr->CreateSwapChain(specs)));

  _viewportList.reset(new gvr::BufferViewportList(
          _gvr->CreateEmptyBufferViewportList()));
}

void VROSceneRendererCardboard::DrawFrame() {
  PrepareFramebuffer();

  // Update the viewports to the latest (these change if the user
  // changed the viewer)
  _viewportList->SetToRecommendedBufferViewports();

  // Acquire a frame from the swap chain
  gvr::Frame frame = _swapchain->AcquireFrame();

  // A client app does its rendering here
  gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
  target_time.monotonic_system_time_nanos +=
      kPredictionTimeWithoutVsyncNanos;

  // Obtain the latest, predicted head pose
  _headView = _gvr->GetHeadSpaceFromStartSpaceRotation(target_time);
  gvr::Mat4f left_eye_matrix = _gvr->GetEyeFromHeadMatrix(GVR_LEFT_EYE);
  gvr::Mat4f right_eye_matrix = _gvr->GetEyeFromHeadMatrix(GVR_RIGHT_EYE);
  gvr::Mat4f left_eye_view = MatrixMul(left_eye_matrix, _headView);
  gvr::Mat4f right_eye_view = MatrixMul(right_eye_matrix, _headView);

  // Render the scene to each eye
  frame.BindBuffer(0);
  _viewportList->GetBufferViewport(GVR_LEFT_EYE, &_scratchViewport);
  DrawWorld(left_eye_view, _scratchViewport);
  _viewportList->GetBufferViewport(GVR_RIGHT_EYE, &_scratchViewport);
  DrawWorld(right_eye_view, _scratchViewport);
  frame.Unbind();

  // Submit frame.
  frame.Submit(*_viewportList, _headView);
}

void VROSceneRendererCardboard::PrepareFramebuffer() {
  // Because we are using 2X MSAA, we can render to half as many pixels and
  // achieve similar quality.
  gvr::Sizei recommended_size =
      HalfPixelCount(_gvr->GetMaximumEffectiveRenderTargetSize());
  if (_renderSize.width != recommended_size.width ||
      _renderSize.height != recommended_size.height) {
    // We need to resize the framebuffer.
    _swapchain->ResizeBuffer(0, recommended_size);
    _renderSize = recommended_size;
  }
}

void VROSceneRendererCardboard::OnTriggerEvent() {

}

void VROSceneRendererCardboard::OnPause() {
  _gvr->PauseTracking();
  _gvrAudio->Pause();
}

void VROSceneRendererCardboard::OnResume() {
  _gvr->RefreshViewerProfile();
  _gvr->ResumeTracking();
  _gvrAudio->Resume();
}

/**
 * Draws a frame for an eye.
 *
 * @param eye The eye to render. Includes all required transformations.
 */
void VROSceneRendererCardboard::DrawWorld(const gvr::Mat4f& view_matrix,
                                     const gvr::BufferViewport& viewport) {

  glClearColor(1.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}





