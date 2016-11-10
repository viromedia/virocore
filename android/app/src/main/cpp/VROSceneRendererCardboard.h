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

#include "VRORenderer.h"
#include "VRORenderDelegate.h"
#include "VRODriverOpenGLAndroid.h"
#include "VROEye.h"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VROSceneControllerInternal;

class VROSceneRendererCardboard {
 public:

    /*
    Create a VROSceneRendererCardboard using a given |gvr_context|.

    @param gvr_api The (non-owned) gvr_context.
     @param gvr_audio_api The (owned) gvr::AudioApi context.
     */
    VROSceneRendererCardboard(gvr_context* gvr_context,
                              std::unique_ptr<gvr::AudioApi> gvr_audio_api);
    ~VROSceneRendererCardboard();

    /*
     GL initialization invoked from rendering thread.
     */
     void initGL();

    /*
     Main render loop.
     */
    void onDrawFrame();

    /*
     Event on trigger.
     */
    void onTriggerEvent();

    /*
     Pause head tracking.
     */
    void onPause();

    /*
     Resume head tracking, refreshing viewer parameters if necessary.
     */
    void onResume();

    void setSceneController(std::shared_ptr<VROSceneControllerInternal> sceneController, VRODriver &driver);

private:

    /*
     Prepares the GvrApi framebuffer for rendering, resizing if needed.
     */
    void prepareFrame(VROViewport leftViewport,
                      VROFieldOfView fov,
                      VROMatrix4f headRotation);

    /*
     Draws the scene for the given eye.
     */
    void renderEye(VROEyeType eyeType,
                   VROMatrix4f eyeFromHeadMatrix,
                   VROViewport viewport,
                   VROFieldOfView fov);

    void extractViewParameters(gvr::BufferViewport &viewport,
                               VROViewport *outViewport, VROFieldOfView *outFov);

    int _frame;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRORenderDelegate> _renderDelegate;
    std::shared_ptr<VRODriverOpenGL> _driver;

    std::unique_ptr<gvr::GvrApi> _gvr;
    std::unique_ptr<gvr::AudioApi> _gvrAudio;
    std::unique_ptr<gvr::BufferViewportList> _viewportList;
    std::unique_ptr<gvr::SwapChain> _swapchain;
    gvr::BufferViewport _scratchViewport;

    gvr::Mat4f _headView;
    gvr::Sizei _renderSize;
};

#endif  // VRO_SCENE_RENDERER_CARDBOARD_H  // NOLINT
