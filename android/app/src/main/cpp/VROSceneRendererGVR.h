//
//  VROSceneRendererGVR.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRO_SCENE_RENDERER_GVR_H_  // NOLINT
#define VRO_SCENE_RENDERER_GVR_H_  // NOLINT

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>

#include <string>
#include <thread>  // NOLINT
#include <vector>
#include "VROSceneRenderer.h"
#include "VRODriverOpenGLAndroid.h"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VROSceneRendererGVR : public VROSceneRenderer, public std::enable_shared_from_this<VROSceneRendererGVR> {

public:

    /*
    Create a VROSceneRendererGVR using a given |gvr_context|.

    @param gvr_api The (non-owned) gvr_context.
     @param gvr_audio_api The (owned) gvr::AudioApi context.
     */
    VROSceneRendererGVR(gvr_context* gvr_context,
                              std::shared_ptr<gvr::AudioApi> gvrAudio);
    virtual ~VROSceneRendererGVR();

    /*
     Inherited from VROSceneRenderer.
     */
    void initGL();
    void onDrawFrame();
    void onTouchEvent(int action, float x, float y);
    void onKeyEvent(int keyCode, int action) {} // Not Required
    void setVRModeEnabled(bool enabled);
    void setSuspended(bool suspendRenderer);

    /*
     Activity lifecycle.
     */
    void onStart() {}
    void onPause();
    void onResume();
    void onStop() {}
    void onDestroy() {}

    /*
     Surface lifecycle.
     */
    void onSurfaceCreated(jobject surface) {}
    void onSurfaceChanged(jobject surface) {}
    void onSurfaceDestroyed() {}

private:

    /*
     Prepares the GvrApi framebuffer for rendering, resizing if needed.
     */
    void prepareFrame(VROViewport leftViewport,
                      VROFieldOfView fov,
                      VROMatrix4f headRotation);

    void renderStereo(VROMatrix4f &headRotation);
    void renderMono(VROMatrix4f &headRotation);

    /*
     Draws the scene for the given eye.
     */
    void renderEye(VROEyeType eyeType,
                   VROMatrix4f eyeFromHeadMatrix,
                   VROViewport viewport,
                   VROFieldOfView fov);

    std::unique_ptr<gvr::GvrApi> _gvr;
    std::unique_ptr<gvr::BufferViewportList> _viewportList;
    std::unique_ptr<gvr::SwapChain> _swapchain;
    gvr::BufferViewport _scratchViewport;

    gvr::Mat4f _headView;
    gvr::Sizei _renderSize;
    gvr::ViewerType _viewerType;

    bool _vrModeEnabled;
    bool _rendererSuspended;

    /*
     Utility methods.
     */
    gvr::Rectf modulateRect(const gvr::Rectf &rect, float width, float height);
    gvr::Recti calculatePixelSpaceRect(const gvr::Sizei &texture_size, const gvr::Rectf &texture_rect);
    gvr::Sizei halfPixelCount(const gvr::Sizei& in);
    void extractViewParameters(gvr::BufferViewport &viewport, VROViewport *outViewport,
                               VROFieldOfView *outFov);

};

#endif  // VRO_SCENE_RENDERER_GVR_H  // NOLINT
