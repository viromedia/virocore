//
//  VRORenderer_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_RENDERER_JNI_H
#define ANDROID_RENDERER_JNI_H

#include <jni.h>
#include <memory>
#include <VROSceneRendererCardboard.h>
#include "PersistentRef.h"

namespace Renderer{
    inline jlong jptr(std::shared_ptr<VROSceneRendererCardboard> native_renderer) {
        PersistentRef<VROSceneRendererCardboard> *persistedRenderer = new PersistentRef<VROSceneRendererCardboard>(native_renderer);
        return reinterpret_cast<intptr_t>(persistedRenderer);
    }

    inline std::shared_ptr<VROSceneRendererCardboard> native(jlong ptr) {
        PersistentRef<VROSceneRendererCardboard> *persistedRenderer = reinterpret_cast<PersistentRef<VROSceneRendererCardboard> *>(ptr);
        return persistedRenderer->get();
    }
}

#endif
