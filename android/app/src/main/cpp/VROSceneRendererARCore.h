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
#include <arcore/VROARSessionARCore.h>
#include <VROARComponentManager.h>
#include "VROSceneRenderer.h"
#include "VRODriverOpenGLAndroid.h"

#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VROSurface;
class VROARCamera;
class VROARFrame;
class VROARSessionARCore;

class VROSceneRendererARCore : public VROSceneRenderer, public std::enable_shared_from_this<VROSceneRendererARCore> {

public:

    /*
     Create a VROSceneRendererARCore.

     @param gvr_audio_api The (owned) gvr::AudioApi context.
     */
    VROSceneRendererARCore(std::shared_ptr<gvr::AudioApi> gvrAudio,
                           jni::Object<arcore::Session> sessionJNI);
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
     Override so that this object can hold onto the VROSceneController as
     well.
    */
    void setSceneController(std::shared_ptr<VROSceneController> sceneController);
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                            VROTimingFunctionType timingFunction);

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

    void renderFrame();
    void renderWithTracking(const std::shared_ptr<VROARCamera> &camera, const std::unique_ptr<VROARFrame> &frame,
                            VROViewport viewport, bool firstFrameSinceResume);
    void renderWaitingForTracking(VROViewport viewport);
    void renderSuspended();
    void initARSession(VROViewport viewport, std::shared_ptr<VROScene> scene);

    std::shared_ptr<VROSurface> _cameraBackground;
    gvr::Sizei _surfaceSize;
    bool _rendererSuspended;
    double _suspendedNotificationTime;

    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROARSessionARCore> _session;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROARComponentManager> _componentManager;

    /*
     The hasTrackingInitialized bool gets flipped to true when tracking
     is initialized for the first time, and stays that way. The resumed
     flag is set to false whenever the activity is paused, then returns
     to true after tracking is resumed.
     */
    bool _hasTrackingInitialized;
    bool _hasTrackingResumed;
};

#endif  // VRO_SCENE_RENDERER_ARCORE_H  // NOLINT
