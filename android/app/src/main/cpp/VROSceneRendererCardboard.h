//
//  VROSceneRendererCardboard.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRO_SCENE_RENDERER_CARDBOARD_H_  // NOLINT
#define VRO_SCENE_RENDERER_CARDBOARD_H_  // NOLINT

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>

#include <memory>
#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VROSceneRendererCardboard {
 public:

  /*
   Create a VROSceneRendererCardboard using a given |gvr_context|.

   @param gvr_api The (non-owned) gvr_context.
   @param gvr_audio_api The (owned) gvr::AudioApi context.
   */
  VROSceneRendererCardboard(gvr_context* gvr_context,
                            std::unique_ptr<gvr::AudioApi> gvr_audio_api);

  /*
   Destructor.
   */
  ~VROSceneRendererCardboard();

  /*
   GL initialization invoked from rendering thread.
   */
  void InitializeGl();

  /*
   Main render loop.
   */
  void DrawFrame();

  /*
   Event on trigger.
   */
  void OnTriggerEvent();

  /*
   Pause head tracking.
   */
  void OnPause();

  /*
   Resume head tracking, refreshing viewer parameters if necessary.
   */
  void OnResume();

 private:

  /*
   Prepares the GvrApi framebuffer for rendering, resizing if needed.
   */
  void PrepareFramebuffer();

  /*
   Draws all world-space objects for one eye.

   @param view_matrix View transformation for the current eye.
   @param viewport The buffer viewport for which we are rendering.
   */
  void DrawWorld(const gvr::Mat4f& view_matrix,
                 const gvr::BufferViewport& viewport);

  std::unique_ptr<gvr::GvrApi> _gvr;
  std::unique_ptr<gvr::AudioApi> _gvrAudio;
  std::unique_ptr<gvr::BufferViewportList> _viewportList;
  std::unique_ptr<gvr::SwapChain> _swapchain;
  gvr::BufferViewport _scratchViewport;

  gvr::Mat4f _headView;
  gvr::Sizei _renderSize;
};

#endif  // VRO_SCENE_RENDERER_CARDBOARD_H  // NOLINT
