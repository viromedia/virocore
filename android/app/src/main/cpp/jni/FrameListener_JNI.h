/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#ifndef ANDROID_FRAMELISTENER_JNI_H
#define ANDROID_FRAMELISTENER_JNI_H
#include <jni.h>
#include <memory>
#include "PersistentRef.h"
#include "VROPortalTraversalListener.h"

namespace FrameListener {
    inline jlong jptr(std::shared_ptr<VROFrameListener> shared_node) {
        PersistentRef<VROFrameListener> *nativeListener = new PersistentRef<VROFrameListener>(shared_node);
        return reinterpret_cast<intptr_t>(nativeListener);
    }

    inline std::shared_ptr<VROFrameListener> native(jlong ptr) {
        PersistentRef<VROFrameListener> *nativeListener = reinterpret_cast<PersistentRef<VROFrameListener> *>(ptr);
        return nativeListener->get();
    }
}

#endif //ANDROID_FRAMELISTENER_JNI_H
