//
//  ARPlane_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ARPlane_JNI_h
#define ARPlane_JNI_h

#include <memory>
#include <VROARDeclarativePlane.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace ARDeclarativePlane {
    inline VRO_REF jptr(std::shared_ptr<VROARDeclarativePlane> shared_plane) {
        PersistentRef<VROARDeclarativePlane> *native_ar_plane = new PersistentRef<VROARDeclarativePlane>(shared_plane);
        return reinterpret_cast<intptr_t>(native_ar_plane);
    }

    inline std::shared_ptr<VROARDeclarativePlane> native(VRO_REF ptr) {
        PersistentRef<VROARDeclarativePlane> *persistentARPlane = reinterpret_cast<PersistentRef<VROARDeclarativePlane> *>(ptr);
        return persistentARPlane->get();
    }
}

#endif