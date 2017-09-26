//
//  ARPlane_JNI.ch
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//


#ifndef ARPlane_JNI_h
#define ARPlane_JNI_h

#include <jni.h>
#include <memory>
#include <VROARPlane.h>
#include "PersistentRef.h"

namespace ARPlane {
    inline jlong jptr(std::shared_ptr<VROARPlane> shared_plane) {
        PersistentRef<VROARPlane> *native_ar_plane = new PersistentRef<VROARPlane>(shared_plane);
        return reinterpret_cast<intptr_t>(native_ar_plane);
    }

    inline std::shared_ptr<VROARPlane> native(jlong ptr) {
        PersistentRef<VROARPlane> *persistentARPlane = reinterpret_cast<PersistentRef<VROARPlane> *>(ptr);
        return persistentARPlane->get();
    }
}

#endif