//
// Created by Raj Advani on 11/7/17.
//

#ifndef ANDROID_VROSCENERENDERERSCENEVIEW_H
#define ANDROID_VROSCENERENDERERSCENEVIEW_H


#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>

#include <string>
#include <thread>  // NOLINT
#include <vector>
#include "VROSceneRenderer.h"
#include "VRODriverOpenGLAndroid.h"

#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

class VROSurface;
class VROARCamera;
class VROARFrame;
class VROARSessionARCore;
class VRORendererConfiguration;

class VROSceneRendererSceneView : public VROSceneRenderer, public std::enable_shared_from_this<VROSceneRendererSceneView> {

public:

    /*
     Create a VROSceneRendererSceneView.

     @param gvr_audio_api The (owned) gvr::AudioApi context.
     */
    VROSceneRendererSceneView(VRORendererConfiguration config,
                              std::shared_ptr<gvr::AudioApi> gvrAudio,
                              jobject viroViewJNI);
    virtual ~VROSceneRendererSceneView();

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
    void renderSuspended();

    gvr::Sizei _surfaceSize;
    bool _rendererSuspended;
    double _suspendedNotificationTime;
};



#endif //ANDROID_VROSCENERENDERERSCENEVIEW_H
