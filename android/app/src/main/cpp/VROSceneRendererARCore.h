//
//  VROSceneRendererARCore.h
//  ViroRenderer
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRO_SCENE_RENDERER_ARCORE_H_  // NOLINT
#define VRO_SCENE_RENDERER_ARCORE_H_  // NOLINT

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

class VROSceneRendererARCore : public VROSceneRenderer, public std::enable_shared_from_this<VROSceneRendererARCore> {

public:

    /*
     Create a VROSceneRendererARCore.

     @param gvr_audio_api The (owned) gvr::AudioApi context.
     */
    VROSceneRendererARCore(std::shared_ptr<gvr::AudioApi> gvrAudio);
    virtual ~VROSceneRendererARCore();

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
    void onSurfaceChanged(jobject surface, jint width, jint height);
    void onSurfaceDestroyed() {}

private:

    /*
     Prepares the GvrApi framebuffer for rendering, resizing if needed.
     */
    void prepareFrame(VROViewport leftViewport,
                      VROFieldOfView fov,
                      VROMatrix4f headRotation);

    void renderMono();

    /*
     Draws the scene for the given eye.
     */
    void renderEye(VROEyeType eyeType,
                   VROMatrix4f eyeFromHeadMatrix,
                   VROViewport viewport,
                   VROFieldOfView fov);

    gvr::Sizei _surfaceSize;
    bool _rendererSuspended;
    double _suspendedNotificationTime;

    /*
     Utility methods.
     */
    gvr::Rectf modulateRect(const gvr::Rectf &rect, float width, float height);
    gvr::Recti calculatePixelSpaceRect(const gvr::Sizei &texture_size, const gvr::Rectf &texture_rect);

};

#endif  // VRO_SCENE_RENDERER_ARCORE_H  // NOLINT
