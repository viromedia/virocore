//
//  VROSceneRendererGVR.h
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
#include "VROInputControllerCardboard.h"
#include "VROInputControllerARAndroid.h"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VRORendererConfiguration;

class VROSceneRendererGVR : public VROSceneRenderer, public std::enable_shared_from_this<VROSceneRendererGVR> {

public:

    /*
    Create a VROSceneRendererGVR using a given |gvr_context|.

    @param gvr_api The (non-owned) gvr_context.
     @param gvr_audio_api The (owned) gvr::AudioApi context.
     */
    VROSceneRendererGVR(VRORendererConfiguration config,
                        gvr_context* gvr_context,
                        std::shared_ptr<gvr::AudioApi> gvrAudio);
    virtual ~VROSceneRendererGVR();

    /*
     Inherited from VROSceneRenderer.
     */
    void initGL();
    void onDrawFrame();
    void onTouchEvent(int action, float x, float y);
    void onPinchEvent(int pinchState, float scaleFactor,
                                           float viewportX, float viewportY);
    void onRotateEvent(int rotateState, float rotateRadians, float viewportX,
                                            float viewportY);
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
    void onSurfaceChanged(jobject surface, VRO_INT width, VRO_INT height);
    void onSurfaceDestroyed() {}

private:

    void renderStereo(VROMatrix4f &headView);
    void renderMono(VROMatrix4f &headView);

    std::unique_ptr<gvr::GvrApi> _gvr;
    std::unique_ptr<gvr::BufferViewportList> _viewportList;
    std::unique_ptr<gvr::SwapChain> _swapchain;

    // These are viewport *templates*. We use viewportList->SetBufferViewport to copy the
    // viewport into the appropriate position after configuring it each frame
    gvr::BufferViewport _hudViewport;
    gvr::BufferViewport _sceneViewport;

    gvr::Mat4f _headView;
    gvr::Sizei _renderSize;
    gvr::Sizei _surfaceSize;
    gvr::ViewerType _viewerType;

    bool _touchTrackingEnabled;
    bool _vrModeEnabled;
    bool _rendererSuspended;
    double _suspendedNotificationTime;
    std::shared_ptr<VROInputControllerARAndroid> _touchController;
    std::shared_ptr<VROInputControllerBase>  _cardboardController;
    /*
     Utility methods.
     */
    void clearViewport(VROViewport viewport, bool transparent);
    gvr::Rectf modulateRect(const gvr::Rectf &rect, float width, float height);
    gvr::Recti calculatePixelSpaceRect(const gvr::Sizei &texture_size, const gvr::Rectf &texture_rect);
    gvr::Sizei halfPixelCount(const gvr::Sizei& in);
    void extractViewParameters(gvr::BufferViewport &viewport, VROViewport *outViewport,
                               VROFieldOfView *outFov);

};

#endif  // VRO_SCENE_RENDERER_GVR_H  // NOLINT
