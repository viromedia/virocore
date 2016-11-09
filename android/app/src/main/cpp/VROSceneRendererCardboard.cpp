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

static VROMatrix4f toMatrix4f(const gvr::Mat4f &glm) {
    float m[16] = {
            glm.m[0][0], glm.m[1][0], glm.m[2][0], glm.m[3][0],
            glm.m[0][1], glm.m[1][1], glm.m[2][1], glm.m[3][1],
            glm.m[0][2], glm.m[1][2], glm.m[2][2], glm.m[3][2],
            glm.m[0][3], glm.m[1][3], glm.m[2][3], glm.m[3][3],
    };

    return VROMatrix4f(m);
}

VROSceneRendererCardboard::VROSceneRendererCardboard(gvr_context* gvr_context,
                                                     std::unique_ptr<gvr::AudioApi> gvr_audio_api) :
    _frame(0),
    _gvr(gvr::GvrApi::WrapNonOwned(gvr_context)),
    _gvrAudio(std::move(gvr_audio_api)),
    _scratchViewport(_gvr->CreateBufferViewport()) {

    _renderer = std::make_shared<VRORenderer>();
    _driver = std::make_shared<VRODriverOpenGLAndroid>();
  // TODO Create sample render delegate here

}

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

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_TRUE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void VROSceneRendererCardboard::DrawFrame() {
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

    // Update the viewports to the latest (these change if the user
    // changed the viewer)
    _viewportList->SetToRecommendedBufferViewports();

    // Acquire a frame from the swap chain
    gvr::Frame frame = _swapchain->AcquireFrame();

    // Obtain the latest, predicted head pose
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos += kPredictionTimeWithoutVsyncNanos;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // Must enable writes to clear depth buffer

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    _headView = _gvr->GetHeadSpaceFromStartSpaceRotation(target_time);
    VROMatrix4f headRotation = toMatrix4f(_headView).invert();

    _renderer->prepareFrame(_frame, headRotation, *_driver.get());

    gvr::Mat4f left_eye_matrix = _gvr->GetEyeFromHeadMatrix(GVR_LEFT_EYE);
    gvr::Mat4f right_eye_matrix = _gvr->GetEyeFromHeadMatrix(GVR_RIGHT_EYE);
    gvr::Mat4f left_eye_view = MatrixMul(left_eye_matrix, _headView);
    gvr::Mat4f right_eye_view = MatrixMul(right_eye_matrix, _headView);

    // Render the scene to each eye
    frame.BindBuffer(0);
    _viewportList->GetBufferViewport(GVR_LEFT_EYE, &_scratchViewport);
    renderEye(VROEyeType::Left, left_eye_view, _scratchViewport);
    _viewportList->GetBufferViewport(GVR_RIGHT_EYE, &_scratchViewport);
    renderEye(VROEyeType::Right, right_eye_view, _scratchViewport);

    _renderer->endFrame(*_driver.get());

    frame.Unbind();

    // Submit frame.
    frame.Submit(*_viewportList, _headView);
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

static gvr::Rectf ModulateRect(const gvr::Rectf& rect, float width,
                               float height) {
    gvr::Rectf result = {rect.left * width, rect.right * width,
                         rect.bottom * height, rect.top * height};
    return result;
}

static gvr::Recti CalculatePixelSpaceRect(const gvr::Sizei& texture_size,
                                          const gvr::Rectf& texture_rect) {
    float width = static_cast<float>(texture_size.width);
    float height = static_cast<float>(texture_size.height);
    gvr::Rectf rect = ModulateRect(texture_rect, width, height);
    gvr::Recti result = {
            static_cast<int>(rect.left), static_cast<int>(rect.right),
            static_cast<int>(rect.bottom), static_cast<int>(rect.top)};
    return result;
}

static gvr::Mat4f PerspectiveMatrixFromView(const gvr::Rectf& fov, float z_near,
                                            float z_far) {
    gvr::Mat4f result;
    const float x_left = -std::tan(fov.left * M_PI / 180.0f) * z_near;
    const float x_right = std::tan(fov.right * M_PI / 180.0f) * z_near;
    const float y_bottom = -std::tan(fov.bottom * M_PI / 180.0f) * z_near;
    const float y_top = std::tan(fov.top * M_PI / 180.0f) * z_near;
    const float zero = 0.0f;

    assert(x_left < x_right && y_bottom < y_top && z_near < z_far &&
           z_near > zero && z_far > zero);
    const float X = (2 * z_near) / (x_right - x_left);
    const float Y = (2 * z_near) / (y_top - y_bottom);
    const float A = (x_right + x_left) / (x_right - x_left);
    const float B = (y_top + y_bottom) / (y_top - y_bottom);
    const float C = (z_near + z_far) / (z_near - z_far);
    const float D = (2 * z_near * z_far) / (z_near - z_far);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0.0f;
        }
    }
    result.m[0][0] = X;
    result.m[0][2] = A;
    result.m[1][1] = Y;
    result.m[1][2] = B;
    result.m[2][2] = C;
    result.m[2][3] = D;
    result.m[3][2] = -1;

    return result;
}

/**
 * Draws a frame for an eye.
 *
 * @param eye The eye to render. Includes all required transformations.
 */
void VROSceneRendererCardboard::renderEye(VROEyeType eyeType,
                                          const gvr::Mat4f& view_matrix,
                                          const gvr::BufferViewport& bufferViewport) {

    const gvr::Recti rect = CalculatePixelSpaceRect(_renderSize, bufferViewport.GetSourceUv());
    VROViewport viewport(rect.bottom, rect.left, rect.right - rect.left, rect.top - rect.bottom);

    VROMatrix4f eyeMatrix = toMatrix4f(view_matrix);

    gvr::Mat4f perspective = PerspectiveMatrixFromView(bufferViewport.GetSourceFov(), 0.01, 100); //TODO near far
    VROMatrix4f projectionMatrix = toMatrix4f(perspective);

    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    glScissor(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());

    _renderer->renderEye(eyeType, eyeMatrix, projectionMatrix, *_driver.get());
}





