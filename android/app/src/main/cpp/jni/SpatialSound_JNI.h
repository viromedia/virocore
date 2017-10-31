//
// Created by Raj Advani on 10/30/17.
//

#ifndef ANDROID_SPATIALSOUND_JNI_H
#define ANDROID_SPATIALSOUND_JNI_H

#include <jni.h>
#include <memory>
#include <VROSoundGVR.h>
#include "PersistentRef.h"

namespace SpatialSound {
    inline jlong jptr(std::shared_ptr<VROSoundGVR> ptr) {
        PersistentRef<VROSoundGVR> *persistentRef = new PersistentRef<VROSoundGVR>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSoundGVR> native(jlong ptr) {
        PersistentRef<VROSoundGVR> *persistentRef = reinterpret_cast<PersistentRef<VROSoundGVR> *>(ptr);
        return persistentRef->get();
    }
}

#endif //ANDROID_SPATIALSOUND_JNI_H
