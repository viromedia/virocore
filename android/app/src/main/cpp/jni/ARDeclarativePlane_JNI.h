//
//  ARPlane_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ARPlane_JNI_h
#define ARPlane_JNI_h

#include <jni.h>
#include <memory>
#include <VROARDeclarativePlane.h>
#include "PersistentRef.h"

namespace ARDeclarativePlane {
    inline jlong jptr(std::shared_ptr<VROARDeclarativePlane> shared_plane) {
        PersistentRef<VROARDeclarativePlane> *native_ar_plane = new PersistentRef<VROARDeclarativePlane>(shared_plane);
        return reinterpret_cast<intptr_t>(native_ar_plane);
    }

    inline std::shared_ptr<VROARDeclarativePlane> native(jlong ptr) {
        PersistentRef<VROARDeclarativePlane> *persistentARPlane = reinterpret_cast<PersistentRef<VROARDeclarativePlane> *>(ptr);
        return persistentARPlane->get();
    }
}

#endif