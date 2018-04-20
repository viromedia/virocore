//
//  VRORenderer_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_RENDERER_JNI_H
#define ANDROID_RENDERER_JNI_H

#include <memory>
#include <VROSceneRenderer.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Renderer{
    inline VRO_REF jptr(std::shared_ptr<VROSceneRenderer> native_renderer) {
        PersistentRef<VROSceneRenderer> *persistedRenderer = new PersistentRef<VROSceneRenderer>(native_renderer);
        return reinterpret_cast<intptr_t>(persistedRenderer);
    }

    inline std::shared_ptr<VROSceneRenderer> native(VRO_REF ptr) {
        PersistentRef<VROSceneRenderer> *persistedRenderer = reinterpret_cast<PersistentRef<VROSceneRenderer> *>(ptr);
        return persistedRenderer->get();
    }
}

#endif
