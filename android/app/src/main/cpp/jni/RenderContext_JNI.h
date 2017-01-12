//
//  RenderContext_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_RENDER_CONTEXT_JNI_H
#define ANDROID_RENDER_CONTEXT_JNI_H

#include <jni.h>
#include <memory>
#include <android/log.h>
#include <VROSceneRenderer.h>
#include "VROImageAndroid.h"
#include "PersistentRef.h"

/**
 * Helper Context for accessing render specific information, without exposing the entire renderer.
 */
class RenderContext {
    public:
    RenderContext(std::shared_ptr<VROSceneRenderer> renderer) {
        _renderer = renderer;
    }
    ~RenderContext(){}

    static jlong jptr(std::shared_ptr<RenderContext> nativeContext) {
        PersistentRef<RenderContext> *persistedContext = new PersistentRef<RenderContext>(nativeContext);
        return reinterpret_cast<intptr_t>(persistedContext);
    }

    static std::shared_ptr<RenderContext> native(jlong ptr) {
        PersistentRef<RenderContext> *persistedContext = reinterpret_cast<PersistentRef<RenderContext> *>(ptr);
        return persistedContext->get();
    }

    std::shared_ptr<VRORenderContext> getContext() {
        return _renderer->getRenderer()->getRenderContext();
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
