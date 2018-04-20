//
// Created by Andy Chu on 6/1/17.
//

#ifndef ANDROID_IMAGETRACKEROUTPUT_JNI_H
#define ANDROID_IMAGETRACKEROUTPUT_JNI_H

#include <memory>

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if ENABLE_OPENCV

#include "VROARImageTracker.h"
#include "PersistentRef.h"

namespace ImageTrackerOutput {
    inline VRO_REF jptr(std::shared_ptr<VROARImageTrackerOutput> tracker) {
        PersistentRef<VROARImageTrackerOutput> *nativeTracker = new PersistentRef<VROARImageTrackerOutput>(tracker);
        return reinterpret_cast<intptr_t>(nativeTracker);
    }

    inline std::shared_ptr<VROARImageTrackerOutput> native(VRO_REF ptr) {
        PersistentRef<VROARImageTrackerOutput> *persistentOutput = reinterpret_cast<PersistentRef<VROARImageTrackerOutput> *>(ptr);
        return persistentOutput->get();
    }
}

#endif /* ENABLE_OPENCV */

#endif //ANDROID_IMAGETRACKEROUTPUT_JNI_H
