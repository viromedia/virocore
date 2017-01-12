//
//  VROSceneRenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

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
     Event on trigger.
     */
    virtual void onScreenTouchEvent(bool enabled) = 0;

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

    /*
     Surface lifecycle.
     */
    virtual void onSurfaceCreated(jobject surface) = 0;
    virtual void onSurfaceChanged(jobject surface) = 0;
    virtual void onSurfaceDestroyed() = 0;

    /*
     Set the render delegate, which responds to renderer initialization and
     receives per-frame callbacks.
     */
    void setRenderDelegate(std::shared_ptr<VRORenderDelegate> delegate) {
        _renderer->setDelegate(delegate);
    }

    /*
     Set the active scene controller, which dictates what scene is rendered.
     */
    void setSceneController(std::shared_ptr<VROSceneController> sceneController) {
        _renderer->setSceneController(sceneController, *_driver.get());
    }
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, bool animated) {
        _renderer->setSceneController(sceneController, animated, *_driver.get());
    }
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                            VROTimingFunctionType timingFunction) {
        _renderer->setSceneController(sceneController, seconds, timingFunction, *_driver.get());
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

protected:

    int _frame;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRODriverOpenGLAndroid> _driver;

};

#endif //ANDROID_VROSCENERENDERER_H
