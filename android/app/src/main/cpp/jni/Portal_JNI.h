/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#ifndef ANDROID_PORTAL_JNI_H_H
#define ANDROID_PORTAL_JNI_H_H
#include <VROPortalFrame.h>
#include "PersistentRef.h"

namespace Portal {
    inline jlong jptr(std::shared_ptr<VROPortalFrame> shared_portal) {
        PersistentRef<VROPortalFrame> *nativePortalFrame = new PersistentRef<VROPortalFrame>(shared_portal);
        return reinterpret_cast<intptr_t>(nativePortalFrame);
    }

    inline std::shared_ptr<VROPortalFrame> native(jlong ptr) {
        PersistentRef<VROPortalFrame> *persistentPortalFrame = reinterpret_cast<PersistentRef<VROPortalFrame> *>(ptr);
        return persistentPortalFrame->get();
    }
}


#endif //ANDROID_PORTAL_JNI_H_H
