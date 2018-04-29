//
// Created by Andy Chu on 11/2/17.
//

#ifndef ANDROID_SURFACE_JNI_H
#define ANDROID_SURFACE_JNI_H

#include <memory>
#include "PersistentRef.h"
#include "VROSurface.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Surface {
    inline VRO_REF jptr(std::shared_ptr<VROSurface> surface) {
        PersistentRef<VROSurface> *persistedSurface = new PersistentRef<VROSurface>(surface);
        return reinterpret_cast<intptr_t>(persistedSurface);
    }

    inline std::shared_ptr<VROSurface> native(VRO_REF ptr) {
        PersistentRef<VROSurface> *persistedSurface = reinterpret_cast<PersistentRef<VROSurface> *>(ptr);
        return persistedSurface->get();
    }
}

#endif //ANDROID_SURFACE_JNI_H
