//
//  ViroContext_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIRO_CONTEXT_JNI_H
#define ANDROID_VIRO_CONTEXT_JNI_H

#include <memory>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class VROCamera;
class VRODriver;
class VROFrameSynchronizer;
class VROInputControllerBase;

/**
 * Context for accessing render specific information, without exposing the entire renderer.
 */
class ViroContext {
public:

    ViroContext() {}
    virtual ~ViroContext(){}

    static VRO_REF(ViroContext) jptr(std::shared_ptr<ViroContext> nativeContext) {
        PersistentRef<ViroContext> *persistedContext = new PersistentRef<ViroContext>(nativeContext);
        return reinterpret_cast<intptr_t>(persistedContext);
    }

    static std::shared_ptr<ViroContext> native(VRO_REF(ViroContext) ptr) {
        PersistentRef<ViroContext> *persistedContext = reinterpret_cast<PersistentRef<ViroContext> *>(ptr);
        return persistedContext->get();
    }

    virtual const VROCamera &getCamera() = 0;
    virtual std::shared_ptr<VRODriver> getDriver() = 0;
    virtual std::shared_ptr<VROFrameSynchronizer> getFrameSynchronizer() = 0;
    virtual std::shared_ptr<VROInputControllerBase> getInputController() = 0;

};

#endif
