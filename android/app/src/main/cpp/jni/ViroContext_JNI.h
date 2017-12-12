//
//  ViroContext_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIRO_CONTEXT_JNI_H
#define ANDROID_VIRO_CONTEXT_JNI_H

#include <jni.h>
#include <memory>
#include <android/log.h>
#include <VROSceneRenderer.h>
#include "VROImageAndroid.h"
#include "PersistentRef.h"

/**
 * Helper Context for accessing render specific information, without exposing the entire renderer.
 */
class ViroContext {
public:
    ViroContext(std::shared_ptr<VROSceneRenderer> renderer) {
        _renderer = renderer;
    }
    virtual ~ViroContext(){}

    static jlong jptr(std::shared_ptr<ViroContext> nativeContext) {
        PersistentRef<ViroContext> *persistedContext = new PersistentRef<ViroContext>(nativeContext);
        return reinterpret_cast<intptr_t>(persistedContext);
    }

    static std::shared_ptr<ViroContext> native(jlong ptr) {
        PersistentRef<ViroContext> *persistedContext = reinterpret_cast<PersistentRef<ViroContext> *>(ptr);
        return persistedContext->get();
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

private:

    std::shared_ptr<VROSceneRenderer>_renderer;

};

#endif
