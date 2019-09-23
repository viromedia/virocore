//
//  Created by Raj Advani on 11/7/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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
    void onSurfaceChanged(jobject surface, VRO_INT width, VRO_INT height);
    void onSurfaceDestroyed() {}

private:

    void renderFrame();
    void renderSuspended();

    gvr::Sizei _surfaceSize;
    bool _rendererSuspended;
    double _suspendedNotificationTime;
};



#endif //ANDROID_VROSCENERENDERERSCENEVIEW_H
