//
//  VROSceneRenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/5/17.
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

#ifndef ANDROID_VROSCENERENDERER_H
#define ANDROID_VROSCENERENDERER_H

#include <memory>
#include "VROEye.h"
#include "VROTimingFunction.h"
#include "VRORenderer.h"
#include "VRORenderDelegate.h"
#include "VRODriverOpenGLAndroid.h"

class VROSceneController;

class VROSceneRenderer {

public:

    VROSceneRenderer() :
        _frame(0) {
    }
    virtual ~VROSceneRenderer() {}

    /*
     GL initialization invoked from rendering thread.
     */
    virtual void initGL() = 0;

    /*
     Main render loop.
     */
    virtual void onDrawFrame() = 0;

    /*
     InputEvent methods for notifying the renderer.
     */
    virtual void onTouchEvent(int action, float xPos, float yPos) = 0;
    virtual void onKeyEvent(int keyCode, int action) = 0;

    /*
     Called to let the renderer know when a pinch event has occurred.
     */
    virtual void onPinchEvent(int pinchState, float scaleFactor,
                              float viewportX, float viewportY) {
        // no-op (not all renderers need to implement this)
    }

    /*
     Called to let the renderer know when a rotate event has occurred.
     */
    virtual void onRotateEvent(int rotateState, float rotateRadians,
                               float viewportX, float viewportY) {
        // no-op (not all renderers need to implement this)
    }

    /*
     Enable or disable stereo rendering (VR mode). When false, we will render
     using the entire device window. When true, we will render in stereo using
     the platform's distortion renderer.
     */
    virtual void setVRModeEnabled(bool enabled) = 0;

    /*
     Activity lifecycle.
     */
    virtual void onStart() = 0;
    virtual void onResume() = 0;
    virtual void onPause() = 0;
    virtual void onStop() = 0;
    virtual void onDestroy() = 0;

    /*
     Surface lifecycle.
     */
    virtual void onSurfaceCreated(jobject surface) = 0;
    virtual void onSurfaceChanged(jobject surface, VRO_INT width, VRO_INT height) = 0;
    virtual void onSurfaceDestroyed() = 0;

    /*
     Set the render delegate, which responds to renderer initialization and
     receives per-frame callbacks.
     */
    void setRenderDelegate(std::shared_ptr<VRORenderDelegate> delegate) {
        _renderer->setDelegate(delegate);
    }

    /*
     Set the active scene controller immediately, which dictates what scene is rendered.
     */
    virtual void setSceneController(std::shared_ptr<VROSceneController> sceneController) {
        _renderer->setSceneController(sceneController, _driver);
    }

    /*
     Set the active scene controller with an animation.
     */
    virtual void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                                    VROTimingFunctionType timingFunction) {
        _sceneController = sceneController;
        _renderer->setSceneController(sceneController, seconds, timingFunction, _driver);
    }

    std::shared_ptr<VRORenderer> getRenderer() {
        return _renderer;
    }
    std::shared_ptr<VRODriver> getDriver() {
        return _driver;
    }
    std::shared_ptr<VROFrameSynchronizer> getFrameSynchronizer() {
        return _renderer->getFrameSynchronizer();
    }

    void setClearColor(VROVector4f color) {
        _renderer->setClearColor(color, _driver);
    }

    std::vector<VROHitTestResult> performHitTest(int x, int y, bool boundsOnly);

    std::vector<VROHitTestResult> performHitTest(VROVector3f origin, VROVector3f ray, bool boundsOnly);
protected:

    int _frame;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRODriverOpenGLAndroid> _driver;
    std::shared_ptr<VROSceneController> _sceneController;

};

#endif //ANDROID_VROSCENERENDERER_H
