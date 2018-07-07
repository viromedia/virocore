//
//  ViroContext_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIRO_CONTEXT_ANDROID_JNI_H
#define ANDROID_VIRO_CONTEXT_ANDROID_JNI_H

#include "ViroContext_JNI.h"
#include "VROSceneRenderer.h"
#include "VROImageAndroid.h"
#include "VROCameraImageListener.h"

/**
 * Helper Context for accessing render specific information, without exposing the entire renderer.
 */
class ViroContextAndroid : public ViroContext {
public:

    ViroContextAndroid(std::shared_ptr<VROSceneRenderer> renderer) {
        _renderer = renderer;
    }
    virtual ~ViroContextAndroid(){
    }

    const VROCamera &getCamera() {
        return _renderer->getRenderer()->getCamera();
    }
    std::shared_ptr<VRODriver> getDriver() {
        return _renderer->getDriver();
    }
    std::shared_ptr<VROFrameSynchronizer> getFrameSynchronizer() {
        return _renderer->getFrameSynchronizer();
    }
    std::shared_ptr<VROInputControllerBase> getInputController() {
        return _renderer->getRenderer()->getInputController();
    }

    void setCameraImageFrameListener(std::shared_ptr<VROCameraImageFrameListener> listener) {
        std::shared_ptr<VROFrameSynchronizer> synchronizer = getFrameSynchronizer();
        std::shared_ptr<VROCameraImageFrameListener> existingListener = _cameraFrameListener;
        if (existingListener) {
            synchronizer->removeFrameListener(existingListener);
        }

        if (listener) {
            synchronizer->addFrameListener(listener);
            _cameraFrameListener = listener;
        } else {
            _cameraFrameListener.reset();
        }
    }

private:

    std::shared_ptr<VROSceneRenderer> _renderer;

    /*
     The currently installed camera frame listener, if any.
     */
    std::shared_ptr<VROCameraImageFrameListener> _cameraFrameListener;

};

#endif
