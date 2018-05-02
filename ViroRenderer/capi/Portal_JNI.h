/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#ifndef ANDROID_PORTAL_JNI_H_H
#define ANDROID_PORTAL_JNI_H_H

#include <VROPortalFrame.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Portal {
    inline VRO_REF(VROPortalFrame) jptr(std::shared_ptr<VROPortalFrame> shared_portal) {
        PersistentRef<VROPortalFrame> *nativePortalFrame = new PersistentRef<VROPortalFrame>(shared_portal);
        return reinterpret_cast<intptr_t>(nativePortalFrame);
    }

    inline std::shared_ptr<VROPortalFrame> native(VRO_REF(VROPortalFrame) ptr) {
        PersistentRef<VROPortalFrame> *persistentPortalFrame = reinterpret_cast<PersistentRef<VROPortalFrame> *>(ptr);
        return persistentPortalFrame->get();
    }
}


#endif //ANDROID_PORTAL_JNI_H_H
