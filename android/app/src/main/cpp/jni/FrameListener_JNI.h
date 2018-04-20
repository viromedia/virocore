/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#ifndef ANDROID_FRAMELISTENER_JNI_H
#define ANDROID_FRAMELISTENER_JNI_H

#include <memory>
#include "PersistentRef.h"
#include "VROPortalTraversalListener.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace FrameListener {
    inline VRO_REF jptr(std::shared_ptr<VROFrameListener> shared_node) {
        PersistentRef<VROFrameListener> *nativeListener = new PersistentRef<VROFrameListener>(shared_node);
        return reinterpret_cast<intptr_t>(nativeListener);
    }

    inline std::shared_ptr<VROFrameListener> native(VRO_REF ptr) {
        PersistentRef<VROFrameListener> *nativeListener = reinterpret_cast<PersistentRef<VROFrameListener> *>(ptr);
        return nativeListener->get();
    }
}

#endif //ANDROID_FRAMELISTENER_JNI_H
