//
// Created by Andy Chu on 11/2/17.
//

#ifndef ANDROID_SURFACE_JNI_H
#define ANDROID_SURFACE_JNI_H

#include <jni.h>
#include <memory>

#include "PersistentRef.h"
#include "VROSurface.h"


namespace Surface {
    inline jlong jptr(std::shared_ptr<VROSurface> surface) {
        PersistentRef<VROSurface> *persistedSurface = new PersistentRef<VROSurface>(surface);
        return reinterpret_cast<intptr_t>(persistedSurface);
    }

    inline std::shared_ptr<VROSurface> native(jlong ptr) {
        PersistentRef<VROSurface> *persistedSurface = reinterpret_cast<PersistentRef<VROSurface> *>(ptr);
        return persistedSurface->get();
    }
}

#endif //ANDROID_SURFACE_JNI_H
