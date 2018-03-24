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
#include "VROSceneRenderer.h"
#include "VRODriverOpenGLAndroid.h"

#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VROSurface;
class VROARCamera;
class VROARFrame;
class VRORendererConfiguration;
class VROARSessionARCore;

class VROSceneRendererARCore : public VROSceneRenderer, public std::enable_shared_from_this<VROSceneRendererARCore> {

public:

    /*
     Create a VROSceneRendererARCore.

     @param gvr_audio_api The (owned) gvr::AudioApi context.
     */
    VROSceneRendererARCore(VRORendererConfiguration config,
                           std::shared_ptr<gvr::AudioApi> gvrAudio);
    virtual ~VROSceneRendererARCore();

    /*
     Inherited from VROSceneRenderer.
     */
    void initGL();
    void onDrawFrame();
    void onTouchEvent(int action, float x, float y);
    void onKeyEvent(int keyCode, int action) {} // Not Required
    void onPinchEvent(int pinchState, float scaleFactor, float viewportX, float viewportY);
    void onRotateEvent(int rotateState, float rotateRadians, float viewportX, float viewportY);

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

    /*
     Set to true when ARCore is installed. Unlocks the renderer.
     */
    void setARCoreSession(arcore::Session *session);

    /*
     Retrieves the texture ID used for the camera background.
     */
    GLuint getCameraTextureId() const;

    /*
     AR hit test using point on the screen in 2D coordinate system.
     */
    std::vector<VROARHitTestResult> performARHitTest(float x, float y);

    /*
     AR hit test using a ray from camera's position into the 3D scene.
     */
    std::vector<VROARHitTestResult> performARHitTest(VROVector3f ray);

    /*
     Set the size of the parent view holding the AR screen. Invoked from ViroViewARCore.
     */
    void setDisplayGeometry(int rotation, int width, int height);

    /*
     Set the anchor detection mode used by ARCore. Returns false if not supported. Currently
     this is a simplistic planes on/off but can be changed in the future.
     */
    bool setPlaneFindingMode(bool enabled);

    /*
     This is a function that enables/disables tracking (for debug purposes!)
     */
    void enableTracking(bool shouldTrack);

private:

    void renderFrame();
    void renderWithTracking(const std::shared_ptr<VROARCamera> &camera, const std::unique_ptr<VROARFrame> &frame,
                            VROViewport viewport);
    void updateARBackground(std::unique_ptr<VROARFrame> &frame, bool forceReset);
    void renderWaitingForTracking(VROViewport viewport);
    void renderNothing(bool suspended);
    void initARSession(VROViewport viewport, std::shared_ptr<VROScene> scene);

    std::shared_ptr<VROSurface> _cameraBackground;
    gvr::Sizei _surfaceSize;
    bool _rendererSuspended;
    double _suspendedNotificationTime;
    bool _arcoreInstalled;

    // Detection types are only stored here so that they can be pushed to the ARScene when that
    // is injected into the scene renderer (from there they are pushed into the VROARSession).
    std::set<VROAnchorDetection> _detectionTypes;

    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROARSessionARCore> _session;
    std::shared_ptr<VROSceneController> _sceneController;
};

#endif  // VRO_SCENE_RENDERER_ARCORE_H  // NOLINT
