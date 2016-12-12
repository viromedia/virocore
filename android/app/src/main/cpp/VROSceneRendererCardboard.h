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
#include "VRODriverOpenGLAndroid.h"
#include "VRORenderer.h"
#include "VROFrameSynchronizer.h"

#include "VROEye.h"
#include "VROTimingFunction.h"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include "VRORenderDelegate.h"

class VROSceneController;
class VRORenderer;
class VRORenderDelegate;

class VROSceneRendererCardboard {
 public:

    /*
    Create a VROSceneRendererCardboard using a given |gvr_context|.

    @param gvr_api The (non-owned) gvr_context.
     @param gvr_audio_api The (owned) gvr::AudioApi context.
     */
    VROSceneRendererCardboard(gvr_context* gvr_context,
                              std::shared_ptr<gvr::AudioApi> gvrAudio);
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

    /*
     Set the render delegate, which responds to renderer initialization and
     receives per-frame callbacks.
     */
    void setRenderDelegate(std::shared_ptr<VRORenderDelegate> delegate);

    /*
     Set the active scene controller, which dictates what scene is rendered.
     */
    void setSceneController(std::shared_ptr<VROSceneController> sceneController);
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, bool animated);
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                            VROTimingFunctionType timingFunction);

    /**
     Returns the contained renderer
     */
    std::shared_ptr<VRORenderer> getRenderer() {
        return _renderer;
    }

    std::shared_ptr<VRODriver> getDriver() {
        return _driver;
    }
    std::shared_ptr<VROFrameSynchronizer> getFrameSynchronizer() {
        return _renderer->getFrameSynchronizer();
    }

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

    int _frame;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRORenderDelegate> _renderDelegate;
    std::shared_ptr<VRODriverOpenGLAndroid> _driver;

    std::unique_ptr<gvr::GvrApi> _gvr;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    std::unique_ptr<gvr::BufferViewportList> _viewportList;
    std::unique_ptr<gvr::SwapChain> _swapchain;
    gvr::BufferViewport _scratchViewport;

    gvr::Mat4f _headView;
    gvr::Sizei _renderSize;

    /*
     Utility methods.
     */
    gvr::Rectf modulateRect(const gvr::Rectf &rect, float width, float height);
    gvr::Recti calculatePixelSpaceRect(const gvr::Sizei &texture_size, const gvr::Rectf &texture_rect);
    gvr::Sizei halfPixelCount(const gvr::Sizei& in);
    VROMatrix4f toMatrix4f(const gvr::Mat4f &glm);
    void extractViewParameters(gvr::BufferViewport &viewport, VROViewport *outViewport,
                               VROFieldOfView *outFov);

};

#endif  // VRO_SCENE_RENDERER_CARDBOARD_H  // NOLINT
